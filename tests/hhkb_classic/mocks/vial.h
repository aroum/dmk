#ifndef MOCK_VIAL_H
#define MOCK_VIAL_H

#include <stdint.h>
#include <stddef.h>

#define DYNAMIC_KEYMAP_MAX_LAYERS 4
#ifndef NUM_KEYS
#define NUM_KEYS 60
#endif
#define VIAL_COMBO_ENTRIES 8
#ifndef DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE
#define DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE 224
#endif

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
extern uint32_t dynamic_keymap[4][NUM_KEYS];

#endif
