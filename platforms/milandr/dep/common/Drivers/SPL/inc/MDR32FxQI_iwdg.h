/**
  ******************************************************************************
  * @file    MDR32FxQI_iwdg.h
  * @author  Milandr Application Team
  * @version V2.0.0i
  * @date    10/03/2022
  * @brief   This file contains all the functions prototypes for the IWDG
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
#ifndef __MDR32FxQI_IWDG_H
#define __MDR32FxQI_IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup IWDG
  * @{
  */

/** @defgroup IWDG_Exported_Types IWDG Exported Types
  * @{
  */

/**
  * @brief IWDG prescaler
  */
typedef enum
{
    IWDG_Prescaler_4   = ((uint32_t)0x00),
    IWDG_Prescaler_8   = ((uint32_t)0x01),
    IWDG_Prescaler_16  = ((uint32_t)0x02),
    IWDG_Prescaler_32  = ((uint32_t)0x03),
    IWDG_Prescaler_64  = ((uint32_t)0x04),
    IWDG_Prescaler_128 = ((uint32_t)0x05),
    IWDG_Prescaler_256 = ((uint32_t)0x06)
} IWDG_Prescaler;

#define IS_IWDG_PRESCALER(PRESCALER) (((PRESCALER) == IWDG_Prescaler_4)  || \
                                      ((PRESCALER) == IWDG_Prescaler_8)  || \
                                      ((PRESCALER) == IWDG_Prescaler_16) || \
                                      ((PRESCALER) == IWDG_Prescaler_32) || \
                                      ((PRESCALER) == IWDG_Prescaler_64) || \
                                      ((PRESCALER) == IWDG_Prescaler_128)|| \
                                      ((PRESCALER) == IWDG_Prescaler_256))

/**
  * @brief IWDG Flags
  */
typedef enum
{
    IWDG_FLAG_PVU = IWDG_SR_PVU, /*!< Prescaler Value Update on going. */
    IWDG_FLAG_RVU = IWDG_SR_RVU  /*!< Reload Value Update on going. */
} IWDG_Flags;

#define IS_IWDG_FLAG(FLAG) (((FLAG) == IWDG_FLAG_PVU) || ((FLAG) == IWDG_FLAG_RVU))

/** @} */ /* End of group IWDG_Exported_Types */


/** @defgroup IWDG_Exported_Constants IWDG Exported Constants
  * @{
  */

/** @defgroup IWDG_Reload IWDG Reload
  * @{
  */

#define IS_IWDG_RELOAD(RELOAD) ((RELOAD) <= 0xFFF)

/** @} */ /* End of group IWDG_Reload */

/** @} */ /* End of group IWDG_Exported_Constants */


/** @defgroup IWDG_Exported_Functions IWDG Exported Functions
  * @{
  */

void IWDG_WriteAccessEnable(void);
void IWDG_WriteAccessDisable(void);
void IWDG_SetPrescaler(IWDG_Prescaler IWDG_Prescaler_Value);
IWDG_Prescaler IWDG_GetPrescaler(void);
void IWDG_SetReload(uint32_t Reload);
uint32_t IWDG_GetReload(void);
void IWDG_ReloadCounter(void);
void IWDG_Enable(void);
FlagStatus IWDG_GetFlagStatus(IWDG_Flags IWDG_FLAG);

/** @} */ /* End of group IWDG_Exported_Functions */

/** @} */ /* End of group IWDG */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_IWDG_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_iwdg.h */

