#include "hal_i2c.h"
#include "hal_gpio.h"
#include <string.h>

// =============================================================================
// 1. Raspberry Pi RP2040 / RP2350 (Hardware I2C)
// =============================================================================
#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/i2c.h"
#include "hardware/gpio.h"

static i2c_inst_t *s_i2c = NULL;

bool hal_i2c_init(pin_t sda, pin_t scl, uint32_t freq_hz) {
    s_i2c = ((sda >> 1) & 1) ? i2c1 : i2c0;
    i2c_init(s_i2c, freq_hz ? freq_hz : HAL_I2C_FREQ_STANDARD);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
    return true;
}

bool hal_i2c_write(uint8_t addr, const uint8_t *data, size_t len) {
    if (!s_i2c || !data || len == 0) return false;
    int res = i2c_write_blocking(s_i2c, addr, data, len, false);
    return (res == (int)len);
}

bool hal_i2c_read(uint8_t addr, uint8_t *data, size_t len) {
    if (!s_i2c || !data || len == 0) return false;
    int res = i2c_read_blocking(s_i2c, addr, data, len, false);
    return (res == (int)len);
}

bool hal_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) {
    if (!s_i2c) return false;
    if (len == 0 || !data) {
        return hal_i2c_write(addr, &reg, 1);
    }
    uint8_t buf[32];
    if (len + 1 <= sizeof(buf)) {
        buf[0] = reg;
        memcpy(&buf[1], data, len);
        int res = i2c_write_blocking(s_i2c, addr, buf, len + 1, false);
        return (res == (int)(len + 1));
    } else {
        int res = i2c_write_blocking(s_i2c, addr, &reg, 1, true);
        if (res != 1) return false;
        res = i2c_write_blocking(s_i2c, addr, data, len, false);
        return (res == (int)len);
    }
}

bool hal_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    if (!s_i2c || !data || len == 0) return false;
    int res = i2c_write_blocking(s_i2c, addr, &reg, 1, true);
    if (res != 1) return false;
    res = i2c_read_blocking(s_i2c, addr, data, len, false);
    return (res == (int)len);
}

// =============================================================================
// 2. Nordic nRF52840 (Hardware TWIM0 with EasyDMA)
// =============================================================================
#elif defined(MCU_nrf52840)
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_twim.h"

static bool s_twim_inited = false;
static uint8_t s_twim_ram_buf[64];

bool hal_i2c_init(pin_t sda, pin_t scl, uint32_t freq_hz) {
    nrf_twim_disable(NRF_TWIM0);
    nrf_twim_pins_set(NRF_TWIM0, scl, sda);

    nrf_twim_frequency_t freq = NRF_TWIM_FREQ_100K;
    if (freq_hz >= 400000U) {
        freq = NRF_TWIM_FREQ_400K;
    } else if (freq_hz >= 250000U) {
        freq = NRF_TWIM_FREQ_250K;
    }
    nrf_twim_frequency_set(NRF_TWIM0, freq);
    nrf_twim_enable(NRF_TWIM0);
    s_twim_inited = true;
    return true;
}

bool hal_i2c_write(uint8_t addr, const uint8_t *data, size_t len) {
    if (!s_twim_inited || !data || len == 0) return false;
    if (len > sizeof(s_twim_ram_buf)) len = sizeof(s_twim_ram_buf);
    memcpy(s_twim_ram_buf, data, len);

    nrf_twim_address_set(NRF_TWIM0, addr);
    nrf_twim_tx_buffer_set(NRF_TWIM0, s_twim_ram_buf, len);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_ERROR);
    nrf_twim_shorts_set(NRF_TWIM0, NRF_TWIM_SHORT_LASTTX_STOP_MASK);
    nrf_twim_task_trigger(NRF_TWIM0, NRF_TWIM_TASK_STARTTX);

    uint32_t timeout = 50000;
    while (!nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED) &&
           !nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_ERROR) && --timeout) {
        nrf_delay_us(1);
    }
    bool error = nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_ERROR) || (timeout == 0);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_ERROR);
    if (error) {
        nrf_twim_errorsrc_get_and_clear(NRF_TWIM0);
        return false;
    }
    return (nrf_twim_txd_amount_get(NRF_TWIM0) == len);
}

bool hal_i2c_read(uint8_t addr, uint8_t *data, size_t len) {
    if (!s_twim_inited || !data || len == 0) return false;
    nrf_twim_address_set(NRF_TWIM0, addr);
    nrf_twim_rx_buffer_set(NRF_TWIM0, data, len);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_ERROR);
    nrf_twim_shorts_set(NRF_TWIM0, NRF_TWIM_SHORT_LASTRX_STOP_MASK);
    nrf_twim_task_trigger(NRF_TWIM0, NRF_TWIM_TASK_STARTRX);

    uint32_t timeout = 50000;
    while (!nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED) &&
           !nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_ERROR) && --timeout) {
        nrf_delay_us(1);
    }
    bool error = nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_ERROR) || (timeout == 0);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_ERROR);
    if (error) {
        nrf_twim_errorsrc_get_and_clear(NRF_TWIM0);
        return false;
    }
    return (nrf_twim_rxd_amount_get(NRF_TWIM0) == len);
}

bool hal_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) {
    if (len + 1 > sizeof(s_twim_ram_buf)) return false;
    s_twim_ram_buf[0] = reg;
    if (len > 0 && data) {
        memcpy(&s_twim_ram_buf[1], data, len);
    }
    return hal_i2c_write(addr, s_twim_ram_buf, len + 1);
}

bool hal_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    if (!s_twim_inited || !data || len == 0) return false;
    s_twim_ram_buf[0] = reg;

    nrf_twim_address_set(NRF_TWIM0, addr);
    nrf_twim_tx_buffer_set(NRF_TWIM0, s_twim_ram_buf, 1);
    nrf_twim_rx_buffer_set(NRF_TWIM0, data, len);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_ERROR);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_LASTTX);
    nrf_twim_shorts_set(NRF_TWIM0, NRF_TWIM_SHORT_LASTTX_STARTRX_MASK | NRF_TWIM_SHORT_LASTRX_STOP_MASK);
    nrf_twim_task_trigger(NRF_TWIM0, NRF_TWIM_TASK_STARTTX);

    uint32_t timeout = 50000;
    while (!nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED) &&
           !nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_ERROR) && --timeout) {
        nrf_delay_us(1);
    }
    bool error = nrf_twim_event_check(NRF_TWIM0, NRF_TWIM_EVENT_ERROR) || (timeout == 0);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(NRF_TWIM0, NRF_TWIM_EVENT_ERROR);
    if (error) {
        nrf_twim_errorsrc_get_and_clear(NRF_TWIM0);
        return false;
    }
    return (nrf_twim_rxd_amount_get(NRF_TWIM0) == len);
}

// =============================================================================
// 3. Universal GPIO Bit-Bang (Milandr, Baikal, or generic platforms)
// =============================================================================
#elif defined(MCU_milandr) || defined(MCU_baikal)

static pin_t s_bb_sda = 0xFF;
static pin_t s_bb_scl = 0xFF;
static uint32_t s_bb_delay_us = 4; // ~100kHz standard mode

static inline void i2c_bb_scl_hi(void) {
    hal_gpio_set_dir(s_bb_scl, false); // release (pull up)
    hal_sleep_us(s_bb_delay_us);
}

static inline void i2c_bb_scl_lo(void) {
    hal_gpio_put(s_bb_scl, false);
    hal_gpio_set_dir(s_bb_scl, true); // drive low
    hal_sleep_us(s_bb_delay_us);
}

static inline void i2c_bb_sda_hi(void) {
    hal_gpio_set_dir(s_bb_sda, false); // release (pull up)
    hal_sleep_us(s_bb_delay_us);
}

static inline void i2c_bb_sda_lo(void) {
    hal_gpio_put(s_bb_sda, false);
    hal_gpio_set_dir(s_bb_sda, true); // drive low
    hal_sleep_us(s_bb_delay_us);
}

static inline bool i2c_bb_sda_read(void) {
    return hal_gpio_get(s_bb_sda);
}

static void i2c_bb_start(void) {
    i2c_bb_sda_hi();
    i2c_bb_scl_hi();
    i2c_bb_sda_lo();
    i2c_bb_scl_lo();
}

static void i2c_bb_stop(void) {
    i2c_bb_sda_lo();
    i2c_bb_scl_hi();
    i2c_bb_sda_hi();
}

static bool i2c_bb_write_byte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        if ((byte >> i) & 1) {
            i2c_bb_sda_hi();
        } else {
            i2c_bb_sda_lo();
        }
        i2c_bb_scl_hi();
        i2c_bb_scl_lo();
    }
    // Sample ACK
    i2c_bb_sda_hi();
    i2c_bb_scl_hi();
    bool ack = !i2c_bb_sda_read();
    i2c_bb_scl_lo();
    return ack;
}

static uint8_t i2c_bb_read_byte(bool send_ack) {
    uint8_t byte = 0;
    i2c_bb_sda_hi();
    for (int i = 7; i >= 0; i--) {
        i2c_bb_scl_hi();
        if (i2c_bb_sda_read()) {
            byte |= (1 << i);
        }
        i2c_bb_scl_lo();
    }
    if (send_ack) {
        i2c_bb_sda_lo();
    } else {
        i2c_bb_sda_hi();
    }
    i2c_bb_scl_hi();
    i2c_bb_scl_lo();
    i2c_bb_sda_hi();
    return byte;
}

bool hal_i2c_init(pin_t sda, pin_t scl, uint32_t freq_hz) {
    s_bb_sda = sda;
    s_bb_scl = scl;
    if (freq_hz >= 400000U) {
        s_bb_delay_us = 1;
    } else {
        s_bb_delay_us = 4;
    }

    hal_gpio_init(sda);
    hal_gpio_init(scl);
    hal_gpio_pull_up(sda);
    hal_gpio_pull_up(scl);
    i2c_bb_sda_hi();
    i2c_bb_scl_hi();
    return true;
}

bool hal_i2c_write(uint8_t addr, const uint8_t *data, size_t len) {
    if (s_bb_sda == 0xFF || !data || len == 0) return false;
    i2c_bb_start();
    if (!i2c_bb_write_byte((addr << 1) | 0)) {
        i2c_bb_stop();
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (!i2c_bb_write_byte(data[i])) {
            i2c_bb_stop();
            return false;
        }
    }
    i2c_bb_stop();
    return true;
}

bool hal_i2c_read(uint8_t addr, uint8_t *data, size_t len) {
    if (s_bb_sda == 0xFF || !data || len == 0) return false;
    i2c_bb_start();
    if (!i2c_bb_write_byte((addr << 1) | 1)) {
        i2c_bb_stop();
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        bool ack = (i + 1 < len);
        data[i] = i2c_bb_read_byte(ack);
    }
    i2c_bb_stop();
    return true;
}

bool hal_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) {
    if (s_bb_sda == 0xFF) return false;
    i2c_bb_start();
    if (!i2c_bb_write_byte((addr << 1) | 0)) {
        i2c_bb_stop();
        return false;
    }
    if (!i2c_bb_write_byte(reg)) {
        i2c_bb_stop();
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (!i2c_bb_write_byte(data[i])) {
            i2c_bb_stop();
            return false;
        }
    }
    i2c_bb_stop();
    return true;
}

bool hal_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    if (s_bb_sda == 0xFF || !data || len == 0) return false;
    i2c_bb_start();
    if (!i2c_bb_write_byte((addr << 1) | 0)) {
        i2c_bb_stop();
        return false;
    }
    if (!i2c_bb_write_byte(reg)) {
        i2c_bb_stop();
        return false;
    }
    // Repeated start
    i2c_bb_start();
    if (!i2c_bb_write_byte((addr << 1) | 1)) {
        i2c_bb_stop();
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        bool ack = (i + 1 < len);
        data[i] = i2c_bb_read_byte(ack);
    }
    i2c_bb_stop();
    return true;
}

// =============================================================================
// 4. Mock / Host build
// =============================================================================
#else
bool hal_i2c_init(pin_t sda, pin_t scl, uint32_t freq_hz) { (void)sda; (void)scl; (void)freq_hz; return true; }
bool hal_i2c_write(uint8_t addr, const uint8_t *data, size_t len) { (void)addr; (void)data; (void)len; return true; }
bool hal_i2c_read(uint8_t addr, uint8_t *data, size_t len) { (void)addr; (void)data; (void)len; return true; }
bool hal_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) { (void)addr; (void)reg; (void)data; (void)len; return true; }
bool hal_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) { (void)addr; (void)reg; (void)data; (void)len; return true; }
#endif
