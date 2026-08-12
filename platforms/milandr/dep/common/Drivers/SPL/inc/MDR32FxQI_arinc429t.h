/**
  ******************************************************************************
  * @file    MDR32FxQI_arinc429t.h
  * @author  Milandr Application Team
  * @version V2.0.2i
  * @date    24/07/2024
  * @brief   This file contains all the functions prototypes for the ARINC429T
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
#ifndef MDR32FxQI_ARINC429T_H_
#define MDR32FxQI_ARINC429T_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup ARINC429T ARINC429T
  * @warning This module can be used only for microcontrollers MDR32F1QI, K1986VE1xI.
  * @{
  */

#if defined (USE_K1986VE1xI)

/** @defgroup ARINC429T_Exported_Types ARINC429T Exported Types
  * @{
  */

/**
  * @brief ARINC429T CLK
  */
typedef enum
{
    ARINC429T_CLK_12_5_KHz = ((uint32_t)0x1), /*!< Frequency of data transfer 12.5 KHz if DIV != 0 */
    ARINC429T_CLK_100_KHz  = ((uint32_t)0x0)  /*!< Frequency of data transfer 100 KHz if DIV != 0 */
} ARINC429T_Clk;

#define IS_ARINC429T_CLK(CLK)  (((CLK) == ARINC429T_CLK_12_5_KHz) || \
                                ((CLK) == ARINC429T_CLK_100_KHz))

/**
  * @brief ARINC429T ODD
  */
typedef enum
{
    ARINC429T_ODD_ADD_TO_ODD    = ((uint32_t)0x1), /*!< Parity bit is formed as odd's complement. */
    ARINC429T_ODD_ADD_TO_PARITY = ((uint32_t)0x0)  /*!< Parity bit is formed as parity's complement. */
} ARINC429T_Odd;

#define IS_ARINC429T_ODD(ODD)  (((ODD) == ARINC429T_ODD_ADD_TO_ODD) || \
                                ((ODD) == ARINC429T_ODD_ADD_TO_PARITY))

/**
  * @brief ARINC429T Channel
  */
typedef enum
{
    ARINC429T_CHANNEL1 = ((uint32_t)0x0),
    ARINC429T_CHANNEL2 = ((uint32_t)0x1),
    ARINC429T_CHANNEL3 = ((uint32_t)0x2),
    ARINC429T_CHANNEL4 = ((uint32_t)0x3)
} ARINC429T_Channel;

#define IS_ARINC429T_CHANNEL(CHANNELx)  (((CHANNELx) & ~0x3) == 0)

/**
  * @brief ARINC429T Interrupt Defintion
  */
typedef enum
{
    ARINC429T_IT_INTE_HTF = ((uint32_t)0x4), /*!< Half-full FIFO interrupt enable */
    ARINC429T_IT_INTE_TXR = ((uint32_t)0x2), /*!< FIFO underrun interrupt enable */
    ARINC429T_IT_INTE_FFT = ((uint32_t)0x1)  /*!< Full FIFO interrupt enable */
} ARINC429T_IT_Def;

#define IS_ARINC429T_IT(IT) (((IT) == ARINC429T_IT_INTE_HTF) || \
                             ((IT) == ARINC429T_IT_INTE_TXR) || \
                             ((IT) == ARINC429T_IT_INTE_FFT))

/**
  * @brief ARINC429T Flags
  */
typedef enum
{
    ARINC429T_FLAG_TX_R = ((uint32_t)0x1), /*!< Data presence in FIFO flag */
    ARINC429T_FLAG_FFT  = ((uint32_t)0x2), /*!< Full FIFO flag */
    ARINC429T_FLAG_HFT  = ((uint32_t)0x4)  /*!< Half-Full FIFO flag */
} ARINC429T_Flags;

#define IS_ARINC429T_FLAG(FLAG) (((FLAG) == ARINC429T_FLAG_TX_R) || \
                                 ((FLAG) == ARINC429T_FLAG_FFT)  || \
                                 ((FLAG) == ARINC429T_FLAG_HFT))

/**
  * @brief ARINC429T Init Channel Structure definition
  */
typedef struct
{
    ARINC429T_Odd   ARINC429T_ODD;    /*!< This member specifies bits even or odd parity for the channel.
                                           This parameter can be a value of @ref ARINC429T_Odd */
    FunctionalState ARINC429T_EN_PAR; /*!< This member enables or disables the 32-bit parity for a given channel.
                                           This parameter can be a value of @ref FunctionalState */
    ARINC429T_Clk   ARINC429T_CLK;    /*!< This member specifies the data sending speed.
                                           This parameter can be a value of @ref ARINC429T_Clk */
    uint32_t        ARINC429T_DIV;    /*!< This member specifies the individual core
                                           frequency divider for channel reference frequency.
                                           This parameter can be a number from 0 to 0xFF. */
} ARINC429T_InitChannelTypeDef;

/** @} */ /* End of group ARINC429T_Exported_Types */


/** @defgroup ARINC429T_Exported_Constants ARINC429T Exported Constants
  * @{
  */

/** @defgroup ARINC429T_Clock_BRG ARINC429T Clock BRG
  * @{
  */
#define ARINC429T_HCLKdiv_MASK            ((uint32_t)0x00007F00)
#define IS_ARINC429T_HCLKdiv(HCLKdiv)    (((HCLKdiv) & ~(ARINC429T_HCLKdiv_MASK)) == 0)
/** @} */ /* End of group ARINC429T_Clock_BRG */

#define IS_ARINC429T_DIV_CH(DIV_CH)  ((DIV_CH) <= 0xFF)

/** @} */ /* End of group ARINC429T_Exported_Constants */


/** @defgroup ARINC429T_Exported_Funstions ARINC429T Exported Funstions
  * @{
  */

void ARINC429T_BRG_Init(uint32_t ARINC429T_BRG);
void ARINC429T_DeInit(void);
void ARINC429T_ChannelInit(ARINC429T_Channel ARINC429T_CHANNELx, ARINC429T_InitChannelTypeDef * ARINC429T_InitChannelStruct);
void ARINC429T_ChannelCmd(ARINC429T_Channel ARINC429T_CHANNELx, FunctionalState NewState);
void ARINC429T_ITConfig(ARINC429T_Channel ARINC429T_CHANNELx, ARINC429T_IT_Def ARINC429T_IT, FunctionalState NewState);
FlagStatus ARINC429T_GetFlagStatus(ARINC429T_Channel ARINC429T_CHANNELx, ARINC429T_Flags ARINC429T_FLAG);
void ARINC429T_SendData(ARINC429T_Channel ARINC429T_CHANNELx, uint32_t Data);

/** @} */ /* End of group ARINC429T_Exported_Funstions */

#endif /* #if defined (USE_K1986VE1xI) */

/** @} */ /* End of group ARINC429T */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* MDR32FxQI_ARINC429T_H_ */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_arinc429t.h */

