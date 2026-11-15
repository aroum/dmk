#include "keyboard.h"
#include "hal_gpio.h"
#include "classic_matrix.h"
#include "dip_switch.h"
#include "stm32l0xx.h"
#include "tusb.h"
#include <string.h>

void USB_IRQHandler(void) {
    tud_int_handler(0);
}

void platform_init(void) {
    // 0. Early check: If SW6 is ON, branch immediately to Bank 1 (stock factory firmware)
    hhkb_early_dip_check();

    // Debug LED: initialize PA8 & PA9 outputs
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    GPIOA->MODER &= ~((3U << (8 * 2)) | (3U << (9 * 2)));
    GPIOA->MODER |=  ((1U << (8 * 2)) | (1U << (9 * 2)));

    // Flash both LEDs 3 times rapidly to show Bank 2 code execution started
    for (int k = 0; k < 3; k++) {
        GPIOA->BSRR = (1U << 8) | (1U << 9);
        for (volatile uint32_t d = 0; d < 80000; d++) __NOP();
        GPIOA->BRR = (1U << 8) | (1U << 9);
        for (volatile uint32_t d = 0; d < 80000; d++) __NOP();
    }
    // Turn PA8 ON indicating platform init in progress
    GPIOA->BSRR = (1U << 8);

    // Configure system clocks for STM32L072: HSI16 + PLL -> 32MHz SYSCLK, HSI48 for USB
    // Enable HSI16 and HSI48
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // Bank 1 may leave PLL and bus prescalers configured. Switch away from
    // PLL before changing it and establish the frequencies expected by RTOS.
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY);
    RCC->CR &= ~RCC_CR_HSIDIVEN;
    while (RCC->CR & RCC_CR_HSIDIVF);
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);

    // Enable HSI48 for crystal-less USB 2.0 Full Speed
    RCC->CRRCR |= RCC_CRRCR_HSI48ON;
    while (!(RCC->CRRCR & RCC_CRRCR_HSI48RDY));

    RCC->CCIPR |= RCC_CCIPR_HSI48SEL;
    RCC->APB1ENR |= RCC_APB1ENR_CRSEN | RCC_APB1ENR_USBEN;
    // 48 MHz / 1 kHz SOF - 1; use the ST default error limit and trim.
    CRS->CR = 32U << CRS_CR_TRIM_Pos;
    CRS->CFGR = (47999U << CRS_CFGR_RELOAD_Pos) |
                (34U << CRS_CFGR_FELIM_Pos) | CRS_CFGR_SYNCSRC_1;
    CRS->CR |= CRS_CR_AUTOTRIMEN | CRS_CR_CEN;
    NVIC_SetPriority(USB_IRQn, 2);

    // Enable Power Control clock & configure Voltage Scaling Range 1 (1.8V) for 32MHz
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR = (PWR->CR & ~PWR_CR_VOS) | PWR_CR_VOS_0; // Range 1
    while (PWR->CSR & PWR_CSR_VOSF);

    // Flash latency: 1 wait state for 32MHz Range 1
    FLASH->ACR |= FLASH_ACR_LATENCY;
    while (!(FLASH->ACR & FLASH_ACR_LATENCY));

    // PLL configuration: HSI16 (16MHz) * 4 / 2 = 32MHz
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
    RCC->CFGR |= (RCC_CFGR_PLLMUL4 | RCC_CFGR_PLLDIV2); // 16 * 4 / 2 = 32 MHz
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // Select PLL as system clock source
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    SystemCoreClock = 32000000;

    // Initialize HHKB matrix hardware (ADC, OpAmp, demuxers)
    hhkb_matrix_hardware_init();
}

void platform_system_reset(void) {
    NVIC_SystemReset();
}
