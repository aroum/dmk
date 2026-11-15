#include "FreeRTOS.h"
#include "task.h"
#include "hooks.h"
#include "layers.h"
#include "rgb.h"
#include "ec_settings.h"
#include "tusb.h"
#include <stdbool.h>
#include <stdint.h>

static bool s_caps_lock = false;

static void capybara_hid_led_change(uint8_t state) {
    s_caps_lock = (state & 0x02) != 0;
}

DMK_HOOK_SUBSCRIBER(capybara_indicator_hooks,
    .hid_led_change = capybara_hid_led_change
);

#ifndef RGB_LIMIT_VAL
#define RGB_LIMIT_VAL 60
#endif

#define IND_COLOR_WHITE (((uint32_t)RGB_LIMIT_VAL << 16) | ((uint32_t)RGB_LIMIT_VAL << 8) | RGB_LIMIT_VAL)
#define IND_COLOR_BLUE  ((uint32_t)RGB_LIMIT_VAL)
#define IND_COLOR_ORANGE (((uint32_t)RGB_LIMIT_VAL << 16) | (((uint32_t)RGB_LIMIT_VAL / 3) << 8))

static uint32_t get_indicator_color(uint8_t mode, bool heartbeat, bool fn_active, bool sys_active) {
    switch (mode) {
    case 1: // Debug heartbeat
        return heartbeat ? IND_COLOR_WHITE : 0x000000;
    case 2: // Caps Lock
        return s_caps_lock ? IND_COLOR_WHITE : 0x000000;
    case 3: // Fn layer
        return fn_active ? IND_COLOR_BLUE : 0x000000;
    case 4: // System 2
        return sys_active ? IND_COLOR_ORANGE : 0x000000;
    default: // Off
        return 0x000000;
    }
}

TickType_t keyboard_platform_timeout(void) {
    return pdMS_TO_TICKS(250);
}

static uint32_t s_last_col0 = 0xFFFFFFFF;
static uint32_t s_last_col1 = 0xFFFFFFFF;

void keyboard_platform_tick(TickType_t now) {
    bool suspended = tud_suspended();
    if (suspended) {
        if (s_last_col0 != 0) {
            rgb_set_pixel_raw(0, 0);
            s_last_col0 = 0;
        }
        if (s_last_col1 != 0) {
            rgb_set_pixel_raw(1, 0);
            s_last_col1 = 0;
        }
        return;
    }

    bool heartbeat = ((now / pdMS_TO_TICKS(500)) & 1U) != 0;
    bool fn_active = (layers_get_state() & ((1U << 1) | (1U << 2))) != 0;
    bool sys_active = (hhkb_get_system() != 0);

#if defined(RGB_NUM) && (RGB_NUM >= 11)
    // Front Indicator 0 (LED 0): "Indicator Mode"
    uint8_t mode0 = hhkb_get_led_mode(0);
    uint32_t col0 = get_indicator_color(mode0, heartbeat, fn_active, sys_active);
    if (col0 != s_last_col0) {
        rgb_set_pixel_raw(0, col0);
        s_last_col0 = col0;
    }

    // Front Indicator 1 (LED 1): "Underglow Mode" / Indicator 1
    uint8_t mode1 = hhkb_get_led_mode(1);
    uint32_t col1 = (mode1 == 0) ? 0x000000 : get_indicator_color(mode1, heartbeat, fn_active, sys_active);
    if (col1 != s_last_col1) {
        rgb_set_pixel_raw(1, col1);
        s_last_col1 = col1;
    }
#elif defined(RGB_NUM)
    // 9 LEDs (front LEDs bypassed by hardware jumper): all LEDs are underglow
    uint8_t mode0 = hhkb_get_led_mode(0);
    if (mode0 != 0) {
        uint32_t col = get_indicator_color(mode0, heartbeat, fn_active, sys_active);
        if (col != 0) {
            if (s_last_col0 != col) {
                rgb_set_pixel_raw(0, col);
                s_last_col0 = col;
            }
        } else {
            if (s_last_col0 != 0) {
                rgb_clear_pixel_raw(0);
                s_last_col0 = 0;
            }
        }
    } else {
        if (s_last_col0 != 0) {
            rgb_clear_pixel_raw(0);
            s_last_col0 = 0;
        }
    }
#endif
}
