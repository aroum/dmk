#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"
#define MATRIX_TYPE DIRECT

/* --- Matrix configuration --- */
#if defined(MCU_rp2040)
// Direct pins for each half (5 keys)
#define DIRECT_PINS {GPIO9, GPIO8, GPIO6, GPIO5, GPIO10}
#define LED_PINS {GPIO15}
#endif

// #define VIAL
#define VIAL_KEYBOARD_NAME "Nizkoteno Split DMK"
#define VIAL_VENDOR_ID 0xCafe
#define VIAL_PRODUCT_ID 0x4011
#define LED_DEBUG 0

/* --- Split Keyboard Settings --- */
#if defined(LEFT)
#define MASTER_SIDE LEFT
#elif defined(RIGHT)
#define MASTER_SIDE RIGHT
#else
#define MASTER_SIDE AUTO
#endif
#define SERIAL_PIN GPIO29

// Split communication option:
// 1 = Option 1 (external pull-up resistor required, 115200 baud)
// 2 = Option 2 (uses internal weak pull-up, 19200 baud)
#ifndef SPLIT_HARDWARE_OPTION
#define SPLIT_HARDWARE_OPTION 2
#endif

#define NUM_ROWS_SPLIT 1
#define NUM_COLS_SPLIT 5
#define SPLIT_COL_OFFSET 5

/* --- RGB Settings --- */
#define RGB_NUM 5 // 5 LEDs on each half
#if defined(MCU_rp2040)
#define RGB_PIN GPIO11
#endif
#define RGB_MAP {0, 1, 2, 3, 4}

#define NUM_ROWS 1
#define NUM_COLS 10
#define NUM_KEYS 10

// clang-format off
#define LAYOUT                                                                 \
  {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4},                                     \
   {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}}
// clang-format on
#define LAYOUT_DEFAULT LAYOUT

#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1

// Flat keymap: one entry per key in LAYOUT order
// clang-format off
const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = {RGB_TOGG, RGB_NEXT, K_D, K_F, K_R, K_W, K_E, K_C, K_L, K_A},
    [FN1] = {K_ESC, K_APOS, K_A_U, K_BRAL, K_BRAR, K_SHFT, K_BKSL, K_SCLN, K_0,
             K_TRNS}};
// clang-format on

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

// clang-format off
const Macro keyboard_macros[] = {};
const uint8_t keyboard_macros_count =
    sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
// clang-format on
#endif

#endif // CONFIG_H
