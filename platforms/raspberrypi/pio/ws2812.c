// Adapted from code released under the BSD-3-Clause license
// supplied by Raspberry Pi (Trading) Ltd.
// Copyright (c) 2020 Raspberry Pi (Trading) Ltd.

#include "hardware/pio.h"
#include "WS2812.pio.h"
#include <stdint.h>
#include <string.h>

// Maximum supported LEDs for static buffer
#define WS2812_MAX_LEDS 64

static uint32_t ws2812_buf[WS2812_MAX_LEDS];
static uint32_t ws2812_len = 0;
static int ws2812_sm = -1;

void ws2812_init(uint32_t pin, uint32_t length) {
    ws2812_len = (length <= WS2812_MAX_LEDS) ? length : WS2812_MAX_LEDS;
    memset(ws2812_buf, 0, sizeof(ws2812_buf));

    // Claim state machine and load PIO program once
    if (ws2812_sm < 0) {
        uint offset = pio_add_program(pio0, &ws2812_program);
        ws2812_sm = pio_claim_unused_sm(pio0, true);
        ws2812_program_init(pio0, (uint)ws2812_sm, offset, (uint)pin, 800000, 24);
    }
}

void ws2812_set_color(uint32_t index, uint32_t color) {
    if (index >= ws2812_len) {
        return;
    }

    // Input: standard 0xRRGGBB
    // WS2812 expects GRB order, packed as G<<24|R<<16|B<<8 (shifted left for 24-bit out)
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    ws2812_buf[index] = ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8);
}

void ws2812_show(void) {
    if (ws2812_sm < 0) {
        return;
    }
    for (uint32_t i = 0; i < ws2812_len; i++) {
        pio_sm_put_blocking(pio0, (uint)ws2812_sm, ws2812_buf[i]);
    }
}
