#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize board LED GPIO pins
void led_init(void);

// Non-blocking periodic update for debug/heartbeat LED (called from keyboard loop)
void led_update(TickType_t now);

// Makes pi pico led blink
void led_blink(void);

// Turns debug LED on
void led_on(void);

// Turns debug LED off
void led_off(void);

// Set host HID LED indicators state (Caps Lock, Num Lock, etc.)
void led_set_hid_state(uint8_t state);

#ifdef __cplusplus
}
#endif

#endif // LED_H