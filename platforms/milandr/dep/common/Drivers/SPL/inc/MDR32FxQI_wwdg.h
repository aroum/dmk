/**
  ******************************************************************************
  * @file    MDR32FxQI_wwdg.h
  * @author  Milandr Application Team
  * @version V2.0.0i
  * @date    14/03/2022
  * @brief   This file contains all the functions prototypes for the WWDG
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
#ifndef __MDR32FxQI_WWDG_H
#define __MDR32FxQI_WWDG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQIStandard Peripherial Driver
  * @{
  */

/** @addtogroup WWDG
  * @{
  */

/** @defgroup WWDG_Exported_Types WWDG Exported Types
  * @{
  */

/**
  * @brief WWDG Clock Prescaler
  */
typedef enum
{
    WWDG_Prescaler_1 = ((uint32_t)0x0000), /*!< WWDG counter clock = (PCLK1/4096)/1 */
    WWDG_Prescaler_2 = ((uint32_t)0x0080), /*!< WWDG counter clock = (PCLK1/4096)/2 */
    WWDG_Prescaler_4 = ((uint32_t)0x0100), /*!< WWDG counter clock = (PCLK1/4096)/4 */
    WWDG_Prescaler_8 = ((uint32_t)0x0180)  /*!< WWDG counter clock = (PCLK1/4096)/8 */
} WWDG_Prescaler;

#define IS_WWDG_PRESCALER(PRESCALER) (((PRESCALER) == WWDG_Prescaler_1) || \
                                      ((PRESCALER) == WWDG_Prescaler_2) || \
                                      ((PRESCALER) == WWDG_Prescaler_4) || \
                                      ((PRESCALER) == WWDG_Prescaler_8))

/** @} */ /* End of group WWDG_Exported_Types */


/** @defgroup WWDG_Exported_Constants WWDG Exported Constants
  * @{
  */

/** @defgroup WWDG_Window_Value WWDG Window value
  * @{
  */
#define IS_WWDG_WINDOW_VALUE(VALUE) (((VALUE) >= 0x40) && ((VALUE) <= 0x7F))
/** @} */ /* End of group WWDG_Window_Value */

/** @defgroup WWDG_Counter WWDG Counter
  * @{
  */
#define IS_WWDG_COUNTER(COUNTER) (((COUNTER) >= 0x40) && ((COUNTER) <= 0x7F))
/** @} */ /* End of group WWDG_Counter */

/** @} */ /* End of group WWDG_Exported_Constants */


/** @defgroup WWDG_Exported_Functions WWDG Exported Functions
  * @{
  */

void WWDG_DeInit(void);
void WWDG_SetPrescaler(WWDG_Prescaler WWDG_Prescaler_Value);
WWDG_Prescaler WWDG_GetPrescaler(void);
void WWDG_SetWindowValue(uint32_t WindowValue);
uint32_t WWDG_GetWindowValue(void);
void WWDG_EnableIT(void);
void WWDG_SetCounter(uint32_t Counter);
uint32_t WWDG_GetCounter(void);
void WWDG_Enable(uint32_t Counter);
FlagStatus WWDG_GetFlagStatus(void);
void WWDG_ClearFlag(void);

/** @} */ /* End of group WWDG_Exported_Functions */

/** @} */ /* End of group WWDG */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_WWDG_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_wwdg.h */


