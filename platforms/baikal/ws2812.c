#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "task.h"
#include <stdbool.h>
#include <string.h>

#define WS2812_MAX_LEDS 64

extern uint32_t SystemCoreClock;

static uint32_t ws2812_colors[WS2812_MAX_LEDS];
static uint32_t ws2812_len = 0;
static uint32_t ws2812_pin = 0;

void ws2812_init(uint32_t pin, uint32_t length) {
    ws2812_pin = pin;
    ws2812_len = (length <= WS2812_MAX_LEDS) ? length : WS2812_MAX_LEDS;
    memset(ws2812_colors, 0, sizeof(ws2812_colors));

    hal_gpio_init(pin);
    hal_gpio_set_dir(pin, true);
    hal_gpio_put(pin, false);
}

void ws2812_set_color(uint32_t index, uint32_t color) {
    if (index >= ws2812_len)
        return;
    ws2812_colors[index] = color;
}

void ws2812_show(void) {
    if (ws2812_len == 0)
        return;

    // Calibrate delays based on SystemCoreClock (assuming ~3 cycles per loop iteration)
    uint32_t cycles_per_us = SystemCoreClock / 1000000;
    if (cycles_per_us == 0)
        cycles_per_us = 80; // Default fallback for 80MHz

    // Delays in loop iterations
    uint32_t t1h_loops = (7 * cycles_per_us) / 30;
    uint32_t t1l_loops = (6 * cycles_per_us) / 30;
    uint32_t t0h_loops = (35 * cycles_per_us) / 300;
    uint32_t t0l_loops = (8 * cycles_per_us) / 30;

    taskENTER_CRITICAL();

    for (uint32_t i = 0; i < ws2812_len; i++) {
        uint32_t color = ws2812_colors[i];
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        // GRB order
        uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

        for (int bit = 23; bit >= 0; bit--) {
            if ((grb >> bit) & 1) {
                // T1H (high for 700ns, low for 600ns)
                hal_gpio_put(ws2812_pin, true);
                for (volatile uint32_t d = 0; d < t1h_loops; d++) {
                }
                hal_gpio_put(ws2812_pin, false);
                for (volatile uint32_t d = 0; d < t1l_loops; d++) {
                }
            } else {
                // T0H (high for 350ns, low for 800ns)
                hal_gpio_put(ws2812_pin, true);
                for (volatile uint32_t d = 0; d < t0h_loops; d++) {
                }
                hal_gpio_put(ws2812_pin, false);
                for (volatile uint32_t d = 0; d < t0l_loops; d++) {
                }
            }
        }
    }

    taskEXIT_CRITICAL();

    // Reset latch delay (>300us)
    hal_sleep_us(300);
}
