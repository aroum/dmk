#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize keyboard subsystems
void keyboard_init(void);

// Main keyboard task executed by FreeRTOS scheduler
void keyboard_task(void *pvParameters);

// Send key press/release event to USB queue
void keyboard_send_key(uint16_t keycode, bool pressed);

// Send modifier bitmask press/release to USB queue
void keyboard_send_modifiers(uint8_t mod_mask, bool pressed);

// Core event processor for resolving actions
void process_key_event(uint8_t row, uint8_t col, uint32_t key, bool pressed);

#ifdef __cplusplus
}
#endif

#endif // KEYBOARD_H
