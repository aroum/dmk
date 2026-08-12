/**
  ******************************************************************************
  * @file    MDR32FxQI_power.h
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    23/07/2024
  * @brief   This file contains all the functions prototypes for the POWER
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
#ifndef __MDR32FxQI_POWER_H
#define __MDR32FxQI_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup POWER
  * @{
  */

/** @defgroup POWER_Exported_Types POWER Exported Types
  * @{
  */

/**
 *  @brief POWER Voltage Detector Level
 */
typedef enum
{
    PWR_PVDlevel_2V0 = ((uint32_t)(0x00 << 3)), /*!< PVD detection level set to 2.0V */
    PWR_PVDlevel_2V2 = ((uint32_t)(0x01 << 3)), /*!< PVD detection level set to 2.2V */
    PWR_PVDlevel_2V4 = ((uint32_t)(0x02 << 3)), /*!< PVD detection level set to 2.4V */
    PWR_PVDlevel_2V6 = ((uint32_t)(0x03 << 3)), /*!< PVD detection level set to 2.6V */
    PWR_PVDlevel_2V8 = ((uint32_t)(0x04 << 3)), /*!< PVD detection level set to 2.8V */
    PWR_PVDlevel_3V0 = ((uint32_t)(0x05 << 3)), /*!< PVD detection level set to 3.0V */
    PWR_PVDlevel_3V2 = ((uint32_t)(0x06 << 3)), /*!< PVD detection level set to 3.2V */
    PWR_PVDlevel_3V4 = ((uint32_t)(0x07 << 3))  /*!< PVD detection level set to 3.4V */
} PWR_Voltage_Detector_Lvl;

#define IS_POWER_PVD_LEVEL(LVL) (((LVL) == PWR_PVDlevel_2V0) || \
                                 ((LVL) == PWR_PVDlevel_2V2) || \
                                 ((LVL) == PWR_PVDlevel_2V4) || \
                                 ((LVL) == PWR_PVDlevel_2V6) || \
                                 ((LVL) == PWR_PVDlevel_2V8) || \
                                 ((LVL) == PWR_PVDlevel_3V0) || \
                                 ((LVL) == PWR_PVDlevel_3V2) || \
                                 ((LVL) == PWR_PVDlevel_3V4))

/**
 *  @brief POWER Battery Voltage Detector Level
 */
typedef enum
{
    PWR_PVBDlevel_1V8 = ((uint32_t)(0x00 << 1)), /*!< PVBD detection level set to 1.8V */
    PWR_PVBDlevel_2V2 = ((uint32_t)(0x01 << 1)), /*!< PVBD detection level set to 2.2V */
    PWR_PVBDlevel_2V6 = ((uint32_t)(0x02 << 1)), /*!< PVBD detection level set to 2.6V */
    PWR_PVBDlevel_3V0 = ((uint32_t)(0x03 << 1))  /*!< PVBD detection level set to 3.0V */
} PWR_Batt_Voltage_Detector_Lvl;

#define IS_POWER_PVBD_LEVEL(LVL) (((LVL) == PWR_PVBDlevel_1V8) || \
                                  ((LVL) == PWR_PVBDlevel_2V2) || \
                                  ((LVL) == PWR_PVBDlevel_2V6) || \
                                  ((LVL) == PWR_PVBDlevel_3V0))

/**
 *  @brief POWER Voltage Detector Output Flags
 */
typedef enum
{
    POWER_FLAG_PVD  = ((uint32_t)POWER_PVDCS_PVD), /*!< Power voltage detection flag */
    POWER_FLAG_PVBD = ((uint32_t)POWER_PVDCS_PVBD) /*!< Battery voltage detection flag */
} PWR_Voltage_Detector_Flags;

#define IS_POWER_FLAG(F) (((F) == POWER_FLAG_PVD) || ((F) == POWER_FLAG_PVBD))

#define IS_POWER_FLAGS(F) (((F) & ~(POWER_FLAG_PVD | POWER_FLAG_PVBD)) == 0x0)

/**
 *  @brief POWER Voltage Detector Interrupt Enable
 */
typedef enum
{
    POWER_PVD_IT  = ((uint32_t)POWER_PVDCS_IEPVD), /*!< PVD interrupt enable  */
    POWER_PVBD_IT = ((uint32_t)POWER_PVDCS_IEPVBD) /*!< PVBD interrupt enable */
} PWR_Voltage_Detector_ITEnable;

#define IS_POWER_PVD_IT(F) (((F) == POWER_PVD_IT) || ((F) == POWER_PVBD_IT))

/**
 *  @brief POWER Voltage Detector Interrupt Inversion
 */
typedef enum
{
    POWER_PVD_INV  = ((uint32_t)POWER_PVDCS_INV), /*!< PVD inversion enable  */
    POWER_PVBD_INV = ((uint32_t)POWER_PVDCS_INVB) /*!< PVBD inversion enable */
} PWR_Voltage_Detector_IT_Inversion;

#define IS_POWER_PVD_INV(F) (((F) == POWER_PVD_INV) || ((F) == POWER_PVBD_INV))


/** @} */ /* End of group POWER_Exported_Types */


/** @defgroup POWER_Exported_Functions POWER Exported Functions
  * @{
  */

void POWER_DeInit(void);

void POWER_PVDenable(FunctionalState NewState);
#if defined (USE_K1986VE1xI)
    void POWER_PVBDenable(FunctionalState NewState);
#endif
void POWER_PVDlevelConfig(PWR_Voltage_Detector_Lvl POWER_PVDlevel);
void POWER_PVBDlevelConfig(PWR_Batt_Voltage_Detector_Lvl POWER_PVBDlevel);
FlagStatus POWER_GetFlagStatus(PWR_Voltage_Detector_Flags POWER_FLAG);
void POWER_ClearFlag(uint32_t POWER_FLAG);
void POWER_PVD_ITconfig(uint32_t POWER_IT, FunctionalState NewState);
void POWER_PVD_INVconfig(uint32_t POWER_INV, FunctionalState NewState);

/** @} */ /* End of group POWER_Exported_Functions */

/** @} */ /* End of group POWER */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_POWER_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_power.h */


