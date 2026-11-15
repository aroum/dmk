#include "keyboard.h"
#include "hal_gpio.h"
#include "stm32f0xx.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_system.h"
#include "tusb.h"
#include <string.h>

void USB_IRQHandler(void) {
    tud_int_handler(0);
}

#define DFU_BOOTLOADER_TOKEN_ADDR   0x20003F80UL
#define DFU_BOOTLOADER_TOKEN_MAGIC1 0xDEADBEEFUL
#define DFU_BOOTLOADER_TOKEN_MAGIC2 0xCAFEBABEL
#define STM32F072_SYSMEM_BASE       0x1FFFC800UL

void platform_init(void) {
    // 0. Safeguard check for DFU bootloader request token
    if (*(volatile uint32_t *)DFU_BOOTLOADER_TOKEN_ADDR == DFU_BOOTLOADER_TOKEN_MAGIC1 &&
        *(volatile uint32_t *)(DFU_BOOTLOADER_TOKEN_ADDR + 4) == DFU_BOOTLOADER_TOKEN_MAGIC2) {
        *(volatile uint32_t *)DFU_BOOTLOADER_TOKEN_ADDR = 0;
        *(volatile uint32_t *)(DFU_BOOTLOADER_TOKEN_ADDR + 4) = 0;

        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
        SYSCFG->CFGR1 = (SYSCFG->CFGR1 & ~SYSCFG_CFGR1_MEM_MODE) | SYSCFG_CFGR1_MEM_MODE_0;
        __DSB();
        __ISB();
        uint32_t boot_sp = *(volatile uint32_t *)STM32F072_SYSMEM_BASE;
        uint32_t boot_entry = *(volatile uint32_t *)(STM32F072_SYSMEM_BASE + 4);
        __asm__ volatile (
            "msr msp, %0\n"
            "bx %1\n"
            :
            : "r" (boot_sp), "r" (boot_entry)
            : "memory"
        );
        while (1) { __NOP(); }
    }

    // 1. Enable Flash Prefetch Buffer and set latency to 1 wait state for 48 MHz operation
    FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY);

    // 2. Enable HSI48 internal 48 MHz oscillator
    LL_RCC_HSI48_Enable();
    while (!LL_RCC_HSI48_IsReady());

    // 3. Switch SYSCLK to HSI48
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI48);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI48);

    // Set AHB and APB prescalers to /1 (HCLK = 48MHz, PCLK = 48MHz)
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    SystemCoreClock = 48000000;

    // 4. Configure USB Clock from HSI48 and enable Clock Recovery System (CRS) with USB SOF
    LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_HSI48);

    // Enable CRS clock and USB clock
    RCC->APB1ENR |= RCC_APB1ENR_CRSEN | RCC_APB1ENR_USBEN;

    // Standard CRS calibration: 48 MHz / 1 kHz SOF - 1 = 47999, FELIM = 34
    LL_CRS_ConfigSynchronization(0x20, 0x22, 47999,
                                 LL_CRS_SYNC_DIV_1 | LL_CRS_SYNC_SOURCE_USB | LL_CRS_SYNC_POLARITY_RISING);
    LL_CRS_EnableAutoTrimming();
    LL_CRS_EnableFreqErrorCounter();

    // 5. Enable GPIO Clocks (A, B, C)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

    // 6. Set USB Interrupt Priority
    NVIC_SetPriority(USB_IRQn, 2);
}

void platform_system_reset(void) {
    NVIC_SystemReset();
}
