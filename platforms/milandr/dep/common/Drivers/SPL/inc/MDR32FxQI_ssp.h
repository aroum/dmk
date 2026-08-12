/**
  ******************************************************************************
  * @file    MDR32FxQI_ssp.h
  * @author  Milandr Application Team
  * @version V2.0.3i
  * @date    27/09/2024
  * @brief   This file contains all the functions prototypes for the SSP
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
#ifndef __MDR32FxQI_SSP_H
#define __MDR32FxQI_SSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup SSP
  * @{
  */

/** @defgroup SSP_Exported_Types SSP Exported Types
  * @{
  */

/**
  * @brief SSP Mode
  */
typedef enum
{
    SSP_ModeMaster = ((uint32_t)0x00),
    SSP_ModeSlave  = ((uint32_t)0x04)
} SSP_Mode_TypeDef;

#define IS_SSP_MODE(MODE)   (((MODE) == SSP_ModeMaster) || \
                             ((MODE) == SSP_ModeSlave))

/**
  * @brief SSP Word Length
  */
typedef enum
{
    SSP_WordLength4b  = ((uint16_t)0x03),
    SSP_WordLength5b  = ((uint16_t)0x04),
    SSP_WordLength6b  = ((uint16_t)0x05),
    SSP_WordLength7b  = ((uint16_t)0x06),
    SSP_WordLength8b  = ((uint16_t)0x07),
    SSP_WordLength9b  = ((uint16_t)0x08),
    SSP_WordLength10b = ((uint16_t)0x09),
    SSP_WordLength11b = ((uint16_t)0x0A),
    SSP_WordLength12b = ((uint16_t)0x0B),
    SSP_WordLength13b = ((uint16_t)0x0C),
    SSP_WordLength14b = ((uint16_t)0x0D),
    SSP_WordLength15b = ((uint16_t)0x0E),
    SSP_WordLength16b = ((uint16_t)0x0F)
} SSP_Word_Length_TypeDef;

#define IS_SSP_WORD_LENGTH(LENGTH)  (((LENGTH) == SSP_WordLength4b)  || \
                                     ((LENGTH) == SSP_WordLength5b)  || \
                                     ((LENGTH) == SSP_WordLength6b)  || \
                                     ((LENGTH) == SSP_WordLength7b)  || \
                                     ((LENGTH) == SSP_WordLength8b)  || \
                                     ((LENGTH) == SSP_WordLength9b)  || \
                                     ((LENGTH) == SSP_WordLength10b) || \
                                     ((LENGTH) == SSP_WordLength11b) || \
                                     ((LENGTH) == SSP_WordLength12b) || \
                                     ((LENGTH) == SSP_WordLength13b) || \
                                     ((LENGTH) == SSP_WordLength14b) || \
                                     ((LENGTH) == SSP_WordLength15b) || \
                                     ((LENGTH) == SSP_WordLength16b))

/**
  * @brief SSP Clock Phase
  */
typedef enum
{
    SSP_SPH_1Edge = ((uint16_t)0x00),
    SSP_SPH_2Edge = ((uint16_t)0x80)
} SSP_Clock_Phase_TypeDef;

#define IS_SSP_SPH(PHASE)   (((PHASE) == SSP_SPH_1Edge) || \
                             ((PHASE) == SSP_SPH_2Edge))

/**
  * @brief SSP Clock Polarity
  */
typedef enum
{
    SSP_SPO_Low  = ((uint16_t)0x00),
    SSP_SPO_High = ((uint16_t)0x40)
} SSP_Clock_Polarity_TypeDef;

#define IS_SSP_SPO(LEVEL)   (((LEVEL) == SSP_SPO_Low) || \
                             ((LEVEL) == SSP_SPO_High))

/**
  * @brief SSP Frame Format
  */
typedef enum
{
    SSP_FRF_SPI_Motorola = ((uint16_t)0x00),
    SSP_FRF_SSI_TI       = ((uint16_t)0x10),
    SSP_FRF_Microwire    = ((uint16_t)0x20)
} SSP_Frame_Format_TypeDef;

#define IS_SSP_FRF(FORMAT)  (((FORMAT) == SSP_FRF_SPI_Motorola) || \
                             ((FORMAT) == SSP_FRF_SSI_TI)       || \
                             ((FORMAT) == SSP_FRF_Microwire))

/**
  * @brief SSP Hardware Flow Control
  */
typedef enum
{
    SSP_HardwareFlowControl_None = ((uint16_t)0x00),
    SSP_HardwareFlowControl_SOD  = ((uint16_t)0x08), /*!< Disable output lines in slave mode */
    SSP_HardwareFlowControl_SSE  = ((uint16_t)0x02), /*!< Transceiver work permit */
    SSP_HardwareFlowControl_LBM  = ((uint16_t)0x01)  /*!< Loopback testing mode */
} SSP_Hardware_Flow_Control_TypeDef;

#define SSP_HardwareFlowControl_MASK  ((uint16_t)(SSP_HardwareFlowControl_None | \
                                                  SSP_HardwareFlowControl_SSE  | \
                                                  SSP_HardwareFlowControl_LBM  | \
                                                  SSP_HardwareFlowControl_SOD))

#define IS_SSP_HARDWARE_FLOW_CONTROL(CONTROL) \
                              ((CONTROL & (~SSP_HardwareFlowControl_MASK)) == 0)

/**
  * @brief SSP SSP Flags
  */
typedef enum
{
    SSP_FLAG_BSY = ((uint16_t)0x10),
    SSP_FLAG_RFF = ((uint16_t)0x08),
    SSP_FLAG_RNE = ((uint16_t)0x04),
    SSP_FLAG_TNF = ((uint16_t)0x02),
    SSP_FLAG_TFE = ((uint16_t)0x01)
} SSP_Flags_TypeDef;

#define IS_SSP_FLAG(FLAG)   (((FLAG) == SSP_FLAG_BSY) || \
                             ((FLAG) == SSP_FLAG_RFF) || \
                             ((FLAG) == SSP_FLAG_RNE) || \
                             ((FLAG) == SSP_FLAG_TNF) || \
                             ((FLAG) == SSP_FLAG_TFE))

/**
  * @brief SSP SSP Interrupt definition
  */
typedef enum
{
    SSP_IT_TX  = ((uint32_t)0x08),
    SSP_IT_RX  = ((uint32_t)0x04),
    SSP_IT_RT  = ((uint32_t)0x02),
    SSP_IT_ROR = ((uint32_t)0x01)
} SSP_IT_TypeDef;

#define SSP_IT_MASK             (SSP_IT_TX | SSP_IT_RX | \
                                 SSP_IT_RT | SSP_IT_ROR)

#define IS_SSP_CONFIG_IT(IT)    (((IT) == SSP_IT_TX) || \
                                 ((IT) == SSP_IT_RX) || \
                                 ((IT) == SSP_IT_RT) || \
                                 ((IT) == SSP_IT_ROR))

#define IS_SSP_RESET_IT(IT)     (((IT) == SSP_IT_RT) || \
                                 ((IT) == SSP_IT_ROR))

/**
  * @brief SSP DMA request
  */
typedef enum
{
    SSP_DMA_RXE = ((uint32_t)0x01),
    SSP_DMA_TXE = ((uint32_t)0x02)
} SSP_DMA_Req_TypeDef;

#define IS_SSP_DMAREQ(DMAREQ)   (((DMAREQ) == SSP_DMA_TXE) || \
                                 ((DMAREQ) == SSP_DMA_RXE) || \
                                 ((DMAREQ) == (SSP_DMA_TXE | SSP_DMA_RXE)))

/**
  * @brief SSP Clock BRG
  */
typedef enum
{
    SSP_HCLKdiv1   = ((uint32_t)0x00),
    SSP_HCLKdiv2   = ((uint32_t)0x01),
    SSP_HCLKdiv4   = ((uint32_t)0x02),
    SSP_HCLKdiv8   = ((uint32_t)0x03),
    SSP_HCLKdiv16  = ((uint32_t)0x04),
    SSP_HCLKdiv32  = ((uint32_t)0x05),
    SSP_HCLKdiv64  = ((uint32_t)0x06),
    SSP_HCLKdiv128 = ((uint32_t)0x07)
} SSP_Clock_BRG_TypeDef;

#define IS_SSP_CLOCK_BRG(BRG)   (((BRG) & ~0x7) == 0)

/**
  * @brief  SSP Init Structure definition
  */
typedef struct
{
    uint16_t                          SSP_SCR;                 /*!< This member configures the SSP communication speed.
                                                                    This parameter is a number from 0 to 255.
                                                                    The information rate is computed using the following formula:
                                                                    F_SSPCLK / (SSP_CPSDVSR * (1 + SSP_SCR)). */
    uint16_t                          SSP_CPSDVSR;             /*!< This member configures the SSP clock divider.
                                                                    This parameter is an even number from 2 to 254. */
    SSP_Mode_TypeDef                  SSP_Mode;                /*!< Specifies the SSP operation mode (master or slave).
                                                                    This parameter is one of @ref SSP_Mode_TypeDef values. */
    SSP_Word_Length_TypeDef           SSP_WordLength;          /*!< Specifies the number of data bits transmitted or received in a frame.
                                                                    This parameter is one of @ref SSP_Word_Length_TypeDef values. */
    SSP_Clock_Phase_TypeDef           SSP_SPH;                 /*!< Specifies the phase of SSP_CLK (data sampling on edge the 1st or 2nd).
                                                                    This parameter is one of @ref SSP_Clock_Phase_TypeDef values. */
    SSP_Clock_Polarity_TypeDef        SSP_SPO;                 /*!< Specifies the polarity of SSP_CLK (the initial state is low or high).
                                                                    This parameter is one of @ref SSP_Clock_Polarity_TypeDef values. */
    SSP_Frame_Format_TypeDef          SSP_FRF;                 /*!< Specifies the frame format (SPI, SSI, Microwire).
                                                                    This parameter is one of @ref SSP_Frame_Format_TypeDef values. */
    SSP_Hardware_Flow_Control_TypeDef SSP_HardwareFlowControl; /*!< Specifies whether the hardware flow control mode is enabled or disabled.
                                                                    This parameter is one of @ref SSP_Hardware_Flow_Control_TypeDef values. */
} SSP_InitTypeDef;

/** @} */ /* End of group SSP_Exported_Types */


/** @defgroup SSP_Exported_Constants SSP Exported Constants
  * @{
  */

#define IS_SSP_SPEED_FACTOR(DIVIDER)    ((DIVIDER) <= 0xFF)

#define IS_SSP_SPEED_DIVIDER(DIVIDER)   (((DIVIDER) <= 0xFE) && \
                                         ((DIVIDER) >= 0x02) && \
                                        (((DIVIDER) & 0x01) == 0))

/** @} */ /* End of group SSP_Exported_Constants */


/** @defgroup SSP_Exported_Functions SSP Exported Functions
  * @{
  */

void SSP_DeInit(MDR_SSP_TypeDef* SSPx);
void SSP_Init(MDR_SSP_TypeDef* SSPx, const SSP_InitTypeDef* SSP_InitStruct);
void SSP_StructInit(SSP_InitTypeDef* SSP_InitStruct);
void SSP_Cmd(MDR_SSP_TypeDef* SSPx, FunctionalState NewState);

void SSP_ITConfig(MDR_SSP_TypeDef* SSPx, uint32_t SSP_IT, FunctionalState NewState);
ITStatus SSP_GetITStatus(MDR_SSP_TypeDef* SSPx, SSP_IT_TypeDef SSP_IT);
ITStatus SSP_GetITStatusMasked(MDR_SSP_TypeDef* SSPx, SSP_IT_TypeDef SSP_IT);
void SSP_ClearITPendingBit(MDR_SSP_TypeDef* SSPx, SSP_IT_TypeDef SSP_IT);

void SSP_DMACmd(MDR_SSP_TypeDef* SSPx, uint32_t SSP_DMAReq, FunctionalState NewState);

void SSP_SendData(MDR_SSP_TypeDef* SSPx, uint16_t Data);
uint16_t SSP_ReceiveData(MDR_SSP_TypeDef* SSPx);

FlagStatus SSP_GetFlagStatus(MDR_SSP_TypeDef* SSPx, SSP_Flags_TypeDef SSP_FLAG);
void SSP_BRGInit(MDR_SSP_TypeDef* SSPx, SSP_Clock_BRG_TypeDef SSP_BRG);

/** @} */ /* End of group SSP_Exported_Functions */

/** @} */ /* End of group SSP */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_SSP_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_ssp.h */


