#ifndef CONFIG_H
#define CONFIG_H

#include "pin_defs.h"

#define DEFAULT_MCU rp2040
#define MATRIX_TYPE DIRECT

/* Direct pin mapping using ProMicro pin aliases:
 * 18 pins: PM_0, PM_1, PM_2, PM_3, PM_4, PM_5, PM_6, PM_7,
 *          PM_8, PM_9, PM_10, PM_14, PM_15, PM_16, PM_18, PM_19, PM_20, PM_21
 */
#define DIRECT_PINS { \
    PM_0,  PM_1,  PM_2,  PM_3,  PM_4,  PM_5,  \
    PM_6,  PM_7,  PM_8,  PM_9,  PM_10, PM_14, \
    PM_15, PM_16, PM_18, PM_19, PM_20, PM_21  \
}

#define LED_PINS {PM_LED}
#define LED_DEBUG 0

#define NUM_ROWS 1
#define NUM_COLS 18
#define NUM_KEYS 18

#define LAYOUT { \
    {0, 0},  {0, 1},  {0, 2},  {0, 3},  {0, 4},  {0, 5},  \
    {0, 6},  {0, 7},  {0, 8},  {0, 9},  {0, 10}, {0, 11}, \
    {0, 12}, {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}  \
}
#define LAYOUT_DEFAULT LAYOUT

#ifdef DEFINE_KEYMAP
#define DEF 0

/* Macros typing each ProMicro pin name upon shorting to GND:
 * PM0, PM1, PM2, PM3, PM4, PM5, PM6, PM7, PM8, PM9, PM10, PM14, PM15, PM16, PM18, PM19, PM20, PM21
 */
#define PM_STEP_3(k1, k2, k3) \
    M_DN(k1), M_UP(k1), M_D(10), \
    M_DN(k2), M_UP(k2), M_D(10), \
    M_DN(k3), M_UP(k3), M_D(10), \
    M_DN(K_SPC), M_UP(K_SPC)

#define PM_STEP_4(k1, k2, k3, k4) \
    M_DN(k1), M_UP(k1), M_D(10), \
    M_DN(k2), M_UP(k2), M_D(10), \
    M_DN(k3), M_UP(k3), M_D(10), \
    M_DN(k4), M_UP(k4), M_D(10), \
    M_DN(K_SPC), M_UP(K_SPC)

static const MacroStep MACRO_PM0[]  = { PM_STEP_3(K_P, K_M, K_0) };
static const MacroStep MACRO_PM1[]  = { PM_STEP_3(K_P, K_M, K_1) };
static const MacroStep MACRO_PM2[]  = { PM_STEP_3(K_P, K_M, K_2) };
static const MacroStep MACRO_PM3[]  = { PM_STEP_3(K_P, K_M, K_3) };
static const MacroStep MACRO_PM4[]  = { PM_STEP_3(K_P, K_M, K_4) };
static const MacroStep MACRO_PM5[]  = { PM_STEP_3(K_P, K_M, K_5) };
static const MacroStep MACRO_PM6[]  = { PM_STEP_3(K_P, K_M, K_6) };
static const MacroStep MACRO_PM7[]  = { PM_STEP_3(K_P, K_M, K_7) };
static const MacroStep MACRO_PM8[]  = { PM_STEP_3(K_P, K_M, K_8) };
static const MacroStep MACRO_PM9[]  = { PM_STEP_3(K_P, K_M, K_9) };
static const MacroStep MACRO_PM10[] = { PM_STEP_4(K_P, K_M, K_1, K_0) };
static const MacroStep MACRO_PM14[] = { PM_STEP_4(K_P, K_M, K_1, K_4) };
static const MacroStep MACRO_PM15[] = { PM_STEP_4(K_P, K_M, K_1, K_5) };
static const MacroStep MACRO_PM16[] = { PM_STEP_4(K_P, K_M, K_1, K_6) };
static const MacroStep MACRO_PM18[] = { PM_STEP_4(K_P, K_M, K_1, K_8) };
static const MacroStep MACRO_PM19[] = { PM_STEP_4(K_P, K_M, K_1, K_9) };
static const MacroStep MACRO_PM20[] = { PM_STEP_4(K_P, K_M, K_2, K_0) };
static const MacroStep MACRO_PM21[] = { PM_STEP_4(K_P, K_M, K_2, K_1) };

const Macro keyboard_macros[] = {
    [0]  = {MACRO_PM0,  sizeof(MACRO_PM0)  / sizeof(MACRO_PM0[0]),  MACRO_NO_LAYER},
    [1]  = {MACRO_PM1,  sizeof(MACRO_PM1)  / sizeof(MACRO_PM1[0]),  MACRO_NO_LAYER},
    [2]  = {MACRO_PM2,  sizeof(MACRO_PM2)  / sizeof(MACRO_PM2[0]),  MACRO_NO_LAYER},
    [3]  = {MACRO_PM3,  sizeof(MACRO_PM3)  / sizeof(MACRO_PM3[0]),  MACRO_NO_LAYER},
    [4]  = {MACRO_PM4,  sizeof(MACRO_PM4)  / sizeof(MACRO_PM4[0]),  MACRO_NO_LAYER},
    [5]  = {MACRO_PM5,  sizeof(MACRO_PM5)  / sizeof(MACRO_PM5[0]),  MACRO_NO_LAYER},
    [6]  = {MACRO_PM6,  sizeof(MACRO_PM6)  / sizeof(MACRO_PM6[0]),  MACRO_NO_LAYER},
    [7]  = {MACRO_PM7,  sizeof(MACRO_PM7)  / sizeof(MACRO_PM7[0]),  MACRO_NO_LAYER},
    [8]  = {MACRO_PM8,  sizeof(MACRO_PM8)  / sizeof(MACRO_PM8[0]),  MACRO_NO_LAYER},
    [9]  = {MACRO_PM9,  sizeof(MACRO_PM9)  / sizeof(MACRO_PM9[0]),  MACRO_NO_LAYER},
    [10] = {MACRO_PM10, sizeof(MACRO_PM10) / sizeof(MACRO_PM10[0]), MACRO_NO_LAYER},
    [11] = {MACRO_PM14, sizeof(MACRO_PM14) / sizeof(MACRO_PM14[0]), MACRO_NO_LAYER},
    [12] = {MACRO_PM15, sizeof(MACRO_PM15) / sizeof(MACRO_PM15[0]), MACRO_NO_LAYER},
    [13] = {MACRO_PM16, sizeof(MACRO_PM16) / sizeof(MACRO_PM16[0]), MACRO_NO_LAYER},
    [14] = {MACRO_PM18, sizeof(MACRO_PM18) / sizeof(MACRO_PM18[0]), MACRO_NO_LAYER},
    [15] = {MACRO_PM19, sizeof(MACRO_PM19) / sizeof(MACRO_PM19[0]), MACRO_NO_LAYER},
    [16] = {MACRO_PM20, sizeof(MACRO_PM20) / sizeof(MACRO_PM20[0]), MACRO_NO_LAYER},
    [17] = {MACRO_PM21, sizeof(MACRO_PM21) / sizeof(MACRO_PM21[0]), MACRO_NO_LAYER},
};

const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);

const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = {
        M(0),  M(1),  M(2),  M(3),  M(4),  M(5),
        M(6),  M(7),  M(8),  M(9),  M(10), M(11),
        M(12), M(13), M(14), M(15), M(16), M(17)
    }
};

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);
#endif // DEFINE_KEYMAP

#endif // CONFIG_H
