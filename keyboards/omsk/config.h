#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"

#define VIAL
#define VIAL_KEYBOARD_NAME "Omsk MIDI"
#define VIAL_VENDOR_ID 0xCAFE
#define VIAL_PRODUCT_ID 0x4006

/* --- Matrix configuration --- */
#define MATRIX_TYPE_ROW2COL

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#define ROW_PINS {GPIO14, GPIO13, GPIO12, GPIO11}
#define COL_PINS {GPIO10, GPIO9, GPIO7, GPIO6}
#endif

#define NUM_ROWS 4
#define NUM_COLS 4
#define NUM_KEYS 16

/* --- RGB Backlight --- */
#define RGB_NUM 22
#if defined(MCU_rp2040) || defined(MCU_rp2350)
#define RGB_PIN GPIO8
#endif

// clang-format off
#define RGB_MAP { \
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, \
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, \
    20, 21 \
}

#define RGB_THEME_DEFAULT { \
    0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, \
    0x00FFFF, 0xFFFFFF, 0xFF8000, 0x8000FF, 0x00FF80, \
    0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, \
    0x00FFFF, 0xFFFFFF, 0xFF8000, 0x8000FF, 0x00FF80, \
    0xFF0000, 0x00FF00 \
}
#define RGB_THEMES { RGB_THEME_DEFAULT }
// clang-format on

/* --- Rotary Encoders --- */
#if defined(MCU_rp2040) || defined(MCU_rp2350)
#define ENCODER_PINS_A {GPIO2, GPIO4, GPIO15, GPIO27}
#define ENCODER_PINS_B {GPIO3, GPIO5, GPIO26, GPIO28}
#endif
#define ENCODER_RESOLUTION 2

/* --- Keymap --- */
// clang-format off
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, \
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, \
    {3, 0}, {3, 1}, {3, 2}, {3, 3} \
}
#define LAYOUT_DEFAULT LAYOUT

#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1

const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = {
        K_Q, K_W, K_E, K_R,
        K_A, K_S, K_D, K_F,
        K_Z, K_X, K_C, K_V,
        K_1, K_2, K_3, FN1
    },
    [FN1] = {
        K_TRNS, K_TRNS, K_TRNS, K_TRNS,
        K_TRNS, K_TRNS, K_TRNS, K_TRNS,
        K_TRNS, K_TRNS, K_TRNS, K_TRNS,
        K_TRNS, K_TRNS, K_TRNS, K_TRNS
    }
};

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

const uint32_t encoder_keymap[][4][2] = {
    [DEF] = {
        { K_VOLU, K_VOLD }, // Enc 1
        { K_MNXT, K_MPRV }, // Enc 2
        { K_PGUP, K_PGDN }, // Enc 3
        { K_LEFT, K_RIGHT } // Enc 4
    },
    [FN1] = {
        { K_TRNS, K_TRNS },
        { K_TRNS, K_TRNS },
        { K_TRNS, K_TRNS },
        { K_TRNS, K_TRNS }
    }
};

const Macro keyboard_macros[] = {};
const uint8_t keyboard_macros_count = 0;
// clang-format on
#endif

#endif // CONFIG_H
