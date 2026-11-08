#include "keyboard.h"
#include "hal_gpio.h"
#include "classic_matrix.h"
#include "dip_switch.h"
#include "stm32l0xx.h"
#include <string.h>

void platform_init(void) {
    // 0. Early check: If SW6 is ON, branch immediately to Bank 1 (stock factory firmware)
    hhkb_early_dip_check();

    // Configure system clocks for STM32L072: HSI16 + PLL -> 32MHz SYSCLK, HSI48 for USB
    // Enable HSI16 and HSI48
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // Enable HSI48 for crystal-less USB 2.0 Full Speed
    RCC->CRRCR |= RCC_CRRCR_HSI48ON;
    while (!(RCC->CRRCR & RCC_CRRCR_HSI48RDY));

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
