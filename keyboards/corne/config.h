#ifndef CONFIG_H
#define CONFIG_H

// #define VIAL // Enable Vial/VIA support

#include "pin_defs.h"

/* --- Настройки сплита --- */
#define MASTER_SIDE LEFT // Указываем сторону (LEFT, RIGHT или AUTO)
#if defined(MCU_milandr)
#define SERIAL_PIN PA0
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define SERIAL_PIN GPIO0
#elif defined(MCU_nrf52840)
#define SERIAL_PIN P0_00
#elif defined(MCU_baikal)
#define SERIAL_PIN GPIO0
#endif

/* --- Настройки матрицы --- */
// Типы подключения: ROW2COL, COL2ROW, DIRECT_PIN (choose one)
#define MATRIX_TYPE_ROW2COL
// #define MATRIX_TYPE_COL2ROW
// #define MATRIX_TYPE_DIRECT_PIN

// Пины для строк и столбцов
#if defined(MCU_milandr)
#define ROW_PINS {PB0, PB1, PB2, PB3}
#define COL_PINS {PA0, PA1, PA2, PA3, PA4}
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define ROW_PINS {GPIO0, GPIO1, GPIO2, GPIO3}
#define COL_PINS {GPIO4, GPIO5, GPIO6, GPIO7, GPIO8}
#elif defined(MCU_nrf52840)
#define ROW_PINS {P0_00, P0_01, P0_02, P0_03}
#define COL_PINS {P0_04, P0_05, P0_06, P0_07, P0_08}
#elif defined(MCU_baikal)
#define ROW_PINS {GPIO0, GPIO1, GPIO2, GPIO3}
#define COL_PINS {GPIO4, GPIO5, GPIO6, GPIO7, GPIO8}
#endif

/* --- RGB подсветка --- */
#if defined(MCU_milandr)
#define RGB_PIN PC0
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
#define RGB_PIN GPIO25
#elif defined(MCU_nrf52840)
#define RGB_PIN P0_13
#elif defined(MCU_baikal)
#define RGB_PIN GPIO25
#endif

/* --- Карта клавиш (Keymap Matrix) --- */
#define NUM_ROWS 4
#define NUM_COLS 10
#define NUM_ROWS_SPLIT 4
#define NUM_COLS_SPLIT 5
#define NUM_KEYS 34

// {ROW, COL}
// clang-format off
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, \
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, \
                            {3, 3}, {3, 4}, {3, 5}, {3, 6}, \
}
// clang-format on
#define LAYOUT_DEFAULT LAYOUT

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
        K_Q,   K_W,   K_E,   K_R,   K_T,     K_Y,   K_U,    K_I,    K_O,   K_P, \
        K_A,   K_S,   K_D,   K_F,   K_G,     K_H,   K_J,    K_K,    K_L,   K_ENT, \
        K_Z,   K_X,   K_C,   K_V,   K_B,     K_N,   K_M,    K_ALT,  K_CTRL,K_SHFT, \
        K_BSPC, FN1, K_SPC, K_TRNS
    },
    [FN1] = {
        K_ESC, K_APOS,K_A_U, K_BRAL,K_BRAR,  K_7,   K_8,    K_9,    K_MIN, K_SLSH, \
        K_TAB, K_A_L, K_A_D, K_A_R, K_HASH,  K_4,   K_5,    K_6,    K_EQU, K_ENT, \
        K_SHFT,K_BKSL,K_SCLN,K_COMM,K_DOT,   K_1,   K_2,    K_3,    K_ALT, K_SHFT, \
        K_BSPC, K_TRNS, K_0, K_TRNS
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
