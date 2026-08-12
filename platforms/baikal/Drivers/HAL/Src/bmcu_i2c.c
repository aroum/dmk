/**
 * *****************************************************************************
 *  @file       bmcu_i2c.c
 *  @author     Baikal electronics SDK team
 *  @brief      Inter-Integrated Circuit (I2C) module driver
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

#include "bmcu_i2c.h"
#include "bmcu_def.h"

#define I2C_DEFAULT_FS_SPKLEN                   5U      /*!< Default spike suppression limit in SS, FS and FS Plus modes */
#define I2C_DEFAULT_HS_SPKLEN                   1U      /*!< Default spike suppression limit in HS mode */

#define I2C_DEFAULT_TAR_ADDR                    0x55U   /*!< Default target address */
#define I2C_DEFAULT_SLAVE_ADDR                  0x55U   /*!< Default slave address */

#define I2C_DEFAULT_MASTER_CODE                 0x01U   /*!< Default HS mode master code */

#define I2C_DEFAULT_TX_TL                       8U      /*!< Default Tx FIFO threshold level */
#define I2C_DEFAULT_RX_TL                       8U      /*!< Default Rx FIFO threshold level */

#define I2C_DEFAULT_SS_SCL_HCNT                 0x0190U /*!< Default Standard Speed SCL High Count */
#define I2C_DEFAULT_SS_SCL_LCNT                 0x01D6U /*!< Default Standard Speed SCL Low Count */
#define I2C_DEFAULT_FS_SCL_HCNT                 0x003CU /*!< Default Fast Speed SCL High Count */
#define I2C_DEFAULT_FS_SCL_LCNT                 0x0082U /*!< Default Fast Speed SCL Low Count */
#define I2C_DEFAULT_HS_SCL_HCNT                 0x0006U /*!< Default High Speed SCL High Count */
#define I2C_DEFAULT_HS_SCL_LCNT                 0x0010U /*!< Default High Speed SCL Low Count */

#define IS_I2C_PERIPHERAL_MODE(__VALUE__)       (((__VALUE__) == I2C_MODE_MASTER) || \
                                                 ((__VALUE__) == I2C_MODE_SLAVE))

#define IS_I2C_SPEED(__VALUE__)                 (((__VALUE__) == I2C_SPEED_STANDARD) || \
                                                 ((__VALUE__) == I2C_SPEED_FAST) || \
                                                 ((__VALUE__) == I2C_SPEED_HIGH))

#define IS_I2C_OWN_ADDRESS1(__VALUE__)          ((__VALUE__) <= 0x3FFU)

#define IS_I2C_OWN_ADDRSIZE(__VALUE__)          (((__VALUE__) == I2C_ADDR_7BITS) || \
                                                 ((__VALUE__) == I2C_ADDR_10BITS))

#define IS_I2C_MASTER_CODE(__VALUE__)           ((__VALUE__) <= 0x7U)

#define IS_I2C_TARGET_ADDRESS(__VALUE__)        ((__VALUE__) <= 0x3FFU)

#define IS_I2C_TX_FIFO_THRESHOLD(__VALUE__)     ((__VALUE__) <= 8U)

#define IS_I2C_RX_FIFO_THRESHOLD(__VALUE__)     ((__VALUE__) <= 8U)

/**
 * @brief Deinitializes an I2C instance.
 * @param I2Cx The I2C instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The I2C instance have been deinitialized.
 *         - ERROR: The I2C instance have not been deinitialized.
 */
ErrorStatus I2C_DeInit(I2C_TypeDef *I2Cx)
{
    ErrorStatus status = SUCCESS;

    volatile uint32_t timeout;

    /* Check parameters */
    assert(IS_I2C_ALL_INSTANCE(I2Cx));

    /* Disable I2C if enabled */
    if (READ_BIT(I2Cx->ENABLE_STATUS, I2C_ENABLE_STATUS_IC_EN) == I2C_ENABLE_STATUS_IC_EN)
    {
        /* I2C is enabled */

        if ((READ_BIT(I2Cx->CON, I2C_CON_MASTER_MODE) == I2C_CON_MASTER_MODE) &&
            (READ_BIT(I2Cx->STATUS, I2C_STATUS_TFE) == 0UL))
        {
            /* Abort transfer */

            /* Disable DMA transfer */
            CLEAR_BIT(I2Cx->DMA_CR, I2C_DMA_CR_TDMAE);

            /* Disable TX_ABRT interrupt */
            CLEAR_BIT(I2Cx->INTR_MASK, I2C_INTR_MASK_TX_ABRT);

            /* Abort transfer */
            SET_BIT(I2Cx->ENABLE, I2C_ENABLE_ABORT);

            timeout = 1000UL;
            while ((READ_BIT(I2Cx->TX_ABRT_SOURCE, I2C_TX_ABRT_SOURCE_USER) == 0UL) && (timeout > 0UL))
            {
                timeout--;
            }

            if (timeout == 0UL)
            {
                status = ERROR;
            }
        }

        if (status == SUCCESS)
        {
            /* Disable I2C */
            CLEAR_BIT(I2Cx->ENABLE, I2C_ENABLE_EN);

            timeout = 1000UL;
            while ((READ_BIT(I2Cx->ENABLE_STATUS, I2C_ENABLE_STATUS_IC_EN) != 0UL) && (timeout > 0UL))
            {
                timeout--;
            }

            if (timeout == 0UL)
            {
                status = ERROR;
            }
        }
    }

    if (status == SUCCESS)
    {
        /* Reset registers to default */
        MODIFY_REG(I2Cx->ENABLE,
                   I2C_ENABLE_SDA_STUCK_REC_EN,
                   I2C_ENABLE_TX_CMD_BLOCK);
        WRITE_REG(I2Cx->CON,
                  ((0x1UL << I2C_CON_SLAVE_DISABLE_Pos) | (0x1UL << I2C_CON_RESTART_EN_Pos) |
                  (0x3UL << I2C_CON_SPEED_Pos) | (0x1UL << I2C_CON_MASTER_MODE_Pos)));
        WRITE_REG(I2Cx->TAR, ((uint32_t)I2C_DEFAULT_TAR_ADDR << I2C_TAR_TARGET_ADDR_Pos));
        WRITE_REG(I2Cx->SAR, ((uint32_t)I2C_DEFAULT_SLAVE_ADDR << I2C_SAR_SLAVE_ADDR_Pos));
        WRITE_REG(I2Cx->HS_MADDR, ((uint32_t)I2C_DEFAULT_MASTER_CODE << I2C_HS_MADDR_MASTER_CODE_Pos));
        WRITE_REG(I2Cx->INTR_MASK,
                  (I2C_INTR_MASK_SCL_STUCK_LOW | I2C_INTR_MASK_GEN_CALL |
                  I2C_INTR_MASK_RX_DONE | I2C_INTR_MASK_TX_ABRT |
                  I2C_INTR_MASK_RD_REQ | I2C_INTR_MASK_TX_EMPTY |
                  I2C_INTR_MASK_TX_OVER | I2C_INTR_MASK_RX_FULL |
                  I2C_INTR_MASK_RX_OVER | I2C_INTR_MASK_RX_UNDER));
        CLEAR_BIT(I2Cx->DMA_CR, I2C_DMA_CR_TDMAE | I2C_DMA_CR_RDMAE);
        WRITE_REG(I2Cx->TX_TL, (uint32_t)I2C_DEFAULT_TX_TL << I2C_TX_TL_Pos);
        WRITE_REG(I2Cx->RX_TL, (uint32_t)I2C_DEFAULT_RX_TL << I2C_RX_TL_Pos);
        WRITE_REG(I2Cx->SS_SCL_HCNT, (uint32_t)I2C_DEFAULT_SS_SCL_HCNT);
        WRITE_REG(I2Cx->SS_SCL_LCNT, (uint32_t)I2C_DEFAULT_SS_SCL_LCNT);
        WRITE_REG(I2Cx->FS_SCL_HCNT, (uint32_t)I2C_DEFAULT_FS_SCL_HCNT);
        WRITE_REG(I2Cx->FS_SCL_LCNT, (uint32_t)I2C_DEFAULT_FS_SCL_LCNT);
        WRITE_REG(I2Cx->HS_SCL_HCNT, (uint32_t)I2C_DEFAULT_HS_SCL_HCNT);
        WRITE_REG(I2Cx->HS_SCL_LCNT, (uint32_t)I2C_DEFAULT_HS_SCL_LCNT);
    }

    return status;
}

/**
 * @brief Initializes an I2C instance.
 * @note I2C_InitStruct structure should be initialized prior to calling this function.
 * @param I2Cx The I2C instance.
 * @param I2C_InitStruct The pointer to I2C_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The I2C instance have been initialized.
 *         - ERROR: The I2C instance have not been initialized.
 */
ErrorStatus I2C_Init(I2C_TypeDef *I2Cx, I2C_InitStruct_TypeDef *I2C_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_I2C_ALL_INSTANCE(I2Cx));
    assert(IS_I2C_PERIPHERAL_MODE(I2C_InitStruct->PeripheralMode));
    assert(IS_I2C_SPEED(I2C_InitStruct->Speed));
    assert(IS_I2C_OWN_ADDRESS1(I2C_InitStruct->OwnAddress1));
    assert(IS_I2C_OWN_ADDRSIZE(I2C_InitStruct->OwnAddrSize));
    assert(IS_I2C_MASTER_CODE(I2C_InitStruct->MasterCode));
    assert(IS_I2C_TARGET_ADDRESS(I2C_InitStruct->TargetAddress));
    assert(IS_I2C_TX_FIFO_THRESHOLD(I2C_InitStruct->TxFIFOThreshold));
    assert(IS_I2C_RX_FIFO_THRESHOLD(I2C_InitStruct->RxFIFOThreshold));

    if (READ_BIT(I2Cx->ENABLE_STATUS, I2C_ENABLE_STATUS_IC_EN) == 0x0UL)
    {
        if (I2C_InitStruct->PeripheralMode == I2C_MODE_MASTER)
        {
            /* I2C master */

            /* Configure master */
            MODIFY_REG(I2Cx->CON,
                    I2C_CON_SPEED,
                    (I2C_CON_SLAVE_DISABLE | I2C_CON_RESTART_EN |
                    I2C_InitStruct->Speed | I2C_CON_MASTER_MODE));

            /* Set address size */
            if (I2C_InitStruct->OwnAddrSize == I2C_ADDR_7BITS)
            {
                CLEAR_BIT(I2Cx->TAR, I2C_TAR_MASTER_10BITADDR);
                CLEAR_BIT(I2Cx->CON, I2C_CON_SLAVE_10BITADDR);
            }
            else
            {
                SET_BIT(I2Cx->TAR, I2C_TAR_MASTER_10BITADDR);
                SET_BIT(I2Cx->CON, I2C_CON_SLAVE_10BITADDR);
            }

            /* Set target address */
            MODIFY_REG(I2Cx->TAR,
                    I2C_TAR_TARGET_ADDR,
                    (I2C_InitStruct->TargetAddress & I2C_TAR_TARGET_ADDR));

            if (I2C_InitStruct->Speed == I2C_SPEED_STANDARD)
            {
                /* Standard speed */

                /* Set clock timing */
                WRITE_REG(I2Cx->SS_SCL_HCNT, I2C_InitStruct->SCL_HCNT);
                WRITE_REG(I2Cx->SS_SCL_LCNT, I2C_InitStruct->SCL_LCNT);
            }
            else if (I2C_InitStruct->Speed == I2C_SPEED_FAST)
            {
                /* Fast speed */

                /* Set clock timing */
                WRITE_REG(I2Cx->FS_SCL_HCNT, I2C_InitStruct->SCL_HCNT);
                WRITE_REG(I2Cx->FS_SCL_LCNT, I2C_InitStruct->SCL_LCNT);
            }
            else
            {
                /* High speed */

                /* Set HS mode master code */
                WRITE_REG(I2Cx->HS_MADDR, I2C_InitStruct->MasterCode);

                /* Set clock timing */
                WRITE_REG(I2Cx->HS_SCL_HCNT, I2C_InitStruct->SCL_HCNT);
                WRITE_REG(I2Cx->HS_SCL_LCNT, I2C_InitStruct->SCL_LCNT);
            }
        }
        else
        {
            /* I2C slave */

            /* Set device own address */
            MODIFY_REG(I2Cx->SAR,
                    I2C_SAR_SLAVE_ADDR,
                    (I2C_InitStruct->OwnAddress1 & I2C_SAR_SLAVE_ADDR));

            /* Configure slave */
            MODIFY_REG(I2Cx->CON,
                    (I2C_CON_SLAVE_DISABLE | I2C_CON_SPEED | I2C_CON_MASTER_MODE),
                    I2C_InitStruct->Speed);

            /* Set address size */
            if (I2C_InitStruct->OwnAddrSize == I2C_ADDR_7BITS)
            {
                CLEAR_BIT(I2Cx->CON, I2C_CON_SLAVE_10BITADDR);
            }
            else
            {
                SET_BIT(I2Cx->CON, I2C_CON_SLAVE_10BITADDR);
            }
        }

        /* Set FIFO threshold levels */
        WRITE_REG(I2Cx->TX_TL, I2C_InitStruct->TxFIFOThreshold);
        WRITE_REG(I2Cx->RX_TL, I2C_InitStruct->RxFIFOThreshold);

        /* Disable transmission blocking */
        CLEAR_BIT(I2Cx->ENABLE, I2C_ENABLE_TX_CMD_BLOCK);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of I2C_InitStruct_TypeDef structure to default value.
 * @param I2C_InitStruct The pointer to I2C_InitStruct_TypeDef structure.
 */
void I2C_StructInit(I2C_InitStruct_TypeDef *I2C_InitStruct)
{
    /* Set I2C_InitStruct fields to default values */
    I2C_InitStruct->PeripheralMode = I2C_MODE_MASTER;
    I2C_InitStruct->Speed = I2C_SPEED_STANDARD;
    I2C_InitStruct->OwnAddress1 = I2C_DEFAULT_SLAVE_ADDR;
    I2C_InitStruct->OwnAddrSize = I2C_ADDR_7BITS;
    I2C_InitStruct->MasterCode = I2C_DEFAULT_MASTER_CODE;
    I2C_InitStruct->TargetAddress = I2C_DEFAULT_TAR_ADDR;
    I2C_InitStruct->SCL_LCNT = I2C_DEFAULT_FS_SPKLEN + 8;
    I2C_InitStruct->SCL_HCNT = I2C_DEFAULT_FS_SPKLEN + 6;
    I2C_InitStruct->TxFIFOThreshold = I2C_DEFAULT_TX_TL;
    I2C_InitStruct->RxFIFOThreshold = I2C_DEFAULT_RX_TL;
}
