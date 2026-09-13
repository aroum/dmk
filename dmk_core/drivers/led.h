#ifndef LED_H
#define LED_H

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(LED_PINS) || defined(LED_DEBUG) || defined(LED_PIN) || defined(LED_ACTIVITY_PIN)) && !defined(NO_LED)

// Initialize board LED GPIO pins
void led_init(void);

// Non-blocking periodic update for debug/heartbeat LED (called from keyboard loop)
void led_update(TickType_t now);
TickType_t led_check_timeouts(TickType_t now);

// Makes pi pico led blink
void led_blink(void);

// Turns debug LED on
void led_on(void);

// Turns debug LED off
void led_off(void);

// Set host HID LED indicators state (Caps Lock, Num Lock, etc.)
void led_set_hid_state(uint8_t state);

// Activity indicator on switch press/release
void led_activity(bool pressed);

#else

#ifndef LED_C_SRC
static inline void led_init(void) {}
static inline void led_update(TickType_t now) {
    (void)now;
}
static inline TickType_t led_check_timeouts(TickType_t now) {
    (void)now;
    return portMAX_DELAY;
}
static inline void led_blink(void) {}
static inline void led_on(void) {}
static inline void led_off(void) {}
static inline void led_set_hid_state(uint8_t state) {
    (void)state;
}
static inline void led_activity(bool pressed) {
    (void)pressed;
}
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif // LED_H