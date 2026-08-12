#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"
#define MATRIX_TYPE DIRECT
/* --- Настройки матрицы --- */

#if defined(MCU_milandr)
#define DIRECT_PINS {PE0, PE1, PE2, PE3, PE4, PD0, PD1, PD2, PA3, PD4}
#elif defined(MCU_nrf52840)
#define DIRECT_PINS {P0_06, P0_08, P1_00, P0_24, P0_09, P0_11, P0_22, P0_17, P0_10, P0_20}
#define LED_PINS {P0_15}
#elif defined(MCU_rp2040)
#define DIRECT_PINS {GPIO9, GPIO8, GPIO6, GPIO5, GPIO10, GPIO7, GPIO4, GPIO2, GPIO1, GPIO3}
#define LED_PINS {GPIO15}

#endif

#define VIAL
#define VIAL_KEYBOARD_NAME "Nizkoteno"
#define VIAL_VENDOR_ID 0xCafe
#define VIAL_PRODUCT_ID 0x4011
#define LED_DEBUG 0
// #define LED_HID_CAPS_LOCK   0
// #define LED_HID_NUM_LOCK    0
// #define LED_HID_COMPOSE     3
// #define LED_HID_KANA        4

/* --- Настройки RGB --- */
#define RGB_NUM 10

#define RGB_PIN GPIO29
// #define RGB_PIN GPIO11
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
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9} \
}
#define LAYOUT_DEFAULT LAYOUT

// Optional editor layout positioning grid coordinates
// Renders the 10 keys visually as two rows of 5 keys in the editor
#define LAYOUT_EDITOR { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {2, 3}, {2, 4} \
}
// clang-format on

#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1

#if __has_include("keymap_external.h")
#include "keymap_external.h"
#else
// Chord action callbacks
static inline void do_copy(void) {
    key_event_t event1 = {K_LCTL, 1};
    xQueueSend(usb_queue, &event1, 0);
    key_event_t event2 = {K_C, 1};
    xQueueSend(usb_queue, &event2, 0);
    key_event_t event3 = {K_C, 0};
    xQueueSend(usb_queue, &event3, 0);
    key_event_t event4 = {K_LCTL, 0};
    xQueueSend(usb_queue, &event4, 0);
}

static inline void do_paste(void) {
    key_event_t event1 = {K_LCTL, 1};
    xQueueSend(usb_queue, &event1, 0);
    key_event_t event2 = {K_V, 1};
    xQueueSend(usb_queue, &event2, 0);
    key_event_t event3 = {K_V, 0};
    xQueueSend(usb_queue, &event3, 0);
    key_event_t event4 = {K_LCTL, 0};
    xQueueSend(usb_queue, &event4, 0);
}

// clang-format off
const Chord my_chords[] = {
    {.keys = {K(0, 0, 0), K(0, 1, 0)}, // K_A and K_S on layer 0 (DEF)
     .key_count = 2,
     .action = do_copy}};
#define CHORDS_COUNT (sizeof(my_chords) / sizeof(my_chords[0]))

// Flat keymap: one entry per key in LAYOUT order
const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = {RGB_TOGG, RGB_NEXT, K_D, K_F, K_R, K_W, K_E, K_C, K_L, K_A},
    [FN1] = {K_ESC, K_APOS, K_A_U, K_BRAL, K_BRAR, K_SHFT, K_BKSL, K_SCLN, K_0, K_TRNS}};

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);

// Macros
const Macro keyboard_macros[] = {};
const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
// clang-format on
#endif
#endif

#endif // CONFIG_H
