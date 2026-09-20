#ifndef MACROS_H
#define MACROS_H

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(NO_MACROS)
static inline bool macros_process_key(uint32_t key, bool pressed) {
    (void)key;
    (void)pressed;
    return false;
}
static inline TickType_t macros_check_timeouts(TickType_t now) {
    (void)now;
    return portMAX_DELAY;
}
static inline bool macros_has_active(void) {
    return false;
}
static inline void macros_run_layer_triggers(uint8_t new_layer) {
    (void)new_layer;
}
#else
// Process a macro keycode (0xC0 - 0xDF). Returns true if handled.
bool macros_process_key(uint32_t key, bool pressed);

// Periodic check for active non-blocking macro playback delay steps
TickType_t macros_check_timeouts(TickType_t now);

// Returns true if a macro is currently playing back
bool macros_has_active(void);

// Execute all macros whose layer_trigger matches the newly activated layer.
// Called automatically by the layer subsystem on every layer change.
void macros_run_layer_triggers(uint8_t new_layer);
#endif

#ifdef __cplusplus
}
#endif

#endif // MACROS_H
