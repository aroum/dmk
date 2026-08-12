#ifndef LAYERS_H
#define LAYERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Current active layer index (maintained for backward compatibility)
extern uint8_t layer;

// Initialize the layer subsystem and layout mapping table
void layers_init(void);

// Get the highest active layer index
uint8_t layers_get_active(void);

// Get the current 16-bit layer state bitmask
uint16_t layers_get_state(void);

// Directly set the layer state bitmask
void layers_set_state(uint16_t state);

// Activate a layer in the bitmask
void layers_on(uint8_t layer_idx);

// Deactivate a layer in the bitmask
void layers_off(uint8_t layer_idx);

// Toggle a layer in the bitmask
void layers_toggle(uint8_t layer_idx);

// Get the total number of defined layers
uint8_t layers_get_count(void);

// Lookup keycode at (row, col) traversing active layers top-to-bottom
uint32_t layers_lookup_key(uint8_t row, uint8_t col);

// Lookup keycode at a specific layer index
uint32_t layers_lookup_key_on_layer(uint8_t layer_idx, uint8_t row, uint8_t col);

// Get flat key index for a given (row, col) in LAYOUT, or -1 if not mapped
int16_t keyboard_get_flat_key_index(uint8_t row, uint8_t col);

#ifdef __cplusplus
}
#endif

#endif // LAYERS_H
