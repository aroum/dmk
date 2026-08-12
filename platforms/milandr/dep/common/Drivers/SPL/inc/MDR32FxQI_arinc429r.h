/**
  ******************************************************************************
  * @file    MDR32FxQI_arinc429r.h
  * @author  Milandr Application Team
  * @version V2.0.3i
  * @date    23/10/2024
  * @brief   This file contains all the functions prototypes for the ARINC429R
  *          firmware library.
  ******************************************************************************
  * <br><br>
  *
  * THE PRESENT FIRMWARE IS FOR GUIDANCE ONLY. IT AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING MILANDR'S PRODUCTS IN ORDER TO FACILITATE
  * THE USE AND SAVE TIME. MILANDR SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES RESULTING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR A USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2025 Milandr</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MDR32FxQI_ARINC429R_H_
#define MDR32FxQI_ARINC429R_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup ARINC429R ARINC429R
  * @warning This module can be used only for microcontrollers MDR32F1QI, K1986VE1xI.
  * @{
  */

#if defined (USE_K1986VE1xI)

/** @defgroup ARINC429R_Exported_Types ARINC429R Exported Types
  * @{
  */

/**
  * @brief ARINC429R Clock
  */
typedef enum
{
    ARINC429R_CLK_12_5_KHz = ((uint32_t)0x1), /*!< Frequency of data reception 12.5 KHz if DIV != 0 */
    ARINC429R_CLK_100_KHz  = ((uint32_t)0x0)  /*!< Frequency of data reception 100 KHz if DIV != 0 */
} ARINC429R_Clk;

#define IS_ARINC429R_CLK(CLK)   (((CLK) == ARINC429R_CLK_12_5_KHz) ||\
                                 ((CLK) == ARINC429R_CLK_100_KHz))

/**
  * @brief ARINC429R Channel
  */
typedef enum
{
    ARINC429R_CHANNEL1  = ((uint32_t)0x0), /*!< Specifies the ARINC429R channel 1. */
    ARINC429R_CHANNEL2  = ((uint32_t)0x1), /*!< Specifies the ARINC429R channel 2. */
    ARINC429R_CHANNEL3  = ((uint32_t)0x2), /*!< Specifies the ARINC429R channel 3. */
    ARINC429R_CHANNEL4  = ((uint32_t)0x3), /*!< Specifies the ARINC429R channel 4. */
    ARINC429R_CHANNEL5  = ((uint32_t)0x4), /*!< Specifies the ARINC429R channel 5. */
    ARINC429R_CHANNEL6  = ((uint32_t)0x5), /*!< Specifies the ARINC429R channel 6. */
    ARINC429R_CHANNEL7  = ((uint32_t)0x6), /*!< Specifies the ARINC429R channel 7 */
    ARINC429R_CHANNEL8  = ((uint32_t)0x7), /*!< Specifies the ARINC429R channel 8. */
} ARINC429R_Channel;

#define IS_ARINC429R_CHANNEL(CHANNEL)   (((CHANNEL) == ARINC429R_CHANNEL1) || \
                                         ((CHANNEL) == ARINC429R_CHANNEL2) || \
                                         ((CHANNEL) == ARINC429R_CHANNEL3) || \
                                         ((CHANNEL) == ARINC429R_CHANNEL4) || \
                                         ((CHANNEL) == ARINC429R_CHANNEL5) || \
                                         ((CHANNEL) == ARINC429R_CHANNEL6) || \
                                         ((CHANNEL) == ARINC429R_CHANNEL7) || \
                                         ((CHANNEL) == ARINC429R_CHANNEL8))

/**
  * @brief ARINC429R Interrupt Defintion
  */
typedef enum
{
    ARINC429R_IT_INT_HF = ((uint32_t)0x8), /*!< Half-Full FIFO global interrupt enable */
    ARINC429R_IT_INT_FF = ((uint32_t)0x4), /*!< Full FIFO global interrupt enable */
    ARINC429R_IT_INT_ER = ((uint32_t)0x2), /*!< Receive error global interrupt enable */
    ARINC429R_IT_INT_DR = ((uint32_t)0x1)  /*!< Data presence in FIFO global interrupt enable */
} ARINC429R_IT_Def;

#define ARINC429R_IT_MASK   (ARINC429R_IT_INT_HF | \
                             ARINC429R_IT_INT_FF | \
                             ARINC429R_IT_INT_ER | \
                             ARINC429R_IT_INT_DR)

#define IS_ARINC429R_IT(IT) (((IT) & ~(uint32_t)(ARINC429R_IT_MASK)) == 0)

/**
  * @brief ARINC429R Flags
  */
typedef enum
{
    ARINC429R_FLAG_ERR = ((uint32_t)0x0), /*!< Receive error flag */
    ARINC429R_FLAG_DR  = ((uint32_t)0x1), /*!< Data presence in FIFO flag */
    ARINC429R_FLAG_HF  = ((uint32_t)0x2), /*!< Half-Full FIFO flag */
    ARINC429R_FLAG_FF  = ((uint32_t)0x3)  /*!< Full FIFO flag */
} ARINC429R_Flags;

#define IS_ARINC429R_FLAG(FLAG) (((FLAG) == ARINC429R_FLAG_ERR) || \
                                 ((FLAG) == ARINC429R_FLAG_DR)  || \
                                 ((FLAG) == ARINC429R_FLAG_HF)  || \
                                 ((FLAG) == ARINC429R_FLAG_FF))

/**
  * @brief ARINC429R ITMask
  * @note This enumeration is only for MCU MDR32F1QI at revision 3 and K1986VE1xI.
  */
typedef enum
{
    ARINC429R_ITMask_HF = ((uint32_t)0x1), /*!< Half-Full FIFO interrupt mask */
    ARINC429R_ITMask_FF = ((uint32_t)0x2), /*!< Full FIFO interrupt mask */
    ARINC429R_ITMask_ER = ((uint32_t)0x4), /*!< Receive error interrupt mask */
    ARINC429R_ITMask_DR = ((uint32_t)0x8) /*!< Data presence interrupt mask */
} ARINC429R_ITMask;

#define IS_ARINC429R_ITMask(ITMask) (((ITMask) == ARINC429R_ITMask_HF) || \
                                     ((ITMask) == ARINC429R_ITMask_FF) || \
                                     ((ITMask) == ARINC429R_ITMask_ER) || \
                                     ((ITMask) == ARINC429R_ITMask_DR))

/**
  * @brief ARINC429 Init Channel Structure definition
  */
typedef struct
{
    ARINC429R_Clk   ARINC429R_CLK;  /*!< This member specifies the data reception speed.
                                         This parameter can be a value of @ref ARINC429R_Clk */
    FunctionalState ARINC429R_SD;   /*!< This member allows decoding 9 and 10 data bits.
                                         This parameter can be a value of @ref FunctionalState */
    FunctionalState ARINC429R_LB;   /*!< This member allows tag detection.
                                         This parameter can be a value of @ref FunctionalState */
    BitStatus       ARINC429R_SDI1; /*!< This member specifies the bit comparison SDI1.
                                         This parameter can be a value of @ref BitStatus */
    BitStatus       ARINC429R_SDI2; /*!< This member specifies the bit comparison SDI2.
                                         This parameter can be a value of @ref BitStatus */
    uint32_t        ARINC429R_DIV;  /*!< This member specifies the individual core
                                         frequency divider for channel reference frequency.
                                         This parameter can be a number from 0 to 0xFF. */
} ARINC429R_InitChannelTypeDef;

/** @} */ /* End of group ARINC429R_Exported_Types */


/** @defgroup ARINC429R_Exported_Constants ARINC429R Exported Constants
  * @{
  */
#define IS_ARINC429R_BRG(BRG)      (((BRG) & ~0x7F) == 0)
#define IS_ARINC429R_DIV(DIV)      ((DIV) <= 0xFF)
#define IS_ARINC429R_LABEL(LABEL)  (((LABEL) & 0xFF) == 0)
/** @} */ /* End of group ARINC429R_Exported_Constants */


/** @defgroup ARINC429R_Exported_Functions ARINC429R Exported Functions
  * @{
  */
void ARINC429R_BRG_Init(uint32_t ARINC429R_BRG);
void ARINC429R_DeInit(void);
void ARINC429R_ChannelInit(ARINC429R_Channel ARINC429R_CHANNELx, ARINC429R_InitChannelTypeDef * ARINC429R_InitChannelStruct);
void ARINC429R_ChannelCmd(ARINC429R_Channel ARINC429R_CHANNELx, FunctionalState NewState);
void ARINC429R_ITConfig(uint32_t ARINC429R_IT, FunctionalState NewState);
void ARINC429R_ITMaskConfig(ARINC429R_Channel ARINC429R_CHANNELx, uint32_t ARINC429R_IT_Mask, FunctionalState NewState);
uint32_t ARINC429R_ReceiveDataDirect(ARINC429R_Channel ARINC429R_CHANNELx);
FlagStatus ARINC429R_GetFlagStatus(ARINC429R_Channel ARINC429R_CHANNELx, ARINC429R_Flags ARINC429R_FLAG);
void ARINC429R_SetChannel(ARINC429R_Channel ARINC429R_CHANNELx);
uint32_t ARINC429R_GetLabel(void);
void ARINC429R_SetLabel(uint8_t label);
uint32_t ARINC429R_ReceiveData(void);

/** @} */ /* End of group ARINC429R_Exported_Functions */

/** @} */ /* End of group ARINC429R */

#endif /* #if defined (USE_K1986VE1xI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* MDR32FxQI_ARINC429R_H_ */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_arinc429r.h */

