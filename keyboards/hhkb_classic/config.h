#ifndef CONFIG_H
#define CONFIG_H

#define VIAL_KEYBOARD_NAME "HHKB Professional Classic"
#define VIA_V3 1
#ifdef HHKB_RAPID_TRIGGER
#define VIA_FIRMWARE_VERSION 2U
#else
#define VIA_FIRMWARE_VERSION 1U
#endif
#define DYNAMIC_KEYMAP_MAX_LAYERS 4

/*
 * VIAL & VIA GUI Support:
 * Set to 1 (or leave #define VIAL) to enable runtime GUI keymap remap and custom Topre tuning menus.
 * To completely disable Vial/VIA and save flash/RAM, comment out '#define VIAL' below:
 * // #define VIAL
 */
#ifndef VIAL
#define VIAL 1
#endif
#define DEBUG_CONSOLE_ENABLE 1
#define DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE 224

#include "pin_defs.h"

#define DEFAULT_MCU stm32l072
#define TAPPING_TERM_DEFAULT 220
#define PERMISSIVE_HOLD 1
#define HOLD_TAP_TAP_PREFERRED 0
#define QUICK_TAP_TERM 150
#define HHKB_SCAN_1KHZ 1

/* --- Matrix Geometry --- */
#define MATRIX_ROWS 4
#define MATRIX_COLS 15
#define MATRIX_REPORT_BATCH
#define QUEUE_DEF_SIZE 48
#define KEYBOARD_REPORT_QUEUE_LENGTH 16
// Keep numeric for the VIA JSON generator, which reads these defines directly.
#define NUM_ROWS 4
#define NUM_COLS 15
#define NUM_KEYS 60
#if MATRIX_ROWS != NUM_ROWS || MATRIX_COLS != NUM_COLS
#error "HHKB matrix dimensions must match VIA dimensions"
#endif

// {ROW, COL} coordinate mapping from physical key position to Topre matrix (Duncaen / QMK)
// clang-format off
#define LAYOUT { \
    {1, 0}, {1, 3}, {1, 1}, {1, 2}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 11}, {1, 8}, {1, 9}, {1, 10}, {1, 12}, {1, 14}, {1, 13}, \
    {2, 0}, {2, 3}, {2, 1}, {2, 2}, {2, 4}, {2, 5}, {2, 7}, {2, 11}, {2, 8}, {2, 9}, {2, 10}, {2, 12}, {2, 14}, {2, 13}, \
    {3, 0}, {3, 3}, {3, 1}, {3, 2}, {0, 5}, {2, 6}, {3, 7}, {3, 11}, {3, 8}, {3, 9}, {3, 10}, {3, 12}, {3, 14}, \
    {0, 0}, {0, 1}, {0, 2}, {3, 4}, {3, 5}, {3, 6}, {0, 7}, {0, 11}, {0, 8}, {0, 9}, {0, 12}, {0, 14}, {3, 13}, \
            {0, 3}, {0, 4},                 {0, 6},                   {0, 10}, {0, 13} \
}
#define LAYOUT_DEFAULT LAYOUT
// clang-format on

/* --- Matrix Connection Mode --- */
// Custom Topre electrostatic capacitive scanning driver
#define CUSTOM_MATRIX 1
#define MATRIX_TYPE CUSTOM

/* --- Row Drive Pins --- */
// PB0, PB1, PB2, PB10
#define HHKB_ROW_PINS { B0, B1, B2, B10 }

/* --- Column Select Multiplexer Pins --- */
// Dual 4051 Demux: U1 (cols 0-7, PC6) and U2 (cols 8-14, PB15)
#define HHKB_U1_PIN C6
#define HHKB_U2_PIN B15
#define HHKB_MUX_ADDR_PINS { B12, B13, B14 }

/* --- Analog & Strobe Control Pins --- */
#define HHKB_POWER_PIN     C14 // Power switch for sensor board
#define HHKB_OPAMP_EN_PIN  C7  // OpAmp enable
#define HHKB_DISCHARGE_PIN C8  // Capacitor discharge switch
#define HHKB_ADC_CHANNEL   1   // ADC1_IN1 on PA1

/* --- DIP Switches (Active Low) --- */
#define HHKB_DIP_PINS { A3, A4, A5, A6, A7, C4 }

/*
 * SW6 Operation Mode:
 * Option 1 (Emergency Recovery): Set HHKB_SW6_REBOOT_TO_BANK1 1
 *   - SW6 ON: EMERGENCY ONLY! Triggers failsafe recovery into Bank 1 by:
 *             1. Setting EEPROM status byte 0x08080348 = 0x01 (Bank 2 broken).
 *             2. Zeroing EEPROM CRC tag at 0x08080370 (invalidates Bank 2 checksum).
 *             3. Erasing the first Flash page (128 B) of Bank 2 at 0x08010000
 *                from a RAM-resident function with interrupts disabled,
 *                destroying SP and Reset vectors so Bank 1 bootloader
 *                cannot jump into Bank 2 regardless of its own validation logic.
 *             4. System reset via NVIC_SystemReset().
 *             The stock Bank 1 bootloader then stays in firmware update mode
 *             until Bank 2 is reflashed (PFU Keymap Tool or hhkb_flasher.py).
 *             NOTE: In normal operation, SW6 MUST ALWAYS BE OFF!
 *   - SW6 OFF: Normal operation; DMK boots in Bank 2.
 *
 * Option 2 (Stock Power Saving): Comment out or set HHKB_SW6_REBOOT_TO_BANK1 0
 *   - SW6 OFF: Power Saving Enabled (no key wakeup from USB suspend).
 *   - SW6 ON:  Power Saving Disabled (immediate key wakeup).
 */
#define HHKB_SW6_REBOOT_TO_BANK1 1

/* --- Status & Indicator LEDs --- */
#define LED_PINS { A8, A9 }
// HHKB module owns both LED outputs.

#ifdef DEFINE_KEYMAP
#include "keys.h"

// Layer identifiers
enum layers {
    BASE = 0,
    HHKB = 1,
    BASE_2 = 2,
    HHKB_2 = 3
};

#define ___ K_TRNS

// clang-format off
/**
 * 60-key HHKB Classic Layout:
 * Row 0 (15): Esc, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, =, \, `
 * Row 1 (14): Tab, Q, W, E, R, T, Y, U, I, O, P, [, ], Backspace
 * Row 2 (13): LCtrl, A, S, D, F, G, H, J, K, L, ;, ', Enter
 * Row 3 (13): LShift, Z, X, C, V, B, N, M, ,, ., /, RShift, MO(HHKB)
 * Row 4 (5):  LAlt, LGui, Space, RGui, RAlt
 */
const uint32_t keymap[][NUM_KEYS] = {
    // -------------------------------------------------------------------------
    // LAYER 0: BASE (Default HHKB Layout)
    // -------------------------------------------------------------------------
    [BASE] = {
        K_ESC,  K_1,    K_2,    K_3,    K_4,    K_5,    K_6,    K_7,    K_8,    K_9,    K_0,    K_MINS, K_EQL,  K_BKSL, K_GRAV,
        K_TAB,  K_Q,    K_W,    K_E,    K_R,    K_T,    K_Y,    K_U,    K_I,    K_O,    K_P,    K_LBRC, K_RBRC, K_BSPC,
        K_LCTL, K_A,    K_S,    K_D,    K_F,    K_G,    K_H,    K_J,    K_K,    K_L,    K_SCLN, K_QUOT, K_ENT,
        K_LSFT, K_Z,    K_X,    K_C,    K_V,    K_B,    K_N,    K_M,    K_COMM, K_DOT,  K_SLSH, K_RSFT, MO(HHKB),
                K_LALT, K_LGUI,                 K_SPC,                  K_RGUI, K_RALT
    },

    // -------------------------------------------------------------------------
    // LAYER 1: HHKB (Fn Layer)
    // -------------------------------------------------------------------------
    [HHKB] = {
        K_PWR,  K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_F11,  K_F12,  K_INS,  K_DEL,
        K_CAPS, ___,    ___,    ___,    ___,    ___,    ___,    ___,    K_PSCR, K_SLCK, K_PAUS, K_UP,   ___,    K_BSPC,
        ___,    K_VOLD, K_VOLU, K_MUTE, ___,    ___,    K_PAST, K_PSLS, K_HOME, K_PGUP, K_LEFT, K_RGHT, K_PENT,
        ___,    ___,    ___,    ___,    ___,    ___,    K_PPLS, K_PMNS, K_END,  K_PGDN, K_DOWN, ___,    ___,
                ___,    ___,                    ___,                    ___,    ___
    },
    [BASE_2] = {
        K_ESC,  K_1,    K_2,    K_3,    K_4,    K_5,    K_6,    K_7,    K_8,    K_9,    K_0,    K_MINS, K_EQL,  K_BKSL, K_GRAV,
        K_TAB,  K_Q,    K_W,    K_E,    K_R,    K_T,    K_Y,    K_U,    K_I,    K_O,    K_P,    K_LBRC, K_RBRC, K_BSPC,
        K_LCTL, K_A,    K_S,    K_D,    K_F,    K_G,    K_H,    K_J,    K_K,    K_L,    K_SCLN, K_QUOT, K_ENT,
        K_LSFT, K_Z,    K_X,    K_C,    K_V,    K_B,    K_N,    K_M,    K_COMM, K_DOT,  K_SLSH, K_RSFT, MO(HHKB_2),
                K_LALT, K_LGUI,                 K_SPC,                  K_RGUI, K_RALT
    },

    // -------------------------------------------------------------------------
    // LAYER 1: HHKB (Fn Layer)
    // -------------------------------------------------------------------------
    [HHKB_2] = {
        K_PWR,  K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_F11,  K_F12,  K_INS,  K_DEL,
        K_CAPS, ___,    ___,    ___,    ___,    ___,    ___,    ___,    K_PSCR, K_SLCK, K_PAUS, K_UP,   ___,    K_BSPC,
        ___,    K_VOLD, K_VOLU, K_MUTE, ___,    ___,    K_PAST, K_PSLS, K_HOME, K_PGUP, K_LEFT, K_RGHT, K_PENT,
        ___,    ___,    ___,    ___,    ___,    ___,    K_PPLS, K_PMNS, K_END,  K_PGDN, K_DOWN, ___,    ___,
                ___,    ___,                    ___,                    ___,    ___
    }
};
// clang-format on

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);
#endif // DEFINE_KEYMAP

#endif // CONFIG_H
