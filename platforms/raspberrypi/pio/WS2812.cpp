// Adapted from code released under the BSD-3-Clause license
// supplied by Raspberry Pi (Trading) Ltd.
// Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
// and code released under the GNU GPL-3 license
// supplied by ForsakenNGS
// Copyright (c) 2022 ForsakenNGS

#include "WS2812.hpp"
#include "WS2812.pio.h"

// #define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

WS2812::WS2812(uint pin, uint length, PIO pio, uint sm) {
    initialize(pin, length, pio, sm, NONE, GREEN, RED, BLUE);
}

WS2812::WS2812(uint pin, uint length, PIO pio, uint sm, DataFormat format) {
    switch (format) {
    case FORMAT_RGB:
        initialize(pin, length, pio, sm, NONE, RED, GREEN, BLUE);
        break;
    case FORMAT_GRB:
        initialize(pin, length, pio, sm, NONE, GREEN, RED, BLUE);
        break;
    case FORMAT_WRGB:
        initialize(pin, length, pio, sm, WHITE, RED, GREEN, BLUE);
        break;
    }
}

WS2812::WS2812(uint pin, uint length, PIO pio, uint sm, DataByte b1, DataByte b2, DataByte b3) {
    initialize(pin, length, pio, sm, b1, b1, b2, b3);
}

WS2812::WS2812(uint pin, uint length, PIO pio, uint sm, DataByte b1, DataByte b2, DataByte b3, DataByte b4) {
    initialize(pin, length, pio, sm, b1, b2, b3, b4);
}

WS2812::~WS2812() {}

void WS2812::initialize(uint pin, uint length, PIO pio, uint sm, DataByte b1, DataByte b2, DataByte b3, DataByte b4) {
    this->pin = pin;
    this->length = length;
    this->pio = pio;
    this->sm = sm;
    this->data = new uint32_t[length];
    this->bytes[0] = b1;
    this->bytes[1] = b2;
    this->bytes[2] = b3;
    this->bytes[3] = b4;
    uint offset = pio_add_program(pio, &ws2812_program);
    uint bits = (b1 == NONE ? 24 : 32);
#ifdef DEBUG
    printf("WS2812 / Initializing SM %u with offset %X at pin %u and %u data bits...\n", sm, offset, pin, bits);
#endif
    ws2812_program_init(pio, sm, offset, pin, 800000, bits);
}

uint32_t WS2812::convertData(uint32_t rgbw) {
    uint32_t result = 0;
    for (uint b = 0; b < 4; b++) {
        switch (bytes[b]) {
        case RED:
            result |= (rgbw & 0xFF);
            break;
        case GREEN:
            result |= (rgbw & 0xFF00) >> 8;
            break;
        case BLUE:
            result |= (rgbw & 0xFF0000) >> 16;
            break;
        case WHITE:
            result |= (rgbw & 0xFF000000) >> 24;
            break;
        }
        result <<= 8;
    }
    return result;
}

void WS2812::setPixelColor(uint index, uint32_t color) {
    if (index < length) {
        data[index] = convertData(color);
    }
}

void WS2812::setPixelColor(uint index, uint8_t red, uint8_t green, uint8_t blue) {
    setPixelColor(index, RGB(red, green, blue));
}

void WS2812::setPixelColor(uint index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
    setPixelColor(index, RGBW(red, green, blue, white));
}

void WS2812::fill(uint32_t color) {
    fill(color, 0, length);
}

void WS2812::fill(uint32_t color, uint first) {
    fill(color, first, length - first);
}

void WS2812::fill(uint32_t color, uint first, uint count) {
    uint last = (first + count);
    if (last > length) {
        last = length;
    }
    color = convertData(color);
    for (uint i = first; i < last; i++) {
        data[i] = color;
    }
}

void WS2812::show() {
#ifdef DEBUG
    for (uint i = 0; i < length; i++) {
        printf("WS2812 / Put data: %08X\n", data[i]);
    }
#endif
    for (uint i = 0; i < length; i++) {
        pio_sm_put_blocking(pio, sm, data[i]);
    }
}

// ---- C wrappers: static implementation, no heap allocation ----
// Maximum supported LEDs for static buffer
#define WS2812_MAX_LEDS 64

static uint32_t ws2812_buf[WS2812_MAX_LEDS];
static uint32_t ws2812_len = 0;
static uint ws2812_sm = 0;

extern "C" void ws2812_init(uint32_t pin, uint32_t length) {
    ws2812_len = (length <= WS2812_MAX_LEDS) ? length : WS2812_MAX_LEDS;
    ws2812_sm = 0;

    // Clear pixel buffer
    for (uint32_t i = 0; i < WS2812_MAX_LEDS; i++)
        ws2812_buf[i] = 0;

    // Load and start PIO program
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, ws2812_sm, offset, (uint)pin, 800000, 24);
}

extern "C" void ws2812_set_color(uint32_t index, uint32_t color) {
    if (index >= ws2812_len)
        return;

    // Input: standard 0xRRGGBB
    // WS2812 expects GRB order, packed as G<<24|R<<16|B<<8 (shifted left for 24-bit out)
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = (color) & 0xFF;
    // Pack as GRB, MSB-first, shifted to top 24 bits for PIO out shift
    ws2812_buf[index] = ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8);
}

extern "C" void ws2812_show(void) {
    for (uint32_t i = 0; i < ws2812_len; i++) {
        pio_sm_put_blocking(pio0, ws2812_sm, ws2812_buf[i]);
    }
}