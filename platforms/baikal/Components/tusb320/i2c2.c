/**
 * *****************************************************************************
 *  @file       BSP/Components/tusb320/i2c2.c
 *  @author     Baikal electronics SDK team
 *  @brief      I2C2 driver source file
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

#include "i2c2.h"
#include "bmcu_cru.h"
#include "bmcu_i2c.h"

#if defined(EVU_BA_2_5)
#include "../../EVU_BA_2_5/bsp.h"
#else
#error "The selected development board does not support I2C2 interface"
#endif

#define I2C_TIMEOUT (1000UL)

/**
 * @brief Initialize I2C2 interface in master mode.
 * @param saddr The slave address.
 */
void bsp_i2c2_init(uint8_t saddr) {
    /* Enable I2C clock */
    BSP_I2C2_CLK_EN_FN(BSP_I2C2_CLK_PERIPH);

    /* SCL */
    CRU_PIN_InitStruct_TypeDef CRU_PIN_InitStruct;
    CRU_PIN_StructInit(&CRU_PIN_InitStruct);
    CRU_PIN_InitStruct.Port = BSP_I2C2_SCL_CRU_PORT;
    CRU_PIN_InitStruct.Pin = BSP_I2C2_SCL_CRU_PIN;
    CRU_PIN_InitStruct.Pull = CRU_PIN_PULL_NO;
    CRU_PIN_InitStruct.Alternate = BSP_I2C2_CRU_PIN_AF;
    CRU_PIN_Init(&CRU_PIN_InitStruct);

    /* SDA */
    CRU_PIN_InitStruct.Port = BSP_I2C2_SDA_CRU_PORT;
    CRU_PIN_InitStruct.Pin = BSP_I2C2_SDA_CRU_PIN;
    CRU_PIN_Init(&CRU_PIN_InitStruct);

    /* Reset I2C data registers */
    while (I2C_DeInit(BSP_I2C2_INSTANCE) != SUCCESS) {
        __delay_ms(100UL);
    }

    /* Init I2C */
    I2C_InitStruct_TypeDef I2C_InitStruct;
    I2C_StructInit(&I2C_InitStruct);

    I2C_InitStruct.PeripheralMode = I2C_MODE_MASTER;
    I2C_InitStruct.Speed = I2C_CON_SPEED_FAST;
    I2C_InitStruct.OwnAddrSize = I2C_ADDR_7BITS;
    I2C_InitStruct.TargetAddress = saddr;
    I2C_InitStruct.SCL_LCNT = 120U;
    I2C_InitStruct.SCL_HCNT = 120U;
    I2C_InitStruct.TxFIFOThreshold = 0U;
    I2C_InitStruct.RxFIFOThreshold = 0U;

    I2C_Init(BSP_I2C2_INSTANCE, &I2C_InitStruct);

    /* Enable I2C */
    I2C_Enable(BSP_I2C2_INSTANCE);
}

/**
 * @brief Write data to I2C2 slave.
 * @param buf The pointer to the data buffer.
 * @param len The data length.
 * @returns The number of bytes written.
 */
uint32_t bsp_i2c2_write(uint8_t *buf, uint32_t len) {
    uint32_t timeout = I2C_TIMEOUT;
    uint32_t i = 0UL;

    while ((i < len) && (timeout > 0UL)) {
        /* Write data */
        if (I2C_IsActiveFlag(BSP_I2C2_INSTANCE, I2C_FLAG_TFNF) == 1UL) {
            I2C_TransmitData(BSP_I2C2_INSTANCE, buf[i], I2C_FLAG_CMD_WRITE);
            i++;
        }

        timeout--;
    }

    while (I2C_IsActiveFlag(BSP_I2C2_INSTANCE, I2C_FLAG_TFE) != 1UL)
        ;

    return i;
}

/**
 * @brief Read data from I2C2 slave.
 * @param buf The pointer to the data buffer.
 * @param len The data length.
 * @returns The number of bytes read.
 */
uint32_t bsp_i2c2_read(uint8_t *buf, uint32_t len) {
    uint32_t timeout = I2C_TIMEOUT;
    uint32_t i = 0UL;

    /* Issue read command */
    while ((i < len) && (timeout > 0UL)) {
        if (I2C_IsActiveFlag(BSP_I2C2_INSTANCE, I2C_FLAG_TFNF) == 1UL) {
            I2C_TransmitData(BSP_I2C2_INSTANCE, 0x0U, I2C_FLAG_CMD_READ);
            i++;
        }

        timeout--;
    }

    while (I2C_IsActiveFlag(BSP_I2C2_INSTANCE, I2C_FLAG_TFE) != 1UL)
        ;

    /* Read data */
    i = 0UL;
    if (timeout > 0UL) {
        timeout = I2C_TIMEOUT;
        while ((i < len) && (timeout > 0UL)) {
            if (I2C_IsActiveFlag(BSP_I2C2_INSTANCE, I2C_FLAG_RFNE) == 1UL) {
                buf[i] = I2C_ReceiveData(BSP_I2C2_INSTANCE);
                i++;
            }

            timeout--;
        }
    }

    return i;
}

/**
 * @brief Flush I2C2 Rx FIFO.
 */
void bsp_i2c2_flush(void) {
    volatile uint8_t tmp;

    while (I2C_IsActiveFlag(BSP_I2C2_INSTANCE, I2C_FLAG_RFNE) == 1UL) {
        tmp = I2C_ReceiveData(BSP_I2C2_INSTANCE);
    }

    (void)tmp;
}
