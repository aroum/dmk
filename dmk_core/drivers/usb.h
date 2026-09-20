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

/**
 * @brief Batch process modifier keys: atomically update modifier bitmask and transmit to host.
 * @param mod_mask 8-bit modifier mask (MOD_LCTRL, etc.)
 * @param pressed True on press, false on release
 */
void usb_process_modifiers(uint8_t mod_mask, bool pressed);

/**
 * @brief Batch process modified key: atomically update modifiers and keycode in a single HID report.
 * @param mod_mask 8-bit modifier mask
 * @param keycode 8-bit USB HID keycode
 * @param pressed True on press, false on release
 */
void usb_process_key_with_modifiers(uint8_t mod_mask, uint8_t keycode, bool pressed);

#ifdef __cplusplus
}
#endif

#endif // _USB_H
