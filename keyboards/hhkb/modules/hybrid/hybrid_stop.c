// SPDX-License-Identifier: MIT
// Adapted from l3tnun's MIT-licensed hhkb_hybrid/power.c STOP wake sequence.
// The upstream copyright and license are in LICENSE.protocol.
#include "hybrid.h"
#include "stm32l072xx.h"

#define WAKE_EXTI_MASK ((1UL << 5) | (1UL << 13) | (1UL << 29))
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
    // Called only on battery after all pressed keys have been released.
    GPIOD->BSRR = 1UL << 2; // Stop the stock nRF before SPI clocks stop.
    TIM2->DIER = 0;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    if (ADC1->CR & ADC_CR_ADEN) ADC1->CR |= ADC_CR_ADDIS;
    GPIOC->BRR = (1UL << 14) | (1UL << 7) | (1UL << 8);
    GPIOA->BRR = (1UL << 8) | (1UL << 9);

    __disable_irq();
    SysTick->CTRL = 0;
    prepare_wake_sources();
    bool button_armed = (GPIOC->IDR & (1UL << 5)) != 0;

    for (;;) {
        IWDG->KR = 0xAAAA;
        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
        EXTI->PR = WAKE_EXTI_MASK;
        LPTIM1->ICR = LPTIM_ICR_ARRMCF;
        NVIC->ICPR[0] = 0xFFFFFFFFUL;
        PWR->CR |= PWR_CR_CWUF;
        PWR->CR = (PWR->CR & ~PWR_CR_PDDS) | PWR_CR_LPSDSR;
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        __DSB();
        __WFI();
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

        if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
        if (GPIOC->IDR & (1UL << 5)) {
            button_armed = true;
        } else if (button_armed) {
            // LPTIM continues through STOP; accumulate across its ARR wrap.
            uint16_t prev = stable_lptim_count();
            uint32_t held_ticks = 0;
            while (!(GPIOC->IDR & (1UL << 5))) {
                uint16_t now = stable_lptim_count();
                held_ticks += now >= prev ? now - prev : LPTIM_ARR + 1U - prev + now;
                prev = now;
                IWDG->KR = 0xAAAA;
                if (held_ticks >= 289U) NVIC_SystemReset(); // About one second.
                if (!(GPIOC->IDR & (1UL << 13))) NVIC_SystemReset();
            }
        }
    }
}
