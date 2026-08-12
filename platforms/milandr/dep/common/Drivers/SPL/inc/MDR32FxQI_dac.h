/**
  ******************************************************************************
  * @file    MDR32FxQI_dac.h
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    24/07/2024
  * @brief   This file contains all the functions prototypes for the DAC
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
#ifndef __MDR32FxQI_DAC_H
#define __MDR32FxQI_DAC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup DAC
  * @{
  */

/** @defgroup DAC_Exported_Types DAC Exported Types
  * @{
  */

#if !defined (USE_K1986VE9xI)
/**
  * @brief DAC Synchronous Mode
  */
typedef enum
{
    DAC_SYNC_MODE_Independent = (((uint32_t)0x0) << DAC_CFG_SYNC_A_Pos), /*!< The independent DAC1, DAC2 operation mode. */
    DAC_SYNC_MODE_Synchronous = (((uint32_t)0x1) << DAC_CFG_SYNC_A_Pos)  /*!< The synchronous DAC1, DAC2 operation mode. */
} DAC_Sync_Mode;

#define IS_DAC_SYNC_MODE(MODE) (((MODE) == DAC_SYNC_MODE_Independent) || \
                                ((MODE) == DAC_SYNC_MODE_Synchronous))

/**
  * @brief DAC1 Reference Voltage source
  */
typedef enum
{
    DAC1_AVCC = (((uint32_t)0x0) << DAC_CFG_M_REF0_Pos), /*!< Selects AVcc as Reference Voltage. */
    DAC1_REF  = (((uint32_t)0x1) << DAC_CFG_M_REF0_Pos)  /*!< Selects DAC1_REF input as Reference Voltage. */
} DAC1_Ref_Src;

#define IS_DAC1_REF_CONFIG(CONFIG) (((CONFIG) == DAC1_AVCC) || \
                                    ((CONFIG) == DAC1_REF ))
#endif

/**
  * @brief DAC2 Reference Voltage source
  */
typedef enum
{
    DAC2_AVCC = (((uint32_t)0x0) << DAC_CFG_M_REF1_Pos), /*!< Selects AVcc as Reference Voltage. */
    DAC2_REF  = (((uint32_t)0x1) << DAC_CFG_M_REF1_Pos)  /*!< Selects DAC2_REF input as Reference Voltage. */
} DAC2_Ref_Src;

#define IS_DAC2_REF_CONFIG(CONFIG) (((CONFIG) == DAC2_AVCC) || \
                                    ((CONFIG) == DAC2_REF ))

/** @defgroup DAC_Data DAC Data
  * @{
  */

#define DAC_LO_DATA_MSK                       ((uint32_t)0x0FFF)
#define DAC_HI_DATA_MSK                       (DAC_LO_DATA_MSK << 16)

#define IS_DAC_DATA(DATA) (((DATA) & ~(DAC_LO_DATA_MSK | DAC_HI_DATA_MSK)) == 0)

/** @} */ /* End of group DAC_Data */

/** @} */ /* End of group DAC_Exported_Types */


/** @defgroup DAC_Exported_Functions DAC Exported Functions
  * @{
  */

void DAC_DeInit(void);

#if !defined (USE_K1986VE9xI)
void DAC_Init(DAC_Sync_Mode SyncMode, DAC1_Ref_Src DAC1_Ref, DAC2_Ref_Src DAC2_Ref);

void DAC1_Init(DAC1_Ref_Src DAC1_Ref);
#endif
void DAC2_Init(DAC2_Ref_Src DAC2_Ref);

#if !defined (USE_K1986VE9xI)
void DAC1_Cmd(FunctionalState NewState);
#endif
void DAC2_Cmd(FunctionalState NewState);

#if !defined (USE_K1986VE9xI)
void DAC1_SetData(uint32_t Data);
#endif
void DAC2_SetData(uint32_t Data);

#if !defined (USE_K1986VE9xI)
uint32_t DAC1_GetData(void);
#endif
uint32_t DAC2_GetData(void);

/** @} */ /* End of group DAC_Exported_Functions */

/** @} */ /* End of group DAC */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_DAC_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_dac.h */

