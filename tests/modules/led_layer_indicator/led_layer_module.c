#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "task.h"
#include <stdint.h>

#ifndef LAYER_LED_PIN_0
#define LAYER_LED_PIN_0 PB3
#endif

#ifndef LAYER_LED_PIN_1
#define LAYER_LED_PIN_1 PB4
#endif

static bool led_module_inited = false;

void hook_early_init(void) {
    hal_gpio_init(LAYER_LED_PIN_0);
    hal_gpio_set_dir(LAYER_LED_PIN_0, true);
    hal_gpio_put(LAYER_LED_PIN_0, true); // Base layer LED active by default

    hal_gpio_init(LAYER_LED_PIN_1);
    hal_gpio_set_dir(LAYER_LED_PIN_1, true);
    hal_gpio_put(LAYER_LED_PIN_1, false);

    led_module_inited = true;
}

void hook_layer_change(uint8_t active_layer) {
    if (!led_module_inited) {
        return;
    }

    // Toggle layer indicator LEDs according to active layer index
    hal_gpio_put(LAYER_LED_PIN_0, active_layer == 0);
    hal_gpio_put(LAYER_LED_PIN_1, active_layer >= 1);
}
