#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "keyboard.h"
#include "task.h"
#include "pin_defs.h"
#include "tusb.h"
#include <stdbool.h>
#include <stdint.h>

// 4 indicator LEDs on Kabarga
static const pin_t layer_led_pins[] = {PM_15, PM_14, PM_16, PM_10};
#define NUM_LAYER_LEDS (sizeof(layer_led_pins) / sizeof(layer_led_pins[0]))
static uint8_t selected_layer;
static bool blanked;
extern volatile bool usb_mounted;

static void set_layer_led(uint8_t layer) {
    for (uint8_t i = 0; i < NUM_LAYER_LEDS; i++) {
        hal_gpio_put(layer_led_pins[i], !blanked && i == layer);
    }
}

static void subscriber_early_init(void) {
    for (uint8_t i = 0; i < NUM_LAYER_LEDS; i++) {
        hal_gpio_init(layer_led_pins[i]);
        hal_gpio_set_dir(layer_led_pins[i], true); // Output
    }
    // Layer 0 is active on boot
    set_layer_led(0);
}

static void subscriber_layer_change(uint8_t active_layer) {
    selected_layer = active_layer;
    set_layer_led(active_layer);
}

TickType_t keyboard_platform_timeout(void) {
    return keyboard_power_saving_enabled() ? pdMS_TO_TICKS(20) : portMAX_DELAY;
}

void keyboard_platform_tick(TickType_t now) {
    (void)now;
    bool should_blank = keyboard_power_saving_enabled() && (!usb_mounted || tud_suspended());
    if (blanked != should_blank) {
        blanked = should_blank;
        set_layer_led(selected_layer);
    }
}

DMK_HOOK_SUBSCRIBER(module_hooks,
    .early_init = subscriber_early_init,
    .layer_change = subscriber_layer_change);
