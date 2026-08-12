#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pin_defs.h"
#include "config.h"
#include "cJSON.h"

// Fallback defaults if macros are not defined in config.h
#ifndef VIAL_KEYBOARD_NAME
#define VIAL_KEYBOARD_NAME "DMK Keyboard"
#endif

#ifndef VIAL_VENDOR_ID
#define VIAL_VENDOR_ID 0xCafe
#endif

#ifndef VIAL_PRODUCT_ID
#define VIAL_PRODUCT_ID 0x0001
#endif

#ifndef NUM_ROWS
#define NUM_ROWS 1
#endif

#ifndef NUM_COLS
#define NUM_COLS 1
#endif

struct layout_key {
    int r;
    int c;
};

// Count encoders from pin arrays defined in config.h
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
static int count_encoders(void) {
    // Use a temporary array to count at compile time via sizeof trick
    typedef int enc_pin_t;
    static const enc_pin_t enc_a[] = ENCODER_PINS_A;
    return (int)(sizeof(enc_a) / sizeof(enc_a[0]));
}
#endif

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        return 1;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", VIAL_KEYBOARD_NAME);

    char vid_str[16];
    sprintf(vid_str, "0x%04X", VIAL_VENDOR_ID);
    cJSON_AddStringToObject(root, "vendorId", vid_str);

    char pid_str[16];
    sprintf(pid_str, "0x%04X", VIAL_PRODUCT_ID);
    cJSON_AddStringToObject(root, "productId", pid_str);

#ifdef RGB_NUM
    cJSON_AddStringToObject(root, "lighting", "qmk_rgblight");
#endif

#ifdef MIDI_USB
    cJSON *vial_obj = cJSON_AddObjectToObject(root, "vial");
    cJSON_AddStringToObject(vial_obj, "midi", "advanced");

    cJSON *custom_keycodes = cJSON_AddArrayToObject(root, "customKeycodes");
    // 1. All INC keycodes (values 0x7E00 to 0x7E0F)
    for (int i = 0; i < 16; i++) {
        cJSON *kc_inc = cJSON_CreateObject();
        char *name_inc = malloc(32);
        char *title_inc = malloc(32);
        char *short_inc = malloc(16);
        sprintf(name_inc, "MIDI_CC_%d_INC", i);
        sprintf(title_inc, "MIDI CC %d Inc", i);
        sprintf(short_inc, "CC%d+", i);
        cJSON_AddStringToObject(kc_inc, "name", name_inc);
        cJSON_AddStringToObject(kc_inc, "title", title_inc);
        cJSON_AddStringToObject(kc_inc, "shortName", short_inc);
        cJSON_AddNumberToObject(kc_inc, "value", 0x7E00 + i);
        cJSON_AddItemToArray(custom_keycodes, kc_inc);
    }

    // 2. All DEC keycodes (values 0x7E10 to 0x7E1F)
    for (int i = 0; i < 16; i++) {
        cJSON *kc_dec = cJSON_CreateObject();
        char *name_dec = malloc(32);
        char *title_dec = malloc(32);
        char *short_dec = malloc(16);
        sprintf(name_dec, "MIDI_CC_%d_DEC", i);
        sprintf(title_dec, "MIDI CC %d Dec", i);
        sprintf(short_dec, "CC%d-", i);
        cJSON_AddStringToObject(kc_dec, "name", name_dec);
        cJSON_AddStringToObject(kc_dec, "title", title_dec);
        cJSON_AddStringToObject(kc_dec, "shortName", short_dec);
        cJSON_AddNumberToObject(kc_dec, "value", 0x7E10 + i);
        cJSON_AddItemToArray(custom_keycodes, kc_dec);
    }

    // 3. All VAL_127 keycodes (values 0x7E20 to 0x7E2F)
    for (int i = 0; i < 16; i++) {
        cJSON *kc_127 = cJSON_CreateObject();
        char *name_127 = malloc(32);
        char *title_127 = malloc(32);
        char *short_127 = malloc(16);
        sprintf(name_127, "MIDI_CC_%d_VAL_127", i);
        sprintf(title_127, "MIDI CC %d Val 127", i);
        sprintf(short_127, "CC%d!", i);
        cJSON_AddStringToObject(kc_127, "name", name_127);
        cJSON_AddStringToObject(kc_127, "title", title_127);
        cJSON_AddStringToObject(kc_127, "shortName", short_127);
        cJSON_AddNumberToObject(kc_127, "value", 0x7E20 + i);
        cJSON_AddItemToArray(custom_keycodes, kc_127);
    }

    // 4. All TOGGLE keycodes (values 0x7E30 to 0x7E3F)
    for (int i = 0; i < 16; i++) {
        cJSON *kc_tog = cJSON_CreateObject();
        char *name_tog = malloc(32);
        char *title_tog = malloc(32);
        char *short_tog = malloc(16);
        sprintf(name_tog, "MIDI_CC_%d_TOGGLE", i);
        sprintf(title_tog, "MIDI CC %d Toggle", i);
        sprintf(short_tog, "CC%dT", i);
        cJSON_AddStringToObject(kc_tog, "name", name_tog);
        cJSON_AddStringToObject(kc_tog, "title", title_tog);
        cJSON_AddStringToObject(kc_tog, "shortName", short_tog);
        cJSON_AddNumberToObject(kc_tog, "value", 0x7E30 + i);
        cJSON_AddItemToArray(custom_keycodes, kc_tog);
    }
#endif

    cJSON *matrix = cJSON_AddObjectToObject(root, "matrix");
    cJSON_AddNumberToObject(matrix, "rows", NUM_ROWS);
    cJSON_AddNumberToObject(matrix, "cols", NUM_COLS);

    cJSON *layouts = cJSON_AddObjectToObject(root, "layouts");
    cJSON *keymap_arr = cJSON_AddArrayToObject(layouts, "keymap");

    struct layout_key layout_keys[] = LAYOUT;
    int num_keys = sizeof(layout_keys) / sizeof(layout_keys[0]);

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
    int num_encoders = count_encoders();
    if (num_encoders > 0) {
        // Add features.encoder = true
        cJSON *features = cJSON_AddObjectToObject(root, "features");
        cJSON_AddBoolToObject(features, "encoder", cJSON_True);

        // Encoder row first: "idx,dir\n\n\n\n\ne" for each encoder CW/CCW
        cJSON *enc_row = cJSON_CreateArray();
        cJSON_AddItemToArray(keymap_arr, enc_row);
        for (int e = 0; e < num_encoders; e++) {
            char cw_str[32];
            sprintf(cw_str, "%d,0\n\n\n\n\ne", e);
            cJSON_AddItemToArray(enc_row, cJSON_CreateString(cw_str));
            char ccw_str[32];
            sprintf(ccw_str, "%d,1\n\n\n\n\ne", e);
            cJSON_AddItemToArray(enc_row, cJSON_CreateString(ccw_str));
        }
    }
#endif

    // Key rows: one array per physical row
    for (int r = 0; r < NUM_ROWS; r++) {
        cJSON *row = cJSON_CreateArray();
        cJSON_AddItemToArray(keymap_arr, row);
        for (int i = 0; i < num_keys; i++) {
            if (layout_keys[i].r == r) {
                char key_str[32];
                sprintf(key_str, "%d,%d", layout_keys[i].r, layout_keys[i].c);
                cJSON_AddItemToArray(row, cJSON_CreateString(key_str));
            }
        }
    }

    char *json_str = cJSON_Print(root);
    FILE *f = fopen(argv[1], "w");
    if (!f) {
        perror("Failed to open output file");
        cJSON_Delete(root);
        free(json_str);
        return 1;
    }
    fprintf(f, "%s\n", json_str);
    fclose(f);

    cJSON_Delete(root);
    free(json_str);
    return 0;
}
