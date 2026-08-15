#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"

/* --- Keyboard Identity & Features --- */
#define VIAL
#define VIAL_KEYBOARD_NAME "Magneteno"
#define VIAL_VENDOR_ID 0xCafe
#define VIAL_PRODUCT_ID 0x4012

/* --- Custom Hall-Effect Matrix & SN74LV4052A Multiplexer --- */
#define CUSTOM_MATRIX 1

// SN74LV4052A Multiplexer Control Pins
#if defined(MCU_rp2040) || defined(MCU_rp2350)
#define MUX_PIN_S0 GPIO15
#define MUX_PIN_S1 GPIO14
#define MUX_PIN_INH GPIO13

// Analog ADC Inputs (RP2040/RP2350 ADC0=GPIO26, ADC1=GPIO27, ADC2=GPIO28, ADC3=GPIO29)
#define MUX_ADC_PIN GPIO29     // Output of MUX connected to ADC3
#define DIRECT_KEY9_PIN GPIO28 // Key 9 direct Hall ADC2
#define DIRECT_KEY10_PIN GPIO27 // Key 10 direct Hall ADC1

/* --- RGB Settings --- */
#define RGB_NUM 10
#define RGB_PIN GPIO12

/* --- Sharp Memory LCD (LS011B7DH03 160x68) Settings --- */
#define SHARP_LCD_PIN_MOSI GPIO11 // TX
#define SHARP_LCD_PIN_SCK GPIO10  // SCK
#define SHARP_LCD_PIN_CS GPIO9    // CSN
#define SHARP_LCD_PIN_DISP GPIO8  // DISP (optional)
#define SHARP_LCD_PIN_EXTCOMIN GPIO7 // EXTCOMIN (optional)

#elif defined(MCU_milandr)
#define MUX_PIN_S0 PA0
#define MUX_PIN_S1 PA1
#define MUX_PIN_INH PA2
#define MUX_ADC_PIN PA3
#define DIRECT_KEY9_PIN PA4
#define DIRECT_KEY10_PIN PA5
#define RGB_NUM 10
#define RGB_PIN PC0
#define SHARP_LCD_PIN_MOSI PB6
#define SHARP_LCD_PIN_SCK PB5
#define SHARP_LCD_PIN_CS PB4
#define SHARP_LCD_PIN_DISP PB7
#define SHARP_LCD_PIN_EXTCOMIN PB8
#endif

// Multiplexer channel mapping for keys 1..8 (hardware channels 2, 4, 3, 1, 7, 6, 8, 5; 0-indexed: 1, 3, 2, 0, 6, 5, 7, 4)
#define MUX_CHANNEL_MAP { 1, 3, 2, 0, 6, 5, 7, 4 }

// Default Hall ADC trigger thresholds (12-bit ADC 0..4095)
#define HALL_ACTUATION_THRESHOLD 2600
#define HALL_RELEASE_THRESHOLD 2300
#define MUX_SETTLE_US 3

// clang-format off
#define RGB_MAP { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
#define RGB_THEME_DEFAULT { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x888888, 0x444444, 0x222222 }
#define RGB_THEME_GAMING  { 0xFF4500, 0xFF8C00, 0xFFD700, 0xADFF2F, 0x00FF7F, 0x00CED1, 0x1E90FF, 0x9370DB, 0xFF1493, 0xFF0000 }
#define RGB_THEMES { RGB_THEME_DEFAULT, RGB_THEME_GAMING }
// clang-format on

/* --- Keymap Matrix Geometry (from Nizkoteno) --- */
#define NUM_ROWS 1
#define NUM_COLS 10
#define NUM_KEYS 10

// {ROW, COL} coordinate mapping
// clang-format off
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, \
    {0, 4}, {0, 5}, {0, 6}, {0, 7}, \
                            {0, 8}, {0, 9} \
}
#define LAYOUT_DEFAULT LAYOUT

#define LAYOUT_EDITOR { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, \
                            {2, 3}, {2, 4} \
}
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
        RGB_TOGG, RGB_NEXT, K_D,    K_F, \
        K_R,      K_W,      K_E,    K_C, \
                                    K_L,      K_A \
    },
    [FN1] = {
        K_ESC,    K_APOS,   K_A_U,  K_BRAL, \
        K_BRAR,   K_SHFT,   K_BKSL, K_SCLN, \
                                    K_0,      K_TRNS \
    }
};

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

// Macros
const Macro keyboard_macros[] = {};
const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
// clang-format on
#endif
#endif

#endif // CONFIG_H
