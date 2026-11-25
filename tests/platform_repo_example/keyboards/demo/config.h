#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include <stddef.h>
#include "keys.h"
#define DEFAULT_MCU external_demo
#define DMK_PLATFORM_DIR "../../platforms/external_demo"
#define CUSTOM_MATRIX 1
#define NO_HOLD_TAP 1
#define NO_ONESHOT 1
#define NO_COMBOS 1
#define NO_MACROS 1
#define NO_MOUSE 1
#define NO_GAMEPAD 1
#define NO_LED 1
#define NO_RGB 1
#define NUM_ROWS 1
#define NUM_COLS 1
#define NUM_KEYS 1
#define MATRIX_ROWS 1
#define MATRIX_COLS 1
#define LAYOUT {{0, 0}}
#ifdef DEFINE_KEYMAP
const uint32_t keymap[1][1] = {{K_A}};
const size_t keymap_layers = 1;
#else
extern const uint32_t keymap[1][1];
extern const size_t keymap_layers;
#endif
#endif
