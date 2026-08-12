/**
  ******************************************************************************
  * @file    MDR32FxQI_dma.h
  * @author  Milandr Application Team
  * @version V2.2.1i
  * @date    23/03/2024
  * @brief   This file contains all the functions prototypes for the DMA
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
#ifndef __MDR32FxQI_DMA_H
#define __MDR32FxQI_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup DMA DMA
  * @{
  */

/** @addtogroup DMA_Exported_Types DMA Exported Types
  * @{
  */

/**
  * @brief DMA alternate data usage
  */
typedef enum
{
    DMA_ALternateDataDisabled = ((uint16_t)(0x00)),
    DMA_ALternateDataEnabled  = ((uint16_t)(0x01))
} DMA_Alt_Data_Usage;

#define IS_DMA_ALTERNATE_DATA(STATE)  (((STATE) == DMA_ALternateDataDisabled) || \
                                       ((STATE)  == DMA_ALternateDataEnabled))

/**
  * @brief DMA valid channels
  */
typedef enum
{
#if defined (USE_K1986VE9xI)
    DMA_Channel_UART1_TX = ((uint8_t)(0)),
    DMA_Channel_UART1_RX = ((uint8_t)(1)),
    DMA_Channel_UART2_TX = ((uint8_t)(2)),
    DMA_Channel_UART2_RX = ((uint8_t)(3)),
    DMA_Channel_SSP1_TX  = ((uint8_t)(4)),
    DMA_Channel_SSP1_RX  = ((uint8_t)(5)),
    DMA_Channel_SSP2_TX  = ((uint8_t)(6)),
    DMA_Channel_SSP2_RX  = ((uint8_t)(7)),
    DMA_Channel_ADC1     = ((uint8_t)(8)),
    DMA_Channel_ADC2     = ((uint8_t)(9)),
    DMA_Channel_TIM1     = ((uint8_t)(10)),
    DMA_Channel_TIM2     = ((uint8_t)(11)),
    DMA_Channel_TIM3     = ((uint8_t)(12)),
    DMA_Channel_SW1      = ((uint8_t)(13)),
    DMA_Channel_SW2      = ((uint8_t)(14)),
    DMA_Channel_SW3      = ((uint8_t)(15)),
    DMA_Channel_SW4      = ((uint8_t)(16)),
    DMA_Channel_SW5      = ((uint8_t)(17)),
    DMA_Channel_SW6      = ((uint8_t)(18)),
    DMA_Channel_SW7      = ((uint8_t)(19)),
    DMA_Channel_SW8      = ((uint8_t)(20)),
    DMA_Channel_SW9      = ((uint8_t)(21)),
    DMA_Channel_SW10     = ((uint8_t)(22)),
    DMA_Channel_SW11     = ((uint8_t)(23)),
    DMA_Channel_SW12     = ((uint8_t)(24)),
    DMA_Channel_SW13     = ((uint8_t)(25)),
    DMA_Channel_SW14     = ((uint8_t)(26)),
    DMA_Channel_SW15     = ((uint8_t)(27)),
    DMA_Channel_SW16     = ((uint8_t)(28)),
    DMA_Channel_SW17     = ((uint8_t)(29)),
    DMA_Channel_SW18     = ((uint8_t)(30)),
    DMA_Channel_SW19     = ((uint8_t)(31)),
#elif defined (USE_K1986VE1xI)
/*-- req DMA request ---------------------------------------------------------*/
    DMA_Channel_REQ_UART1_TX = ((uint8_t)(0)),
    DMA_Channel_REQ_UART1_RX = ((uint8_t)(1)),
    DMA_Channel_REQ_UART2_TX = ((uint8_t)(2)),
    DMA_Channel_REQ_UART2_RX = ((uint8_t)(3)),
    DMA_Channel_REQ_SSP1_TX  = ((uint8_t)(4)),
    DMA_Channel_REQ_SSP1_RX  = ((uint8_t)(5)),
    DMA_Channel_REQ_SSP2_TX  = ((uint8_t)(6)),
    DMA_Channel_REQ_SSP2_RX  = ((uint8_t)(7)),
    DMA_Channel_REQ_SSP3_TX  = ((uint8_t)(8)),
    DMA_Channel_REQ_SSP3_RX  = ((uint8_t)(9)),
    DMA_Channel_REQ_TIM1     = ((uint8_t)(10)),
    DMA_Channel_REQ_TIM2     = ((uint8_t)(11)),
    DMA_Channel_REQ_TIM3     = ((uint8_t)(12)),
    DMA_Channel_REQ_TIM4     = ((uint8_t)(13)),
    DMA_Channel_SW1          = ((uint8_t)(14)),
    DMA_Channel_SW2          = ((uint8_t)(15)),
    DMA_Channel_SW3          = ((uint8_t)(16)),
    DMA_Channel_SW4          = ((uint8_t)(17)),
    DMA_Channel_SW5          = ((uint8_t)(18)),
    DMA_Channel_SW6          = ((uint8_t)(19)),
    DMA_Channel_SW7          = ((uint8_t)(20)),
    DMA_Channel_SW8          = ((uint8_t)(21)),
    DMA_Channel_SW9          = ((uint8_t)(22)),
    DMA_Channel_SW10         = ((uint8_t)(23)),
    DMA_Channel_SW11         = ((uint8_t)(24)),
    DMA_Channel_SW12         = ((uint8_t)(25)),
    DMA_Channel_SW13         = ((uint8_t)(26)),
    DMA_Channel_SW14         = ((uint8_t)(27)),
    DMA_Channel_SW15         = ((uint8_t)(28)),
    DMA_Channel_SW16         = ((uint8_t)(29)),
    DMA_Channel_SW17         = ((uint8_t)(30)),
    DMA_Channel_SW18         = ((uint8_t)(31)),
/*-- sreq DMA request --------------------------------------------------------*/
    DMA_Channel_SREQ_UART1_TX  = ((uint8_t)(0)),
    DMA_Channel_SREQ_UART1_RX  = ((uint8_t)(1)),
    DMA_Channel_SREQ_UART2_TX  = ((uint8_t)(2)),
    DMA_Channel_SREQ_UART2_RX  = ((uint8_t)(3)),
    DMA_Channel_SREQ_SSP1_TX   = ((uint8_t)(4)),
    DMA_Channel_SREQ_SSP1_RX   = ((uint8_t)(5)),
    DMA_Channel_SREQ_SSP2_TX   = ((uint8_t)(6)),
    DMA_Channel_SREQ_SSP2_RX   = ((uint8_t)(7)),
    DMA_Channel_SREQ_SSP3_TX   = ((uint8_t)(8)),
    DMA_Channel_SREQ_SSP3_RX   = ((uint8_t)(9)),
    DMA_Channel_SREQ_TIM1      = ((uint8_t)(10)),
    DMA_Channel_SREQ_TIM2      = ((uint8_t)(11)),
    DMA_Channel_SREQ_TIM3      = ((uint8_t)(12)),
    DMA_Channel_SREQ_TIM4      = ((uint8_t)(13)),
    DMA_Channel_SREQ_TIM1_REQ1 = ((uint8_t)(14)),
    DMA_Channel_SREQ_TIM1_REQ2 = ((uint8_t)(15)),
    DMA_Channel_SREQ_TIM1_REQ3 = ((uint8_t)(16)),
    DMA_Channel_SREQ_TIM1_REQ4 = ((uint8_t)(17)),
    DMA_Channel_SREQ_TIM2_REQ1 = ((uint8_t)(18)),
    DMA_Channel_SREQ_TIM2_REQ2 = ((uint8_t)(19)),
    DMA_Channel_SREQ_TIM2_REQ3 = ((uint8_t)(20)),
    DMA_Channel_SREQ_TIM2_REQ4 = ((uint8_t)(21)),
    DMA_Channel_SREQ_TIM3_REQ1 = ((uint8_t)(22)),
    DMA_Channel_SREQ_TIM3_REQ2 = ((uint8_t)(23)),
    DMA_Channel_SREQ_TIM3_REQ3 = ((uint8_t)(24)),
    DMA_Channel_SREQ_TIM3_REQ4 = ((uint8_t)(25)),
    DMA_Channel_SREQ_TIM4_REQ1 = ((uint8_t)(26)),
    DMA_Channel_SREQ_TIM4_REQ2 = ((uint8_t)(27)),
    DMA_Channel_SREQ_TIM4_REQ3 = ((uint8_t)(28)),
    DMA_Channel_SREQ_TIM4_REQ4 = ((uint8_t)(29)),
    DMA_Channel_SREQ_ADCDMA    = ((uint8_t)(30)),
    DMA_Channel_SREQ_SW1       = ((uint8_t)(31)),
#elif defined (USE_MDR32FG16S1QI)
/*-- req DMA request ---------------------------------------------------------*/
    DMA_Channel_REQ_UART1_TX  = ((uint8_t)(0)),
    DMA_Channel_REQ_UART1_RX  = ((uint8_t)(1)),
    DMA_Channel_REQ_UART2_TX  = ((uint8_t)(2)),
    DMA_Channel_REQ_UART2_RX  = ((uint8_t)(3)),
    DMA_Channel_REQ_SSP1_TX   = ((uint8_t)(4)),
    DMA_Channel_REQ_SSP1_RX   = ((uint8_t)(5)),
    DMA_Channel_REQ_SSP2_TX   = ((uint8_t)(6)),
    DMA_Channel_REQ_SSP2_RX   = ((uint8_t)(7)),
    DMA_Channel_REQ_DSP0      = ((uint8_t)(9)),
    DMA_Channel_REQ_DSP1      = ((uint8_t)(10)),
    DMA_Channel_REQ_DSP2      = ((uint8_t)(14)),
    DMA_Channel_REQ_DSP3      = ((uint8_t)(15)),
    DMA_Channel_REQ_SSP3_TX   = ((uint8_t)(16)),
    DMA_Channel_REQ_SSP3_RX   = ((uint8_t)(17)),
    DMA_Channel_REQ_SSP4_TX   = ((uint8_t)(18)),
    DMA_Channel_REQ_SSP4_RX   = ((uint8_t)(19)),
    DMA_Channel_REQ_UART3_TX  = ((uint8_t)(20)),
    DMA_Channel_REQ_UART3_RX  = ((uint8_t)(21)),
    DMA_Channel_REQ_AUC_ADC   = ((uint8_t)(22)),
    DMA_Channel_REQ_AUC_DAC   = ((uint8_t)(23)),
    DMA_Channel_REQ_CRYPTO    = ((uint8_t)(24)),
    DMA_Channel_REQ_DSP_TIM   = ((uint8_t)(25)),
    DMA_Channel_REQ_MCBSP1_TX = ((uint8_t)(26)),
    DMA_Channel_REQ_MCBSP1_RX = ((uint8_t)(27)),
    DMA_Channel_REQ_MCBSP2_TX = ((uint8_t)(28)),
    DMA_Channel_REQ_MCBSP2_RX = ((uint8_t)(29)),
    DMA_Channel_REQ_MCBSP3_TX = ((uint8_t)(30)),
    DMA_Channel_REQ_MCBSP3_RX = ((uint8_t)(31)),
/*-- sreq DMA request --------------------------------------------------------*/
    DMA_Channel_SREQ_UART1_TX  = ((uint8_t)(0)),
    DMA_Channel_SREQ_UART1_RX  = ((uint8_t)(1)),
    DMA_Channel_SREQ_UART2_TX  = ((uint8_t)(2)),
    DMA_Channel_SREQ_UART2_RX  = ((uint8_t)(3)),
    DMA_Channel_SREQ_SSP1_TX   = ((uint8_t)(4)),
    DMA_Channel_SREQ_SSP1_RX   = ((uint8_t)(5)),
    DMA_Channel_SREQ_SSP2_TX   = ((uint8_t)(6)),
    DMA_Channel_SREQ_SSP2_RX   = ((uint8_t)(7)),
    DMA_Channel_SREQ_ADC1      = ((uint8_t)(8)),
    DMA_Channel_SREQ_ADC2      = ((uint8_t)(9)),
    DMA_Channel_SREQ_TIM1      = ((uint8_t)(10)),
    DMA_Channel_SREQ_TIM2      = ((uint8_t)(11)),
    DMA_Channel_SREQ_TIM3      = ((uint8_t)(12)),
    DMA_Channel_SREQ_SDIO      = ((uint8_t)(13)),
    DMA_Channel_SREQ_DSP0      = ((uint8_t)(14)),
    DMA_Channel_SREQ_DSP1      = ((uint8_t)(15)),
    DMA_Channel_SREQ_SSP3_TX   = ((uint8_t)(16)),
    DMA_Channel_SREQ_SSP3_RX   = ((uint8_t)(17)),
    DMA_Channel_SREQ_SSP4_TX   = ((uint8_t)(18)),
    DMA_Channel_SREQ_SSP4_RX   = ((uint8_t)(19)),
    DMA_Channel_SREQ_UART3_TX  = ((uint8_t)(20)),
    DMA_Channel_SREQ_UART3_RX  = ((uint8_t)(21)),
    DMA_Channel_SREQ_DSP_TIM   = ((uint8_t)(25)),
    DMA_Channel_SREQ_MCBSP1_TX = ((uint8_t)(26)),
    DMA_Channel_SREQ_MCBSP1_RX = ((uint8_t)(27)),
    DMA_Channel_SREQ_MCBSP2_TX = ((uint8_t)(28)),
    DMA_Channel_SREQ_MCBSP2_RX = ((uint8_t)(29)),
    DMA_Channel_SREQ_MCBSP3_TX = ((uint8_t)(30)),
    DMA_Channel_SREQ_MCBSP3_RX = ((uint8_t)(31)),
#endif
} DMA_Valid_Channels;

#define IS_DMA_CHANNEL(CHANNEL) ((CHANNEL) <= (DMA_Channels_Number - 1))

/**
  * @brief DMA source incremented mode
  */
typedef enum
{
    DMA_SourceIncByte     = ((uint32_t)0x00),
    DMA_SourceIncHalfword = ((uint32_t)0x01),
    DMA_SourceIncWord     = ((uint32_t)0x02),
    DMA_SourceIncNo       = ((uint32_t)0x03)
} DMA_Src_Inc_Mode;

#define IS_DMA_SOURCE_INC_STATE(STATE)  (((STATE) == DMA_SourceIncByte)     || \
                                         ((STATE) == DMA_SourceIncHalfword) || \
                                         ((STATE) == DMA_SourceIncWord)     || \
                                         ((STATE) == DMA_SourceIncNo))

/**
  * @brief DMA destination incremented mode
  */
typedef enum
{
    DMA_DestIncByte     = ((uint32_t)0x00),
    DMA_DestIncHalfword = ((uint32_t)0x01),
    DMA_DestIncWord     = ((uint32_t)0x02),
    DMA_DestIncNo       = ((uint32_t)0x03)
} DMA_Dest_Inc_Mode;

#define IS_DMA_DEST_INC_STATE(STATE)  (((STATE)  == DMA_DestIncByte)     || \
                                       ((STATE)  == DMA_DestIncHalfword) || \
                                       ((STATE)  == DMA_DestIncWord)     || \
                                       ((STATE)  == DMA_DestIncNo))

/**
  * @brief DMA memory data size
  */
typedef enum
{
    DMA_MemoryDataSize_Byte     = ((uint32_t)(0x00 << 24)),
    DMA_MemoryDataSize_HalfWord = ((uint32_t)(0x11 << 24)),
    DMA_MemoryDataSize_Word     = ((uint32_t)(0x22 << 24))
} DMA_Mem_Data_Size;

#define IS_DMA_MEMORY_DATA_SIZE(SIZE)  (((SIZE) == DMA_MemoryDataSize_Byte)     || \
                                        ((SIZE) == DMA_MemoryDataSize_HalfWord) || \
                                        ((SIZE) == DMA_MemoryDataSize_Word))

/**
  * @brief DMA operating mode
  */
typedef enum
{
    DMA_Mode_Stop               = ((uint32_t)0x0),
    DMA_Mode_Basic              = ((uint32_t)0x1),
    DMA_Mode_AutoRequest        = ((uint32_t)0x2),
    DMA_Mode_PingPong           = ((uint32_t)0x3),
    DMA_Mode_MemScatterPri      = ((uint32_t)0x4),
    DMA_Mode_MemScatterAlt      = ((uint32_t)0x5),
    DMA_Mode_PerScatterPri      = ((uint32_t)0x6),
    DMA_Mode_PerScatterAlt      = ((uint32_t)0x7),
    DMA_Mode_PerScatterAltBurst = ((uint32_t)0xF)
} DMA_Operating_Mode;

#define IS_DMA_MODE(MODE)  (((MODE) == DMA_Mode_Stop)          || \
                            ((MODE) == DMA_Mode_Basic)         || \
                            ((MODE) == DMA_Mode_AutoRequest)   || \
                            ((MODE) == DMA_Mode_PingPong)      || \
                            ((MODE) == DMA_Mode_MemScatterPri) || \
                            ((MODE) == DMA_Mode_MemScatterAlt) || \
                            ((MODE) == DMA_Mode_PerScatterPri) || \
                            ((MODE) == DMA_Mode_PerScatterAlt) || \
                            ((MODE) == DMA_Mode_PerScatterAltBurst))

/**
  * @brief DMA priority level
  */
typedef enum
{
    DMA_Priority_Default = ((uint8_t)0x00),
    DMA_Priority_High    = ((uint8_t)0x01)
} DMA_Priority_Level;

#define IS_DMA_PRIORITY(PRIORITY)  (((PRIORITY) == DMA_Priority_Default) || \
                                    ((PRIORITY)  == DMA_Priority_High))

/**
  * @brief DMA burst mode
  */
typedef enum
{
    DMA_BurstClear = ((uint8_t)0x00),
    DMA_BurstSet   = ((uint8_t)0x01)
} DMA_Burst_Mode;

#define IS_DMA_BURST(STATE)   (((STATE) == DMA_BurstClear) || \
                               ((STATE) == DMA_BurstSet))

/**
  * @brief DMA source protection control
  */
typedef enum
{
    DMA_SourceCacheable  = ((uint32_t)(0x01 << 20)),
    DMA_SourceBufferable = ((uint32_t)(0x01 << 19)),
    DMA_SourcePrivileged = ((uint32_t)(0x01 << 18))
} DMA_Src_Protection_Control;

#define IS_DMA_SOURCE_PROT(STATE)  (((STATE) & (~(0x7 << 18))) == 0)

/**
  * @brief DMA destination protection control
  */
typedef enum
{
    DMA_DestCacheable  = ((uint32_t)(0x01 << 23)),
    DMA_DestBufferable = ((uint32_t)(0x01 << 22)),
    DMA_DestPrivileged = ((uint32_t)(0x01 << 21))
} DMA_Dest_Protection_Control;

#define IS_DMA_DEST_PROT(STATE)  (((STATE) & (~(0x7 << 21))) == 0)

/**
  * @brief DMA AHB-Lite protection control
  */
typedef enum
{
    DMA_AHB_Cacheable  = ((uint32_t)(0x01 << 7)),
    DMA_AHB_Bufferable = ((uint32_t)(0x01 << 6)),
    DMA_AHB_Privileged = ((uint32_t)(0x01 << 5))
} DMA_AHB_Protection_Control;

#define IS_DMA_AHB_PROT(STATE)  (((STATE) & (~(0x7 << 5))) == 0)

/**
  * @brief DMA number continuous transfers
  */
typedef enum
{
    DMA_Transfers_1    = ((uint32_t)(0x00 << 14)),
    DMA_Transfers_2    = ((uint32_t)(0x01 << 14)),
    DMA_Transfers_4    = ((uint32_t)(0x02 << 14)),
    DMA_Transfers_8    = ((uint32_t)(0x03 << 14)),
    DMA_Transfers_16   = ((uint32_t)(0x04 << 14)),
    DMA_Transfers_32   = ((uint32_t)(0x05 << 14)),
    DMA_Transfers_64   = ((uint32_t)(0x06 << 14)),
    DMA_Transfers_128  = ((uint32_t)(0x07 << 14)),
    DMA_Transfers_256  = ((uint32_t)(0x08 << 14)),
    DMA_Transfers_512  = ((uint32_t)(0x09 << 14)),
    DMA_Transfers_1024 = ((uint32_t)(0x0A << 14))
} DMA_Number_Continuous_Transfers;

#define IS_DMA_CONTINUOUS_NUMBER(NUMBER)  (((NUMBER)  == DMA_Transfers_1)   || \
                                           ((NUMBER)  == DMA_Transfers_2)   || \
                                           ((NUMBER)  == DMA_Transfers_4)   || \
                                           ((NUMBER)  == DMA_Transfers_8)   || \
                                           ((NUMBER)  == DMA_Transfers_16)  || \
                                           ((NUMBER)  == DMA_Transfers_32)  || \
                                           ((NUMBER)  == DMA_Transfers_64)  || \
                                           ((NUMBER)  == DMA_Transfers_128) || \
                                           ((NUMBER)  == DMA_Transfers_256) || \
                                           ((NUMBER)  == DMA_Transfers_512) || \
                                           ((NUMBER)  == DMA_Transfers_1024))

/**
  * @brief DMA data structure selection
  */
typedef enum
{
    DMA_CTRL_DATA_PRIMARY   = ((uint8_t)0x00),
#if (DMA_AlternateData == 1)
    DMA_CTRL_DATA_ALTERNATE = ((uint8_t)0x01)
#endif
} DMA_Data_Struct_Selection;

#if (DMA_AlternateData == 1)
    #define IS_DMA_SELECT_DATA_STRUCTURE(TYPE) (((TYPE) == DMA_CTRL_DATA_PRIMARY) || \
                                                ((TYPE) == DMA_CTRL_DATA_ALTERNATE))
#else
    #define IS_DMA_SELECT_DATA_STRUCTURE(TYPE) (((TYPE) == DMA_CTRL_DATA_PRIMARY))
#endif

/**
  * @brief DMA flags definition
  */
typedef enum
{
    DMA_FLAG_DMA_ENA       = ((uint8_t)0x01), /*!< DMA unit global enable status */
    DMA_FLAG_DMA_ERR       = ((uint8_t)0x02), /*!< DMA unit bus error status */
    DMA_FLAG_CHNL_ENA      = ((uint8_t)0x03), /*!< DMA channel enable status */
    DMA_FLAG_CHNL_MASK     = ((uint8_t)0x04), /*!< DMA channel request mask status */
    DMA_FLAG_CHNL_WAIT     = ((uint8_t)0x05), /*!< DMA channel wait on request status */
    DMA_FLAG_CHNL_BURST    = ((uint8_t)0x06), /*!< DMA channel burst mode status */
    DMA_FLAG_CHNL_ALT      = ((uint8_t)0x07), /*!< DMA channel alternate control data status */
    DMA_FLAG_CHNL_PRIORITY = ((uint8_t)0x08)  /*!< DMA channel priority status */
} DMA_Flags;

#define IS_DMA_FLAG(FLAG)  (((FLAG)  == DMA_FLAG_DMA_ENA)    || \
                            ((FLAG)  == DMA_FLAG_DMA_ERR)    || \
                            ((FLAG)  == DMA_FLAG_CHNL_ENA)   || \
                            ((FLAG)  == DMA_FLAG_CHNL_MASK)  || \
                            ((FLAG)  == DMA_FLAG_CHNL_WAIT)  || \
                            ((FLAG)  == DMA_FLAG_CHNL_BURST) || \
                            ((FLAG)  == DMA_FLAG_CHNL_ALT)   || \
                            ((FLAG)  == DMA_FLAG_CHNL_PRIORITY))

/**
  * @brief DMA Channel Primary / Alternate Control Data initialization structure definition
  */
typedef struct
{
    uint32_t                        DMA_SourceBaseAddr;  /*!< Specifies the source base address for DMA Channelx. */
    uint32_t                        DMA_DestBaseAddr;    /*!< Specifies the destination base address for DMA Channelx. */
    DMA_Src_Inc_Mode                DMA_SourceIncSize;   /*!< Specifies the source address register increment value.
                                                              This parameter can be a value of @ref DMA_Src_Inc_Mode */
    DMA_Dest_Inc_Mode               DMA_DestIncSize;     /*!< Specifies the destination address register increment value.
                                                              This parameter can be a value of @ref DMA_Dest_Inc_Mode */
    DMA_Mem_Data_Size               DMA_MemoryDataSize;  /*!< Specifies the Memory data width.
                                                              This parameter can be a value of @ref DMA_Mem_Data_Size */
    DMA_Operating_Mode              DMA_Mode;            /*!< Specifies the operation mode of the DMA Channelx.
                                                              This parameter can be a value of @ref DMA_Operating_Mode. */
    uint32_t                        DMA_CycleSize;       /*!< Specifies the  total number of DMA transfers that the DMA cycle contains.
                                                              This parameter can be a value of [1; 1024] (@ref DMA_cycle_size). */
    DMA_Number_Continuous_Transfers DMA_NumContinuous;   /*!< Specifies how many DMA transfers can occur before the DMA controller rearbitrates.
                                                              This parameter can be a value of @ref DMA_Number_Continuous_Transfers. */
    uint32_t                        DMA_SourceProtCtrl;  /*!< Specifies the DMA Source protection control.
                                                              This parameter can be a value of @ref DMA_Src_Protection_Control. */
    uint32_t                        DMA_DestProtCtrl;    /*!< Specifies the DMA Destination protection control.
                                                              This parameter can be a value of @ref DMA_Dest_Protection_Control. */
} DMA_CtrlDataInitTypeDef;

/**
  * @brief DMA Channel Control Data structure definition
  */
typedef struct
{
    uint32_t DMA_SourceEndAddr;  /*!< Specifies the DMA channel source end address */
    uint32_t DMA_DestEndAddr;    /*!< Specifies the DMA channel destination end address. */
    uint32_t DMA_Control;        /*!< Specifies the DMA channel control data configuration. */
    uint32_t DMA_Unused;         /*!< Specifies the DMA channel unused memory. */
} DMA_CtrlDataTypeDef;

/**
  * @brief DMA Channel Init structure definition
  */
typedef struct
{
    DMA_CtrlDataInitTypeDef    *DMA_PriCtrlData;         /*!< Specifies the pointer to the primary control data structure.
                                                              This parameter should be NULL if primary data structure is not used */
    DMA_CtrlDataInitTypeDef    *DMA_AltCtrlData;         /*!< Specifies the pointer to the alternate control data structure.
                                                              This parameter should be NULL if alternate data structure is not used */
    uint32_t                    DMA_ProtCtrl;             /*!< Specifies the AHB-Lite protection control.
                                                              This parameter can be a values of @ref DMA_AHB_Protection_Control. */
    DMA_Priority_Level         DMA_Priority;             /*!< Specifies the software priority for the DMA Channelx.
                                                              This parameter can be a value of @ref DMA_Priority_Level */
    DMA_Burst_Mode             DMA_UseBurst;             /*!< Disables single request to generate DMA requests for the DMA Channelx.
                                                              This parameter can be a value of @ref DMA_Burst_Mode */
    DMA_Data_Struct_Selection  DMA_SelectDataStructure;  /*!< Specifies the primary /alternate data structure to use by the DMA channel.
                                                              This parameter can be a value of @ref DMA_Data_Struct_Selection. */
} DMA_ChannelInitTypeDef;

/**
  * @brief DMA Channel Scatter-Gather Init structure definition
  */
typedef struct
{
    DMA_CtrlDataTypeDef         *DMA_SG_TaskArray;  /*!< Specifies the pointer to the Scatter-Gather task structures array. The size
                                                         of the array should be equal to DMA_SG_TaskNumber.
                                                         Every structure of the array should contain the copy of the alternate control
                                                         data for the related task. It will be used by DMA to configure
                                                         the alternate control data while operating in the scatter-gather mode */
    uint32_t                    DMA_SG_TaskNumber;  /*!< Specifies the number of scatter-gather tasks to be executed by DMA */
    uint32_t                    DMA_SourceProtCtrl; /*!< Specifies the Primary Control Data DMA Source protection control.
                                                         This parameter can be a value of @ref DMA_Src_Protection_Control. */
    uint32_t                    DMA_DestProtCtrl;   /*!< Specifies the Primary Control Data DMA Destination protection control.
                                                         This parameter can be a value of @ref DMA_Dest_Protection_Control. */
    uint32_t                    DMA_ProtCtrl;       /*!< Specifies the AHB-Lite protection control.
                                                         This parameter can be a value of @ref DMA_AHB_Protection_Control. */
    DMA_Priority_Level          DMA_Priority;       /*!< Specifies the software priority for the DMA Channelx.
                                                         This parameter can be a value of @ref DMA_Priority_Level */
    DMA_Burst_Mode              DMA_UseBurst;       /*!< Disables single request to generate DMA requests for the DMA Channelx.
                                                         This parameter can be a value of @ref DMA_Burst_Mode */
} DMA_Channel_SG_InitTypeDef;

/** @} */ /* End of group DMA_Exported_Types */


/** @defgroup DMA_Exported_Constants DMA Exported Constants
  * @{
  */

/** @defgroup DMA_channels_number DMA channels number
  * @{
  */

#define IS_DMA_CHANNELS(NUMBER) (((NUMBER) >= 1) && ((NUMBER) <= 32))

/** @} */ /* End of group DMA_channels_number */

/** @defgroup DMA_cycle_size DMA cycle size
  * @{
  */

#define IS_DMA_CYCLE_SIZE(SIZE)   (((SIZE) >= 1) && ((SIZE) <= 1024))

/** @} */ /* End of group DMA_cycle_size */

/** @defgroup DMA_Control_Minus_1 DMA Control Minus 1
  * @{
  */

#define DMA_CONTROL_MINUS_1  ((uint32_t)0x3FF0)  /*!< DMA control "n_minus_1" field mask */

/** @} */ /* End of group DMA_Control_Minus_1 */

/** @} */ /* End of group DMA_Exported_Constants */


/** @addtogroup DMA_Exported_Variables DMA Exported Variables
 * @{
 */

/**
 * @brief DMA Channel Control Data Table
 */
extern DMA_CtrlDataTypeDef DMA_ControlTable[];

/** @} */ /* End of group DMA_Exported_Variables */


/** @addtogroup DMA_Exported_Functions DMA Exported Functions
  * @{
  */

void DMA_DeInit(void);

void DMA_CtrlDataInit(DMA_CtrlDataInitTypeDef *DMA_ctrl_data_ptr, DMA_CtrlDataTypeDef *DMA_ctrl_table_ptr);
void DMA_CtrlInit(uint8_t DMA_Channel, DMA_Data_Struct_Selection DMA_CtrlDataType, DMA_CtrlDataInitTypeDef* DMA_CtrlStruct);
void DMA_SG_CtrlInit(uint32_t DMA_Task, DMA_CtrlDataTypeDef *DMA_SG_TaskArray, DMA_CtrlDataInitTypeDef* DMA_CtrlStruct);

void DMA_SG_Init(uint8_t DMA_Channel, DMA_Channel_SG_InitTypeDef *DMA_SG_InitStruct);
void DMA_Init(uint8_t DMA_Channel, DMA_ChannelInitTypeDef* DMA_InitStruct);

void DMA_StructInit(DMA_ChannelInitTypeDef* DMA_InitStruct);
void DMA_SG_StructInit(DMA_Channel_SG_InitTypeDef* DMA_InitStruct);

void DMA_Cmd(uint8_t DMA_Channel, FunctionalState NewState);

void DMA_Request(uint8_t DMA_Channel);
void DMA_ClearError(void);

void DMA_ChannelReloadCycle(uint8_t DMA_Channel, DMA_Data_Struct_Selection SelectDataStruct,
                            uint32_t CycleSize, DMA_Operating_Mode ChannelMode);

uint32_t DMA_GetCurrTransferCounter(uint8_t DMA_Channel, DMA_Data_Struct_Selection DMA_CtrlData);

FlagStatus DMA_GetFlagStatus(uint8_t DMA_Channel, DMA_Flags DMA_Flag);

/** @} */ /* End of group DMA_Exported_Functions */

/** @} */ /* End of group DMA */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_DMA_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_dma.h */


