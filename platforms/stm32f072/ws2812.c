#include "FreeRTOS.h"
#include "hal_gpio.h"
#include "task.h"
#include "stm32f0xx.h"
#include "config.h"
#include <stdbool.h>
#include <string.h>

#ifndef WS2812_MAX_LEDS
#if defined(RGB_NUM)
#define WS2812_MAX_LEDS RGB_NUM
#else
#define WS2812_MAX_LEDS 32
#endif
#endif

static uint32_t ws2812_colors[WS2812_MAX_LEDS];
static uint8_t ws2812_grb_stream[WS2812_MAX_LEDS * 3];
static uint32_t ws2812_len = 0;
static uint32_t ws2812_pin = 0;

#define NOP1   __asm__ volatile("nop")
#define NOP2   NOP1; NOP1
#define NOP4   NOP2; NOP2
#define NOP8   NOP4; NOP4
#define NOP11  NOP8; NOP2; NOP1
#define NOP22  NOP8; NOP8; NOP4; NOP2
#define NOP26  NOP8; NOP8; NOP8; NOP2
#define NOP39  NOP26; NOP11; NOP2

/**
 * @brief Cycle-accurate WS2812 bit-banging transmission loop for Cortex-M0 @ 48 MHz.
 *
 * Placed in SRAM (.RamFunc) to eliminate Flash wait states and instruction prefetch jitter.
 *
 * Timings @ 48 MHz (1 clock cycle = 20.833 ns):
 *   Bit 0: T0H ~396 ns (19 cycles), T0L ~854 ns (41 cycles). Target: 350ns +/- 150ns
 *   Bit 1: T1H ~667 ns (32 cycles), T1L ~583 ns (28 cycles). Target: 700ns +/- 150ns
 *   Bit period: ~1.25 us (60 cycles)
 */
__attribute__((section(".RamFunc"), noinline))
static void ws2812_send_stream(volatile uint32_t *bsrr, uint32_t set_mask, uint32_t reset_mask,
                               const uint8_t *data, uint32_t len) {
    while (len--) {
        uint8_t byte = *data++;
        for (int i = 0; i < 8; i++) {
            if (byte & 0x80) {
                // Bit 1: HIGH (~670ns, 32 cycles), LOW (~580ns, 28 cycles)
                *bsrr = set_mask;
                NOP26;
                *bsrr = reset_mask;
                NOP22;
            } else {
                // Bit 0: HIGH (~390ns, 19 cycles), LOW (~860ns, 41 cycles)
                *bsrr = set_mask;
                NOP11;
                *bsrr = reset_mask;
                NOP39;
            }
            byte <<= 1;
        }
    }
}

void ws2812_init(uint32_t pin, uint32_t length) {
    ws2812_pin = pin;
    ws2812_len = (length <= WS2812_MAX_LEDS) ? length : WS2812_MAX_LEDS;
    memset(ws2812_colors, 0, sizeof(ws2812_colors));
    memset(ws2812_grb_stream, 0, sizeof(ws2812_grb_stream));

    hal_gpio_init((pin_t)pin);
    hal_gpio_set_dir((pin_t)pin, true);
    hal_gpio_put((pin_t)pin, false);
    hal_sleep_us(400); // WS2812 reset latch
}

void ws2812_set_color(uint32_t index, uint32_t color) {
    if (index >= ws2812_len)
        return;
    ws2812_colors[index] = color;
}

void ws2812_show(void) {
    if (ws2812_len == 0)
        return;

    // Pack into linear GRB buffer before disabling interrupts
    for (uint32_t i = 0; i < ws2812_len; i++) {
        uint32_t color = ws2812_colors[i];
        ws2812_grb_stream[i * 3 + 0] = (color >> 8) & 0xFF;  // Green
        ws2812_grb_stream[i * 3 + 1] = (color >> 16) & 0xFF; // Red
        ws2812_grb_stream[i * 3 + 2] = color & 0xFF;         // Blue
    }

    uint8_t port_idx = (uint8_t)(ws2812_pin >> 4);
    GPIO_TypeDef *port = (port_idx == 0) ? GPIOA :
                         (port_idx == 1) ? GPIOB :
                         (port_idx == 2) ? GPIOC :
                         (port_idx == 3) ? GPIOD : GPIOF;
    uint32_t pin_mask = 1U << (ws2812_pin & 0x0F);
    uint32_t set_mask = pin_mask;
    uint32_t reset_mask = pin_mask << 16;

    taskENTER_CRITICAL();

    // WS2812 reset latch low (>300 us)
    port->BSRR = reset_mask;
    hal_sleep_us(350);

    ws2812_send_stream(&port->BSRR, set_mask, reset_mask, ws2812_grb_stream, ws2812_len * 3);

    // Keep line low after transmission
    port->BSRR = reset_mask;

    taskEXIT_CRITICAL();
}
