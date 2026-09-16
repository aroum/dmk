#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize keyboard core subsystems, timers, and driver queues.
 */
void keyboard_init(void);

/**
 * @brief Main keyboard task loop executed by FreeRTOS scheduler.
 * @param pvParameters FreeRTOS task parameters pointer (unused)
 */
void keyboard_task(void *pvParameters);

/**
 * @brief Queue standard key press or release event to USB report task.
 * @param keycode 16-bit USB HID keycode
 * @param pressed True on press, false on release
 */
void keyboard_send_key(uint16_t keycode, bool pressed);

/**
 * @brief Send modifier bitmask update directly to USB report task.
 * @param mod_mask 8-bit modifier mask
 * @param pressed True on press, false on release
 */
void keyboard_send_modifiers(uint8_t mod_mask, bool pressed);

/**
 * @brief Core event processor for resolving matrix coordinate into keycode action.
 * @param row Matrix row index
 * @param col Matrix col index
 * @param key Resolved 32-bit keycode
 * @param pressed True on press, false on release
 */
void process_key_event(uint8_t row, uint8_t col, uint32_t key, bool pressed);

/**
 * @brief Reset MCU and jump to platform bootloader.
 */
void bootloader_jump(void);

#ifdef __cplusplus
}
#endif

#endif // KEYBOARD_H
