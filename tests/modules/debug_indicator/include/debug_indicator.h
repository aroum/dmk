#ifndef DEBUG_INDICATOR_H
#define DEBUG_INDICATOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Configuration options (can be overridden in config.h):
 *
 * 1. Key Press Indicator:
 *    #define DEBUG_KEY_PIN PB3               // Map key press to GPIO pin
 *    #define DEBUG_KEY_RGB_INDEX 2            // Map key press to RGB LED index
 *    #define DEBUG_KEY_RGB_COLOR 0x00FF00     // Color for key press (Green)
 *
 * 2. USB Connection Indicator:
 *    #define DEBUG_USB_PIN PB4               // Map USB mounted state to GPIO pin
 *    #define DEBUG_USB_RGB_INDEX 3            // Map USB state to RGB LED index
 *    #define DEBUG_USB_RGB_COLOR 0x0000FF     // Color for USB mounted (Blue)
 */

void debug_indicator_init(void);
void debug_indicator_update_usb(bool mounted);
void debug_indicator_update_key(bool pressed);

#ifdef __cplusplus
}
#endif

#endif // DEBUG_INDICATOR_H
