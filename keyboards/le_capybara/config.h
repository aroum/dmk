#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"

#define DEFAULT_MCU stm32f072

/* --- Keyboard Identity & Features --- */
#ifndef VIAL
#define VIAL
#endif

// Uncomment to switch from default Vial (Protocol 9) to VIA v3 (Protocol 12) for usevia.app
// #define VIA_V3 1

#define VIAL_KEYBOARD_NAME "Le Capybara"
#define VIAL_VENDOR_ID 0x0069
#define VIAL_PRODUCT_ID 0x0420
#define VIA_FULL_SETTINGS_RESET 1

#define DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE 224
#define DYNAMIC_KEYMAP_MAX_LAYERS 4

/* Matrix Dimensions: 4 rows EC + 1 row discrete (KNOB_SW) */
#define MATRIX_ROWS 5
#define MATRIX_COLS 10
#define NUM_ROWS 5
#define NUM_COLS 10
#define NUM_KEYS 37
#define QUEUE_DEF_SIZE 48
#define MATRIX_REPORT_BATCH
#define DEBUG_CONSOLE_ENABLE
#define CONSOLE_BUF_SIZE 128
#define VIA_LAYOUT_OPTIONS_MASK 0x03u

#define EC_ROWS 4
#define EC_COLS 10

/* --- EC Matrix Configuration --- */
#ifndef CUSTOM_MATRIX
#define CUSTOM_MATRIX 1
#endif

#define TOPRE_SCAN_1KHZ 1

/* Pins for EC Matrix */
// Row strobe pins (PA3, PA5, PA7, PB1)
#define CAPYBARA_ROW0 PA3
#define CAPYBARA_ROW1 PA5
#define CAPYBARA_ROW2 PA7
#define CAPYBARA_ROW3 PB1

// Analog Multiplexers (74HC4051): S0=PB15, S1=PB14, S2=PB13
#define CAPYBARA_MUX_S0 PB15
#define CAPYBARA_MUX_S1 PB14
#define CAPYBARA_MUX_S2 PB13

// MUX Enablers (active LOW): EN0=PA8, EN1=PC14
#define CAPYBARA_MUX_EN0 PA8
#define CAPYBARA_MUX_EN1 PC14

// OpAmp / ADC & Discharge
#define CAPYBARA_DISCHARGE_PIN PA1 // Drive HIGH to sense, drive LOW to discharge
#define CAPYBARA_ADC_PIN PA2       // ADC1 Channel 2

// Discrete switch (Rotary encoder button)
#define CAPYBARA_KNOB_SW PB3       // Row 4, Col 0, Active LOW with pull-up

/* --- Rotary Encoder --- */
#define ENCODER_PINS_A { PB4 }
#define ENCODER_PINS_B { PB5 }
#define ENCODER_RESOLUTION 2

/* --- RGB: WS2812, 11 LEDs --- */
#define RGB_PIN PA15
#define RGB_NUM 11
#define RGB_LIMIT_VAL 60
#ifndef STM32F072_WS2812_PA15_DMA
// Match the reference Le Capybara firmware's GPIO transmitter.
#define STM32F072_WS2812_PA15_DMA 0
#endif

/* --- Layout Mapping (37 unique positions matching vial.json) --- */
// clang-format off
#define LAYOUT { \
    {0, 0}, {0, 1}, {4, 0}, {0, 2}, {0, 3}, \
    {0, 4}, {1, 2}, {1, 1}, {1, 3}, {1, 4}, \
    {1, 0}, {2, 2}, {2, 1}, {2, 3}, {2, 4}, \
    {2, 0}, {3, 1}, {3, 3}, {3, 2}, {0, 7}, \
    {0, 5}, {0, 6}, {0, 8}, {0, 9}, {1, 7}, \
    {1, 5}, {1, 6}, {1, 8}, {1, 9}, {2, 7}, \
    {2, 5}, {2, 6}, {2, 8}, {2, 9}, {3, 6}, \
    {3, 8}, {3, 7} \
}
#define LAYOUT_DEFAULT LAYOUT

#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1
#define FN2 2

const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = {
        K_Q,    K_W,    K_MUTE, K_E,    K_R, \
        K_T,    K_D,    K_S,    K_F,    K_G, \
        K_A,    K_C,    K_X,    K_V,    K_B, \
        K_Z,    K_ESC,  K_SPC,  K_ENT,  K_U, \
        K_Y,    K_I,    K_O,    K_P,    K_J, \
        K_H,    K_K,    K_L,    K_QUOT, K_M, \
        K_N,    K_COMM, K_DOT,  K_SLSH, K_BSPC, \
        K_TAB,  K_SPC \
    },
    [FN1] = {
        K_TRNS, K_TRNS, K_BOOT, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS, K_TRNS, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS, K_TRNS, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS, K_TRNS, K_TRNS, K_8, \
        K_7,    K_9,    K_0,    K_EQL,  K_5, \
        K_4,    K_6,    K_MINS, K_TRNS, K_2, \
        K_1,    K_3,    K_BSLS, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS \
    },
    [FN2] = {
        K_TRNS, K_TRNS, K_BOOT, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS, K_TRNS, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS, K_TRNS, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS, K_TRNS, K_TRNS, K_TRNS, \
        K_TRNS, K_BSPC, K_TRNS, K_TRNS, K_DOWN, \
        K_LEFT, K_UP,   K_RGHT, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS, K_TRNS, K_TRNS, K_TRNS, \
        K_TRNS, K_TRNS \
    }
};

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

const uint32_t encoder_keymap[][1][2] = {
    [DEF] = { { K_VOLD, K_VOLU } },
    [FN1] = { { K_TRNS, K_TRNS } },
    [FN2] = { { K_TRNS, K_TRNS } }
};
#endif
// clang-format on

#endif // CONFIG_H
