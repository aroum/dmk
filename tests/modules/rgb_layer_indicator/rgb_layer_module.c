#include "FreeRTOS.h"
#include "hooks.h"
#include "rgb.h"
#include "task.h"
#include <stdint.h>

void hook_layer_change(uint8_t active_layer) {
    // Check if RGB subsystem is available
    if (!rgb_get_enabled()) {
        rgb_set_enabled(true);
    }

    // Set solid color based on active layer
    switch (active_layer) {
    case 0:
        // Base layer: Calm Cyan / Green
        rgb_set_color(85, 255);
        break;
    case 1:
        // Navigation / Symbols: Deep Blue
        rgb_set_color(160, 255);
        break;
    case 2:
        // Media / Numbers: Purple / Magenta
        rgb_set_color(213, 255);
        break;
    default:
        // System / Function: Bright Red / Orange
        rgb_set_color(0, 255);
        break;
    }

    rgb_set_mode(RGBLIGHT_MODE_STATIC_LIGHT);
}
