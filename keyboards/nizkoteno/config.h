#ifndef CONFIG_H
#define CONFIG_H

#define MCU_rp2040
#define DEFAULT_MCU rp2040
#define MATRIX_TYPE DIRECT

/* --- Matrix Settings (RP2040) --- */
#define DIRECT_PINS {GPIO9, GPIO8, GPIO6, GPIO5, GPIO10, GPIO7, GPIO4, GPIO2, GPIO1, GPIO3}
#define LED_PINS {GPIO15}
#define RGB_PIN GPIO29

#define VIAL
#define VIAL_KEYBOARD_NAME "Nizkoteno"
#define VIAL_VENDOR_ID 0xCafe
#define VIAL_PRODUCT_ID 0x4011
#define LED_DEBUG 0
// #define LED_HID_CAPS_LOCK   0
// #define LED_HID_NUM_LOCK    0
// #define LED_HID_COMPOSE     3
// #define LED_HID_KANA        4

/* --- RGB Settings --- */
#define RGB_NUM 10
// clang-format off
#define RGB_MAP { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
#define RGB_THEME_DEFAULT { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x888888, 0x444444, 0x222222 }
#define RGB_THEME_GAMING  { 0xFF4500, 0xFF8C00, 0xFFD700, 0xADFF2F, 0x00FF7F, 0x00CED1, 0x1E90FF, 0x9370DB, 0xFF1493, 0xFF0000 }
#define RGB_THEMES { RGB_THEME_DEFAULT, RGB_THEME_GAMING }
// clang-format on

#define NUM_ROWS 1
#define NUM_COLS 10
#define NUM_KEYS 10

// {ROW, COL}
// clang-format off
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, \
    {0, 4}, {0, 5}, {0, 6}, {0, 7}, \
                            {0, 8}, {0, 9} \
}
#define LAYOUT_DEFAULT LAYOUT

// Visual editor layout grid: 3 rows, key 9 under 4 and 8, key 10 to its right
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
extern void keyboard_send_key(uint16_t keycode, bool pressed);

// Chord action callbacks
static inline void do_copy(void) {
    keyboard_send_key(K_LCTL, true);
    keyboard_send_key(K_C, true);
    keyboard_send_key(K_C, false);
    keyboard_send_key(K_LCTL, false);
}

static inline void do_paste(void) {
    keyboard_send_key(K_LCTL, true);
    keyboard_send_key(K_V, true);
    keyboard_send_key(K_V, false);
    keyboard_send_key(K_LCTL, false);
}

// clang-format off
const Chord my_chords[] = {
    {.keys = {K(0, 0, 0), K(0, 1, 0)}, // K_A and K_S on layer 0 (DEF)
     .key_count = 2,
     .action = do_copy}};
#define CHORDS_COUNT (sizeof(my_chords) / sizeof(my_chords[0]))

// Flat keymap: one entry per key in LAYOUT order
// Row 1: 1, 2, 3, 4
// Row 2: 5, 6, 7, 8
// Row 3: _, _, _, 9, 10
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
