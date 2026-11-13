#include "FreeRTOS.h"
#include "task.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "layers.h"
#include "classic_matrix.h"
#include "tusb.h"
#include <stdbool.h>
#include <stdint.h>

/* One owner for PA8 and PA9: off, heartbeat, Caps Lock, Fn, system 2. */
static const pin_t led_pins[2] = { A8, A9 };
// VIA protocol 12 assigns custom keycodes from 0x7E00 in JSON order.
enum {
    HHKB_SYSTEM_1 = 0x7E00,
    HHKB_SYSTEM_2 = 0x7E01,
};

static bool caps_lock;

static void update_system_layer(void) {
    uint16_t state = layers_get_state() & (uint16_t)~(1U << 2);
    if (hhkb_get_system()) state |= 1U << 2;
    layers_set_state(state | 1U);
}

void keyboard_platform_init(void) { update_system_layer(); }
TickType_t keyboard_platform_timeout(void) { return pdMS_TO_TICKS(250); }
void keyboard_platform_tick(TickType_t now) {
    bool heartbeat = ((now / pdMS_TO_TICKS(500)) & 1U) != 0;
    bool suspended = tud_suspended();
    for (uint8_t i = 0; i < 2; i++) {
        bool on = false;
        switch (hhkb_get_led_mode(i)) {
        case 1: on = heartbeat; break;
        case 2: on = caps_lock; break;
        case 3: on = (layers_get_state() & ((1U << 1) | (1U << 3))) != 0; break;
        case 4: on = hhkb_get_system() != 0; break;
        default: break;
        }
        hal_gpio_put(led_pins[i], on && !suspended);
    }
}

static bool hhkb_process_key(uint32_t key, bool pressed) {
    if (key != HHKB_SYSTEM_1 && key != HHKB_SYSTEM_2) return false;
    if (pressed) {
        hhkb_set_system(key == HHKB_SYSTEM_2);
        update_system_layer();
    }
    return true;
}

static void hhkb_hid_led_change(uint8_t state) { caps_lock = (state & 0x02) != 0; }
DMK_HOOK_SUBSCRIBER(hhkb_indicator_hooks,
    .process_key = hhkb_process_key,
    .hid_led_change = hhkb_hid_led_change
);
