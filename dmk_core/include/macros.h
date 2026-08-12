#ifndef MACROS_H
#define MACROS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Process a macro keycode (0xC0 - 0xDF). Returns true if handled.
bool macros_process_key(uint32_t key, bool pressed);

// Execute all macros whose layer_trigger matches the newly activated layer.
// Called automatically by the layer subsystem on every layer change.
void macros_run_layer_triggers(uint8_t new_layer);

#ifdef __cplusplus
}
#endif

#endif // MACROS_H
