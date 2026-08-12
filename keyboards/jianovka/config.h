#ifndef CONFIG_H
#define CONFIG_H

// #define VIAL // Enable Vial/VIA support if needed

#include "pin_defs.h"

/* --- Matrix Settings --- */
// Connection type: MATRIX_MH3SS2
#define MATRIX_MH3SS2
#define MH3SS2_ROW_PINS_PER_KEY 1

// PB10 - питание линейки датчиков, B9, PB8, A7 - выход с датчиков

// Pins for columns (14 cols) and rows (6 pins, paired 1a/1b, 2a/2b, 3a/3b)
#if defined(MCU_milandr)
#define COL_PINS {PA0, PB10, PA1, PA2, PA3, PA4, PA5, PA6, PB2, PA8, PA9, PB3, PB1, PB0}
#define ROW_PINS {PB9, PB8, PA7, PB4, PB5, PB6}
#define LED_ACTIVITY_PIN PB7
#else
#define COL_PINS {GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7, GPIO8, GPIO9, GPIO10, GPIO11, GPIO12, GPIO13}
#define ROW_PINS {GPIO14, GPIO15, GPIO16, GPIO17, GPIO18, GPIO19}
// #define LED_ACTIVITY_PIN GPIO25
#endif

/* --- Keymap Matrix --- */
#define NUM_ROWS 3
#define NUM_COLS 14
#define NUM_KEYS 42

// {ROW, COL} coordinate mapping (physically 6 cols per side + 3 thumb keys per side, 3 rows)
// clang-format off
#define LAYOUT { \
    {0, 0},  {0, 1},  {0, 2},  {0, 3}, {0, 4}, {0, 5}, {0, 7},  {0, 8},  {0, 9}, {0, 10}, {0, 11}, \
    {0, 12}, {1, 0},  {1, 1},  {1, 2}, {1, 3}, {1, 4}, {1, 5},  {1, 7},  {1, 8}, {1, 9},  {1, 10}, \
    {1, 11}, {1, 12}, {2, 0},  {2, 1}, {2, 2}, {2, 3}, {2, 4},  {2, 5},  {2, 7}, {2, 8},  {2, 9}, \
    {2, 10}, {2, 11}, {2, 12}, {2, 6}, {1, 6}, {0, 6}, {0, 13}, {1, 13}, {2, 13} \
}
// clang-format on

#define LAYOUT_DEFAULT LAYOUT

// ==========================================================================
// config.h - Идентификаторы слоев и макросы
// ==========================================================================
#include "keys.h"

#ifdef DEFINE_KEYMAP
#define TAPPING_TERM_DEFAULT 200

// Layer identifiers used directly in the keymap
enum layers { DEF = 0, LWR = 1, RSE = 2, ADJ = 3 };

// clang-format off
// Макрос 0: ZM_numdot (Alt + 4 6)
const MacroStep MACRO_NUMDOT[] = {M_DN(K_LALT), M_D(40),     M_DN(K_KP4), M_UP(K_KP4), M_D(40),
                                  M_DN(K_KP6),  M_UP(K_KP6), M_D(40),     M_UP(K_LALT)};

// Макрос 1: ZM_grtsgn (Alt + 6 2)
const MacroStep MACRO_GRTSGN[] = {M_DN(K_LALT), M_D(40),     M_DN(K_KP6), M_UP(K_KP6), M_D(40),
                                  M_DN(K_KP2),  M_UP(K_KP2), M_D(40),     M_UP(K_LALT)};

// Регистрация макросов в глобальном массиве
const Macro keyboard_macros[] = {[0] = {MACRO_NUMDOT, sizeof(MACRO_NUMDOT) / sizeof(MACRO_NUMDOT[0]), MACRO_NO_LAYER},
                                 [1] = {MACRO_GRTSGN, sizeof(MACRO_GRTSGN) / sizeof(MACRO_GRTSGN[0]), MACRO_NO_LAYER}};


// Определяем общее количество макросов для прошивки
const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
// clang-format on

#define ___ K_TRNS // Transparent key
// #define K_NO K_NULL // Null / No action

// Flat keymap: one entry per key in LAYOUT order
// clang-format off
const uint32_t keymap[][NUM_KEYS] = {

    // ------------------------------------------------------------------------------------------------
    // LAYER 0: DEFAULT
    // Key order: row0_L, row0_R, row1_L, row1_R, row2_L, row2_R, thumbs
    // ------------------------------------------------------------------------------------------------
    [DEF] = {
        HT(K_LGUI, K_RBRC), K_Q,    K_W,    K_E,    K_R,    K_T,    K_Y,    K_U,    K_I,    K_O,    K_P,    HT(K_RGUI, K_LBRC), \
        HT(K_LCTL, K_GRAV), K_A,    K_S,    K_D,    K_F,    K_G,    K_H,    K_J,    K_K,    K_L,    K_SCLN, HT(K_RCTL, K_QUOT), \
        HT(K_LALT, K_BKSL), K_Z,    K_X,    K_C,    K_V,    K_B,    K_N,    K_M,    K_COMM, K_DOT,  K_SLSH, HT(K_RALT, K_MINS), \
        HT(LWR, K_ESC), HT(K_RSFT, K_BSPC), HT(RSE, K_DEL), HT(RSE, K_TAB), HT(K_LSFT, K_SPC), HT(LWR, K_ENT)
    },

    // ------------------------------------------------------------------------------------------------
    // LAYER 1: LOWER (LWR)
    // ------------------------------------------------------------------------------------------------
    [LWR] = {
        HT(K_LGUI, K_CAPS), K_KP_ASTERISK,       K_7,    K_8,    K_9,    MK(MOD_LSHIFT, K_EQL), MK(MOD_LSHIFT, K_1), MK(MOD_LSHIFT, K_2), MK(MOD_LSHIFT, K_3), MK(MOD_LSHIFT, K_4), MK(MOD_LSHIFT, K_5), HT(K_RGUI, K_F1), \
        HT(K_LCTL, K_APP),  K_KP_SLASH,          K_4,    K_5,    K_6,    K_MINS,                MK(MOD_LSHIFT, K_6), MK(MOD_LSHIFT, K_7), MK(MOD_LSHIFT, K_8), MK(MOD_LSHIFT, K_9), MK(MOD_LSHIFT, K_0), HT(K_RALT, K_F2), \
        HT(K_LALT, K_EQL),  K_0,                 K_1,    K_2,    K_3,    M(0),                  MK(MOD_LGUI, K_SPC), K_F4,                K_F5,                K_F6,                K_F7,                HT(K_RALT, K_F3), \
        ___, ___, HT(ADJ, K_DEL), HT(ADJ, K_TAB), ___, ___
    },

    // ------------------------------------------------------------------------------------------------
    // LAYER 2: RAISE (RSE)
    // ------------------------------------------------------------------------------------------------
    [RSE] = {
        K_LGUI, K_VOLU, K_PGUP, K_NO,                          K_HOME, K_PAUS, K_0,    K_HOME, K_NO, K_PGUP, K_VOLU, K_RGUI, \
        K_LCTL, K_VOLD, K_LEFT, K_UP,                          K_DOWN, K_RIGHT,K_LEFT, K_DOWN, K_UP, K_RIGHT,K_VOLD, K_RALT, \
        K_LALT, K_MUTE, K_PGDN, MK(MOD_LGUI | MOD_LSHIFT, K_S),K_END, K_NO,   K_0,    K_END,  M(1), K_PGDN, K_MUTE, K_RALT, \
        HT(ADJ, K_ESC), ___, ___, ___, ___, HT(ADJ, K_ENT)
    },

    // ------------------------------------------------------------------------------------------------
    // LAYER 3: ADJUST (ADJ)
    // ------------------------------------------------------------------------------------------------
    [ADJ] = {
        K_NO /*BLDR*/, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, \
        K_NO,          K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, \
        K_NO,          K_NLCK, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, K_NO, \
        ___, K_NO, ___, ___, K_NO, ___
    }
};
// clang-format on

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

#if defined(MCU_milandr)
// #define LED_PINS {PB7} // Commented out to prevent conflict with LED_ACTIVITY_PIN (PB7)
#else
#define LED_PINS {GPIO25}
#endif

#define LED_DEBUG 0

#endif // DEFINE_KEYMAP

#endif // CONFIG_H
