// SPDX-License-Identifier: MIT
// Adapted from l3tnun's MIT-licensed hhkb_hybrid/power.c STOP wake sequence.
// The upstream copyright and license are in LICENSE.protocol.
#include "FreeRTOS.h"
#include "task.h"
#include "hybrid.h"
#include "stm32l072xx.h"

#define WAKE_EXTI_MASK ((1UL << 5) | (1UL << 13) | (1UL << 29))
// Volatile RAM markers for SWD; these are not persisted across reset.
// 1: preparing, 2: waiting for release, 3: before WFI, 4: woke, 5: held, 6: reset.
volatile uint32_t hhkb_stop_stage;
volatile uint32_t hhkb_stop_wake_count;

#define LPTIM_ARR 500U // LSI / 128: about 1.7 seconds between watchdog refreshes.

static TaskHandle_t sleep_wake_task;
static volatile bool sleep_exti_enabled;
void hhkb_hybrid_set_wake_task(void *task) { sleep_wake_task = task; }

void hhkb_hybrid_sleep_exti(bool enabled) {
    const uint32_t lines = (1UL << 5) | (1UL << 13);
    sleep_exti_enabled = false;
    EXTI->IMR &= ~lines;
    if (!enabled) return;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI5) | SYSCFG_EXTICR2_EXTI5_PC;
    SYSCFG->EXTICR[3] = (SYSCFG->EXTICR[3] & ~SYSCFG_EXTICR4_EXTI13) | SYSCFG_EXTICR4_EXTI13_PC;
    EXTI->FTSR |= lines;
    EXTI->RTSR |= lines;
    EXTI->PR = lines;
    NVIC_SetPriority(EXTI4_15_IRQn, (1U << __NVIC_PRIO_BITS) - 1U);
    NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
    sleep_exti_enabled = true;
    EXTI->IMR |= lines;
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void EXTI4_15_IRQHandler(void) {
    uint32_t pending = EXTI->PR & ((1UL << 5) | (1UL << 13));
    EXTI->PR = pending;
    if (pending && sleep_exti_enabled && sleep_wake_task) {
        BaseType_t woken = pdFALSE;
        vTaskNotifyGiveFromISR(sleep_wake_task, &woken);
        portYIELD_FROM_ISR(woken);
    }
}
void LPTIM1_IRQHandler(void) { LPTIM1->ICR = LPTIM_ICR_ARRMCF; }

static uint16_t stable_lptim_count(void) {
    uint16_t first, second;
    do {
        first = (uint16_t)LPTIM1->CNT;
        second = (uint16_t)LPTIM1->CNT;
    } while (first != second);
    return first;
}

static void prepare_wake_sources(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI5) |
                         SYSCFG_EXTICR2_EXTI5_PC;
    SYSCFG->EXTICR[3] = (SYSCFG->EXTICR[3] & ~SYSCFG_EXTICR4_EXTI13) |
                         SYSCFG_EXTICR4_EXTI13_PC;
    // Terminal path owns EXTI: do not inherit unrelated bootloader sources.
    EXTI->FTSR = (1UL << 5) | (1UL << 13);
    EXTI->IMR = WAKE_EXTI_MASK;
    EXTI->EMR = 0;
    EXTI->RTSR = (1UL << 5) | (1UL << 29); // Arm PC5 after release.

    RCC->CSR |= RCC_CSR_LSION;
    while (!(RCC->CSR & RCC_CSR_LSIRDY)) {}
    RCC->APB1ENR |= RCC_APB1ENR_LPTIM1EN | RCC_APB1ENR_PWREN;
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_LPTIM1SEL) |
                 RCC_CCIPR_LPTIM1SEL_0;
    LPTIM1->CR = 0;
    LPTIM1->CFGR = 7UL << LPTIM_CFGR_PRESC_Pos;
    LPTIM1->IER = LPTIM_IER_ARRMIE;
    LPTIM1->CR = LPTIM_CR_ENABLE;
    LPTIM1->ICR = LPTIM_ICR_ARROKCF;
    LPTIM1->ARR = LPTIM_ARR;
    while (!(LPTIM1->ISR & LPTIM_ISR_ARROK)) {}
    LPTIM1->CR |= LPTIM_CR_CNTSTRT;

    NVIC->ICER[0] = 0xFFFFFFFFUL;
    NVIC->ICPR[0] = 0xFFFFFFFFUL;
    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_EnableIRQ(LPTIM1_IRQn);
}

// LSI/128 is nominally 289 ticks/s. Wake only after release; a stuck
// button at >=10 seconds cancels the attempt, as in the analyzed stock path.
static unsigned off_press_action(uint32_t ticks, bool released) {
    if (ticks >= 2890U) return 2;
    return released && ticks >= 289U ? 1 : 0;
}

static void lptim_wait_ticks(uint32_t ticks) {
    uint16_t prev = stable_lptim_count();
    uint32_t elapsed = 0;
    while (elapsed < ticks) {
        uint16_t now = stable_lptim_count();
        elapsed += now >= prev ? now - prev : LPTIM_ARR + 1U - prev + now;
        prev = now;
        IWDG->KR = 0xAAAA;
        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
    }
}

static void stop_once(uint32_t stage) {
    EXTI->PR = WAKE_EXTI_MASK;
    LPTIM1->ICR = LPTIM_ICR_ARRMCF;
    NVIC->ICPR[0] = 0xFFFFFFFFUL;
    PWR->CR |= PWR_CR_CWUF;
    PWR->CR = (PWR->CR & ~PWR_CR_PDDS) | PWR_CR_LPSDSR;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    hhkb_stop_stage = stage;
    __DSB();
    __WFI();
    if (stage == 3) hhkb_stop_stage = 4;
    ++hhkb_stop_wake_count;
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
}

void hhkb_hybrid_enter_stop(void) {
    // This terminal path never returns to TinyUSB or the scheduler.
    hhkb_stop_stage = 1;
    hhkb_stop_wake_count = 0;
    // Called only on battery after all pressed keys have been released.
    GPIOD->BSRR = 1UL << 2; // Stop the stock nRF before SPI clocks stop.
    TIM2->DIER = 0;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    if (ADC1->CR & ADC_CR_ADEN) {
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN) {}
    }
    ADC1->CR &= ~ADC_CR_ADVREGEN;
    GPIOC->BRR = (1UL << 14) | (1UL << 7) | (1UL << 8);
    GPIOA->BRR = (1UL << 8) | (1UL << 9);
    // Remove drive from all rows (including JIS PB11) and mux address lines.
    GPIOB->BRR = 0xFC07UL;
    GPIOC->BRR = 1UL << 6;

    __disable_irq();
    SysTick->CTRL = 0;
    // ICER/ICPR below cover external IRQs only. A pending RTOS exception
    // must not keep this PRIMASK-protected terminal WFI loop awake.
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
    prepare_wake_sources();
    // Preserve an attached debug session, but do not inherit low-power debug
    // clock retention when no debugger is attached.
    // CMSIS M0+ omits CoreDebug; DHCSR.C_DEBUGEN is architectural bit 0.
    if (!(*(volatile const uint32_t *)0xE000EDF0UL & 1U))
        DBGMCU->CR &= ~(DBGMCU_CR_DBG_SLEEP | DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY);
    // USB is unavailable on this battery-only terminal path. Shut down the
    // transceiver before its clock; merely disabling its IRQ is insufficient.
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    USB->BCDR &= ~USB_BCDR_DPPU;
    USB->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN;
    RCC->APB1ENR &= ~(RCC_APB1ENR_USBEN | RCC_APB1ENR_CRSEN);
    RCC->CRRCR &= ~RCC_CRRCR_HSI48ON;
    SYSCFG->CFGR3 &= ~(SYSCFG_CFGR3_ENREF_HSI48 |
                          SYSCFG_CFGR3_ENBUF_VREFINT_ADC |
                          SYSCFG_CFGR3_ENBUF_SENSOR_ADC);
    // No code after this point needs the PLL or the 32 MHz system clock.
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) {}
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {}
    RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_MSION);

    // Finish the OFF press before sleeping. Otherwise its release can be
    // cleared just before WFI, leaving the next press unarmed.
    hhkb_stop_stage = 2;
    while (!(GPIOC->IDR & (1UL << 5))) {
        IWDG->KR = 0xAAAA;
        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
        stop_once(2);
    }

    for (;;) {
        IWDG->KR = 0xAAAA;
        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
        stop_once(3);

        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
        if (!(GPIOC->IDR & (1UL << 5))) {
            hhkb_stop_stage = 5;
            // LPTIM continues through STOP; accumulate across its ARR wrap.
            uint16_t prev = stable_lptim_count();
            uint32_t held_ticks = 0;
            while (!(GPIOC->IDR & (1UL << 5))) {
                uint16_t now = stable_lptim_count();
                held_ticks += now >= prev ? now - prev : LPTIM_ARR + 1U - prev + now;
                prev = now;
                IWDG->KR = 0xAAAA;
                if (held_ticks >= 289U) GPIOA->BSRR = 1UL << 8;
                if (off_press_action(held_ticks, false) == 2) {
                    GPIOA->BRR = 1UL << 8;
                    GPIOA->BSRR = 1UL << 9;
                    lptim_wait_ticks(867U); // Approximately 3 s orange warning.
                    GPIOA->BRR = 1UL << 9;
                    while (!(GPIOC->IDR & (1UL << 5))) {
                        IWDG->KR = 0xAAAA;
                        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
                        stop_once(2);
                    }
                    held_ticks = 0;
                    break;
                }
                if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
            }
            GPIOA->BRR = 1UL << 8;
            if (off_press_action(held_ticks, true) == 1) {
                hhkb_stop_stage = 6;
                NVIC_SystemReset();
            }
        }
    }
}
