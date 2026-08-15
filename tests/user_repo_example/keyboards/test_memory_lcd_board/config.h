#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"
#include <stddef.h>
#include <stdint.h>

/* --- Matrix Configuration --- */
#define MATRIX_TYPE COL2ROW

#if defined(MCU_milandr)
#define ROW_PINS {PB0, PB1, PB2, PB3}
#define COL_PINS {PA0, PA1, PA2, PA3}
#define SHARP_LCD_PIN_CS PB4
#define SHARP_LCD_PIN_SCK PB5
#define SHARP_LCD_PIN_MOSI PB6
#define SHARP_LCD_PIN_DISP PB7
#define SHARP_LCD_PIN_EXTCOMIN PB8
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define ROW_PINS {GPIO4, GPIO5, GPIO6, GPIO7}
#define COL_PINS {GPIO26, GPIO27, GPIO28, GPIO29}
#define SHARP_LCD_PIN_CS GPIO10
#define SHARP_LCD_PIN_SCK GPIO11
#define SHARP_LCD_PIN_MOSI GPIO12
#define SHARP_LCD_PIN_DISP GPIO13
#define SHARP_LCD_PIN_EXTCOMIN GPIO14
#elif defined(MCU_nrf52840)
#define ROW_PINS {P0_00, P0_01, P0_02, P0_03}
#define COL_PINS {P0_04, P0_05, P0_06, P0_07}
#define SHARP_LCD_PIN_CS P0_10
#define SHARP_LCD_PIN_SCK P0_11
#define SHARP_LCD_PIN_MOSI P0_12
#define SHARP_LCD_PIN_DISP P0_13
#define SHARP_LCD_PIN_EXTCOMIN P0_14
#elif defined(MCU_baikal)
#define ROW_PINS {GPIO0, GPIO1, GPIO2, GPIO3}
#define COL_PINS {GPIO4, GPIO5, GPIO6, GPIO7}
#define SHARP_LCD_PIN_CS GPIO10
#define SHARP_LCD_PIN_SCK GPIO11
#define SHARP_LCD_PIN_MOSI GPIO12
#define SHARP_LCD_PIN_DISP GPIO13
#define SHARP_LCD_PIN_EXTCOMIN GPIO14
#endif

/* --- Keymap Geometry --- */
#define NUM_ROWS 4
#define NUM_COLS 4
#define NUM_KEYS 16

#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, \
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, \
    {3, 0}, {3, 1}, {3, 2}, {3, 3}  \
}
#define LAYOUT_DEFAULT LAYOUT

#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1

const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = {
        K_7,    K_8,    K_9,    K_PSLS,
        K_4,    K_5,    K_6,    K_PAST,
        K_1,    K_2,    K_3,    K_PMNS,
        K_0,    K_DOT,  K_ENT,  K_PPLS
    },
    [FN1] = {
        K_HOME, K_UP,   K_PGUP, K_MUTE,
        K_LEFT, K_DOWN, K_RGHT, K_VOLU,
        K_END,  K_DOWN, K_PGDN, K_VOLD,
        K_INS,  K_DEL,  K_TRNS, K_NO
    }
};

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

const Macro keyboard_macros[] = {};
const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
#endif

#endif // CONFIG_H
