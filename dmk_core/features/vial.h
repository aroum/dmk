#ifndef __VIAL_H
#define __VIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Dynamic keymap array that can be edited at runtime
#include "config.h"
#ifndef DYNAMIC_KEYMAP_MAX_LAYERS
#define DYNAMIC_KEYMAP_MAX_LAYERS 8
#endif

#ifdef VIAL
extern uint32_t dynamic_keymap[DYNAMIC_KEYMAP_MAX_LAYERS][NUM_KEYS];

#define VIAL_COMBO_ENTRIES 8

typedef struct {
    uint16_t input[4];
    uint16_t output;
} vial_combo_entry_t;

extern vial_combo_entry_t vial_combos[VIAL_COMBO_ENTRIES];
#endif

uint16_t to_via_keycode(uint32_t dmk_key);
uint32_t from_via_keycode(uint16_t via_key);

/**
 * @brief Initialize the dynamic keymap by copying the compile-time keymap to RAM.
 */
void vial_init(void);

/**
 * @brief Process an incoming 32-byte VIA/Vial request packet and generate a 32-byte response.
 * @param request Pointer to the 32-byte incoming buffer.
 * @param response Pointer to the 32-byte outgoing buffer.
 */
void vial_process_packet(uint8_t const *request, uint8_t *response);
void vial_macro_send(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif // __VIAL_H
