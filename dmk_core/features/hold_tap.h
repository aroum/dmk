#ifndef HOLD_TAP_H
#define HOLD_TAP_H

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t HTState;
enum { HT_STATE_IDLE = 0, HT_STATE_PRESSED = 1, HT_STATE_HOLD = 2 };

typedef struct {
    uint32_t keycode;
    TickType_t press_time;
    TickType_t timeout_ticks;
    uint8_t row;
    uint8_t col;
    HTState state;
} HTTracker;

#ifndef MAX_HT_TRACKERS
#define MAX_HT_TRACKERS 4
#endif

#if defined(NO_HOLD_TAP)
static inline void hold_tap_init(void) {}
static inline bool hold_tap_process_event(uint8_t row, uint8_t col, uint32_t key, bool pressed) {
    (void)row; (void)col; (void)key; (void)pressed; return false;
}
static inline void hold_tap_permissive_resolve(uint8_t except_row, uint8_t except_col) {
    (void)except_row; (void)except_col;
}
static inline TickType_t hold_tap_check_timeouts(TickType_t now) {
    (void)now; return portMAX_DELAY;
}
#else
// Initialize the hold-tap tracker pool
void hold_tap_init(void);

// Process a Hold-Tap key event (returns true if handled by Hold-Tap subsystem)
bool hold_tap_process_event(uint8_t row, uint8_t col, uint32_t key, bool pressed);

// Permissive hold: immediately resolves any pending HT keys as holds when another key is pressed
void hold_tap_permissive_resolve(uint8_t except_row, uint8_t except_col);

// Check timeouts for all active hold-tap keys; returns ticks until next deadline or portMAX_DELAY
TickType_t hold_tap_check_timeouts(TickType_t now);
#endif

#ifdef __cplusplus
}
#endif

#endif // HOLD_TAP_H
