#ifndef MOUSE_H
#define MOUSE_H

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Standard USB HID Mouse Button Bitmasks (Up to 5 buttons) */
#define MOUSE_BTN_LEFT    (1 << 0)
#define MOUSE_BTN_RIGHT   (1 << 1)
#define MOUSE_BTN_MIDDLE  (1 << 2)
#define MOUSE_BTN_BACK    (1 << 3)
#define MOUSE_BTN_FORWARD (1 << 4)

/**
 * @brief Initialize mouse subsystem state and timers.
 */
void mouse_init(void);

/**
 * @brief Process decoded mouse keycodes (cursor movement, buttons, wheel, accel).
 * @param key 32-bit composite keycode.
 * @param pressed True on press, false on release.
 * @return true if key was handled by mouse subsystem, false otherwise.
 */
bool mouse_process_key(uint32_t key, bool pressed);

/**
 * @brief Periodic timer tick handler called from main keyboard loop.
 * @param now Current FreeRTOS tick count.
 * @return Next deadline timeout in ticks (or portMAX_DELAY if no mouse motion).
 */
TickType_t mouse_check_timeouts(TickType_t now);

/* -------------------------------------------------------------------------- */
/* Direct API for Trackballs, Optical Sensors (PMW3360/3389), Encoders, Modules */
/* -------------------------------------------------------------------------- */

/**
 * @brief Move mouse cursor relatively by (dx, dy).
 * Passes through hook_mouse_move() for optional interception/filtering/scrolling.
 * @param dx Relative horizontal movement (-127 to 127).
 * @param dy Relative vertical movement (-127 to 127).
 */
void mouse_move(int8_t dx, int8_t dy);

/**
 * @brief Scroll relatively by (wheel, pan).
 * Passes through hook_mouse_scroll() for optional interception.
 * @param wheel Relative vertical scroll (-127 to 127, positive = up).
 * @param pan Relative horizontal pan scroll (-127 to 127, positive = right).
 */
void mouse_scroll(int8_t wheel, int8_t pan);

/**
 * @brief Press one or more mouse buttons.
 * @param button_mask Bitmask of buttons to press (e.g. MOUSE_BTN_LEFT).
 */
void mouse_button_press(uint8_t button_mask);

/**
 * @brief Release one or more mouse buttons.
 * @param button_mask Bitmask of buttons to release.
 */
void mouse_button_release(uint8_t button_mask);

/**
 * @brief Set exact state of a button mask.
 * @param button_mask Bitmask of buttons.
 * @param pressed True to press, false to release.
 */
void mouse_button_set(uint8_t button_mask, bool pressed);

/**
 * @brief Send raw mouse report immediately.
 * @param buttons Bitmask of pressed buttons.
 * @param dx Relative horizontal movement.
 * @param dy Relative vertical movement.
 * @param wheel Relative vertical scroll.
 * @param pan Relative horizontal pan.
 */
void mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan);

/**
 * @brief Get current mouse button bitmask state.
 */
uint8_t mouse_get_buttons(void);

#ifdef __cplusplus
}
#endif

#endif // MOUSE_H
