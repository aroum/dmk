/**
 * *****************************************************************************
 *  @file       bmcu_i2c.h
 *  @author     Baikal electronics SDK team
 *  @brief      Inter-Integrated Circuit (I2C) module driver header file
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

#ifndef __BMCU_I2C_H
#define __BMCU_I2C_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_FLAG_CMD_WRITE                      I2C_DATA_CMD_DIR_WRITE  /*!< Write command */
#define I2C_FLAG_CMD_READ                       I2C_DATA_CMD_DIR_READ   /*!< Read command */

/** @brief I2C peripheral mode */
typedef enum {
    I2C_MODE_MASTER = 0,    /*!< I2C Master */
    I2C_MODE_SLAVE          /*!< I2C Slave */
} I2C_PeriphMode_TypeDef;

/** @brief I2C speed */
typedef enum {
    I2C_SPEED_STANDARD = I2C_CON_SPEED_STANDARD,    /*!< Standard speed mode of operation */
    I2C_SPEED_FAST = I2C_CON_SPEED_FAST,            /*!< Fast or Fast Plus speed mode of operation */
    I2C_SPEED_HIGH = I2C_CON_SPEED_HIGH             /*!< High speed mode of operation */
} I2C_Speed_TypeDef;

/** @brief I2C speed */
typedef enum {
    I2C_ADDR_7BITS = 0, /*!< Standard speed mode of operation */
    I2C_ADDR_10BITS     /*!< High speed mode of operation */
} I2C_AddrSize_TypeDef;

/** @brief Flags */
typedef enum {
    I2C_FLAG_ACTIVITY                = 0,   /*!< Activity status */
    I2C_FLAG_TFNF                    = 1,   /*!< Transmit FIFO not full */
    I2C_FLAG_TFE                     = 2,   /*!< Transmit FIFO empty */
    I2C_FLAG_RFNE                    = 3,   /*!< Receive FIFO not empty */
    I2C_FLAG_RFF                     = 4,   /*!< Receive FIFO full */
    I2C_FLAG_MST_ACTIVITY            = 5,   /*!< Master activity status */
    I2C_FLAG_SLV_ACTIVITY            = 6,   /*!< Slave activity status */
    I2C_FLAG_MST_HOLD_TX_FIFO_EMPTY  = 7,   /*!< Master holds the bus due to Tx FIFO is empty */
    I2C_FLAG_MST_HOLD_RX_FIFO_FULL   = 8,   /*!< Master holds the bus due to Rx FIFO is full */
    I2C_FLAG_SLV_HOLD_TX_FIFO_EMPTY  = 9,   /*!< Slave holds the bus due to Tx FIFO is empty */
    I2C_FLAG_SLV_HOLD_RX_FIFO_FULL   = 10,  /*!< Slave holds the bus due to Rx FIFO is full */
    I2C_FLAG_SDA_STUCK_NOT_RECOVERED = 11   /*!< SDA stuck at low is not recovered after recovery mechanism */
} I2C_Flag_TypeDef;

/** @brief Interrupts */
typedef enum {
    I2C_IT_RX_UNDER         = 0,    /*!< Attempt to read Rx FIFO when it is empty */
    I2C_IT_RX_OVER          = 1,    /*!< Rx FIFO is full and an additional byte is received */
    I2C_IT_RX_FULL          = 2,    /*!< Rx FIFO reached threshold */
    I2C_IT_TX_OVER          = 3,    /*!< Attempt to write Tx FIFO when it is full */
    I2C_IT_TX_EMPTY         = 4,    /*!< Tx FIFO is empty */
    I2C_IT_RD_REQ           = 5,    /*!< Read request from master is received */
    I2C_IT_TX_ABRT          = 6,    /*!< Transmit abort */
    I2C_IT_RX_DONE          = 7,    /*!< Transmission is done */
    I2C_IT_ACTIVITY         = 8,    /*!< Activity on the bus */
    I2C_IT_STOP_DET         = 9,    /*!< STOP condition has occured */
    I2C_IT_START_DET        = 10,   /*!< START condition has occured */
    I2C_IT_GEN_CALL         = 11,   /*!< General Call address is received and acknowledged */
    I2C_IT_SCL_STUCK_AT_LOW = 14    /*!< SCL line is stuck at low */
} I2C_IT_TypeDef;

/** @brief I2C Init Structure definition */
typedef struct
{
    I2C_PeriphMode_TypeDef PeripheralMode;  /*!< Peripheral mode */
    I2C_Speed_TypeDef Speed;                /*!< Speed */
    uint16_t OwnAddress1;                   /*!< Device own address 1 */
    I2C_AddrSize_TypeDef OwnAddrSize;       /*!< Own address 1 size */
    uint8_t MasterCode;                     /*!< HS mode master code */
    uint16_t TargetAddress;                 /*!< Target address */
    uint16_t SCL_LCNT;                      /*!< SCL clock low period count */
    uint16_t SCL_HCNT;                      /*!< SCL clock high period count */
    uint8_t TxFIFOThreshold;                /*!< Tx FIFO threshold level */
    uint8_t RxFIFOThreshold;                /*!< Rx FIFO threshold level */
} I2C_InitStruct_TypeDef;

/**
 * @brief Writes a value in I2C register.
 * @param INSTANCE The I2C instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define I2C_WriteReg(INSTANCE, REG, VALUE)      WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in I2C register.
 * @param INSTANCE The I2C instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define I2C_ReadReg(INSTANCE, REG)              READ_REG(INSTANCE->REG)

/**
 * @brief Enables I2C.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_Enable(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->ENABLE, I2C_ENABLE_EN);
}

/**
 * @brief Disables I2C.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_Disable(I2C_TypeDef *I2Cx)
{
    CLEAR_BIT(I2Cx->ENABLE, I2C_ENABLE_EN);
}

/**
 * @brief Checks if I2C enabled.
 * @param I2Cx The I2C instance.
 * @retval 1 if I2C is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_IsEnabled(I2C_TypeDef *I2Cx)
{
    return ((READ_BIT(I2Cx->ENABLE_STATUS, I2C_ENABLE_STATUS_IC_EN) == I2C_ENABLE_STATUS_IC_EN) ? 1UL : 0UL);
}

/**
 * @brief Enables I2C master.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_Master_Enable(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->CON, (I2C_CON_SLAVE_DISABLE | I2C_CON_MASTER_MODE));
}

/**
 * @brief Disables I2C master.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_Master_Disable(I2C_TypeDef *I2Cx)
{
    CLEAR_BIT(I2Cx->CON, I2C_CON_MASTER_MODE);
}

/**
 * @brief Checks if I2C master enabled.
 * @param I2Cx The I2C instance.
 * @retval 1 if I2C master is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_Master_IsEnabled(I2C_TypeDef *I2Cx)
{
    return ((READ_BIT(I2Cx->CON, I2C_CON_MASTER_MODE) == I2C_CON_MASTER_MODE) ? 1UL : 0UL);
}

/**
 * @brief Enables I2C slave.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_Slave_Enable(I2C_TypeDef *I2Cx)
{
    CLEAR_BIT(I2Cx->CON, (I2C_CON_SLAVE_DISABLE | I2C_CON_MASTER_MODE));
}

/**
 * @brief Disables I2C slave.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_Slave_Disable(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->CON, I2C_CON_SLAVE_DISABLE);
}

/**
 * @brief Checks if I2C slave enabled.
 * @param I2Cx The I2C instance.
 * @retval 1 if I2C slave is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_Slave_IsEnabled(I2C_TypeDef *I2Cx)
{
    return ((READ_BIT(I2Cx->CON, I2C_CON_SLAVE_DISABLE) == I2C_CON_SLAVE_DISABLE) ? 0UL : 1UL);
}

/**
 * @brief Enables DMA transmission requests.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_EnableDMAReq_TX(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->DMA_CR, I2C_DMA_CR_TDMAE);
}

/**
 * @brief Disables DMA transmission requests.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_DisableDMAReq_TX(I2C_TypeDef *I2Cx)
{
    CLEAR_BIT(I2Cx->DMA_CR, I2C_DMA_CR_TDMAE);
}

/**
 * @brief Checks if DMA transmission requests are enabled.
 * @param I2Cx The I2C instance.
 * @retval 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_IsEnabledDMAReq_TX(I2C_TypeDef *I2Cx)
{
    return ((READ_BIT(I2Cx->DMA_CR, I2C_DMA_CR_TDMAE) == I2C_DMA_CR_TDMAE) ? 1UL : 0UL);
}

/**
 * @brief Enables DMA reception requests.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_EnableDMAReq_RX(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->DMA_CR, I2C_DMA_CR_RDMAE);
}

/**
 * @brief Disables DMA reception requests.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_DisableDMAReq_RX(I2C_TypeDef *I2Cx)
{
    CLEAR_BIT(I2Cx->DMA_CR, I2C_DMA_CR_RDMAE);
}

/**
 * @brief Checks if DMA reception requests are enabled.
 * @param I2Cx The I2C instance.
 * @retval 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_IsEnabledDMAReq_RX(I2C_TypeDef *I2Cx)
{
    return ((READ_BIT(I2Cx->DMA_CR, I2C_DMA_CR_RDMAE) == I2C_DMA_CR_RDMAE) ? 1UL : 0UL);
}

/**
 * @brief Sets DMA transmit data level.
 * @param I2Cx The I2C instance.
 * @param Level The transmit data level.
 */
__STATIC_INLINE void I2C_SetDMADataLevel_TX(I2C_TypeDef *I2Cx, uint32_t Level)
{
    MODIFY_REG(I2Cx->DMA_TDLR, I2C_DMA_TDLR, Level & I2C_DMA_TDLR);
}

/**
 * @brief Returns DMA transmit data level.
 * @param I2Cx The I2C instance.
 * @returns The transmit data level.
 */
__STATIC_INLINE uint32_t I2C_GetDMADataLevel_TX(I2C_TypeDef *I2Cx)
{
    return ((uint32_t)READ_BIT(I2Cx->DMA_TDLR, I2C_DMA_TDLR));
}

/**
 * @brief Sets DMA receive data level.
 * @param I2Cx The I2C instance.
 * @param Level The receive data level.
 */
__STATIC_INLINE void I2C_SetDMADataLevel_RX(I2C_TypeDef *I2Cx, uint32_t Level)
{
    MODIFY_REG(I2Cx->DMA_RDLR, I2C_DMA_RDLR, Level & I2C_DMA_RDLR);
}

/**
 * @brief Returns DMA receive data level.
 * @param I2Cx The I2C instance.
 * @returns The receive data level.
 */
__STATIC_INLINE uint32_t I2C_GetDMADataLevel_RX(I2C_TypeDef *I2Cx)
{
    return ((uint32_t)READ_BIT(I2Cx->DMA_RDLR, I2C_DMA_RDLR));
}

/**
 * @brief Enables General Call.
 * @note When enabled the device generates ACK for General Call, otherwise NACK.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_EnableGeneralCall(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->ACK_GENERAL_CALL, I2C_ACK_GENERAL_CALL);
}

/**
 * @brief Disables General Call.
 * @note When enabled the device generates ACK for General Call, otherwise NACK.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_DisableGeneralCall(I2C_TypeDef *I2Cx)
{
    CLEAR_BIT(I2Cx->ACK_GENERAL_CALL, I2C_ACK_GENERAL_CALL);
}

/**
 * @brief Checks if General Call is enabled.
 * @param I2Cx The I2C instance.
 * @retval 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_IsEnabledGeneralCall(I2C_TypeDef *I2Cx)
{
    return ((READ_BIT(I2Cx->ACK_GENERAL_CALL, I2C_ACK_GENERAL_CALL) == I2C_ACK_GENERAL_CALL) ? 1UL : 0UL);
}

/**
 * @brief Sets own address 1.
 * @param I2Cx The I2C instance.
 * @param OwnAddress1 The own address 1.
 * @param OwnAddrSize The address size.
 */
__STATIC_INLINE void I2C_SetOwnAddress1(I2C_TypeDef *I2Cx, uint16_t OwnAddress1, I2C_AddrSize_TypeDef OwnAddrSize)
{
    MODIFY_REG(I2Cx->SAR,
               I2C_SAR_SLAVE_ADDR,
               (OwnAddress1 & I2C_SAR_SLAVE_ADDR));

    if (OwnAddrSize == I2C_ADDR_7BITS)
    {
        CLEAR_BIT(I2Cx->CON, I2C_CON_SLAVE_10BITADDR);
    }
    else
    {
        SET_BIT(I2Cx->CON, I2C_CON_SLAVE_10BITADDR);
    }
}

/**
 * @brief Configures the I2C clock speed mode.
 * @param I2Cx The I2C instance.
 * @param ClockSpeedMode The clock speed mode.
 */
__STATIC_INLINE void I2C_SetClockSpeedMode(I2C_TypeDef *I2Cx, I2C_Speed_TypeDef ClockSpeedMode)
{
    MODIFY_REG(I2Cx->CON, I2C_CON_SPEED, (uint32_t)ClockSpeedMode);
}

/**
 * @brief Returns the I2C clock speed mode.
 * @param I2Cx The I2C instance.
 * @returns The clock speed mode.
 */
__STATIC_INLINE I2C_Speed_TypeDef I2C_GetClockSpeedMode(I2C_TypeDef *I2Cx)
{
    return (I2C_Speed_TypeDef)READ_BIT(I2Cx->CON, I2C_CON_SPEED);
}

/**
 * @brief Configures clock high and low period.
 * @param I2Cx The I2C instance.
 * @param ClockSpeedMode The clock speed mode.
 * @param HCNT The high period.
 * @param LCNT The low period.
 */
__STATIC_INLINE void I2C_SetSCLPeriods(I2C_TypeDef *I2Cx, I2C_Speed_TypeDef ClockSpeedMode, uint16_t HCNT, uint16_t LCNT)
{
    if (ClockSpeedMode == I2C_SPEED_STANDARD)
    {
        /* Standard speed */
        I2Cx->SS_SCL_HCNT = HCNT;
        I2Cx->SS_SCL_LCNT = LCNT;
    }
    else if (ClockSpeedMode == I2C_SPEED_FAST)
    {
        /* Fast speed */
        I2Cx->FS_SCL_HCNT = HCNT;
        I2Cx->FS_SCL_LCNT = LCNT;
    }
    else
    {
        /* High speed */
        I2Cx->HS_SCL_HCNT = HCNT;
        I2Cx->HS_SCL_LCNT = LCNT;
    }
}

/**
 * @brief Checks if flag is set.
 * @param I2Cx The I2C instance.
 * @param Flag The flag. Can be one of I2C_Flag_TypeDef values.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t I2C_IsActiveFlag(I2C_TypeDef *I2Cx, I2C_Flag_TypeDef Flag)
{
    uint32_t Mask = (1UL << Flag);

    return ((READ_BIT(I2Cx->STATUS, Mask) == Mask) ? 1UL : 0UL);
}

/**
 * @brief Enables interrupt.
 * @param I2Cx The I2C instance.
 * @param IT The interrupt source. Can be one of I2C_IT_TypeDef values.
 */
__STATIC_INLINE void I2C_EnableIT(I2C_TypeDef *I2Cx, I2C_IT_TypeDef IT)
{
    SET_BIT(I2Cx->INTR_MASK, (1UL << IT));
}

/**
 * @brief Disables interrupt.
 * @param I2Cx The I2C instance.
 * @param IT The interrupt source. Can be one of I2C_IT_TypeDef values.
 */
__STATIC_INLINE void I2C_DisableIT(I2C_TypeDef *I2Cx, I2C_IT_TypeDef IT)
{
    CLEAR_BIT(I2Cx->INTR_MASK, (1UL << IT));
}

/**
 * @brief Checks if interrupt is enabled.
 * @param I2Cx The I2C instance.
 * @param IT The interrupt source. Can be one of I2C_IT_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_IsEnabledIT(I2C_TypeDef *I2Cx, I2C_IT_TypeDef IT)
{
    uint32_t Mask = (1UL << IT);

    return ((READ_BIT(I2Cx->INTR_MASK, Mask) == Mask) ? 1UL : 0UL);
}

/**
 * @brief Checks if interrupt flag is active or not.
 * @param I2Cx The I2C instance.
 * @param IT The interrupt source. Can be one of I2C_IT_TypeDef values.
 * @retval The state of bit (1 or 0).
 */
__STATIC_INLINE uint32_t I2C_IsActiveIT(I2C_TypeDef *I2Cx, I2C_IT_TypeDef IT)
{
    uint32_t Mask = (1UL << IT);

    return ((READ_BIT(I2Cx->RAW_INTR_STAT, Mask) == Mask) ? 1UL : 0UL);
}

/**
 * @brief Clears interrupt.
 * @param I2Cx The I2C instance.
 * @param IT The interrupt source. Can be one of I2C_IT_TypeDef values.
 */
__STATIC_INLINE void I2C_ClearIT(I2C_TypeDef *I2Cx, I2C_IT_TypeDef IT)
{
    __IO uint32_t tmpreg;

    switch (IT)
    {
        case I2C_IT_RX_UNDER:
            tmpreg = I2Cx->CLR_RX_UNDER;
            break;
        case I2C_IT_RX_OVER:
            tmpreg = I2Cx->CLR_RX_OVER;
            break;
        case I2C_IT_TX_OVER:
            tmpreg = I2Cx->CLR_TX_OVER;
            break;
        case I2C_IT_RD_REQ:
            tmpreg = I2Cx->CLR_RD_REQ;
            break;
        case I2C_IT_TX_ABRT:
            tmpreg = I2Cx->CLR_TX_ABRT;
            break;
        case I2C_IT_RX_DONE:
            tmpreg = I2Cx->CLR_RX_DONE;
            break;
        case I2C_IT_ACTIVITY:
            tmpreg = I2Cx->CLR_ACTIVITY;
            break;
        case I2C_IT_STOP_DET:
            tmpreg = I2Cx->CLR_STOP_DET;
            break;
        case I2C_IT_START_DET:
            tmpreg = I2Cx->CLR_START_DET;
            break;
        case I2C_IT_GEN_CALL:
            tmpreg = I2Cx->CLR_GEN_CALL;
            break;
        case I2C_IT_SCL_STUCK_AT_LOW:
            tmpreg = I2Cx->CLR_SCL_STUCK_DET;
            break;
        default:
            /* The interrupt is cleared by hardware */
            break;
    }

    (void)tmpreg;
}

/**
 * @brief Clears all interrupts.
 * @note It clears the combined interrupt, all individual interrupts and
 *       TX_ABRT_SOURCE register. This does not clear hardware clearable
 *       interrupts but software clearable ones.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_ClearIT_All(I2C_TypeDef *I2Cx)
{
    __IO uint32_t tmpreg;
    tmpreg = I2Cx->CLR_INTR;
    (void)tmpreg;
}

/**
 * @brief Enables block of the data transmission on I2C bus.
 * @note Set TX_CMD_BLOCK only when Tx FIFO is empty and master is in idle state.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_EnableTxCmdBlock(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->ENABLE, I2C_ENABLE_TX_CMD_BLOCK);
}

/**
 * @brief Disables block of the data transmission on I2C bus.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_DisableTxCmdBlock(I2C_TypeDef *I2Cx)
{
    CLEAR_BIT(I2Cx->ENABLE, I2C_ENABLE_TX_CMD_BLOCK);
}

/**
 * @brief Checks if block of the data transmission on I2C bus enabled.
 * @param I2Cx The I2C instance.
 * @retval 1 if block is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2C_IsEnabledTxCmdBlock(I2C_TypeDef *I2Cx)
{
    return ((READ_BIT(I2Cx->ENABLE, I2C_ENABLE_TX_CMD_BLOCK) == I2C_ENABLE_TX_CMD_BLOCK) ? 1UL : 0UL);
}

/**
 * @brief Initiates the transfer abort.
 * @param I2Cx The I2C instance.
 */
__STATIC_INLINE void I2C_Abort(I2C_TypeDef *I2Cx)
{
    SET_BIT(I2Cx->ENABLE, I2C_ENABLE_ABORT);
}

/**
 * @brief Returns the data received.
 * @param I2Cx The I2C instance.
 * @retval The data received.
 */
__STATIC_INLINE uint8_t I2C_ReceiveData(I2C_TypeDef *I2Cx)
{
    return (uint8_t)(READ_BIT(I2Cx->DATA_CMD, I2C_DATA_CMD_DAT));
}

/**
 * @brief Performs data write or data read.
 * @param I2Cx The I2C instance.
 * @param Data The data to be written.
 * @param Flags The flags.
 *              - I2C_FLAG_CMD_WRITE Perform data write. The data parameter must contain a valid value.
 *              - I2C_FLAG_CMD_READ Perform data read. The data is ignored.
 */
__STATIC_INLINE void I2C_TransmitData(I2C_TypeDef *I2Cx, uint8_t Data, uint32_t Flags)
{
    WRITE_REG(I2Cx->DATA_CMD,
              (Flags & I2C_DATA_CMD_DIR) | ((uint32_t)Data << I2C_DATA_CMD_DAT_Pos));
}

ErrorStatus I2C_DeInit(I2C_TypeDef *I2Cx);
ErrorStatus I2C_Init(I2C_TypeDef *I2Cx, I2C_InitStruct_TypeDef *I2C_InitStruct);
void I2C_StructInit(I2C_InitStruct_TypeDef *I2C_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_I2C_H */
