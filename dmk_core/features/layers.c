#include "layers.h"
#include "FreeRTOS.h"
#include "hooks.h"
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
static uint16_t latched_state = 1;
static uint8_t held_refs[16];

// Flat key index lookup table: layout_map[row][col] = flat layout index or 0xFF (unmapped)
static uint8_t layout_map[NUM_ROWS][NUM_COLS];

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
    memset(layout_map, 0xFF, sizeof(layout_map));
    for (size_t i = 0; i < sizeof(layout) / sizeof(layout[0]); i++) {
        if (layout[i].r < NUM_ROWS && layout[i].c < NUM_COLS) {
            layout_map[layout[i].r][layout[i].c] = (uint8_t)i;
        }
    }
    layer_state = latched_state = 1;
    memset(held_refs, 0, sizeof(held_refs));
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
static void publish_layers(void) {
    uint16_t state = latched_state;
    for (unsigned i = 0; i < 16; ++i) if (held_refs[i]) state |= 1U << i;
    if (!state) state = 1;
    if (state == layer_state) return;
    layer_state = state;
    layer = compute_active_layer(state);
    macros_run_layer_triggers(layer);
    hooks_dispatch_layer_change(layer);
}

uint16_t layers_get_latched_state(void) { return latched_state; }

// Absolute persistent state; momentary owners remain effective until released.
void layers_set_state(uint16_t state) {
    latched_state = state;
    publish_layers();
}

void layers_on(uint8_t layer_idx) {
    if (layer_idx >= 16) return;
    if (held_refs[layer_idx] != UINT8_MAX) ++held_refs[layer_idx];
    publish_layers();
}

void layers_off(uint8_t layer_idx) {
    if (layer_idx >= 16) return;
    if (held_refs[layer_idx]) --held_refs[layer_idx];
    publish_layers();
}

void layers_toggle(uint8_t layer_idx) {
    if (layer_idx >= 16) return;
    latched_state ^= 1U << layer_idx;
    publish_layers();
}

/**
 * @brief Translate (row, col) matrix coordinates into linear 1D layout index.
 * @param row Matrix row.
 * @param col Matrix column.
 * @return 0-based key index or -1 if unmapped.
 */
int16_t keyboard_get_flat_key_index(uint8_t row, uint8_t col) {
    if (row < NUM_ROWS && col < NUM_COLS) {
        uint8_t idx = layout_map[row][col];
        return (idx == 0xFF) ? -1 : (int16_t)idx;
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
    return layer_idx < DYNAMIC_KEYMAP_MAX_LAYERS ? dynamic_keymap[layer_idx][ki] : 0;
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
        if (l < DYNAMIC_KEYMAP_MAX_LAYERS) key = dynamic_keymap[l][ki];
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
