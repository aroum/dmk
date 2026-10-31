#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "task.h"
#include "bmcu_cru.h"
#include <stdbool.h>
#include <string.h>

#define WS2812_MAX_LEDS 64

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

#define DELAY_CYCLES(count) do { \
    uint32_t _c = (count); \
    if (_c > 0) { \
        __asm__ volatile( \
            "1: addi %0, %0, -1\n" \
            "   bnez %0, 1b\n" \
            : "+r"(_c) \
            : \
            : "cc" \
        ); \
    } \
} while (0)

void ws2812_show(void) {
    if (ws2812_len == 0)
        return;

    // Calibrate delays based on CPU clock frequency (~3 cycles per delay iteration)
    CRU_Clocks_TypeDef cru_clocks;
    CRU_GetSystemClocksFreq(&cru_clocks);
    uint32_t cpu_freq = cru_clocks.CCLK_Frequency ? cru_clocks.CCLK_Frequency : 200000000;
    uint32_t cycles_per_us = cpu_freq / 1000000;
    if (cycles_per_us == 0)
        cycles_per_us = 200;

    // Loop iteration count for 3-cycle loop:
    // T0H: ~350ns, T0L: ~800ns
    // T1H: ~700ns, T1L: ~600ns
    uint32_t t0h_cycles = (35 * cycles_per_us) / 300;
    uint32_t t0l_cycles = (80 * cycles_per_us) / 300;
    uint32_t t1h_cycles = (70 * cycles_per_us) / 300;
    uint32_t t1l_cycles = (60 * cycles_per_us) / 300;

    GPIO_TypeDef *port = (ws2812_pin < 16) ? GPIO0 : (ws2812_pin < 32) ? GPIO1 : GPIO2;
    uint32_t mask = 1U << (ws2812_pin % 16);
    volatile uint32_t *dr = &port->DR;
    uint32_t val_high = *dr | mask;
    uint32_t val_low = *dr & ~mask;

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
                *dr = val_high;
                DELAY_CYCLES(t1h_cycles);
                *dr = val_low;
                DELAY_CYCLES(t1l_cycles);
            } else {
                *dr = val_high;
                DELAY_CYCLES(t0h_cycles);
                *dr = val_low;
                DELAY_CYCLES(t0l_cycles);
            }
        }
    }

    *dr = val_low;

    taskEXIT_CRITICAL();

    // Reset latch delay (>300us)
    hal_sleep_us(300);
}
