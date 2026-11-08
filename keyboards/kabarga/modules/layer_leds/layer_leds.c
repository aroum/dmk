#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "pin_defs.h"
#include <stdbool.h>
#include <stdint.h>

// 4 indicator LEDs on Kabarga
static const pin_t layer_led_pins[] = {PM_15, PM_14, PM_16, PM_10};
#define NUM_LAYER_LEDS (sizeof(layer_led_pins) / sizeof(layer_led_pins[0]))

static void set_layer_led(uint8_t layer) {
    for (uint8_t i = 0; i < NUM_LAYER_LEDS; i++) {
        hal_gpio_put(layer_led_pins[i], (i == layer));
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
    set_layer_led(active_layer);
}

DMK_HOOK_SUBSCRIBER(module_hooks,
    .early_init = subscriber_early_init,
    .layer_change = subscriber_layer_change);
