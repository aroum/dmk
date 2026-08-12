/**
 * *****************************************************************************
 *  @file       bmcu_dma.h
 *  @author     Baikal electronics SDK team
 *  @brief      Direct Memory Access (DMA) module driver header file
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

#ifndef __BMCU_DMA_H
#define __BMCU_DMA_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DMA instance and hardware handshake interface assignments

    BMCU_U

    +---------------------------------------------------------------------------------------------------------------------+
    |    APB0    |    APB1    |    APB2    |    AHB     |         DMA0        |         DMA1        |         DMA2        |
    | Peripheral | Peripheral | Peripheral | Peripheral | Tx HW HS | Rx HW HS | Tx HW HS | Rx HW HS | Tx HW HS | Rx HW HS |
    +---------------------------------------------------------------------------------------------------------------------+
    |    I2C0    |            |            |            |    0     |    1     |          |          |          |          |
    |    I2C1    |            |            |            |    2     |    3     |          |          |          |          |
    |    I2S0    |            |            |            |    4     |    5     |          |          |          |          |
    |    QSPI0   |            |            |            |    6     |    7     |          |          |          |          |
    |    SPIM0   |            |            |            |    4     |    5     |          |          |          |          |
    |    SPIS0   |            |            |            |    6     |    7     |          |          |          |          |
    |    UART0   |            |            |            |    8     |    9     |          |          |          |          |
    |    UART1   |            |            |            |    10    |    11    |          |          |          |          |
    |    UART2   |            |            |            |    2     |    3     |          |          |          |          |
    +---------------------------------------------------------------------------------------------------------------------+
    |            |    I2C2    |            |            |          |          |    0     |    1     |          |          |
    |            |    I2C3    |            |            |          |          |    2     |    3     |          |          |
    |            |    I2S1    |            |            |          |          |    4     |    5     |          |          |
    |            |    QSPI1   |            |            |          |          |    6     |    7     |          |          |
    |            |    SPIM1   |            |            |          |          |    4     |    5     |          |          |
    |            |    SPIS1   |            |            |          |          |    6     |    7     |          |          |
    |            |    UART3   |            |            |          |          |    8     |    9     |          |          |
    |            |    UART4   |            |            |          |          |    10    |    11    |          |          |
    |            |    UART5   |            |            |          |          |    2     |    3     |          |          |
    +---------------------------------------------------------------------------------------------------------------------+
    |            |            |    ADC0    |            |          |          |          |          |    0     |          |
    |            |            |    ADC1    |            |          |          |          |          |    2     |          |
    |            |            |    ADC2    |            |          |          |          |          |    4     |          |
    |            |            |    PWMA0   |            |    12    |          |          |          |          |          |
    |            |            |    PWMA1   |            |          |          |    12    |          |          |          |
    |            |            |    PWMA2   |            |    14    |          |          |          |          |          |
    |            |            |    PWMA3   |            |          |          |    14    |          |          |          |
    +---------------------------------------------------------------------------------------------------------------------+
    |            |            |            |            |          |          |          |          |    6     |    7     |
    |            |            |            |            |          |          |          |          |    8     |    9     |
    |            |            |            |    USB     |          |          |          |          |    10    |    11    |
    |            |            |            |            |          |          |          |          |    12    |    13    |
    |            |            |            |            |          |          |          |          |    14    |    15    |
    +---------------------------------------------------------------------------------------------------------------------+

    BE_U1000

    +-----------------------------------------------------------------------------------------------+
    |    APB0    |    APB1    |    APB2    |    AHB     |         DMA0        |         DMA1        |
    | Peripheral | Peripheral | Peripheral | Peripheral | Tx HW HS | Rx HW HS | Tx HW HS | Rx HW HS |
    +-----------------------------------------------------------------------------------------------+
    |    I2C0    |            |            |            |    0     |    1     |          |          |
    |    I2C1    |            |            |            |    2     |    3     |          |          |
    |    I2S0    |            |            |            |    4     |    5     |          |          |
    |    QSPI0   |            |            |            |    6     |    7     |          |          |
    |    SPIM0   |            |            |            |    4     |    5     |          |          |
    |    SPIS0   |            |            |            |    6     |    7     |          |          |
    |    UART0   |            |            |            |    8     |    9     |          |          |
    |    UART1   |            |            |            |    10    |    11    |          |          |
    |    UART2   |            |            |            |    2     |    3     |          |          |
    +-----------------------------------------------------------------------------------------------+
    |            |    I2C2    |            |            |          |          |    0     |    1     |
    |            |    I2C3    |            |            |          |          |    2     |    3     |
    |            |    I2S1    |            |            |          |          |    4     |    5     |
    |            |    QSPI1   |            |            |          |          |    6     |    7     |
    |            |    SPIM1   |            |            |          |          |    4     |    5     |
    |            |    SPIS1   |            |            |          |          |    6     |    7     |
    |            |    UART3   |            |            |          |          |    8     |    9     |
    |            |    UART4   |            |            |          |          |    10    |    11    |
    |            |    UART5   |            |            |          |          |    2     |    3     |
    +-----------------------------------------------------------------------------------------------+
    |            |            |    ADC0    |            |          |    13    |          |          |
    |            |            |    ADC1    |            |          |    15    |          |          |
    |            |            |    ADC2    |            |          |          |          |    13    |
    |            |            |    PWMA0   |            |    12    |          |          |          |
    |            |            |    PWMA1   |            |          |          |    12    |          |
    |            |            |    PWMA2   |            |    14    |          |          |          |
    |            |            |    PWMA3   |            |          |          |    14    |          |
    |            |            |    UART6   |            |    0     |    1     |          |          |
    |            |            |    UART7   |            |          |          |    0     |    1     |
    +-----------------------------------------------------------------------------------------------+
*/

/** 
 * @brief DMA channels.
 * @note (BMCU_U, BE_U1000) Channels 2 and 3 support multi-block DMA transfers
 *       using either auto-reloading of channel registers or block chaining.
 *       The rest of channels do single block transfers only.
 */
typedef enum {
    DMA_CH0 = 0,    /*!< Channel 0 */
    DMA_CH1,        /*!< Channel 1 */
    DMA_CH2,        /*!< Channel 2 */
    DMA_CH3,        /*!< Channel 3 */
    DMA_CH4,        /*!< Channel 4 */
    DMA_CH5,        /*!< Channel 5 */
    DMA_CH6,        /*!< Channel 6 */
    DMA_CH7         /*!< Channel 7 */
} DMA_Channels_TypeDef;

/** @brief DMA channel priority */
typedef enum {
    DMA_PRIORITY_0 = DMA_CH_CFG0_PRIOR_0,   /*!< Channel priority is 0 (lowest) */
    DMA_PRIORITY_1 = DMA_CH_CFG0_PRIOR_1,   /*!< Channel priority is 1 */
    DMA_PRIORITY_2 = DMA_CH_CFG0_PRIOR_2,   /*!< Channel priority is 2 */
    DMA_PRIORITY_3 = DMA_CH_CFG0_PRIOR_3,   /*!< Channel priority is 3 */
    DMA_PRIORITY_4 = DMA_CH_CFG0_PRIOR_4,   /*!< Channel priority is 4 */
    DMA_PRIORITY_5 = DMA_CH_CFG0_PRIOR_5,   /*!< Channel priority is 5 */
    DMA_PRIORITY_6 = DMA_CH_CFG0_PRIOR_6,   /*!< Channel priority is 6 */
    DMA_PRIORITY_7 = DMA_CH_CFG0_PRIOR_7    /*!< Channel priority is 7 (highest) */
} DMA_Priority_TypeDef;

/** @brief DMA transfer direction */
typedef enum {
    DMA_DIR_MEMORY_TO_MEMORY = DMA_CH_CTL0_TT_FC_0,     /*!< Memory to Memory */
    DMA_DIR_MEMORY_TO_PERIPH = DMA_CH_CTL0_TT_FC_1,     /*!< Memory to Peripheral */
    DMA_DIR_PERIPH_TO_MEMORY = DMA_CH_CTL0_TT_FC_2,     /*!< Peripheral to Memory */
    DMA_DIR_PERIPH_TO_PERIPH = DMA_CH_CTL0_TT_FC_3      /*!< Peripheral to Peripheral */
} DMA_Direction_TypeDef;

/** @brief DMA mode */
typedef enum {
    DMA_MODE_NORMAL = 0,    /*!< Normal mode */
    DMA_MODE_AUTO_RELOAD    /*!< Automatic address reload mode */
} DMA_Mode_TypeDef;

/** @brief DMA address increment mode */
typedef enum {
    DMA_INC_MODE_INCREMENT = 0, /*!< Increments address */
    DMA_INC_MODE_DECREMENT,     /*!< Decrements address */
    DMA_INC_MODE_NO_CHANGE      /*!< No change in address */
} DMA_IncMode_TypeDef;

/** @brief DMA transfer width */
typedef enum {
    DMA_TR_WIDTH_8BITS = 0,     /*!< Transfer width is 8 bits */
    DMA_TR_WIDTH_16BITS,        /*!< Transfer width is 16 bits */
    DMA_TR_WIDTH_32BITS         /*!< Transfer width is 32 bits */
} DMA_TransferWidth_TypeDef;

/** @brief DMA burst transaction length */
typedef enum {
    DMA_BURST_LENGTH_1 = 0,     /*!< Number of data items to be transferred is 1 */
    DMA_BURST_LENGTH_4,         /*!< Number of data items to be transferred is 4 */
    DMA_BURST_LENGTH_8,         /*!< Number of data items to be transferred is 8 */
    DMA_BURST_LENGTH_16,        /*!< Number of data items to be transferred is 16 */
    DMA_BURST_LENGTH_32,        /*!< Number of data items to be transferred is 32 */
    DMA_BURST_LENGTH_64,        /*!< Number of data items to be transferred is 64 */
    DMA_BURST_LENGTH_128,       /*!< Number of data items to be transferred is 128 */
    DMA_BURST_LENGTH_256        /*!< Number of data items to be transferred is 256 */
} DMA_BurstLength_TypeDef;

/** @brief DMA FIFO mode */
typedef enum {
    DMA_FIFO_MODE_0 = DMA_CH_CFG1_FIFO_MODE_0,  /*!< FIFO can transmit or accept a single AHB transfer */
    DMA_FIFO_MODE_1 = DMA_CH_CFG1_FIFO_MODE_1   /*!< FIFO is less or equal than half full to fetch data from source or greater or equal than half full to send data to destination */
} DMA_FIFOMode_TypeDef;

/** @brief DMA handshaking interface */
typedef enum {
    DMA_HS_HARDWARE = 0,    /*!< Hardware handshaking interface */
    DMA_HS_SOFTWARE         /*!< Software handshaking interface */
} DMA_HandshakeMode_TypeDef;

/** @brief DMA hardware handshaking interface polarity */
typedef enum {
    DMA_HS_POLARITY_HIGH = 0,   /*!< Active high */
    DMA_HS_POLARITY_LOW         /*!< Active low */
} DMA_HandshakePolarity_TypeDef;

/** 
 * @brief DMA block descriptor (LLI)
 * @note The DMA_LLI_TypeDef structure must be aligned to 32-bit boundary.
 */
typedef struct
{
    uint32_t SAR;       /*!< SAR */
    uint32_t DAR;       /*!< DAR */
    uint32_t LLP;       /*!< LLP */
    uint32_t CTL0;      /*!< CTL0 */
    uint32_t CTL1;      /*!< CTL1 */
} DMA_LLI_TypeDef;

/** @brief DMA Init Structure definition */
typedef struct
{
    uint32_t SrcAddress;                                    /*!< Source address of DMA transfer */
    DMA_Mode_TypeDef SrcMode;                               /*!< Source operation mode (normal or auto reload) */
    DMA_IncMode_TypeDef SrcIncMode;                         /*!< Source address increment mode */
    DMA_TransferWidth_TypeDef SrcTransferWidth;             /*!< Source transfer width */
    DMA_BurstLength_TypeDef SrcBurstLength;                 /*!< Source burst transaction length */
    FunctionalState SrcLinkedListCtrl;                      /*!< Source block chaining enable */
    DMA_HandshakeMode_TypeDef SrcHandshake;                 /*!< Source handshaking interface mode */
    DMA_HandshakePolarity_TypeDef SrcHWHandshakePolarity;   /*!< Source hardware handshaking interface polarity */
    uint8_t SrcHWHandshakeInterface;                        /*!< Source hardware handshaking interface number */
    uint32_t DstAddress;                                    /*!< Destination address of DMA transfer */
    DMA_Mode_TypeDef DstMode;                               /*!< Destination operation mode (normal or auto reload) */
    DMA_IncMode_TypeDef DstIncMode;                         /*!< Destination address increment mode */
    DMA_TransferWidth_TypeDef DstTransferWidth;             /*!< Destination transfer width */
    DMA_BurstLength_TypeDef DstBurstLength;                 /*!< Destination burst transaction length */
    FunctionalState DstLinkedListCtrl;                      /*!< Destination block chaining enable */
    DMA_HandshakeMode_TypeDef DstHandshake;                 /*!< Destination handshaking interface mode */
    DMA_HandshakePolarity_TypeDef DstHWHandshakePolarity;   /*!< Destination hardware handshaking interface polarity */
    uint8_t DstHWHandshakeInterface;                        /*!< Destination hardware handshaking interface number */
    DMA_Direction_TypeDef Direction;                        /*!< Data transfer direction */
    DMA_Priority_TypeDef Priority;                          /*!< Channel priority level */
    DMA_FIFOMode_TypeDef FIFOMode;                          /*!< FIFO mode */
    uint32_t LinkedListPointer;                             /*!< Linked List Pointer */
    uint8_t NbData;                                         /*!< Block transfer size */
} DMA_InitStruct_TypeDef;

/**
 * @brief Writes a value in DMA register.
 * @param INSTANCE The DMA instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define DMA_WriteReg(INSTANCE, REG, VALUE)      WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in DMA register.
 * @param INSTANCE The DMA instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define DMA_ReadReg(INSTANCE, REG)              READ_REG(INSTANCE->REG)

/**
 * @brief Enables the selected DMA instance.
 * @param DMAx The DMA instance.
 */
__STATIC_INLINE void DMA_Enable(DMA_TypeDef *DMAx)
{
    SET_BIT(DMAx->MSC.CFG, DMA_MSC_CFG_EN);
}

/**
 * @brief Disables the selected DMA instance.
 * @param DMAx The DMA instance.
 */
__STATIC_INLINE void DMA_Disable(DMA_TypeDef *DMAx)
{
    CLEAR_BIT(DMAx->MSC.CFG, DMA_MSC_CFG_EN);
}

/**
 * @brief Checks if the selected DMA instance is enabled.
 * @param DMAx The DMA instance.
 * @retval 1 if DMA is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabled(DMA_TypeDef *DMAx)
{
    return ((READ_BIT(DMAx->MSC.CFG, DMA_MSC_CFG_EN) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables DMA channel.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableChannel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->MSC.CH_EN, ((DMA_MSC_CH_EN_WE_CH0 << Channel) | (DMA_MSC_CH_EN_CH0 << Channel)));
}

/**
 * @brief Disables DMA channel.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableChannel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->MSC.CH_EN, (DMA_MSC_CH_EN_WE_CH0 << Channel));
}

/**
 * @brief Checks if DMA channel is enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the DMA channel is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledChannel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->MSC.CH_EN, (DMA_MSC_CH_EN_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Suspends DMA channel transfer.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_SuspendChannel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    SET_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_SUSP);
}

/**
 * @brief Resumes DMA channel transfer.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_ResumeChannel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    CLEAR_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_SUSP);
}

/**
 * @brief Checks if DMA channel transfer is suspended.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the DMA channel transfer is suspended, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsSuspendedChannel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_SUSP) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Sets source address of DMA transfer.
 * @note The source address must be aligned to CTL0.STW.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param SrcAddress The source address.
 */
__STATIC_INLINE void DMA_SetSrcAddress(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, uint32_t SrcAddress)
{
    WRITE_REG(DMAx->CH[Channel].SAR, SrcAddress);
}

/**
 * @brief Returns source address of DMA transfer.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The source address.
 */
__STATIC_INLINE uint32_t DMA_GetSrcAddress(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return READ_REG(DMAx->CH[Channel].SAR);
}

/**
 * @brief Sets destination address of DMA transfer.
 * @note The destination address must be aligned to CTL0.DTW.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param DstAddress The destination address.
 */
__STATIC_INLINE void DMA_SetDstAddress(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, uint32_t DstAddress)
{
    WRITE_REG(DMAx->CH[Channel].DAR, DstAddress);
}

/**
 * @brief Returns destination address of DMA transfer.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The destination address.
 */
__STATIC_INLINE uint32_t DMA_GetDstAddress(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return READ_REG(DMAx->CH[Channel].DAR);
}

/**
 * @brief Sets DMA channel priority level.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Priority The priority level. Can be one of DMA_Priority_TypeDef values.
 */
__STATIC_INLINE void DMA_SetChannelPriorityLevel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_Priority_TypeDef Priority)
{
    MODIFY_REG(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_PRIOR, (uint32_t)Priority);
}

/**
 * @brief Returns DMA channel priority level.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The priority level. Can be one of DMA_Priority_TypeDef values.
 */
__STATIC_INLINE DMA_Priority_TypeDef DMA_GetChannelPriorityLevel(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_Priority_TypeDef)READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_PRIOR);
}

/**
 * @brief Sets data transfer direction.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Direction The transfer direction. Can be one of DMA_Direction_TypeDef values.
 */
__STATIC_INLINE void DMA_SetDataTransferDirection(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_Direction_TypeDef Direction)
{
    MODIFY_REG(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_TT_FC, ((uint32_t)Direction & DMA_CH_CTL0_TT_FC));
}

/**
 * @brief Returns data transfer direction.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The transfer direction. Can be one of DMA_Direction_TypeDef values.
 */
__STATIC_INLINE DMA_Direction_TypeDef DMA_GetDataTransferDirection(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_Direction_TypeDef)READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_TT_FC);
}

/**
 * @brief Sets source operation mode (normal or auto reload).
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Mode The operation mode. Can be one of DMA_Mode_TypeDef values.
 */
__STATIC_INLINE void DMA_SetSrcMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_Mode_TypeDef Mode)
{
    MODIFY_REG(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_RELOAD_SRC, (uint32_t)Mode << DMA_CH_CFG0_RELOAD_SRC_Pos);
}

/**
 * @brief Returns source operation mode (normal or auto reload).
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The operation mode. Can be one of DMA_Mode_TypeDef values.
 */
__STATIC_INLINE DMA_Mode_TypeDef DMA_GetSrcMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_Mode_TypeDef)(READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_RELOAD_SRC) >> DMA_CH_CFG0_RELOAD_SRC_Pos);
}

/**
 * @brief Sets destination operation mode (normal or auto reload).
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Mode The operation mode. Can be one of DMA_Mode_TypeDef values.
 */
__STATIC_INLINE void DMA_SetDstMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_Mode_TypeDef Mode)
{
    MODIFY_REG(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_RELOAD_DST, (uint32_t)Mode << DMA_CH_CFG0_RELOAD_DST_Pos);
}

/**
 * @brief Returns destination operation mode (normal or auto reload).
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The operation mode. Can be one of DMA_Mode_TypeDef values.
 */
__STATIC_INLINE DMA_Mode_TypeDef DMA_GetDstMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_Mode_TypeDef)(READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_RELOAD_DST) >> DMA_CH_CFG0_RELOAD_DST_Pos);
}

/**
 * @brief Sets source address increment mode.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param IncMode The increment mode. Can be one of DMA_IncMode_TypeDef values.
 */
__STATIC_INLINE void DMA_SetSrcIncMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_IncMode_TypeDef IncMode)
{
    MODIFY_REG(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_SINC, (uint32_t)IncMode << DMA_CH_CTL0_SINC_Pos);
}

/**
 * @brief Returns source address increment mode.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The increment mode. Can be one of DMA_IncMode_TypeDef values.
 */
__STATIC_INLINE DMA_IncMode_TypeDef DMA_GetSrcIncMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_IncMode_TypeDef)(READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_SINC) >> DMA_CH_CTL0_SINC_Pos);
}

/**
 * @brief Sets destination address increment mode.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param IncMode The increment mode. Can be one of DMA_IncMode_TypeDef values.
 */
__STATIC_INLINE void DMA_SetDstIncMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_IncMode_TypeDef IncMode)
{
    MODIFY_REG(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_DINC, (uint32_t)IncMode << DMA_CH_CTL0_DINC_Pos);
}

/**
 * @brief Returns destination address increment mode.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The increment mode. Can be one of DMA_IncMode_TypeDef values.
 */
__STATIC_INLINE DMA_IncMode_TypeDef DMA_GetDstIncMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_IncMode_TypeDef)(READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_DINC) >> DMA_CH_CTL0_DINC_Pos);
}

/**
 * @brief Sets source transfer width.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param TransferWidth The transfer width. Can be one of DMA_TransferWidth_TypeDef values.
 */
__STATIC_INLINE void DMA_SetSrcTransferWidth(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_TransferWidth_TypeDef TransferWidth)
{
    MODIFY_REG(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_STW, (uint32_t)TransferWidth << DMA_CH_CTL0_STW_Pos);
}

/**
 * @brief Returns source transfer width.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The transfer width. Can be one of DMA_TransferWidth_TypeDef values.
 */
__STATIC_INLINE DMA_TransferWidth_TypeDef DMA_GetSrcTransferWidth(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_TransferWidth_TypeDef)(READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_STW) >> DMA_CH_CTL0_STW_Pos);
}

/**
 * @brief Sets destination transfer width.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param TransferWidth The transfer width. Can be one of DMA_TransferWidth_TypeDef values.
 */
__STATIC_INLINE void DMA_SetDstTransferWidth(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_TransferWidth_TypeDef TransferWidth)
{
    MODIFY_REG(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_DTW, (uint32_t)TransferWidth << DMA_CH_CTL0_DTW_Pos);
}

/**
 * @brief Returns destination transfer width.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The transfer width. Can be one of DMA_TransferWidth_TypeDef values.
 */
__STATIC_INLINE DMA_TransferWidth_TypeDef DMA_GetDstTransferWidth(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_TransferWidth_TypeDef)(READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_DTW) >> DMA_CH_CTL0_DTW_Pos);
}

/**
 * @brief Sets source burst transaction length.
 * @note The source burst transaction length is the number of data items, each of
 *       width CTL0.STW, to be read from the source every time a burst transaction
 *       request is made.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param BurstLength The burst transaction length. Can be one of DMA_BurstLength_TypeDef values.
 */
__STATIC_INLINE void DMA_SetSrcBurstLength(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_BurstLength_TypeDef BurstLength)
{
    MODIFY_REG(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_SRC_MSIZE, (uint32_t)BurstLength << DMA_CH_CTL0_SRC_MSIZE_Pos);
}

/**
 * @brief Returns source burst transaction length.
 * @note The source burst transaction length is the number of data items, each of
 *       width CTL0.STW, to be read from the source every time a burst transaction
 *       request is made.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The burst transaction length. Can be one of DMA_BurstLength_TypeDef values.
 */
__STATIC_INLINE DMA_BurstLength_TypeDef DMA_GetSrcBurstLength(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_BurstLength_TypeDef)(READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_SRC_MSIZE) >> DMA_CH_CTL0_SRC_MSIZE_Pos);
}

/**
 * @brief Sets destination burst transaction length.
 * @note The destination burst transaction length is the number of data items,
 *       each of width CTL0.DTW, to be written to the destination every time
 *       a burst transaction request is made.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param BurstLength The burst transaction length. Can be one of DMA_BurstLength_TypeDef values.
 */
__STATIC_INLINE void DMA_SetDstBurstLength(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_BurstLength_TypeDef BurstLength)
{
    MODIFY_REG(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_DST_MSIZE, (uint32_t)BurstLength << DMA_CH_CTL0_DST_MSIZE_Pos);
}

/**
 * @brief Returns destination burst transaction length.
 * @note The destination burst transaction length is the number of data items,
 *       each of width CTL0.DTW, to be written to the destination every time
 *       a burst transaction request is made.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The burst transaction length. Can be one of DMA_BurstLength_TypeDef values.
 */
__STATIC_INLINE DMA_BurstLength_TypeDef DMA_GetDstBurstLength(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_BurstLength_TypeDef)(READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_DST_MSIZE) >> DMA_CH_CTL0_DST_MSIZE_Pos);
}

/**
 * @brief Sets DMA block transfer size.
 * @note Must be set before the DMA channel is enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param NbData The number of data items to be transferred. Must be in the range of 0 to 255 bytes.
 */
__STATIC_INLINE void DMA_SetDataLength(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, uint32_t NbData)
{
    MODIFY_REG(DMAx->CH[Channel].CTL1, DMA_CH_CTL1_BLOCK_TS, ((NbData & DMA_CH_CTL1_BLOCK_TS_Msk) << DMA_CH_CTL1_BLOCK_TS_Pos));
}

/**
 * @brief Returns DMA block transfer size.
 * @note Once the transfer starts, the read back value is the total number of 
 *       data items already read from the source.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The number of data items to be transferred.
 */
__STATIC_INLINE uint32_t DMA_GetDataLength(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return READ_BIT(DMAx->CH[Channel].CTL1, DMA_CH_CTL1_BLOCK_TS);
}

/**
 * @brief Sets DMA FIFO mode.
 * @note The FIFO mode determines how much space or data needs to be available
 *       in FIFO before a burst transaction request is serviced.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Mode The FIFO mode. Can be one of DMA_FIFOMode_TypeDef values.
 */
__STATIC_INLINE void DMA_SetFIFOMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_FIFOMode_TypeDef Mode)
{
    MODIFY_REG(DMAx->CH[Channel].CFG1, DMA_CH_CFG1_FIFO_MODE, (uint32_t)Mode);
}

/**
 * @brief Returns DMA FIFO mode.
 * @note The FIFO mode determines how much space or data needs to be available
 *       in FIFO before a burst transaction request is serviced.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The FIFO mode. Can be one of DMA_FIFOMode_TypeDef values.
 */
__STATIC_INLINE DMA_FIFOMode_TypeDef DMA_GetFIFOMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_FIFOMode_TypeDef)READ_BIT(DMAx->CH[Channel].CFG1, DMA_CH_CFG1_FIFO_MODE);
}

/**
 * @brief Sets linked list pointer for multi-block transfer.
 * @note The address must be aligned to a 32-bit boundary.
 * @note If the address is set to 0x0, then transfers using linked lists
 *       are not enabled.
 * @note The address must be programmed prior to enabling the channel
 *       in order to set up transfer type.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param LLI The pointer to the first Linked List Item (LLI) in the list.
 */
__STATIC_INLINE void DMA_SetLLP(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_LLI_TypeDef *LLI)
{
    MODIFY_REG(DMAx->CH[Channel].LLP, DMA_CH_LLP_LOC, ((uint32_t)LLI & DMA_CH_LLP_LOC));
}

/**
 * @brief Returns linked list pointer for multi-block transfer.
 * @note If the address is set to 0x0, then transfers using linked lists
 *       are not enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The pointer to the first Linked List Item (LLI) in the list.
 */
__STATIC_INLINE DMA_LLI_TypeDef* DMA_GetLLP(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((DMA_LLI_TypeDef*)READ_BIT(DMAx->CH[Channel].LLP, DMA_CH_LLP_LOC));
}

/**
 * @brief Enables block chaining on the source side.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableSrcBlockChaining(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    SET_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_LLP_SRC_EN);
}

/**
 * @brief Disables block chaining on the source side.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableSrcBlockChaining(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    CLEAR_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_LLP_SRC_EN);
}

/**
 * @brief Checks if block chaining is enabled on the source side.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the block chaining is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledSrcBlockChaining(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_LLP_SRC_EN) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables block chaining on the destination side.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableDstBlockChaining(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    SET_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_LLP_DST_EN);
}

/**
 * @brief Disables block chaining on the destination side.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableDstBlockChaining(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    CLEAR_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_LLP_DST_EN);
}

/**
 * @brief Checks if block chaining is enabled on the destination side.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the block chaining is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledDstBlockChaining(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_LLP_DST_EN) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Selects source peripheral handshaking interface mode (hardware or software).
 * @note If the source peripheral is memory, then this setting is ignored.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Handshake The handshaking interface. Can be one of DMA_HandshakeMode_TypeDef values.
 */
__STATIC_INLINE void DMA_SetSrcHandshakeMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_HandshakeMode_TypeDef Handshake)
{
    MODIFY_REG(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_HS_SEL_SRC, ((uint32_t)Handshake << DMA_CH_CFG0_HS_SEL_SRC_Pos));
}

/**
 * @brief Returns source peripheral handshaking interface mode (hardware or software).
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The handshaking interface. Can be one of DMA_HandshakeMode_TypeDef values.
 */
__STATIC_INLINE DMA_HandshakeMode_TypeDef DMA_GetSrcHandshakeMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_HandshakeMode_TypeDef)(READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_HS_SEL_SRC) >> DMA_CH_CFG0_HS_SEL_SRC_Pos);
}

/**
 * @brief Selects destination peripheral handshaking interface mode (hardware or software).
 * @note If the destination peripheral is memory, then this setting is ignored.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Handshake The handshaking interface. Can be one of DMA_HandshakeMode_TypeDef values.
 */
__STATIC_INLINE void DMA_SetDstHandshakeMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_HandshakeMode_TypeDef Handshake)
{
    MODIFY_REG(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_HS_SEL_DST, ((uint32_t)Handshake << DMA_CH_CFG0_HS_SEL_DST_Pos));
}

/**
 * @brief Returns destination peripheral handshaking interface mode (hardware or software).
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The handshaking interface. Can be one of DMA_HandshakeMode_TypeDef values.
 */
__STATIC_INLINE DMA_HandshakeMode_TypeDef DMA_GetDstHandshakeMode(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_HandshakeMode_TypeDef)(READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_HS_SEL_DST) >> DMA_CH_CFG0_HS_SEL_DST_Pos);
}

/**
 * @brief Sets source peripheral hardware handshaking interface polarity.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Polarity The handshaking interface polarity. Can be one of DMA_HandshakePolarity_TypeDef values.
 */
__STATIC_INLINE void DMA_SetSrcHWHandshakePolarity(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_HandshakePolarity_TypeDef Polarity)
{
    MODIFY_REG(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_SRC_HS_POL, ((uint32_t)Polarity << DMA_CH_CFG0_SRC_HS_POL_Pos));
}

/**
 * @brief Returns source peripheral hardware handshaking interface polarity.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The handshaking interface polarity. Can be one of DMA_HandshakePolarity_TypeDef values.
 */
__STATIC_INLINE DMA_HandshakePolarity_TypeDef DMA_GetSrcHWHandshakePolarity(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_HandshakePolarity_TypeDef)(READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_SRC_HS_POL) >> DMA_CH_CFG0_SRC_HS_POL_Pos);
}

/**
 * @brief Sets destination peripheral hardware handshaking interface polarity.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Polarity The handshaking interface polarity. Can be one of DMA_HandshakePolarity_TypeDef values.
 */
__STATIC_INLINE void DMA_SetDstHWHandshakePolarity(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_HandshakePolarity_TypeDef Polarity)
{
    MODIFY_REG(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_DST_HS_POL, ((uint32_t)Polarity << DMA_CH_CFG0_DST_HS_POL_Pos));
}

/**
 * @brief Returns destination peripheral hardware handshaking interface polarity.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The handshaking interface polarity. Can be one of DMA_HandshakePolarity_TypeDef values.
 */
__STATIC_INLINE DMA_HandshakePolarity_TypeDef DMA_GetDstHWHandshakePolarity(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (DMA_HandshakePolarity_TypeDef)(READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_DST_HS_POL) >> DMA_CH_CFG0_DST_HS_POL_Pos);
}

/**
 * @brief Assigns a hardware handshaking interface to the source peripheral.
 * @note For correct operation only one peripheral (source or destination)
 *       should be assigned to the same handshaking interface.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Interface The hardware handshaking interface number in the range of 0 to 15.
 */
__STATIC_INLINE void DMA_SetSrcHWHandshakeInterface(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, uint8_t Interface)
{
    MODIFY_REG(DMAx->CH[Channel].CFG1, DMA_CH_CFG1_SRC_PER, (((uint32_t)Interface & 0xFUL) << DMA_CH_CFG1_SRC_PER_Pos));
}

/**
 * @brief Returns a hardware handshaking interface assigned to the source peripheral.
 * @note For correct operation only one peripheral (source or destination)
 *       should be assigned to the same handshaking interface.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The hardware handshaking interface number in the range of 0 to 15.
 */
__STATIC_INLINE uint8_t DMA_GetSrcHWHandshakeInterface(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (uint8_t)(READ_BIT(DMAx->CH[Channel].CFG1, DMA_CH_CFG1_SRC_PER) >> DMA_CH_CFG1_SRC_PER_Pos);
}

/**
 * @brief Assigns a hardware handshaking interface to the destination peripheral.
 * @note For correct operation only one peripheral (source or destination)
 *       should be assigned to the same handshaking interface.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @param Interface The hardware handshaking interface number in the range of 0 to 15.
 */
__STATIC_INLINE void DMA_SetDstHWHandshakeInterface(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, uint8_t Interface)
{
    MODIFY_REG(DMAx->CH[Channel].CFG1, DMA_CH_CFG1_DST_PER, (((uint32_t)Interface & 0xFUL) << DMA_CH_CFG1_DST_PER_Pos));
}

/**
 * @brief Returns a hardware handshaking interface assigned to the destination peripheral.
 * @note For correct operation only one peripheral (source or destination)
 *       should be assigned to the same handshaking interface.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @returns The hardware handshaking interface number in the range of 0 to 15.
 */
__STATIC_INLINE uint8_t DMA_GetDstHWHandshakeInterface(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return (uint8_t)(READ_BIT(DMAx->CH[Channel].CFG1, DMA_CH_CFG1_DST_PER) >> DMA_CH_CFG1_DST_PER_Pos);
}

/**
 * @brief Checks if FIFO_EMPTY flag is set or not.
 * @note FIFO_EMPTY bit indicates if there is data left in the channel FIFO.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval The state of bit (1 or 0).
 */
__STATIC_INLINE uint32_t DMA_IsActiveFlag_FIFO_EMPTY(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->CH[Channel].CFG0, DMA_CH_CFG0_FIFO_EMPTY) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if DONE flag is set or not.
 * @note Software can poll the CTL1.DONE bit to see if block transfer is complete.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval The state of bit (1 or 0).
 */
__STATIC_INLINE uint32_t DMA_IsActiveFlag_DONE(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->CH[Channel].CTL1, DMA_CH_CTL1_DONE) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears DONE flag.
 * @note The CTL1.DONE bit should be cleared prior to enabling DMA channel.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_ClearFlag_DONE(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    CLEAR_BIT(DMAx->CH[Channel].CTL1, DMA_CH_CTL1_DONE);
}

/**
 * @brief Enables DMA interrupts.
 * @note For interrupts to propagate past the RAW_* interrupt register stage,
 *       CTL0.INT_EN must be set and the relevant interrupt must be unmasked
 *       in MASK_* interrupt register. This function only controls the state of
 *       CTL0.INT_EN bit.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableIT(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    SET_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_INT_EN);
}

/**
 * @brief Disables DMA interrupts.
 * @note For interrupts to propagate past the RAW_* interrupt register stage,
 *       CTL0.INT_EN must be set and the relevant interrupt must be unmasked
 *       in MASK_* interrupt register. This function only controls the state of
 *       CTL0.INT_EN bit.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableIT(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    CLEAR_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_INT_EN);
}

/**
 * @brief Checks if DMA interrupts are enabled.
 * @note For interrupts to propagate past the RAW_* interrupt register stage,
 *       CTL0.INT_EN must be set and the relevant interrupt must be unmasked
 *       in MASK_* interrupt register. This function checks if CTL0.INT_EN bit
 *       is set.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if DMA interrupts are enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledIT(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->CH[Channel].CTL0, DMA_CH_CTL0_INT_EN) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables IntTfr interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableIT_IntTfr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_TFR, ((DMA_INT_MASK_TFR_WE_CH0 << Channel) | (DMA_INT_MASK_TFR_CH0 << Channel)));
}

/**
 * @brief Enables IntBlock interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableIT_IntBlock(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_BLOCK, ((DMA_INT_MASK_BLOCK_WE_CH0 << Channel) | (DMA_INT_MASK_BLOCK_CH0 << Channel)));
}

/**
 * @brief Enables IntSrcTran interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableIT_IntSrcTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_SRC_TRAN, ((DMA_INT_MASK_SRC_TRAN_WE_CH0 << Channel) | (DMA_INT_MASK_SRC_TRAN_CH0 << Channel)));
}

/**
 * @brief Enables IntDstTran interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableIT_IntDstTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_DST_TRAN, ((DMA_INT_MASK_DST_TRAN_WE_CH0 << Channel) | (DMA_INT_MASK_DST_TRAN_CH0 << Channel)));
}

/**
 * @brief Enables IntErr interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_EnableIT_IntErr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_ERR, ((DMA_INT_MASK_ERR_WE_CH0 << Channel) | (DMA_INT_MASK_ERR_CH0 << Channel)));
}

/**
 * @brief Disables IntTfr interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableIT_IntTfr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_TFR, (DMA_INT_MASK_TFR_WE_CH0 << Channel));
}

/**
 * @brief Disables IntBlock interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableIT_IntBlock(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_BLOCK, (DMA_INT_MASK_BLOCK_WE_CH0 << Channel));
}

/**
 * @brief Disables IntSrcTran interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableIT_IntSrcTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_SRC_TRAN, (DMA_INT_MASK_SRC_TRAN_WE_CH0 << Channel));
}

/**
 * @brief Disables IntDstTran interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableIT_IntDstTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_DST_TRAN, (DMA_INT_MASK_DST_TRAN_WE_CH0 << Channel));
}

/**
 * @brief Disables IntErr interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_DisableIT_IntErr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.MASK_ERR, (DMA_INT_MASK_ERR_WE_CH0 << Channel));
}

/**
 * @brief Checks if IntTfr interrupt is enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledIT_IntTfr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.MASK_TFR, (DMA_INT_MASK_TFR_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntBlock interrupt is enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledIT_IntBlock(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.MASK_BLOCK, (DMA_INT_MASK_BLOCK_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntSrcTran interrupt is enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledIT_IntSrcTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.MASK_SRC_TRAN, (DMA_INT_MASK_SRC_TRAN_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntDstTran interrupt is enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledIT_IntDstTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.MASK_DST_TRAN, (DMA_INT_MASK_DST_TRAN_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntErr interrupt is enabled.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsEnabledIT_IntErr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.MASK_ERR, (DMA_INT_MASK_ERR_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntTfr interrupt flag is active.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt flag is active, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsActiveIT_IntTfr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.RAW_TFR, (DMA_INT_RAW_TFR_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntBlock interrupt flag is active.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt flag is active, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsActiveIT_IntBlock(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.RAW_BLOCK, (DMA_INT_RAW_BLOCK_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntSrcTran interrupt flag is active.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt flag is active, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsActiveIT_IntSrcTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.RAW_SRC_TRAN, (DMA_INT_RAW_SRC_TRAN_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntDstTran interrupt flag is active.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt flag is active, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsActiveIT_IntDstTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.RAW_DST_TRAN, (DMA_INT_RAW_DST_TRAN_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Checks if IntErr interrupt flag is active.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 * @retval 1 if the interrupt flag is active, otherwise 0.
 */
__STATIC_INLINE uint32_t DMA_IsActiveIT_IntErr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    return ((READ_BIT(DMAx->INT.RAW_ERR, (DMA_INT_RAW_ERR_CH0 << Channel)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Clears IntTfr interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_ClearIT_IntTfr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.CLR_TFR, (DMA_INT_CLR_TFR_CH0 << Channel));
}

/**
 * @brief Clears IntBlock interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_ClearIT_IntBlock(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.CLR_BLOCK, (DMA_INT_CLR_BLOCK_CH0 << Channel));
}

/**
 * @brief Clears IntSrcTran interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_ClearIT_IntSrcTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.CLR_SRC_TRAN, (DMA_INT_CLR_SRC_TRAN_CH0 << Channel));
}

/**
 * @brief Clears IntDstTran interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_ClearIT_IntDstTran(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.CLR_DST_TRAN, (DMA_INT_CLR_DST_TRAN_CH0 << Channel));
}

/**
 * @brief Clears IntErr interrupt.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_ClearIT_IntErr(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->INT.CLR_ERR, (DMA_INT_CLR_ERR_CH0 << Channel));
}

/**
 * @brief Requests source software burst transaction.
 * @note The selected channel must be enabled prior to requesting transaction.
 * @note The request is ignored if software handshaking is not enabled
 *       for the source of the channel.
 * @note In order for a burst transaction request to start software must
 *       also request a single transaction.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_RequestSrcBurstTransaction(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->HS.REQ_SRC, ((DMA_HS_REQ_SRC_WE_CH0 << Channel) | (DMA_HS_REQ_SRC_CH0 << Channel)));
}

/**
 * @brief Requests destination software burst transaction.
 * @note The selected channel must be enabled prior to requesting transaction.
 * @note The request is ignored if software handshaking is not enabled
 *       for the destination of the channel.
 * @note In order for a burst transaction request to start software must
 *       also request a single transaction.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_RequestDstBurstTransaction(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->HS.REQ_DST, ((DMA_HS_REQ_DST_WE_CH0 << Channel) | (DMA_HS_REQ_DST_CH0 << Channel)));
}

/**
 * @brief Requests source software single transaction.
 * @note The selected channel must be enabled prior to requesting transaction.
 * @note The request is ignored if software handshaking is not enabled
 *       for the source of the channel.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_RequestSrcSingleTransaction(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->HS.SGL_REQ_SRC, ((DMA_HS_SGL_REQ_SRC_WE_CH0 << Channel) | (DMA_HS_SGL_REQ_SRC_CH0 << Channel)));
}

/**
 * @brief Requests destination software single transaction.
 * @note The selected channel must be enabled prior to requesting transaction.
 * @note The request is ignored if software handshaking is not enabled
 *       for the destination of the channel.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_RequestDstSingleTransaction(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    WRITE_REG(DMAx->HS.SGL_REQ_DST, ((DMA_HS_SGL_REQ_DST_WE_CH0 << Channel) | (DMA_HS_SGL_REQ_DST_CH0 << Channel)));
}

/**
 * @brief Waits for source software burst or single transaction to complete.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_WaitForSrcTransactionComplete(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    while ((READ_BIT(DMAx->HS.REQ_SRC, (DMA_HS_REQ_SRC_CH0 << Channel)) != 0x0UL) ||
           (READ_BIT(DMAx->HS.SGL_REQ_SRC, (DMA_HS_SGL_REQ_SRC_CH0 << Channel)) != 0x0UL));
}

/**
 * @brief Waits for destination software burst or single transaction to complete.
 * @param DMAx The DMA instance.
 * @param Channel The DMA channel. Can be one of DMA_Channels_TypeDef values.
 */
__STATIC_INLINE void DMA_WaitForDstTransactionComplete(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel)
{
    while ((READ_BIT(DMAx->HS.REQ_DST, (DMA_HS_REQ_DST_CH0 << Channel)) != 0x0UL) ||
           (READ_BIT(DMAx->HS.SGL_REQ_DST, (DMA_HS_SGL_REQ_DST_CH0 << Channel)) != 0x0UL));
}

ErrorStatus DMA_DeInit(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel);
ErrorStatus DMA_Init(DMA_TypeDef *DMAx, DMA_Channels_TypeDef Channel, DMA_InitStruct_TypeDef *DMA_InitStruct);
void DMA_StructInit(DMA_InitStruct_TypeDef *DMA_InitStruct);
void DMA_LLI_Init(DMA_LLI_TypeDef *LLI, DMA_InitStruct_TypeDef *DMA_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_DMA_H */
