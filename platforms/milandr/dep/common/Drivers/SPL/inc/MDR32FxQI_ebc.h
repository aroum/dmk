/**
  ******************************************************************************
  * @file    MDR32FxQI_ebc.h
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    23/03/2024
  * @brief   This file contains all the functions prototypes for the EBC
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
#ifndef __MDR32FxQI_EBC_H
#define __MDR32FxQI_EBC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup EBC
  * @{
  */

/** @defgroup EBC_Exported_Types EBC Exported Types
  * @{
  */

/**
  * @brief EBC mode
  */
typedef enum
{
    EBC_MODE_OFF  =  ((uint32_t)0x0),       /*!< EBC disabled */
    EBC_MODE_ROM  = (((uint32_t)0x1) << 0), /*!< EBC works with external ROM */
    EBC_MODE_RAM  = (((uint32_t)0x1) << 1), /*!< EBC works with external RAM */
    EBC_MODE_NAND = (((uint32_t)0x1) << 2)  /*!< EBC works with external NAND Flash memory */
} EBC_Mode_Sel;

#define EBC_MODE_MSK      (EBC_MODE_ROM | EBC_MODE_RAM | EBC_MODE_NAND)

#define IS_EBC_MODE(MODE) (((MODE) == EBC_MODE_OFF ) || \
                           ((MODE) == EBC_MODE_ROM ) || \
                           ((MODE) == EBC_MODE_RAM ) || \
                           ((MODE) == EBC_MODE_NAND))

/**
  * @brief EBC CLOCK polarity
  */
typedef enum
{
    EBC_CPOL_POSITIVE = (((uint32_t)0x0) << 3), /*!< EBC generates the positive CLOCK signal */
    EBC_CPOL_NEGATIVE = (((uint32_t)0x1) << 3)  /*!< EBC generates the negative CLOCK signal */
} EBC_Clk_Polarity;

#define IS_EBC_CPOL(CPOL)  (((CPOL) == EBC_CPOL_POSITIVE) || \
                            ((CPOL) == EBC_CPOL_NEGATIVE))

/**
  * @brief EBC Wait States number
  */
typedef enum
{
    EBC_WAIT_STATE_3HCLK  = ((uint32_t)0x0), /*!< Wait State = 3 HCLK clocks */
    EBC_WAIT_STATE_4HCLK  = ((uint32_t)0x1), /*!< Wait State = 4 HCLK clocks */
    EBC_WAIT_STATE_5HCLK  = ((uint32_t)0x2), /*!< Wait State = 5 HCLK clocks */
    EBC_WAIT_STATE_6HCLK  = ((uint32_t)0x3), /*!< Wait State = 6 HCLK clocks */
    EBC_WAIT_STATE_7HCLK  = ((uint32_t)0x4), /*!< Wait State = 7 HCLK clocks */
    EBC_WAIT_STATE_8HCLK  = ((uint32_t)0x5), /*!< Wait State = 8 HCLK clocks */
    EBC_WAIT_STATE_9HCLK  = ((uint32_t)0x6), /*!< Wait State = 9 HCLK clocks */
    EBC_WAIT_STATE_10HCLK = ((uint32_t)0x7), /*!< Wait State = 10 HCLK clocks */
    EBC_WAIT_STATE_11HCLK = ((uint32_t)0x8), /*!< Wait State = 11 HCLK clocks */
    EBC_WAIT_STATE_12HCLK = ((uint32_t)0x9), /*!< Wait State = 12 HCLK clocks */
    EBC_WAIT_STATE_13HCLK = ((uint32_t)0xA), /*!< Wait State = 13 HCLK clocks */
    EBC_WAIT_STATE_14HCLK = ((uint32_t)0xB), /*!< Wait State = 14 HCLK clocks */
    EBC_WAIT_STATE_15HCLK = ((uint32_t)0xC), /*!< Wait State = 15 HCLK clocks */
    EBC_WAIT_STATE_16HCLK = ((uint32_t)0xD), /*!< Wait State = 16 HCLK clocks */
    EBC_WAIT_STATE_17HCLK = ((uint32_t)0xE), /*!< Wait State = 17 HCLK clocks */
    EBC_WAIT_STATE_18HCLK = ((uint32_t)0xF)  /*!< Wait State = 18 HCLK clocks */
} EBC_Wait_State;

#define EBC_WAIT_STATE_MSK            ((uint32_t)0xF) /*!< Wait State value mask */

#define IS_EBC_WAIT_STATE(WAIT_STATE) (((WAIT_STATE) & ~EBC_WAIT_STATE_MSK) == 0)

/**
  * @brief EBC NAND Cycles
  */
typedef enum
{
    EBC_NAND_CYCLES_0HCLK  = ((uint32_t)0x0), /*!< NAND Cycles = 0 HCLK clocks */
    EBC_NAND_CYCLES_1HCLK  = ((uint32_t)0x1), /*!< NAND Cycles = 1 HCLK clocks */
    EBC_NAND_CYCLES_2HCLK  = ((uint32_t)0x2), /*!< NAND Cycles = 2 HCLK clocks */
    EBC_NAND_CYCLES_3HCLK  = ((uint32_t)0x3), /*!< NAND Cycles = 3 HCLK clocks */
    EBC_NAND_CYCLES_4HCLK  = ((uint32_t)0x4), /*!< NAND Cycles = 4 HCLK clocks */
    EBC_NAND_CYCLES_5HCLK  = ((uint32_t)0x5), /*!< NAND Cycles = 5 HCLK clocks */
    EBC_NAND_CYCLES_6HCLK  = ((uint32_t)0x6), /*!< NAND Cycles = 6 HCLK clocks */
    EBC_NAND_CYCLES_7HCLK  = ((uint32_t)0x7), /*!< NAND Cycles = 7 HCLK clocks */
    EBC_NAND_CYCLES_8HCLK  = ((uint32_t)0x8), /*!< NAND Cycles = 8 HCLK clocks */
    EBC_NAND_CYCLES_9HCLK  = ((uint32_t)0x9), /*!< NAND Cycles = 9 HCLK clocks */
    EBC_NAND_CYCLES_10HCLK = ((uint32_t)0xA), /*!< NAND Cycles = 10 HCLK clocks */
    EBC_NAND_CYCLES_11HCLK = ((uint32_t)0xB), /*!< NAND Cycles = 11 HCLK clocks */
    EBC_NAND_CYCLES_12HCLK = ((uint32_t)0xC), /*!< NAND Cycles = 12 HCLK clocks */
    EBC_NAND_CYCLES_13HCLK = ((uint32_t)0xD), /*!< NAND Cycles = 13 HCLK clocks */
    EBC_NAND_CYCLES_14HCLK = ((uint32_t)0xE), /*!< NAND Cycles = 14 HCLK clocks */
    EBC_NAND_CYCLES_15HCLK = ((uint32_t)0xF)  /*!< NAND Cycles = 15 HCLK clocks */
} EBC_NAND_Cycles;

#define EBC_NAND_CYCLES_MSK             ((uint32_t)0xF) /*!< NAND Cycles value mask */

#define IS_EBC_NAND_CYCLES(NAND_CYCLES) (((NAND_CYCLES) & ~EBC_NAND_CYCLES_MSK) == 0)


#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))

/**
  * @brief EBC Data Alignment
  */
typedef enum
{
    EBC_EBC_DATA_ALIGNMENT_OFF = ((uint32_t)0x00), /*!< Not use data alignment */
    EBC_EBC_DATA_ALIGNMENT_8   = ((uint32_t)0x20), /*!< Data alignment to 8 junior ranks EBC */
    EBC_EBC_DATA_ALIGNMENT_16  = ((uint32_t)0x40)  /*!< Data alignment to 16 junior ranks EBC */
} EBC_Data_Alignment;

#define IS_EBC_DATA_ALIGNMENT(EBC_DATA_ALIGNMENT)  (((EBC_DATA_ALIGNMENT) == EBC_EBC_DATA_ALIGNMENT_OFF) || \
                                                    ((EBC_DATA_ALIGNMENT) == EBC_EBC_DATA_ALIGNMENT_8)   || \
                                                    ((EBC_DATA_ALIGNMENT) == EBC_EBC_DATA_ALIGNMENT_16))

/**
  * @brief EBC Memory Region
  */
typedef enum
{
    EBC_USE_MEM_REGION_NONE = ((uint32_t)0x0),
    EBC_USE_MEM_REGION_1    = ((uint32_t)0x1),
    EBC_USE_MEM_REGION_2    = ((uint32_t)0x2),
    EBC_USE_MEM_REGION_3    = ((uint32_t)0x4),
    EBC_USE_MEM_REGION_4    = ((uint32_t)0x8)
} EBC_Mem_Region_Use;

#define IS_EBC_USE_MEM_REGION(EBC_USE_MEM_REGION)  (((EBC_USE_MEM_REGION) == EBC_USE_MEM_REGION_NONE) || \
                                                    ((EBC_USE_MEM_REGION) == EBC_USE_MEM_REGION_1)    || \
                                                    ((EBC_USE_MEM_REGION) == EBC_USE_MEM_REGION_2)    || \
                                                    ((EBC_USE_MEM_REGION) == EBC_USE_MEM_REGION_3)    || \
                                                    ((EBC_USE_MEM_REGION) == EBC_USE_MEM_REGION_4))

/**
  * @brief EBC WS HOLD Cycles
  */
typedef enum
{
    EBC_WS_HOLD_CYCLE_1HCLK = ((uint32_t)0x00), /*!< WS HOLD Cycles = 1 HCLK clocks */
    EBC_WS_HOLD_CYCLE_2HCLK = ((uint32_t)0x01), /*!< WS HOLD Cycles = 2 HCLK clocks */
    EBC_WS_HOLD_CYCLE_3HCLK = ((uint32_t)0x02), /*!< WS HOLD Cycles = 3 HCLK clocks */
    EBC_WS_HOLD_CYCLE_4HCLK = ((uint32_t)0x03), /*!< WS HOLD Cycles = 4 HCLK clocks */
    EBC_WS_HOLD_CYCLE_5HCLK = ((uint32_t)0x04), /*!< WS HOLD Cycles = 5 HCLK clocks */
    EBC_WS_HOLD_CYCLE_6HCLK = ((uint32_t)0x05), /*!< WS HOLD Cycles = 6 HCLK clocks */
    EBC_WS_HOLD_CYCLE_7HCLK = ((uint32_t)0x06), /*!< WS HOLD Cycles = 7 HCLK clocks */
    EBC_WS_HOLD_CYCLE_8HCLK = ((uint32_t)0x07)  /*!< WS HOLD Cycles = 8 HCLK clocks */
} EBC_WS_Hold_Cycles;

#define EBC_WS_HOLD_CYCLES_MSK    0x07

#define IS_EBC_WS_HOLD_CYCLES(EBC_WS_HOLD_CYCLES) (((EBC_WS_HOLD_CYCLES) & ~EBC_WS_HOLD_CYCLES_MSK) == 0)

/**
  * @brief EBC WS Setup Cycles
  */
typedef enum
{
    EBC_WS_SETUP_CYCLE_1HCLK = ((uint32_t)0x00), /*!< WS SETUP Cycles = 1 HCLK clocks */
    EBC_WS_SETUP_CYCLE_2HCLK = ((uint32_t)0x01), /*!< WS SETUP Cycles = 2 HCLK clocks */
    EBC_WS_SETUP_CYCLE_3HCLK = ((uint32_t)0x02), /*!< WS SETUP Cycles = 3 HCLK clocks */
    EBC_WS_SETUP_CYCLE_4HCLK = ((uint32_t)0x03), /*!< WS SETUP Cycles = 4 HCLK clocks */
    EBC_WS_SETUP_CYCLE_5HCLK = ((uint32_t)0x04), /*!< WS SETUP Cycles = 5 HCLK clocks */
    EBC_WS_SETUP_CYCLE_6HCLK = ((uint32_t)0x05), /*!< WS SETUP Cycles = 6 HCLK clocks */
    EBC_WS_SETUP_CYCLE_7HCLK = ((uint32_t)0x06), /*!< WS SETUP Cycles = 7 HCLK clocks */
    EBC_WS_SETUP_CYCLE_8HCLK = ((uint32_t)0x07)  /*!< WS SETUP Cycles = 8 HCLK clocks */
} EBC_WS_Setup_Cycles;

#define EBC_WS_SETUP_CYCLES_MSK    0x07

#define IS_EBC_WS_SETUP_CYCLES(EBC_WS_SETUP_CYCLES) (((EBC_WS_SETUP_CYCLES) & ~EBC_WS_SETUP_CYCLES_MSK) == 0)

/**
  * @brief EBC Memory Region Selection
  */
typedef enum
{
    EBC_MEM_REGION_10000000 = (uint32_t)(0x00),
    EBC_MEM_REGION_50000000 = (uint32_t)(0x01),
    EBC_MEM_REGION_60000000 = (uint32_t)(0x02),
    EBC_MEM_REGION_70000000 = (uint32_t)(0x03)
} EBC_Mem_Region_Select;

#define IS_EBC_MEM_REGION(EBC_MEM_REGION)  (((EBC_MEM_REGION) == EBC_MEM_REGION_10000000) || \
                                            ((EBC_MEM_REGION) == EBC_MEM_REGION_50000000) || \
                                            ((EBC_MEM_REGION) == EBC_MEM_REGION_60000000) || \
                                            ((EBC_MEM_REGION) == EBC_MEM_REGION_70000000))

#define IS_EBC_WS_ACTIVE(WS_ACTIVE)        (((WS_ACTIVE) & ~0x7F) == 0)

#endif /* #if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI)) */

/**
  * @brief EBC Init structure definition
  */
typedef struct
{
    EBC_Mode_Sel       EBC_Mode;          /*!< Specifies external bus mode.
                                               This parameter can be a value of @ref EBC_Mode_Sel. */
    EBC_Clk_Polarity   EBC_Cpol;          /*!< Specifies CLOCK signal polarity.
                                               This parameter can be a value of @ref EBC_Clk_Polarity. */
    EBC_Wait_State     EBC_WaitState;     /*!< Specifies wait states number.
                                               This parameter can be a value of @ref EBC_Wait_State. */
    EBC_NAND_Cycles    EBC_NandTrc;       /*!< Specifies NAND read cycle time t_rc.
                                               This parameter can be a value of @ref EBC_NAND_Cycles. */
    EBC_NAND_Cycles    EBC_NandTwc;       /*!< Specifies NAND write cycle time t_wc.
                                               This parameter can be a value of @ref EBC_NAND_Cycles. */
    EBC_NAND_Cycles    EBC_NandTrea;      /*!< Specifies NAND read access time t_rea.
                                               This parameter can be a value of @ref EBC_NAND_Cycles. */
    EBC_NAND_Cycles    EBC_NandTwp;       /*!< Specifies NAND write access time t_wp.
                                               This parameter can be a value of @ref EBC_NAND_Cycles. */
    EBC_NAND_Cycles    EBC_NandTwhr;      /*!< Specifies NAND status register access time t_whr.
                                               This parameter can be a value of @ref EBC_NAND_Cycles. */
    EBC_NAND_Cycles    EBC_NandTalea;     /*!< Specifies NAND ID registers access time t_alea.
                                               This parameter can be a value of @ref EBC_NAND_Cycles. */
    EBC_NAND_Cycles    EBC_NandTrr;       /*!< Specifies NAND delay from Busy release to read operation.
                                               This parameter can be a value of @ref EBC_NAND_Cycles. */
#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
    EBC_Data_Alignment EBC_DataAlignment; /*!< Specifies data Alignment.
                                               This parameter can be a value of @ref EBC_Data_Alignment. */
    EBC_Mem_Region_Use EBC_UseMemRegion;  /*!< Specifies resolution settings of exchange of the appropriate range of addresses.
                                               This parameter can be value of @ref EBC_Mem_Region_Use. */
#endif /* #if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI)) */
} EBC_InitTypeDef;

#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
/**
 *  @brief EBC RAM RAM_Cycles structure definition
 */
typedef struct
{
    FunctionalState     Use_Ready;   /*!< Specifies resolution survey of the external signal to the READY output PF [2].
                                          This parameter can be a value of @ref FunctionalState */
    EBC_WS_Hold_Cycles  WS_Hold;     /*!< Specifies hold time signals nWE / nOE.
                                          This parameter can be a value of @ref EBC_WS_Hold_Cycles. */
    EBC_WS_Setup_Cycles WS_Setup;    /*!< Specifies time preset signals nWE / nOE in cycle read / write.
                                          This parameter can be a value of @ref EBC_WS_Setup_Cycles. */
    uint8_t             WS_Active;   /*!< Specifies continued low level signals nWE / nOE in cycle read / write.
                                          This parameter can be a number between 0x0000 and 0x7F. */
    FunctionalState     Enable_Tune; /*!< Specifies resolution settings of exchange of the appropriate range of addresses.
                                          This parameter can be a value of @ref FunctionalState */
} EBC_MemRegionInitTypeDef;

#endif /* #if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI)) */

/** @} */ /* End of group EBC_Exported_Types */


/** @defgroup EBC_Exported_Functions EBC Exported Functions
  * @{
  */

void EBC_DeInit(void);
void EBC_Init(const EBC_InitTypeDef* EBC_InitStruct);
void EBC_StructInit(EBC_InitTypeDef* EBC_InitStruct);
uint32_t EBC_CalcWaitStates(uint32_t HCLK_Frequency_KHz, uint32_t Time_ns);
uint32_t EBC_CalcNandCycles(uint32_t HCLK_Frequency_KHz, uint32_t Time_ns);
FlagStatus EBC_GetBusyStatus(void);
#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
    void EBC_MemRegionStructInit(EBC_MemRegionInitTypeDef* EBC_MemRegionInitStruct);
    void EBC_MemRegionInit(EBC_MemRegionInitTypeDef* EBC_MemRegionInitStruct, EBC_Mem_Region_Select EBC_MEM_REGIONx);
    void EBC_MemRegionCMD(EBC_Mem_Region_Select EBC_MEM_REGIONx, FunctionalState NewState);
#endif

/** @} */ /* End of group EBC_Exported_Functions */

/** @} */ /* End of group EBC */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_EBC_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_ebc.h */


