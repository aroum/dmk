#ifndef MOUSE_H
#define MOUSE_H

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard USB HID Mouse Button Bitmasks (Up to 5 buttons) */
#define MOUSE_BTN_LEFT (1 << 0)
#define MOUSE_BTN_RIGHT (1 << 1)
#define MOUSE_BTN_MIDDLE (1 << 2)
#define MOUSE_BTN_BACK (1 << 3)
#define MOUSE_BTN_FORWARD (1 << 4)

#ifndef NO_MOUSE

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

void mouse_move(int8_t dx, int8_t dy);
void mouse_scroll(int8_t wheel, int8_t pan);
void mouse_button_press(uint8_t button_mask);
void mouse_button_release(uint8_t button_mask);
void mouse_button_set(uint8_t button_mask, bool pressed);
void mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan);
uint8_t mouse_get_buttons(void);

#else // NO_MOUSE

static inline void mouse_init(void) {}
static inline bool mouse_process_key(uint32_t key, bool pressed) {
    (void)key;
    (void)pressed;
    return false;
}
static inline TickType_t mouse_check_timeouts(TickType_t now) {
    (void)now;
    return portMAX_DELAY;
}
static inline void mouse_move(int8_t dx, int8_t dy) { (void)dx; (void)dy; }
static inline void mouse_scroll(int8_t wheel, int8_t pan) { (void)wheel; (void)pan; }
static inline void mouse_button_press(uint8_t button_mask) { (void)button_mask; }
static inline void mouse_button_release(uint8_t button_mask) { (void)button_mask; }
static inline void mouse_button_set(uint8_t button_mask, bool pressed) { (void)button_mask; (void)pressed; }
static inline void mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan) {
    (void)buttons; (void)dx; (void)dy; (void)wheel; (void)pan;
}
static inline uint8_t mouse_get_buttons(void) { return 0; }

#endif // NO_MOUSE

#ifdef __cplusplus
}
#endif

#endif // MOUSE_H
