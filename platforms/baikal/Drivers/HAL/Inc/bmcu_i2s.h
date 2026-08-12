/**
 * *****************************************************************************
 *  @file       bmcu_i2s.h
 *  @author     Baikal electronics SDK team
 *  @brief      Inter-Integrated Circuit Sound (I2S) module driver header file
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

#ifndef __BMCU_I2S_H
#define __BMCU_I2S_H

/* Includes ------------------------------------------------------------------*/
#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @brief I2S operation mode */
typedef enum {
    I2S_MODE_SLAVE_TX = 0,  /*!< Slave transmitter */
    I2S_MODE_SLAVE_RX       /*!< Slave receiver */
} I2S_Mode_TypeDef;

/** @brief I2S data resolution */
typedef enum {
    I2S_DATA_RESOLUTION_IGNORE = 0,     /*!< Ignore the word length */
    I2S_DATA_RESOLUTION_12BIT,          /*!< 12-bit data resolution */
    I2S_DATA_RESOLUTION_16BIT,          /*!< 16-bit data resolution */
    I2S_DATA_RESOLUTION_20BIT,          /*!< 20-bit data resolution */
    I2S_DATA_RESOLUTION_24BIT,          /*!< 24-bit data resolution */
    I2S_DATA_RESOLUTION_32BIT           /*!< 32-bit data resolution */
} I2S_DataResolution_TypeDef;

/** @brief I2S FIFO threshold level */
typedef enum {
    I2S_FIFO_TRIGGER_LEVEL_1 = 0,   /*!< FIFO trigger level 1 */
    I2S_FIFO_TRIGGER_LEVEL_2,       /*!< FIFO trigger level 2 */
    I2S_FIFO_TRIGGER_LEVEL_3,       /*!< FIFO trigger level 3 */
    I2S_FIFO_TRIGGER_LEVEL_4,       /*!< FIFO trigger level 4 */
    I2S_FIFO_TRIGGER_LEVEL_5,       /*!< FIFO trigger level 5 */
    I2S_FIFO_TRIGGER_LEVEL_6,       /*!< FIFO trigger level 6 */
    I2S_FIFO_TRIGGER_LEVEL_7,       /*!< FIFO trigger level 7 */
    I2S_FIFO_TRIGGER_LEVEL_8,       /*!< FIFO trigger level 8 */
    I2S_FIFO_TRIGGER_LEVEL_9,       /*!< FIFO trigger level 9 */
    I2S_FIFO_TRIGGER_LEVEL_10,      /*!< FIFO trigger level 10 */
    I2S_FIFO_TRIGGER_LEVEL_11,      /*!< FIFO trigger level 11 */
    I2S_FIFO_TRIGGER_LEVEL_12,      /*!< FIFO trigger level 12 */
    I2S_FIFO_TRIGGER_LEVEL_13,      /*!< FIFO trigger level 13 */
    I2S_FIFO_TRIGGER_LEVEL_14,      /*!< FIFO trigger level 14 */
    I2S_FIFO_TRIGGER_LEVEL_15,      /*!< FIFO trigger level 15 */
    I2S_FIFO_TRIGGER_LEVEL_16       /*!< FIFO trigger level 16 */
} I2S_FIFOThreshold_TypeDef;

/* Exported types ------------------------------------------------------------*/
/** @brief I2S Init Structure definition */
typedef struct
{
    I2S_Mode_TypeDef Mode;                      /*!< Operation mode */
    I2S_DataResolution_TypeDef DataResolution;  /*!< Data resolution */
    FunctionalState DMATransfer;                /*!< DMA transfer control */
    I2S_FIFOThreshold_TypeDef FIFOThreshold;    /*!< FIFO threshold level */
} I2S_InitStruct_TypeDef;

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Writes a value in I2S register.
 * @param INSTANCE The I2S instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define I2S_WriteReg(INSTANCE, REG, VALUE)      WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in I2S register.
 * @param INSTANCE The I2S instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define I2S_ReadReg(INSTANCE, REG)              READ_REG(INSTANCE->REG)

/**
 * @brief Enables the selected I2S instance.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_Enable(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->IER, I2S_IER_IEN);
}

/**
 * @brief Disables the selected I2S instance.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_Disable(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->IER, I2S_IER_IEN);
}

/**
 * @brief Checks if the selected I2S instance is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if the I2S instance is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabled(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->IER, I2S_IER_IEN) == I2S_IER_IEN) ? 1UL : 0UL);
}

/**
 * @brief Enables the I2S receiver.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_Enable_RX(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->RER0, I2S_RER0_RXCHEN0);
    SET_BIT(I2Sx->IRER, I2S_IRER_RXEN);
}

/**
 * @brief Disables the I2S receiver.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_Disable_RX(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->IRER, I2S_IRER_RXEN);
    CLEAR_BIT(I2Sx->RER0, I2S_RER0_RXCHEN0);
}

/**
 * @brief Checks if the I2S receiver is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if the I2S receiver is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabled_RX(I2S_TypeDef *I2Sx)
{
    return (((READ_BIT(I2Sx->IRER, I2S_IRER_RXEN) == I2S_IRER_RXEN) &
            (READ_BIT(I2Sx->RER0, I2S_RER0_RXCHEN0) == I2S_RER0_RXCHEN0)) ? 1UL : 0UL);
}

/**
 * @brief Enables the I2S transmitter.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_Enable_TX(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->TER0, I2S_TER0_TXCHEN0);
    SET_BIT(I2Sx->ITER, I2S_ITER_TXEN);
}

/**
 * @brief Disables the I2S transmitter.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_Disable_TX(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->ITER, I2S_ITER_TXEN);
    CLEAR_BIT(I2Sx->TER0, I2S_TER0_TXCHEN0);
}

/**
 * @brief Checks if the I2S transmitter is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if the I2S transmitter is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabled_TX(I2S_TypeDef *I2Sx)
{
    return (((READ_BIT(I2Sx->ITER, I2S_ITER_TXEN) == I2S_ITER_TXEN) &
            (READ_BIT(I2Sx->TER0, I2S_TER0_TXCHEN0) == I2S_TER0_TXCHEN0)) ? 1UL : 0UL);
}

/**
 * @brief Sets I2S receiver data resolution.
 * @param I2Sx The I2S instance.
 * @param DataResolution The data resolution. Can be one of I2S_DataResolution_TypeDef values.
 */
__STATIC_INLINE void I2S_SetDataResolution_RX(I2S_TypeDef *I2Sx, I2S_DataResolution_TypeDef DataResolution)
{
    MODIFY_REG(I2Sx->RCR0, I2S_RCR0_WLEN, ((uint32_t)DataResolution << I2S_RCR0_WLEN_Pos));
}

/**
 * @brief Returns I2S receiver data resolution.
 * @param I2Sx The I2S instance.
 * @returns The data resolution. Can be one of I2S_DataResolution_TypeDef values.
 */
__STATIC_INLINE I2S_DataResolution_TypeDef I2S_GetDataResolution_RX(I2S_TypeDef *I2Sx)
{
    return (I2S_DataResolution_TypeDef)(READ_BIT(I2Sx->RCR0, I2S_RCR0_WLEN) >> I2S_RCR0_WLEN_Pos);
}

/**
 * @brief Sets I2S transmitter data resolution.
 * @param I2Sx The I2S instance.
 * @param DataResolution The data resolution. Can be one of I2S_DataResolution_TypeDef values.
 */
__STATIC_INLINE void I2S_SetDataResolution_TX(I2S_TypeDef *I2Sx, I2S_DataResolution_TypeDef DataResolution)
{
    MODIFY_REG(I2Sx->TCR0, I2S_TCR0_WLEN, (uint32_t)DataResolution << I2S_TCR0_WLEN_Pos);
}

/**
 * @brief Returns I2S transmitter data resolution.
 * @param I2Sx The I2S instance.
 * @returns The data resolution. Can be one of I2S_DataResolution_TypeDef values.
 */
__STATIC_INLINE I2S_DataResolution_TypeDef I2S_GetDataResolution_TX(I2S_TypeDef *I2Sx)
{
    return (I2S_DataResolution_TypeDef)(READ_BIT(I2Sx->TCR0, I2S_TCR0_WLEN) >> I2S_TCR0_WLEN_Pos);
}

/**
 * @brief Enables RXDA interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_EnableIT_RXDA(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->IMR0, I2S_IMR0_RXDAM);
}

/**
 * @brief Enables RXFO interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_EnableIT_RXFO(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->IMR0, I2S_IMR0_RXFOM);
}

/**
 * @brief Enables TXFE interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_EnableIT_TXFE(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->IMR0, I2S_IMR0_TXFEM);
}

/**
 * @brief Enables TXFO interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_EnableIT_TXFO(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->IMR0, I2S_IMR0_TXFOM);
}

/**
 * @brief Disables RXDA interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_DisableIT_RXDA(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->IMR0, I2S_IMR0_RXDAM);
}

/**
 * @brief Disables RXFO interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_DisableIT_RXFO(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->IMR0, I2S_IMR0_RXFOM);
}

/**
 * @brief Disables TXFE interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_DisableIT_TXFE(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->IMR0, I2S_IMR0_TXFEM);
}

/**
 * @brief Disables TXFO interrupt.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_DisableIT_TXFO(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->IMR0, I2S_IMR0_TXFOM);
}

/**
 * @brief Checks if RXDA interrupt is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabledIT_RXDA(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->IMR0, I2S_IMR0_RXDAM) == I2S_IMR0_RXDAM) ? 0UL : 1UL);
}

/**
 * @brief Checks if RXFO interrupt is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabledIT_RXFO(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->IMR0, I2S_IMR0_RXFOM) == I2S_IMR0_RXFOM) ? 0UL : 1UL);
}

/**
 * @brief Checks if TXFE interrupt is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabledIT_TXFE(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->IMR0, I2S_IMR0_TXFEM) == I2S_IMR0_TXFEM) ? 0UL : 1UL);
}

/**
 * @brief Checks if TXFO interrupt is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabledIT_TXFO(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->IMR0, I2S_IMR0_TXFOM) == I2S_IMR0_TXFOM) ? 0UL : 1UL);
}

/**
 * @brief Checks if RXDA interrupt flag is set or not.
 * @param I2Sx The I2S instance.
 * @retval The state of bit (1 or 0).
 */
__STATIC_INLINE uint32_t I2S_IsActiveIT_RXDA(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->ISR0, I2S_ISR0_RXDA) == I2S_ISR0_RXDA) ? 1UL : 0UL);
}

/**
 * @brief Checks if RXFO interrupt flag is set or not.
 * @param I2Sx The I2S instance.
 * @retval The state of bit (1 or 0).
 */
__STATIC_INLINE uint32_t I2S_IsActiveIT_RXFO(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->ISR0, I2S_ISR0_RXFO) == I2S_ISR0_RXFO) ? 1UL : 0UL);
}

/**
 * @brief Checks if TXFE interrupt flag is set or not.
 * @param I2Sx The I2S instance.
 * @retval The state of bit (1 or 0).
 */
__STATIC_INLINE uint32_t I2S_IsActiveIT_TXFE(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->ISR0, I2S_ISR0_TXFE) == I2S_ISR0_TXFE) ? 1UL : 0UL);
}

/**
 * @brief Checks if TXFO interrupt flag is set or not.
 * @param I2Sx The I2S instance.
 * @retval The state of bit (1 or 0).
 */
__STATIC_INLINE uint32_t I2S_IsActiveIT_TXFO(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->ISR0, I2S_ISR0_TXFO) == I2S_ISR0_TXFO) ? 1UL : 0UL);
}

/**
 * @brief Clears RXFO flag.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_ClearIT_RXFO(I2S_TypeDef *I2Sx)
{
    __IO uint32_t tmpreg;
    tmpreg = I2Sx->ROR0;
    (void)tmpreg;
}

/**
 * @brief Clears TXFO flag.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_ClearIT_TXFO(I2S_TypeDef *I2Sx)
{
    __IO uint32_t tmpreg;
    tmpreg = I2Sx->TOR0;
    (void)tmpreg;
}

/**
 * @brief Enables the receiver DMA.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_EnableDMAReq_RX(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->DMACR, I2S_DMACR_DMAEN_RX);
}

/**
 * @brief Disables the receiver DMA.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_DisableDMAReq_RX(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->DMACR, I2S_DMACR_DMAEN_RX);
}

/**
 * @brief Checks if the receiver DMA is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabledDMAReq_RX(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->DMACR, I2S_DMACR_DMAEN_RX) == I2S_DMACR_DMAEN_RX) ? 1UL : 0UL);
}

/**
 * @brief Enables the transmitter DMA.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_EnableDMAReq_TX(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->DMACR, I2S_DMACR_DMAEN_TX);
}

/**
 * @brief Disables the transmitter DMA.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_DisableDMAReq_TX(I2S_TypeDef *I2Sx)
{
    CLEAR_BIT(I2Sx->DMACR, I2S_DMACR_DMAEN_TX);
}

/**
 * @brief Checks if the transmitter DMA is enabled.
 * @param I2Sx The I2S instance.
 * @retval 1 if enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t I2S_IsEnabledDMAReq_TX(I2S_TypeDef *I2Sx)
{
    return ((READ_BIT(I2Sx->DMACR, I2S_DMACR_DMAEN_TX) == I2S_DMACR_DMAEN_TX) ? 1UL : 0UL);
}

/**
 * @brief Reads the left stereo sample from FIFO.
 * @note This must be followed by reading the right stereo sample.
 * @param I2Sx The I2S instance.
 * @returns The data received.
 */
__STATIC_INLINE uint32_t I2S_ReceiveDataLeft(I2S_TypeDef *I2Sx)
{
    return READ_REG(I2Sx->LRBR0);
}

/**
 * @brief Reads the right stereo sample from FIFO.
 * @note This must be preceeded by reading the left stereo sample.
 * @param I2Sx The I2S instance.
 * @returns The data received.
 */
__STATIC_INLINE uint32_t I2S_ReceiveDataRight(I2S_TypeDef *I2Sx)
{
    return READ_REG(I2Sx->RRBR0);
}

/**
 * @brief Writes the left stereo sample to FIFO.
 * @note This must be followed by writing the right stereo sample.
 * @param I2Sx The I2S instance.
 * @param TxData The data to be written.
 */
__STATIC_INLINE void I2S_TransmitDataLeft(I2S_TypeDef *I2Sx, uint32_t TxData)
{
    WRITE_REG(I2Sx->LTHR0, TxData);
}

/**
 * @brief Writes the right stereo sample to FIFO.
 * @note This must be preceeded by writing the left stereo sample.
 * @param I2Sx The I2S instance.
 * @param TxData The data to be written.
 */
__STATIC_INLINE void I2S_TransmitDataRight(I2S_TypeDef *I2Sx, uint32_t TxData)
{
    WRITE_REG(I2Sx->RTHR0, TxData);
}

/**
 * @brief Reads data from the receiver DMA register.
 * @note The left stereo sample must be read first followed by reading
 *       the right stereo sample.
 * @param I2Sx The I2S instance.
 * @returns The data received.
 */
__STATIC_INLINE uint32_t I2S_ReceiveDMAData(I2S_TypeDef *I2Sx)
{
    return READ_REG(I2Sx->RXDMA);
}

/**
 * @brief Writes data to the transmitter DMA register.
 * @note The left stereo sample must be written first followed by writing
 *       the right stereo sample.
 * @param I2Sx The I2S instance.
 * @param TxData The data to be written.
 */
__STATIC_INLINE void I2S_TransmitDMAData(I2S_TypeDef *I2Sx, uint32_t TxData)
{
    WRITE_REG(I2Sx->TXDMA, TxData);
}

/**
 * @brief Flush I2S TX FIFO buffer.
 * @note This is a self clearing bit.
 *       The TX channel or block must be disabled prior to writing to this bit.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_FlushTransmitFIFO(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->TFF0, I2S_TFF0_TXCHFR);
}

/**
 * @brief Flush I2S RX FIFO buffer.
 * @note This is a self clearing bit.
 *       The RX channel or block must be disabled prior to writing to this bit.
 * @param I2Sx The I2S instance.
 */
__STATIC_INLINE void I2S_FlushReceiveFIFO(I2S_TypeDef *I2Sx)
{
    SET_BIT(I2Sx->RFF0, I2S_RFF0_RXCHFR);
}

/**
 * @brief Sets each field of I2S_InitStruct_TypeDef structure to default value.
 * @param I2S_InitStruct The pointer to I2S_InitStruct_TypeDef structure.
 */
void I2S_StructInit(I2S_InitStruct_TypeDef *I2S_InitStruct);

/**
 * @brief Initializes an I2S instance.
 * @note I2S_InitStruct structure should be initialized prior to calling this function.
 * @param I2Sx The I2S instance.
 * @param I2S_InitStruct The pointer to I2S_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The I2S instance have been initialized.
 *         - ERROR: The I2S instance have not been initialized.
 * @see     I2S_Deinit()
 */
ErrorStatus I2S_Init(I2S_TypeDef *I2Sx, I2S_InitStruct_TypeDef *I2S_InitStruct);

/**
 * @brief Deinitializes an I2S instance.
 * @param I2Sx The I2S instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The I2S instance have been deinitialized.
 *         - ERROR: The I2S instance have not been deinitialized.
 * @see    I2S_Init()
 */
ErrorStatus I2S_DeInit(I2S_TypeDef *I2Sx);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_I2S_H */
