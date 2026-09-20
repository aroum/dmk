#ifndef COMBOS_H
#define COMBOS_H

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef COMBO_TERM_MS
#define COMBO_TERM_MS 50
#endif

#ifndef CHORD_TIMEOUT_MS
#define CHORD_TIMEOUT_MS 50
#endif

#if defined(NO_COMBOS)
static inline void combos_init(void) {}
static inline bool combos_process_event(uint8_t row, uint8_t col, bool pressed, TickType_t now) {
    (void)row;
    (void)col;
    (void)pressed;
    (void)now;
    return false;
}
static inline TickType_t combos_check_timeouts(TickType_t now) {
    (void)now;
    return portMAX_DELAY;
}
static inline void chords_flush(void) {}
static inline bool combos_has_active(void) {
    return false;
}
#else
// Initialize Combos and Chords subsystem
void combos_init(void);

// Process a matrix event through Combos/Chords engine.
// Returns true if the event was absorbed or handled by a combo/chord.
bool combos_process_event(uint8_t row, uint8_t col, bool pressed, TickType_t now);

// Check timeouts for pending combos/chords; returns ticks until next deadline or portMAX_DELAY
TickType_t combos_check_timeouts(TickType_t now);

// Returns true if there are buffered keys waiting for combo/chord timeout
bool combos_has_active(void);

// Explicitly flush any buffered chords
void chords_flush(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // COMBOS_H
