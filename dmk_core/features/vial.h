#ifndef __VIAL_H
#define __VIAL_H

#include "FreeRTOS.h"
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
#define VIAL_COMBO_ENTRIES 8
#define DYNAMIC_KEYMAP_MACRO_COUNT 16
#define DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE 2048

typedef struct {
    uint16_t input[4];
    uint16_t output;
} vial_combo_entry_t;

typedef struct {
    uint32_t magic;
    uint32_t keymap[DYNAMIC_KEYMAP_MAX_LAYERS][NUM_KEYS];
    uint32_t encoder_keymap[DYNAMIC_KEYMAP_MAX_LAYERS][4][2];
    uint8_t rgb_cfg[8];
    uint8_t macro_buffer[DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE];
    vial_combo_entry_t combos[VIAL_COMBO_ENTRIES];
} vial_storage_data_t;

typedef union {
    vial_storage_data_t data;
    uint8_t padding[((sizeof(vial_storage_data_t) + 255) / 256) * 256];
} vial_storage_t;

extern vial_storage_t vial_storage;

#define dynamic_keymap (vial_storage.data.keymap)
#define dynamic_encoder_keymap (vial_storage.data.encoder_keymap)
#define dynamic_macro_buffer (vial_storage.data.macro_buffer)
#define vial_combos (vial_storage.data.combos)
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
void vial_eeprom_task(void);
void vial_eeprom_save(void);
void vial_macro_send(uint8_t id);
bool vial_macro_has_active(void);
TickType_t vial_macro_check_timeouts(TickType_t now);

#ifdef __cplusplus
}
#endif

#endif // __VIAL_H
