#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"
#include <stddef.h>
#include <stdint.h>

/* --- Custom Matrix Scanning Configuration --- */
#define CUSTOM_MATRIX 1

/* --- Multiplexer & Sensor Pinout --- */
#if defined(MCU_milandr)
#define MUX_PIN_S0 PA0
#define MUX_PIN_S1 PA1
#define MUX_PIN_S2 PA2
#define MUX_PIN_S3 PA3
#define MUX_PIN_ENABLE PA4
#define HALL_SIGNAL_PIN PA5
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define MUX_PIN_S0 GPIO2
#define MUX_PIN_S1 GPIO3
#define MUX_PIN_S2 GPIO4
#define MUX_PIN_S3 GPIO5
#define MUX_PIN_ENABLE GPIO6
#define HALL_SIGNAL_PIN GPIO26
#elif defined(MCU_nrf52840)
#define MUX_PIN_S0 P0_02
#define MUX_PIN_S1 P0_03
#define MUX_PIN_S2 P0_04
#define MUX_PIN_S3 P0_05
#define MUX_PIN_ENABLE P0_06
#define HALL_SIGNAL_PIN P0_28
#elif defined(MCU_baikal)
#define MUX_PIN_S0 GPIO0
#define MUX_PIN_S1 GPIO1
#define MUX_PIN_S2 GPIO2
#define MUX_PIN_S3 GPIO3
#define MUX_PIN_ENABLE GPIO4
#define HALL_SIGNAL_PIN GPIO5
#endif

#define MUX_SETTLE_US 2
#define HALL_ACTUATION_THRESHOLD 2500
#define HALL_RELEASE_THRESHOLD 2200

/* --- Keymap Matrix Geometry (4x4 Numpad / Macro Pad) --- */
#define NUM_ROWS 4
#define NUM_COLS 4
#define NUM_KEYS 16

// Physical coordinate layout
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

// Macros
const Macro keyboard_macros[] = {};
const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
#endif

#endif // CONFIG_H
