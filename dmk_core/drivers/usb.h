#ifndef _USB_H
#define _USB_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_CONSUMER_FLAG 0x8000

/**
 * @brief Initialize USB hardware controller and endpoints.
 */
void usb_init(void);

/**
 * @brief Fast-path handler: update USB HID report buffer and transmit to host.
 * @param keycode 16-bit USB HID keycode
 * @param pressed True on press, false on release
 */
void usb_process_key(uint16_t keycode, bool pressed);

#ifdef MATRIX_REPORT_BATCH
void usb_process_batched_key(uint16_t keycode, bool pressed);
void usb_flush_batched_keys(void);
#endif

/**
 * @brief Batch process modifier keys: atomically update modifier bitmask and transmit to host.
 * @param mod_mask 8-bit modifier mask (MOD_LCTRL, etc.)
 * @param pressed True on press, false on release
 */
// Current physical/HT/One-Shot modifiers combined with MK weak modifiers.
uint8_t usb_get_effective_modifiers(void);
#ifdef HHKB_GRAVE_ESCAPE
void usb_set_suppressed_modifiers(uint8_t mask);
void usb_press_key_suppressed(uint8_t keycode,uint8_t mask);
#endif

void usb_process_modifiers(uint8_t mod_mask, bool pressed);

/**
 * @brief Batch process modified key: atomically update modifiers and keycode in a single HID report.
 * @param mod_mask 8-bit modifier mask
 * @param keycode 8-bit USB HID keycode
 * @param pressed True on press, false on release
 */
void usb_process_key_with_modifiers(uint8_t mod_mask, uint8_t keycode, bool pressed);

bool usb_remote_wakeup_enabled(void);
void keyboard_usb_resume_cb(void);
bool keyboard_paranoid_mode(void);

#ifdef __cplusplus
}
#endif

#endif // _USB_H
