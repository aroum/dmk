#ifndef HAL_SPI_H
#define HAL_SPI_H

#include "pin_defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_SPI_MODE_0 0 // CPOL = 0, CPHA = 0
#define HAL_SPI_MODE_1 1 // CPOL = 0, CPHA = 1
#define HAL_SPI_MODE_2 2 // CPOL = 1, CPHA = 0
#define HAL_SPI_MODE_3 3 // CPOL = 1, CPHA = 1

#define HAL_SPI_FREQ_1MHZ 1000000U
#define HAL_SPI_FREQ_4MHZ 4000000U
#define HAL_SPI_FREQ_8MHZ 8000000U
#define HAL_SPI_FREQ_10MHZ 10000000U

/**
 * @brief Initialize SPI master bus.
 * @param sck Hardware clock pin identifier
 * @param mosi Hardware MOSI pin identifier
 * @param miso Hardware MISO pin identifier (use 0xFF or invalid pin if not needed)
 * @param freq_hz Clock speed in Hz
 * @param mode SPI Mode (HAL_SPI_MODE_0 .. HAL_SPI_MODE_3)
 * @return true on success, false on failure
 */
bool hal_spi_init(pin_t sck, pin_t mosi, pin_t miso, uint32_t freq_hz, uint8_t mode);

/**
 * @brief Transmit data over SPI (MISO discarded).
 * @param tx Pointer to transmit data
 * @param len Number of bytes
 * @return true on success, false on error
 */
bool hal_spi_write(const uint8_t *tx, size_t len);

/**
 * @brief Receive data over SPI by sending a dummy byte (e.g. 0x00 or 0xFF).
 * @param rx_fill Dummy byte sent during reception
 * @param rx Destination buffer
 * @param len Number of bytes to receive
 * @return true on success, false on error
 */
bool hal_spi_read(uint8_t rx_fill, uint8_t *rx, size_t len);

/**
 * @brief Full-duplex simultaneous transmit and receive.
 * @param tx Transmit buffer
 * @param rx Receive buffer
 * @param len Number of bytes
 * @return true on success, false on error
 */
bool hal_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len);

#ifdef __cplusplus
}
#endif

#endif // HAL_SPI_H
