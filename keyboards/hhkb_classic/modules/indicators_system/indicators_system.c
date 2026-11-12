#include "FreeRTOS.h"
#include "task.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "keys.h"
#include "layers.h"
#include "classic_matrix.h"
#include "tusb.h"
#include <stdbool.h>
#include <stdint.h>

/* One owner for PA8 and PA9: off, heartbeat, Caps Lock, Fn, system 2. */
static const pin_t led_pins[2] = { A8, A9 };
static bool fn_down, switch_key_down, caps_lock;

static void update_layers(void) {
    uint16_t state = layers_get_state() & (uint16_t)~((1U << 1) | (1U << 2) | (1U << 3));
    state |= 1U;
    if (hhkb_get_system()) state |= 1U << 2;
    if (fn_down) state |= 1U << (hhkb_get_system() ? 3 : 1);
    layers_set_state(state);
}

void keyboard_platform_init(void) { update_layers(); }
TickType_t keyboard_platform_timeout(void) { return pdMS_TO_TICKS(250); }
void keyboard_platform_tick(TickType_t now) {
    bool heartbeat = ((now / pdMS_TO_TICKS(500)) & 1U) != 0;
    bool suspended = tud_suspended();
    for (uint8_t i = 0; i < 2; i++) {
        bool on = false;
        switch (hhkb_get_led_mode(i)) {
        case 1: on = heartbeat; break;
        case 2: on = caps_lock; break;
        case 3: on = fn_down; break;
        case 4: on = hhkb_get_system() != 0; break;
        default: break;
        }
        hal_gpio_put(led_pins[i], on && !suspended);
    }
}

static bool hhkb_process_key(uint32_t key, bool pressed) {
    if (key == MO(1) || key == MO(3)) {
        fn_down = pressed;
        update_layers();
        return true;
    }
    if (key == K_SPC && ((pressed && fn_down) || (!pressed && switch_key_down))) {
        if (pressed && fn_down) {
            switch_key_down = true;
            hhkb_set_system(!hhkb_get_system());
            update_layers();
        } else if (!pressed && switch_key_down) {
            switch_key_down = false;
        }
        return true;
    }
    return false;
}

static void hhkb_hid_led_change(uint8_t state) { caps_lock = (state & 0x02) != 0; }
DMK_HOOK_SUBSCRIBER(hhkb_indicator_hooks,
    .process_key = hhkb_process_key,
    .hid_led_change = hhkb_hid_led_change
);
