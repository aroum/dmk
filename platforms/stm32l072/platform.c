#include "keyboard.h"
#include "hal_gpio.h"
#include "classic_matrix.h"
#include "dip_switch.h"
#include "stm32l0xx.h"
#include "tusb.h"
#include <string.h>

// Kept in the image so local tools can verify model/layout without filenames.
#if defined(HHKB_HYBRID)
# if defined(HHKB_JIS)
#  define HHKB_IMAGE_MODEL "AHJX01"
# else
#  define HHKB_IMAGE_MODEL "AHUX01"
# endif
#elif defined(HHKB_CLASSIC_TYPE_S)
# if defined(HHKB_JIS)
#  define HHKB_IMAGE_MODEL "AHKS01"
# else
#  define HHKB_IMAGE_MODEL "AHHS01"
# endif
#else
# define HHKB_IMAGE_MODEL "AHHX01"
#endif
static const char hhkb_image_identity[] = "DMKHHKB1|" HHKB_IMAGE_MODEL "|" DMK_FW_VERSION "|END";

void USB_IRQHandler(void) {
    tud_int_handler(0);
}

#ifdef HHKB_HYBRID
static void hhkb_hybrid_startup_pd2(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIODEN;
    (void)RCC->IOPENR; // Complete clock enable before accessing GPIOD.
    // Preload HIGH before selecting push-pull output; keep it through init.
    GPIOD->BSRR = 1U << 2;
    GPIOD->OTYPER &= ~(1U << 2);
    GPIOD->PUPDR &= ~(3U << 4);
    GPIOD->MODER = (GPIOD->MODER & ~(3U << 4)) | (1U << 4);
}
#endif

#ifdef HHKB_BOOT_TRACE
// Pre-scheduler landmarks only. Orange pulse groups are readable on video
// even if neither USB nor the RTOS starts.
void hhkb_boot_pulses(unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        GPIOA->BSRR = 1U << 9;
        for (volatile unsigned d = 0; d < 320000; ++d) __NOP();
        GPIOA->BRR = 1U << 9;
        for (volatile unsigned d = 0; d < 320000; ++d) __NOP();
    }
    for (volatile unsigned d = 0; d < 1000000; ++d) __NOP();
}
#endif

void platform_init(void) {
    __asm__ volatile("" : : "r"(hhkb_image_identity) : "memory");
    // 0. Early check: If SW6 is ON, branch immediately to Bank 1 (stock factory firmware)
    hhkb_early_dip_check();
#ifdef HHKB_HYBRID
    hhkb_hybrid_startup_pd2();
#endif

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

    // Restore the HSI48 reference explicitly: the wake boot path may arrive
    // with this reference disabled and otherwise deadlock waiting for RDY.
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->CFGR3 |= SYSCFG_CFGR3_ENREF_HSI48;
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
#ifdef HHKB_BOOT_TRACE
    hhkb_boot_pulses(1);
#endif
    hhkb_matrix_hardware_init();
}

void platform_system_reset(void) {
    NVIC_SystemReset();
}
