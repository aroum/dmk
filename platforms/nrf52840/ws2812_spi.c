#include "nrf_gpio.h"
#include "nrfx_spim.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Maximum supported WS2812 LEDs
#define WS2812_MAX_LEDS 64

// 300us reset pulse at 4MHz SPI (each byte takes 2us) to support all WS2812B variants
#define WS2812_RESET_BYTES 150

static uint32_t ws2812_colors[WS2812_MAX_LEDS];
static uint32_t ws2812_len = 0;

// Use SPIM1 instance for the WS2812 driver
#define WS2812_SPI_INSTANCE 1
static const nrfx_spim_t m_spi = NRFX_SPIM_INSTANCE(WS2812_SPI_INSTANCE);

// SPI buffer: 15 bytes per LED + reset bytes
static uint8_t m_spi_tx_buf[15 * WS2812_MAX_LEDS + WS2812_RESET_BYTES];

static void write_5_bits(uint32_t *bit_idx, uint8_t pattern) {
    for (int i = 0; i < 5; i++) {
        uint32_t byte_pos = (*bit_idx) / 8;
        uint32_t bit_pos = 7 - ((*bit_idx) % 8); // SPI shifts MSB first
        bool bit_val = (pattern >> (4 - i)) & 1;
        if (bit_val) {
            m_spi_tx_buf[byte_pos] |= (1 << bit_pos);
        } else {
            m_spi_tx_buf[byte_pos] &= ~(1 << bit_pos);
        }
        (*bit_idx)++;
    }
}

void ws2812_init(uint32_t pin, uint32_t length) {
    ws2812_len = (length <= WS2812_MAX_LEDS) ? length : WS2812_MAX_LEDS;

    // Clear local buffers
    memset(ws2812_colors, 0, sizeof(ws2812_colors));
    memset(m_spi_tx_buf, 0, sizeof(m_spi_tx_buf));

    // Configure SPIM1
    nrfx_spim_config_t config = NRFX_SPIM_DEFAULT_CONFIG;

    // Use P0_27 as dummy SCK pin (required by hardware to enable SPIM)
    config.sck_pin = 27;
    config.mosi_pin = pin;
    config.miso_pin = NRFX_SPIM_PIN_NOT_USED;
    config.ss_pin = NRFX_SPIM_PIN_NOT_USED;
    config.frequency = NRF_SPIM_FREQ_4M;
    config.orc = 0x00;

    nrfx_spim_init(&m_spi, &config, NULL, NULL);

    // Override MOSI pin to use High Drive (H0H1) for fast rise/fall times required by WS2812
    nrf_gpio_cfg(pin, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_H0H1, // High drive '0', High drive '1'
                 NRF_GPIO_PIN_NOSENSE);
}

void ws2812_set_color(uint32_t index, uint32_t color) {
    if (index >= ws2812_len)
        return;
    ws2812_colors[index] = color;
}

void ws2812_show(void) {
    uint32_t bit_idx = 0;

    // Clear buffer (non-written bits remain low as reset states)
    memset(m_spi_tx_buf, 0, sizeof(m_spi_tx_buf));

    for (uint32_t i = 0; i < ws2812_len; i++) {
        uint32_t color = ws2812_colors[i];
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        // Pack as GRB order, MSB-first
        uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
        for (int bit = 23; bit >= 0; bit--) {
            bool is_one = (grb >> bit) & 1;
            // 0x1C -> 11100 (high for 750ns, low for 500ns)
            // 0x10 -> 10000 (high for 250ns, low for 1000ns)
            write_5_bits(&bit_idx, is_one ? 0x1C : 0x10);
        }
    }

    uint32_t total_bytes = (bit_idx + 7) / 8;
    total_bytes += WS2812_RESET_BYTES;
    if (total_bytes > sizeof(m_spi_tx_buf)) {
        total_bytes = sizeof(m_spi_tx_buf);
    }

    // Trigger EasyDMA transfer
    nrfx_spim_xfer_desc_t xfer = NRFX_SPIM_XFER_TX(m_spi_tx_buf, total_bytes);
    nrfx_spim_xfer(&m_spi, &xfer, 0);
}
