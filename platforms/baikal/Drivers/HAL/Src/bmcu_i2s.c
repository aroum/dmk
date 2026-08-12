/**
 * *****************************************************************************
 *  @file       bmcu_i2s.c
 *  @author     Baikal electronics SDK team
 *  @brief      Inter-Integrated Circuit Sound (I2S) module driver
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

/* Includes ------------------------------------------------------------------*/
#include "bmcu_i2s.h"
#include "bmcu_def.h"

#define IS_I2S_MODE(__VALUE__)                  (((__VALUE__) == I2S_MODE_SLAVE_TX) || \
                                                 ((__VALUE__) == I2S_MODE_SLAVE_RX))

#define IS_I2S_DATA_RESOLUTION(__VALUE__)       (((__VALUE__) == I2S_DATA_RESOLUTION_IGNORE) || \
                                                 ((__VALUE__) == I2S_DATA_RESOLUTION_12BIT) || \
                                                 ((__VALUE__) == I2S_DATA_RESOLUTION_16BIT) || \
                                                 ((__VALUE__) == I2S_DATA_RESOLUTION_20BIT) || \
                                                 ((__VALUE__) == I2S_DATA_RESOLUTION_24BIT) || \
                                                 ((__VALUE__) == I2S_DATA_RESOLUTION_32BIT))

#define IS_I2S_FIFO_THRESHOLD(__VALUE__)        (((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_1) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_2) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_3) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_4) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_5) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_6) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_7) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_8) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_9) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_10) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_11) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_12) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_13) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_14) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_15) || \
                                                 ((__VALUE__) == I2S_FIFO_TRIGGER_LEVEL_16))

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief Deinitializes an I2S instance.
 * @param I2Sx The I2S instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The I2S instance have been deinitialized.
 *         - ERROR: The I2S instance have not been deinitialized.
 */
ErrorStatus I2S_DeInit(I2S_TypeDef *I2Sx)
{
    ErrorStatus status = SUCCESS;
  
    /* Check the parameters */
    assert(IS_I2S_ALL_INSTANCE(I2Sx));

    /* Disable the I2S instance */
    I2S_Disable(I2Sx);

    /* Disable the I2S receiver */
    I2S_Disable_RX(I2Sx);

    /* Disable the I2S transmitter */
    I2S_Disable_TX(I2Sx);

    /* Disable DMA transfer */
    I2S_DisableDMAReq_RX(I2Sx);
    I2S_DisableDMAReq_TX(I2Sx);

    /* Set default FIFO trigger level */
    MODIFY_REG(I2Sx->TFCR0, I2S_TFCR0_TXCHET, (uint32_t)I2S_FIFO_TRIGGER_LEVEL_3 << I2S_TFCR0_TXCHET_Pos);
    MODIFY_REG(I2Sx->RFCR0, I2S_RFCR0_RXCHDT, (uint32_t)I2S_FIFO_TRIGGER_LEVEL_3 << I2S_RFCR0_RXCHDT_Pos);

    /* Set default data resolution */
    MODIFY_REG(I2Sx->TCR0, I2S_TCR0_WLEN, (uint32_t)I2S_DATA_RESOLUTION_32BIT << I2S_TCR0_WLEN_Pos);
    MODIFY_REG(I2Sx->RCR0, I2S_RCR0_WLEN, (uint32_t)I2S_DATA_RESOLUTION_32BIT << I2S_RCR0_WLEN_Pos);

    return status;
}

/**
 * @brief Initializes an I2S instance.
 * @note I2S_InitStruct structure should be initialized prior to calling this function.
 * @param I2Sx The I2S instance.
 * @param I2S_InitStruct The pointer to I2S_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The I2S instance have been initialized.
 *         - ERROR: The I2S instance have not been initialized.
 */
ErrorStatus I2S_Init(I2S_TypeDef *I2Sx, I2S_InitStruct_TypeDef *I2S_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check parameters */
    assert(IS_I2S_ALL_INSTANCE(I2Sx));
    assert(IS_I2S_MODE(I2S_InitStruct->Mode));
    assert(IS_I2S_DATA_RESOLUTION(I2S_InitStruct->DataResolution));
    assert(IS_FUNCTIONAL_STATE(I2S_InitStruct->DMATransfer));
    assert(IS_I2S_FIFO_THRESHOLD(I2S_InitStruct->FIFOThreshold));

    if (I2S_IsEnabled(I2Sx) == 0U)
    {
        I2S_Enable(I2Sx);

        if (I2S_InitStruct->Mode == I2S_MODE_SLAVE_TX)
        {
            /* Slave transmitter */

            /* Configure data resolution */
            MODIFY_REG(I2Sx->TCR0, I2S_TCR0_WLEN, (uint32_t)I2S_InitStruct->DataResolution << I2S_TCR0_WLEN_Pos);

            if (I2S_InitStruct->DMATransfer == ENABLE)
            {
                /* DMA transfer mode */

                /* Enable DMA transfer */
                I2S_EnableDMAReq_TX(I2Sx);
            }
            else
            {
                /* FIFO transfer mode */

                /* Set FIFO trigger level */
                MODIFY_REG(I2Sx->TFCR0, I2S_TFCR0_TXCHET, (uint32_t)I2S_InitStruct->FIFOThreshold << I2S_TFCR0_TXCHET_Pos);
            }

        }
        else
        {
            /* Slave receiver */

            /* Configure data resolution */
            MODIFY_REG(I2Sx->RCR0, I2S_RCR0_WLEN, (uint32_t)I2S_InitStruct->DataResolution << I2S_RCR0_WLEN_Pos);

            if (I2S_InitStruct->DMATransfer == ENABLE)
            {
                /* DMA transfer mode */

                /* Enable DMA transfer */
                I2S_EnableDMAReq_RX(I2Sx);
            }
            else
            {
                /* FIFO transfer mode */

                /* Set FIFO trigger level */
                MODIFY_REG(I2Sx->RFCR0, I2S_RFCR0_RXCHDT, (uint32_t)I2S_InitStruct->FIFOThreshold << I2S_RFCR0_RXCHDT_Pos);
            }
        }

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Sets each field of I2S_InitStruct_TypeDef structure to default value.
 * @param I2S_InitStruct The pointer to I2S_InitStruct_TypeDef structure.
 */
void I2S_StructInit(I2S_InitStruct_TypeDef *I2S_InitStruct)
{
    /* Set I2S_InitStruct fields to default values */
    I2S_InitStruct->Mode           = I2S_MODE_SLAVE_TX;
    I2S_InitStruct->DataResolution = I2S_DATA_RESOLUTION_32BIT;
    I2S_InitStruct->DMATransfer    = DISABLE;
    I2S_InitStruct->FIFOThreshold  = I2S_FIFO_TRIGGER_LEVEL_3;
}
