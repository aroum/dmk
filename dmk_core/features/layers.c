#include "layers.h"
#include "FreeRTOS.h"
#include "keys.h"
#include "macros.h"
#include "queue.h"
#include "usb.h"

#define DEFINE_KEYMAP
#include "config.h"

#ifdef VIAL
#include "vial.h"
#endif

#include <string.h>

// Current highest active layer index (0..15)
uint8_t layer = 0;

// Layer state bitmask (bit 0 is active by default for base layer)
static uint16_t layer_state = 1;

// Flat key index lookup table: layout_map[row][col] = flat layout index or -1
static int16_t layout_map[NUM_ROWS][NUM_COLS];

/**
 * @brief Fast calculation of the highest active layer index using CLZ instruction.
 * @param state Bitmask of active layers.
 * @return Highest set layer bit index (0..15).
 */
static inline uint8_t compute_active_layer(uint16_t state) {
    if (state == 0)
        return 0;
#if defined(__GNUC__) || defined(__clang__)
    // 31 - leading zeros of 16-bit value in 32-bit register
    return (uint8_t)(31 - __builtin_clz((uint32_t)state));
#else
    for (int i = 15; i >= 0; i--) {
        if (state & (1U << i)) {
            return (uint8_t)i;
        }
    }
    return 0;
#endif
}

/**
 * @brief Initialize layer stack and pre-compute 2D row/col to flat key index mapping table.
 */
void layers_init(void) {
    typedef struct {
        uint8_t r, c;
    } RC;
    static const RC layout[] = LAYOUT;
    memset(layout_map, -1, sizeof(layout_map));
    for (size_t i = 0; i < sizeof(layout) / sizeof(layout[0]); i++) {
        if (layout[i].r < NUM_ROWS && layout[i].c < NUM_COLS) {
            layout_map[layout[i].r][layout[i].c] = (int16_t)i;
        }
    }
    layer_state = 1;
    layer = 0;
}

/**
 * @brief Get total number of configured layers in keymap.
 */
uint8_t layers_get_count(void) {
    return (uint8_t)keymap_layers;
}

/**
 * @brief Get the highest active layer index.
 */
uint8_t layers_get_active(void) {
    return layer;
}

/**
 * @brief Get the full 16-bit layer bitmask state.
 */
uint16_t layers_get_state(void) {
    return layer_state;
}

/**
 * @brief Set the full layer bitmask state and recalculate the active layer index.
 * @param state New 16-bit layer bitmask.
 */
void layers_set_state(uint16_t state) {
    layer_state = state ? state : 1;
    layer = compute_active_layer(layer_state);
    macros_run_layer_triggers(layer);
}

/**
 * @brief Enable a specific layer in the active layer bitmask.
 * @param layer_idx Layer index (0..15).
 */
void layers_on(uint8_t layer_idx) {
    if (layer_idx < 16) {
        layer_state |= (1U << layer_idx);
        layer = compute_active_layer(layer_state);
        macros_run_layer_triggers(layer);
    }
}

/**
 * @brief Disable a specific layer in the active layer bitmask (base layer 0 fallback guaranteed).
 * @param layer_idx Layer index (0..15).
 */
void layers_off(uint8_t layer_idx) {
    if (layer_idx < 16) {
        layer_state &= ~(1U << layer_idx);
        if (layer_state == 0) {
            layer_state = 1; // Default base layer always remains active
        }
        layer = compute_active_layer(layer_state);
        macros_run_layer_triggers(layer);
    }
}

/**
 * @brief Toggle the state of a specific layer in the active bitmask.
 * @param layer_idx Layer index (0..15).
 */
void layers_toggle(uint8_t layer_idx) {
    if (layer_idx < 16) {
        layer_state ^= (1U << layer_idx);
        if (layer_state == 0) {
            layer_state = 1;
        }
        layer = compute_active_layer(layer_state);
        macros_run_layer_triggers(layer);
    }
}

/**
 * @brief Translate (row, col) matrix coordinates into linear 1D layout index.
 * @param row Matrix row.
 * @param col Matrix column.
 * @return 0-based key index or -1 if unmapped.
 */
int16_t keyboard_get_flat_key_index(uint8_t row, uint8_t col) {
    if (row < NUM_ROWS && col < NUM_COLS) {
        return layout_map[row][col];
    }
    return -1;
}

/**
 * @brief Look up a keycode on an explicit layer for a specific physical key position.
 * @param layer_idx Target layer.
 * @param row Matrix row.
 * @param col Matrix column.
 * @return Keycode defined on the layer, or 0 if out of bounds.
 */
uint32_t layers_lookup_key_on_layer(uint8_t layer_idx, uint8_t row, uint8_t col) {
    int16_t ki = keyboard_get_flat_key_index(row, col);
    if (ki < 0 || ki >= (int16_t)NUM_KEYS) {
        return 0;
    }
#ifdef VIAL
    return dynamic_keymap[layer_idx][ki];
#else
    if (layer_idx < keymap_layers) {
        return keymap[layer_idx][ki];
    }
    return 0;
#endif
}

/**
 * @brief Resolve active keycode by scanning down the active layer stack (highest layer to base layer).
 * Transparent keys (K_TRNS / 0) fall through to the layer below.
 * @param row Matrix row.
 * @param col Matrix column.
 * @return Resolved 32-bit keycode.
 */
uint32_t layers_lookup_key(uint8_t row, uint8_t col) {
    int16_t ki = keyboard_get_flat_key_index(row, col);
    if (ki < 0 || ki >= (int16_t)NUM_KEYS) {
        return 0;
    }

    uint8_t top_layer = compute_active_layer(layer_state);
    for (int l = top_layer; l >= 0; l--) {
        if (!(layer_state & (1U << l))) {
            continue;
        }
        uint32_t key = 0;
#ifdef VIAL
        key = dynamic_keymap[l][ki];
#else
        if ((size_t)l < keymap_layers) {
            key = keymap[l][ki];
        }
#endif
        if (key != 0 && key != K_TRNS) {
            return key;
        }
    }
    return 0;
}
