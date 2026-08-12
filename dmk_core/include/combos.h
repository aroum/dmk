#ifndef COMBOS_H
#define COMBOS_H

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Combos and Chords subsystem
void combos_init(void);

// Process a matrix event through Combos/Chords engine.
// Returns true if the event was absorbed or handled by a combo/chord.
bool combos_process_event(uint8_t row, uint8_t col, bool pressed, TickType_t now);

// Check timeouts for pending combos/chords; returns ticks until next deadline or portMAX_DELAY
TickType_t combos_check_timeouts(TickType_t now);

// Explicitly flush any buffered chords
void chords_flush(void);

#ifdef __cplusplus
}
#endif

#endif // COMBOS_H
