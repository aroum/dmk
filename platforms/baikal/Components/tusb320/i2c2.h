/**
 * *****************************************************************************
 *  @file       BSP/Components/tusb320/i2c2.h
 *  @author     Baikal electronics SDK team
 *  @brief      I2C2 driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __I2C2_H
#define __I2C2_H

#include <stdint.h>

/** @brief Initialize I2C2 interface in master mode. */
void bsp_i2c2_init(uint8_t saddr);

/** @brief Write data to I2C2 slave. */
uint32_t bsp_i2c2_write(uint8_t *buf, uint32_t len);

/** @brief Read data from I2C2 slave. */
uint32_t bsp_i2c2_read(uint8_t *buf, uint32_t len);

/** @brief Flush I2C2 Rx FIFO. */
void bsp_i2c2_flush(void);

#endif /* __I2C2_H */
