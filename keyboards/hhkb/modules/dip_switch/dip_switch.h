#ifndef DIP_SWITCH_H
#define DIP_SWITCH_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DIP_SWITCH_COUNT 6

typedef enum {
    DIP_SW1 = 0, // Mode select bit 0 (HHK / Win / Mac)
    DIP_SW2 = 1, // Mode select bit 1
    DIP_SW3 = 2, // Delete vs Backspace
    DIP_SW4 = 3, // Left ◇ (Gui) vs Fn
    DIP_SW5 = 4, // Alt / ◇ swap
    DIP_SW6 = 5  // SW6: Stock Mode or Power Saving
} dip_switch_num_t;

/**
 * Addresses in STM32L072 integrated EEPROM / Flash for recovery:
 * - HHKB_EEPROM_BANK2_STATUS_ADDR (0x08080348): Bank 2 status flag in EEPROM (0x01 = broken).
 * - HHKB_EEPROM_BANK2_CRC_ADDR (0x08080370): 2-byte CRC tag of Bank 2 in EEPROM.
 * - HHKB_BANK2_VECTOR_PAGE_ADDR (0x08010000): First 128-byte page of Bank 2 flash (vector table).
 */
#define HHKB_EEPROM_BANK2_STATUS_ADDR 0x08080348
#define HHKB_EEPROM_BANK2_CRC_ADDR    0x08080370
#define HHKB_BANK2_VECTOR_PAGE_ADDR   0x08010000

/**
 * @brief Early check executed before full DMK init.
 * When HHKB_SW6_REBOOT_TO_BANK1 is enabled:
 * - If SW6 is ON: triggers failsafe recovery to Bank 1 by:
 *   1. Invalidating EEPROM status byte (0x01) and CRC tag (0x0000).
 *   2. Erasing the first page (128 bytes) of Bank 2 Flash (clearing SP & Reset Vector).
 *   3. Resetting system into Bank 1 stock recovery bootloader.
 * - If SW6 is OFF: boots DMK normally in Bank 2.
 */
void hhkb_early_dip_check(void);

/**
 * @brief Initialize all 6 DIP switch pins with internal pull-ups.
 */
void dip_switch_init(void);

/**
 * @brief Read all DIP switches as a 6-bit bitmask (bit 0 = SW1, bit 5 = SW6).
 * 1 means ON, 0 means OFF.
 */
uint8_t dip_switch_read(void);

/**
 * @brief Query individual DIP switch state.
 */
bool dip_switch_get(dip_switch_num_t sw);

/**
 * @brief Check if Power Saving mode is active (when SW6 is used for power saving).
 * OFF = Power Saving Enabled (USB Remote Wakeup disabled / low power).
 * ON  = Power Saving Disabled (Full speed / responsive wakeup).
 */
bool hhkb_is_power_saving_enabled(void);

#ifdef __cplusplus
}
#endif

#endif // DIP_SWITCH_H
