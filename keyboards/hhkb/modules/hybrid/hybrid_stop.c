// SPDX-License-Identifier: MIT
// Adapted from l3tnun's MIT-licensed hhkb_hybrid/power.c STOP wake sequence.
// The upstream copyright and license are in LICENSE.protocol.
#include "hybrid.h"
#include "stm32l072xx.h"

#define WAKE_EXTI_MASK ((1UL << 5) | (1UL << 13) | (1UL << 29))
// Volatile RAM markers for SWD; these are not persisted across reset.
// 1: preparing, 2: waiting for release, 3: before WFI, 4: woke, 5: held, 6: reset.
volatile uint32_t hhkb_stop_stage;
volatile uint32_t hhkb_stop_wake_count;

#define LPTIM_ARR 500U // LSI / 128: about 1.7 seconds between watchdog refreshes.

void EXTI4_15_IRQHandler(void) { EXTI->PR = EXTI->PR & ((1UL << 5) | (1UL << 13)); }
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
    EXTI->FTSR |= (1UL << 5) | (1UL << 13);
    EXTI->IMR |= WAKE_EXTI_MASK;
    EXTI->RTSR |= (1UL << 5) | (1UL << 29); // Arm PC5 after release.

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

void hhkb_hybrid_enter_stop(void) {
    // This terminal path never returns to TinyUSB or the scheduler.
    hhkb_stop_stage = 1;
    hhkb_stop_wake_count = 0;
    // Called only on battery after all pressed keys have been released.
    GPIOD->BSRR = 1UL << 2; // Stop the stock nRF before SPI clocks stop.
    TIM2->DIER = 0;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    if (ADC1->CR & ADC_CR_ADEN) ADC1->CR |= ADC_CR_ADDIS;
    GPIOC->BRR = (1UL << 14) | (1UL << 7) | (1UL << 8);
    GPIOA->BRR = (1UL << 8) | (1UL << 9);
    // Remove drive from all rows (including JIS PB11) and mux address lines.
    GPIOB->BRR = 0xFC07UL;
    GPIOC->BRR = 1UL << 6;

    __disable_irq();
    SysTick->CTRL = 0;
    prepare_wake_sources();
    // USB is unavailable on this battery-only terminal path. Shut down the
    // transceiver before its clock; merely disabling its IRQ is insufficient.
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    USB->BCDR &= ~USB_BCDR_DPPU;
    USB->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN;
    RCC->APB1ENR &= ~(RCC_APB1ENR_USBEN | RCC_APB1ENR_CRSEN);
    RCC->CRRCR &= ~RCC_CRRCR_HSI48ON;

    // Finish the OFF press before sleeping. Otherwise its release can be
    // cleared just before WFI, leaving the next press unarmed.
    hhkb_stop_stage = 2;
    while (!(GPIOC->IDR & (1UL << 5))) {
        IWDG->KR = 0xAAAA;
        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
    }

    for (;;) {
        IWDG->KR = 0xAAAA;
        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
        EXTI->PR = WAKE_EXTI_MASK;
        LPTIM1->ICR = LPTIM_ICR_ARRMCF;
        NVIC->ICPR[0] = 0xFFFFFFFFUL;
        PWR->CR |= PWR_CR_CWUF;
        PWR->CR = (PWR->CR & ~PWR_CR_PDDS) | PWR_CR_LPSDSR;
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        hhkb_stop_stage = 3;
        __DSB();
        __WFI();
        hhkb_stop_stage = 4;
        ++hhkb_stop_wake_count;
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

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
                if (held_ticks >= 289U) {
                    hhkb_stop_stage = 6;
                    NVIC_SystemReset();
                } // About one second.
                if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
            }
        }
    }
}
