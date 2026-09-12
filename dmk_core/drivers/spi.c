#include "hal_gpio.h"
#include "hal_spi.h"
#include <string.h>

// =============================================================================
// 1. Raspberry Pi RP2040 / RP2350 (Hardware SPI)
// =============================================================================
#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/gpio.h"
#include "hardware/spi.h"

static spi_inst_t *s_spi = NULL;

bool hal_spi_init(pin_t sck, pin_t mosi, pin_t miso, uint32_t freq_hz, uint8_t mode) {
    s_spi = ((sck >> 3) & 1) ? spi1 : spi0;
    spi_init(s_spi, freq_hz ? freq_hz : HAL_SPI_FREQ_4MHZ);

    spi_cpol_t cpol = (mode == HAL_SPI_MODE_2 || mode == HAL_SPI_MODE_3) ? SPI_CPOL_1 : SPI_CPOL_0;
    spi_cpha_t cpha = (mode == HAL_SPI_MODE_1 || mode == HAL_SPI_MODE_3) ? SPI_CPHA_1 : SPI_CPHA_0;
    spi_set_format(s_spi, 8, cpol, cpha, SPI_MSB_FIRST);

    gpio_set_function(sck, GPIO_FUNC_SPI);
    if (mosi != 0xFF && mosi != 255) {
        gpio_set_function(mosi, GPIO_FUNC_SPI);
    }
    if (miso != 0xFF && miso != 255) {
        gpio_set_function(miso, GPIO_FUNC_SPI);
    }
    return true;
}

bool hal_spi_write(const uint8_t *tx, size_t len) {
    if (!s_spi || !tx || len == 0)
        return false;
    int res = spi_write_blocking(s_spi, tx, len);
    return (res == (int)len);
}

bool hal_spi_read(uint8_t rx_fill, uint8_t *rx, size_t len) {
    if (!s_spi || !rx || len == 0)
        return false;
    int res = spi_read_blocking(s_spi, rx_fill, rx, len);
    return (res == (int)len);
}

bool hal_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len) {
    if (!s_spi || len == 0)
        return false;
    if (tx && rx) {
        int res = spi_write_read_blocking(s_spi, tx, rx, len);
        return (res == (int)len);
    } else if (tx) {
        return hal_spi_write(tx, len);
    } else if (rx) {
        return hal_spi_read(0x00, rx, len);
    }
    return false;
}

// =============================================================================
// 2. Nordic nRF52840 (Hardware SPIM2 with EasyDMA)
// =============================================================================
#elif defined(MCU_nrf52840)
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_spim.h"

static bool s_spim_inited = false;
static uint8_t s_spim_tx_buf[64];
static uint8_t s_spim_rx_buf[64];

bool hal_spi_init(pin_t sck, pin_t mosi, pin_t miso, uint32_t freq_hz, uint8_t mode) {
    nrf_spim_disable(NRF_SPIM2);
    uint32_t miso_pin = (miso != 0xFF && miso != 255) ? miso : NRF_SPIM_PIN_NOT_CONNECTED;
    uint32_t mosi_pin = (mosi != 0xFF && mosi != 255) ? mosi : NRF_SPIM_PIN_NOT_CONNECTED;
    nrf_spim_pins_set(NRF_SPIM2, sck, mosi_pin, miso_pin);

    nrf_spim_mode_t spim_mode;
    switch (mode) {
    case HAL_SPI_MODE_1:
        spim_mode = NRF_SPIM_MODE_1;
        break;
    case HAL_SPI_MODE_2:
        spim_mode = NRF_SPIM_MODE_2;
        break;
    case HAL_SPI_MODE_3:
        spim_mode = NRF_SPIM_MODE_3;
        break;
    default:
        spim_mode = NRF_SPIM_MODE_0;
        break;
    }
    nrf_spim_configure(NRF_SPIM2, spim_mode, NRF_SPIM_BIT_ORDER_MSB_FIRST);

    nrf_spim_frequency_t freq = NRF_SPIM_FREQ_4M;
    if (freq_hz >= 8000000U) {
        freq = NRF_SPIM_FREQ_8M;
    } else if (freq_hz >= 4000000U) {
        freq = NRF_SPIM_FREQ_4M;
    } else if (freq_hz >= 2000000U) {
        freq = NRF_SPIM_FREQ_2M;
    } else if (freq_hz >= 1000000U) {
        freq = NRF_SPIM_FREQ_1M;
    }
    nrf_spim_frequency_set(NRF_SPIM2, freq);
    nrf_spim_enable(NRF_SPIM2);
    s_spim_inited = true;
    return true;
}

bool hal_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len) {
    if (!s_spim_inited || len == 0)
        return false;
    if (len > sizeof(s_spim_tx_buf))
        len = sizeof(s_spim_tx_buf);

    if (tx) {
        memcpy(s_spim_tx_buf, tx, len);
        nrf_spim_tx_buffer_set(NRF_SPIM2, s_spim_tx_buf, len);
    } else {
        memset(s_spim_tx_buf, 0, len);
        nrf_spim_tx_buffer_set(NRF_SPIM2, s_spim_tx_buf, len);
    }

    nrf_spim_rx_buffer_set(NRF_SPIM2, s_spim_rx_buf, len);
    nrf_spim_event_clear(NRF_SPIM2, NRF_SPIM_EVENT_END);
    nrf_spim_task_trigger(NRF_SPIM2, NRF_SPIM_TASK_START);

    uint32_t timeout = 50000;
    while (!nrf_spim_event_check(NRF_SPIM2, NRF_SPIM_EVENT_END) && --timeout) {
        nrf_delay_us(1);
    }
    nrf_spim_event_clear(NRF_SPIM2, NRF_SPIM_EVENT_END);

    if (timeout > 0 && rx) {
        memcpy(rx, s_spim_rx_buf, len);
    }
    return (timeout > 0);
}

bool hal_spi_write(const uint8_t *tx, size_t len) {
    return hal_spi_transfer(tx, NULL, len);
}

bool hal_spi_read(uint8_t rx_fill, uint8_t *rx, size_t len) {
    (void)rx_fill;
    return hal_spi_transfer(NULL, rx, len);
}

// =============================================================================
// 3. Universal GPIO Bit-Bang (Milandr, Baikal, or generic platforms)
// =============================================================================
#elif defined(MCU_milandr) || defined(MCU_baikal)

static pin_t s_sck = 0xFF;
static pin_t s_mosi = 0xFF;
static pin_t s_miso = 0xFF;
static uint8_t s_mode = 0;
static uint32_t s_spi_dly_us = 1;

bool hal_spi_init(pin_t sck, pin_t mosi, pin_t miso, uint32_t freq_hz, uint8_t mode) {
    s_sck = sck;
    s_mosi = mosi;
    s_miso = miso;
    s_mode = mode;
    (void)freq_hz;

    hal_gpio_init(sck);
    hal_gpio_set_dir(sck, true);
    bool cpol = (mode == HAL_SPI_MODE_2 || mode == HAL_SPI_MODE_3);
    hal_gpio_put(sck, cpol);

    if (mosi != 0xFF && mosi != 255) {
        hal_gpio_init(mosi);
        hal_gpio_set_dir(mosi, true);
        hal_gpio_put(mosi, false);
    }
    if (miso != 0xFF && miso != 255) {
        hal_gpio_init(miso);
        hal_gpio_set_dir(miso, false);
        hal_gpio_pull_up(miso);
    }
    return true;
}

bool hal_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len) {
    if (s_sck == 0xFF || len == 0)
        return false;
    bool cpol = (s_mode == HAL_SPI_MODE_2 || s_mode == HAL_SPI_MODE_3);
    bool cpha = (s_mode == HAL_SPI_MODE_1 || s_mode == HAL_SPI_MODE_3);

    for (size_t i = 0; i < len; i++) {
        uint8_t txb = tx ? tx[i] : 0xFF;
        uint8_t rxb = 0;

        for (int bit = 7; bit >= 0; bit--) {
            bool out_bit = (txb >> bit) & 1;

            if (!cpha) {
                // CPOL: setup data before first edge
                if (s_mosi != 0xFF && s_mosi != 255)
                    hal_gpio_put(s_mosi, out_bit);
                hal_sleep_us(s_spi_dly_us);
                hal_gpio_put(s_sck, !cpol); // Leading edge
                hal_sleep_us(s_spi_dly_us);
                if (s_miso != 0xFF && s_miso != 255 && hal_gpio_get(s_miso)) {
                    rxb |= (1 << bit);
                }
                hal_gpio_put(s_sck, cpol); // Trailing edge
            } else {
                // CPHA=1: leading edge first, then data setup, sample on trailing edge
                hal_gpio_put(s_sck, !cpol);
                if (s_mosi != 0xFF && s_mosi != 255)
                    hal_gpio_put(s_mosi, out_bit);
                hal_sleep_us(s_spi_dly_us);
                hal_gpio_put(s_sck, cpol);
                hal_sleep_us(s_spi_dly_us);
                if (s_miso != 0xFF && s_miso != 255 && hal_gpio_get(s_miso)) {
                    rxb |= (1 << bit);
                }
            }
        }
        if (rx)
            rx[i] = rxb;
    }
    return true;
}

bool hal_spi_write(const uint8_t *tx, size_t len) {
    return hal_spi_transfer(tx, NULL, len);
}

bool hal_spi_read(uint8_t rx_fill, uint8_t *rx, size_t len) {
    (void)rx_fill;
    return hal_spi_transfer(NULL, rx, len);
}

// =============================================================================
// 4. Mock / Host build
// =============================================================================
#else
bool hal_spi_init(pin_t sck, pin_t mosi, pin_t miso, uint32_t freq_hz, uint8_t mode) {
    (void)sck;
    (void)mosi;
    (void)miso;
    (void)freq_hz;
    (void)mode;
    return true;
}
bool hal_spi_write(const uint8_t *tx, size_t len) {
    (void)tx;
    (void)len;
    return true;
}
bool hal_spi_read(uint8_t rx_fill, uint8_t *rx, size_t len) {
    (void)rx_fill;
    (void)rx;
    (void)len;
    return true;
}
bool hal_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len) {
    (void)tx;
    (void)rx;
    (void)len;
    return true;
}
#endif
