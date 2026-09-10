#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "pin_defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_I2C_FREQ_STANDARD 100000U
#define HAL_I2C_FREQ_FAST     400000U

/**
 * @brief Initialize I2C master bus with specified SDA and SCL pins.
 * @param sda Hardware SDA pin identifier (pin_t)
 * @param scl Hardware SCL pin identifier (pin_t)
 * @param freq_hz Clock frequency in Hz (e.g., HAL_I2C_FREQ_STANDARD, HAL_I2C_FREQ_FAST)
 * @return true on success, false on failure
 */
bool hal_i2c_init(pin_t sda, pin_t scl, uint32_t freq_hz);

/**
 * @brief Transmit data over I2C to a slave address.
 * @param addr 7-bit slave address
 * @param data Pointer to buffer to transmit
 * @param len Number of bytes to transmit
 * @return true on success (ACK received), false on NACK or bus error
 */
bool hal_i2c_write(uint8_t addr, const uint8_t *data, size_t len);

/**
 * @brief Receive data over I2C from a slave address.
 * @param addr 7-bit slave address
 * @param data Destination buffer
 * @param len Number of bytes to receive
 * @return true on success, false on error
 */
bool hal_i2c_read(uint8_t addr, uint8_t *data, size_t len);

/**
 * @brief Write to a specific register on an I2C device.
 * @param addr 7-bit slave address
 * @param reg Register address/offset
 * @param data Pointer to buffer with payload data
 * @param len Number of data bytes
 * @return true on success, false on error
 */
bool hal_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len);

/**
 * @brief Read from a specific register on an I2C device.
 * @param addr 7-bit slave address
 * @param reg Register address/offset
 * @param data Destination buffer
 * @param len Number of data bytes to read
 * @return true on success, false on error
 */
bool hal_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // HAL_I2C_H
