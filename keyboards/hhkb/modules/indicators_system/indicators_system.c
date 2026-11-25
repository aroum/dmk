#include "FreeRTOS.h"
#include "task.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "keyboard.h"
#include "layers.h"
#include "classic_matrix.h"
#ifdef HHKB_HYBRID
#include "hybrid.h"
#endif
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
    uint16_t state = layers_get_latched_state() & (uint16_t)~(1U << 2);
    if (hhkb_get_system()) state |= 1U << 2;
    layers_set_state(state | 1U);
}

void keyboard_platform_init(void) { update_system_layer(); }
TickType_t keyboard_platform_timeout(void) {
#ifdef HHKB_HYBRID
    return pdMS_TO_TICKS(hhkb_hybrid_indicator_wait_ms(xTaskGetTickCount()));
#else
    return pdMS_TO_TICKS(250);
#endif
}
void keyboard_platform_tick(TickType_t now) {
    bool heartbeat = ((now / pdMS_TO_TICKS(500)) & 1U) != 0;
    bool mounted = tud_mounted();
    bool suspended = tud_suspended();
    bool power_saving_active = keyboard_power_saving_enabled() && (!mounted || suspended);
#ifdef HHKB_HYBRID
    bool battery = hhkb_hybrid_on_battery();
    if (battery || hhkb_hybrid_output_bluetooth()) power_saving_active = false;
    if (!hhkb_hybrid_powered()) power_saving_active = false;
#endif
    for (uint8_t i = 0; i < 2; i++) {
        bool on = false;
        if (power_saving_active) {
            on = false;
        }
#ifdef HHKB_HYBRID
        else if (hhkb_hybrid_led_override(i, now, &on)) {
            // Hybrid status has priority over the configurable default mode.
        }
#endif
        else {
            switch (hhkb_get_led_mode(i)) {
            case 1: on = heartbeat; break;
            case 2: on = caps_lock; break;
            case 3: on = (layers_get_state() & ((1U << 1) | (1U << 3))) != 0; break;
            case 4: on = hhkb_get_system() != 0; break;
            default: break;
            }
        }
        hal_gpio_put(led_pins[i], on && !suspended && !power_saving_active);
    }
}

#ifdef HHKB_HYBRID
void hhkb_hybrid_refresh_indicators(void) { keyboard_platform_tick(xTaskGetTickCount()); }
#endif

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
