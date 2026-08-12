#ifndef CONFIG_H
#define CONFIG_H

// #define VIAL // Enable Vial/VIA support

#include "pin_defs.h"

/* --- Split Settings --- */
#define MASTER_SIDE LEFT // Specify side (LEFT, RIGHT, or AUTO)
#if defined(MCU_milandr)
#define SERIAL_PIN PA0
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define SERIAL_PIN GPIO1 // D2 (ProMicro) -> GPIO1 (RP2040)
#elif defined(MCU_nrf52840)
#define SERIAL_PIN P0_00
#elif defined(MCU_baikal)
#define SERIAL_PIN GPIO0
#endif

/* --- Matrix Settings --- */
// Connection type: COL2ROW
#define MATRIX_TYPE COL2ROW

// Pins for rows and columns (RP2040 pins correspond to ProMicro pinout)
#if defined(MCU_milandr)
#define ROW_PINS {PB0, PB1, PB2, PB3}
#define COL_PINS {PA0, PA1, PA2, PA3, PA4, PA5}
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define ROW_PINS {GPIO4, GPIO5, GPIO6, GPIO7}                         // D4, C6, D7, E6 -> GPIO4, GPIO5, GPIO6, GPIO7
#define COL_PINS {GPIO29, GPIO28, GPIO27, GPIO26, GPIO22, GPIO20}      // F4, F5, F6, F7, B1, B3 -> GPIO29, GPIO28, GPIO27, GPIO26, GPIO22, GPIO20
#elif defined(MCU_nrf52840)
#define ROW_PINS {P0_00, P0_01, P0_02, P0_03}
#define COL_PINS {P0_04, P0_05, P0_06, P0_07, P0_08, P0_09}
#elif defined(MCU_baikal)
#define ROW_PINS {GPIO0, GPIO1, GPIO2, GPIO3}
#define COL_PINS {GPIO4, GPIO5, GPIO6, GPIO7, GPIO8, GPIO9}
#endif

/* --- RGB Backlight --- */
#if defined(MCU_milandr)
#define RGB_PIN PC0
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define RGB_PIN GPIO0 // D3 (ProMicro) -> GPIO0 (RP2040)
#elif defined(MCU_nrf52840)
#define RGB_PIN P0_13
#elif defined(MCU_baikal)
#define RGB_PIN GPIO25
#endif

/* --- Keymap Matrix --- */
#define NUM_ROWS 4
#define NUM_COLS 12
#define NUM_ROWS_SPLIT 4
#define NUM_COLS_SPLIT 6
#define NUM_KEYS 42

// {ROW, COL} coordinate mapping (6 cols per side + 3 thumbs per side, 4 rows)
// clang-format off
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {0, 11}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {1, 11}, \
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, {2, 10}, {2, 11}, \
                            {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8} \
}
#define LAYOUT_DEFAULT LAYOUT
// clang-format on

#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1

#if __has_include("keymap_external.h")
#include "keymap_external.h"
#else
// Flat keymap: one entry per key in LAYOUT order
// clang-format off
const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = {
        K_TAB, K_Q,   K_W,   K_E,   K_R,   K_T,     K_Y,   K_U,   K_I,   K_O,   K_P,   K_BSPC, \
        K_LCTL,K_A,   K_S,   K_D,   K_F,   K_G,     K_H,   K_J,   K_K,   K_L,   K_SCLN,K_QUOT, \
        K_LSFT,K_Z,   K_X,   K_C,   K_V,   K_B,     K_N,   K_M,   K_COMM,K_DOT, K_SLSH,K_ESC, \
                             K_LALT,FN1,   K_SPC,   K_ENT, FN1,   K_RGUI
    },
    [FN1] = {
        K_ESC, K_1,   K_2,   K_3,   K_4,   K_5,     K_6,   K_7,   K_8,   K_9,   K_0,   K_DEL, \
        K_LCTL,K_TRNS,K_TRNS,K_TRNS,K_TRNS,K_TRNS,  K_LEFT,K_DOWN,K_UP,  K_RIGHT,K_TRNS,K_TRNS, \
        K_LSFT,K_TRNS,K_TRNS,K_TRNS,K_TRNS,K_TRNS,  K_TRNS,K_TRNS,K_TRNS,K_TRNS,K_TRNS,K_TRNS, \
                             K_LALT,K_TRNS,K_SPC,   K_ENT, K_TRNS,K_RGUI
    }
};
// clang-format on

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

// Macros
const Macro keyboard_macros[] = {};
const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
#endif
#endif

#endif // CONFIG_H
