#include "dip_switch.h"
#include "pin_defs.h"
#include "config.h"
#include "stm32l0xx.h"
#include "stm32l0xx_ll_gpio.h"
#include <stdbool.h>

/**
 * HHKB Classic DIP Switches:
 * SW1: PA3
 * SW2: PA4
 * SW3: PA5
 * SW4: PA6
 * SW5: PA7
 * SW6: PC4
 *
 * All switches connect to GND when ON (pull-up required, active LOW).
 */

static const struct {
    GPIO_TypeDef *port;
    uint32_t pin_mask;
} s_dip_map[DIP_SWITCH_COUNT] = {
    { GPIOA, 1U << 3 }, // SW1
    { GPIOA, 1U << 4 }, // SW2
    { GPIOA, 1U << 5 }, // SW3
    { GPIOA, 1U << 6 }, // SW4
    { GPIOA, 1U << 7 }, // SW5
    { GPIOC, 1U << 4 }  // SW6
};

static void write_eeprom_byte(uint32_t addr, uint8_t val) {
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        FLASH->PEKEYR = 0x89ABCDEF; // FLASH_PEKEY1
        FLASH->PEKEYR = 0x02030405; // FLASH_PEKEY2
    }
    while (FLASH->PECR & FLASH_PECR_PELOCK);

    while (FLASH->SR & FLASH_SR_BSY);
    *(volatile uint8_t *)addr = val;
    while (FLASH->SR & FLASH_SR_BSY);

    FLASH->PECR |= FLASH_PECR_PELOCK;
}

/**
 * @brief Erases the first 128-byte page of Flash Bank 2 (address 0x08010000).
 * Executed strictly from RAM with interrupts disabled to prevent instruction fetch
 * stalls during flash write/erase operations.
 */
__attribute__((section(".RamFunc"), noinline))
static void erase_bank2_vector_page_ram(void) {
    __disable_irq();

    // 1. Unlock FLASH_PECR using PEKEY1 & PEKEY2 if locked
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        FLASH->PEKEYR = 0x89ABCDEF;
        FLASH->PEKEYR = 0x02030405;
    }
    while (FLASH->PECR & FLASH_PECR_PELOCK);

    // 2. Unlock Program memory using PRGKEY1 & PRGKEY2 if PRGLOCK is set
    if (FLASH->PECR & FLASH_PECR_PRGLOCK) {
        FLASH->PRGKEYR = 0x8C9DAEBF; // FLASH_PRGKEY1
        FLASH->PRGKEYR = 0x13141516; // FLASH_PRGKEY2
    }
    while (FLASH->PECR & FLASH_PECR_PRGLOCK);

    // Wait until FLASH is ready
    while (FLASH->SR & FLASH_SR_BSY);

    // Clear any pending error/completion flags before starting
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_WRPERR | FLASH_SR_PGAERR
              | FLASH_SR_SIZERR | FLASH_SR_OPTVERR | FLASH_SR_RDERR
              | FLASH_SR_FWWERR | FLASH_SR_NOTZEROERR;

    // 3. Set ERASE and PROG bits in PECR for page erase
    FLASH->PECR |= (FLASH_PECR_ERASE | FLASH_PECR_PROG);

    // 4. Write 0x00000000 to the first word of the page (0x08010000) to trigger erase
    *(volatile uint32_t *)HHKB_BANK2_VECTOR_PAGE_ADDR = 0x00000000;

    // 5. Wait for erase operation to complete
    while (FLASH->SR & FLASH_SR_BSY);

    // 6. Check for flash errors (useful for debugging via J-Link / SWD readout)
    // Errors are non-fatal here: EEPROM flags and the erase itself provide redundancy.
    // On error, s_flash_erase_error is readable via debugger at the SRAM address.
    volatile uint32_t sr = FLASH->SR;
    if (sr & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR
            | FLASH_SR_NOTZEROERR | FLASH_SR_FWWERR)) {
        // Store error code in a fixed SRAM location for post-mortem debugging
        *(volatile uint32_t *)0x20004FF0 = sr;
        // Clear error flags
        FLASH->SR = sr;
    }

    // 7. Clear EOP flag and ERASE/PROG bits
    if (sr & FLASH_SR_EOP) {
        FLASH->SR = FLASH_SR_EOP;
    }
    FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_PROG);

    // 8. Re-lock Program memory and PECR
    FLASH->PECR |= FLASH_PECR_PRGLOCK;
    FLASH->PECR |= FLASH_PECR_PELOCK;

    // 9. Trigger system reset into Bank 1 bootloader
    NVIC_SystemReset();

    while (1) { __NOP(); }
}

void hhkb_early_dip_check(void) {
    // Enable GPIOA and GPIOC peripheral clocks
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOCEN;

    // Configure PC4 (SW6) as Input with Pull-Up
    GPIOC->MODER &= ~(3U << (4 * 2));
    GPIOC->PUPDR &= ~(3U << (4 * 2));
    GPIOC->PUPDR |=  (1U << (4 * 2)); // 01: Pull-up

    // Small delay to let line settle
    for (volatile uint32_t i = 0; i < 500; i++) __NOP();

#if defined(HHKB_SW6_REBOOT_TO_BANK1) && (HHKB_SW6_REBOOT_TO_BANK1 == 1)
    bool sw6_on = ((GPIOC->IDR & (1U << 4)) == 0);

    if (sw6_on) {
        // SW6 is ON: User requests failsafe stock recovery in Bank 1!
        // Combine all recovery mechanisms:
        // 1. Mark status byte as broken in EEPROM (0x01)
        write_eeprom_byte(HHKB_EEPROM_BANK2_STATUS_ADDR, 0x01);
        // 2. Corrupt Bank 2 CRC tag in EEPROM
        write_eeprom_byte(HHKB_EEPROM_BANK2_CRC_ADDR, 0x00);
        write_eeprom_byte(HHKB_EEPROM_BANK2_CRC_ADDR + 1, 0x00);
        // 3. Erase the first page of Bank 2 (vector table) and reset into Bank 1
        erase_bank2_vector_page_ram();
    }
#endif
}

void dip_switch_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOCEN;

    // SW1..SW5 on GPIOA (PA3..PA7)
    for (uint8_t pin = 3; pin <= 7; pin++) {
        GPIOA->MODER &= ~(3U << (pin * 2));
        GPIOA->PUPDR &= ~(3U << (pin * 2));
        GPIOA->PUPDR |=  (1U << (pin * 2)); // Pull-up
    }

    // SW6 on GPIOC (PC4)
    GPIOC->MODER &= ~(3U << (4 * 2));
    GPIOC->PUPDR &= ~(3U << (4 * 2));
    GPIOC->PUPDR |=  (1U << (4 * 2)); // Pull-up
}

uint8_t dip_switch_read(void) {
    uint8_t state = 0;
    for (uint8_t i = 0; i < DIP_SWITCH_COUNT; i++) {
        // Active LOW: 0 when closed/ON
        if ((s_dip_map[i].port->IDR & s_dip_map[i].pin_mask) == 0) {
            state |= (1U << i);
        }
    }
    return state;
}

bool dip_switch_get(dip_switch_num_t sw) {
    if (sw >= DIP_SWITCH_COUNT) return false;
    return (s_dip_map[sw].port->IDR & s_dip_map[sw].pin_mask) == 0;
}

bool hhkb_is_power_saving_enabled(void) {
#if defined(HHKB_SW6_REBOOT_TO_BANK1) && (HHKB_SW6_REBOOT_TO_BANK1 == 1)
    // When SW6 is assigned to Bank 1 recovery, power saving defaults to standard USB mode
    return false;
#else
    // Official HHKB behavior: SW6 OFF = Power Saving Enabled; SW6 ON = Power Saving Disabled
    return !dip_switch_get(DIP_SW6);
#endif
}

// Override core weak power saving check
bool keyboard_power_saving_enabled(void) {
    return hhkb_is_power_saving_enabled();
}
