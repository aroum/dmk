/**
  ******************************************************************************
  * @file    MDR32FxQI_iwdg.c
  * @author  Milandr Application Team
  * @version V2.0.3i
  * @date    17/03/2022
  * @brief   This file contains all the IWDG firmware functions.
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

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_iwdg.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup IWDG IWDG
  * @{
  */

/** @defgroup IWDG_Private_Defines IWDG Private Defines
  * @{
  */

/* ---------------------- IWDG registers bit mask ----------------------------*/
/* KR register bit mask */
#define KR_KEY_Reload       ((uint32_t)0xAAAA)
#define KR_KEY_Enable       ((uint32_t)0xCCCC)
#define KR_KEY_WriteEnable  ((uint32_t)0x5555)
#define KR_KEY_WriteDisable ((uint32_t)0x0000)

/** @} */ /* End of group IWDG_Private_Defines */


/** @defgroup IWDG_Exported_Functions IWDG Exported Functions
  * @{
  */

/**
  * @brief  Enables write access to IWDG_PR and IWDG_RLR registers.
  * @param  None
  * @retval None
  */
void IWDG_WriteAccessEnable(void)
{
    MDR_IWDG->KR = KR_KEY_WriteEnable;
}

/**
  * @brief  Disables write access to IWDG_PR and IWDG_RLR registers.
  * @param  None
  * @retval None
  */
void IWDG_WriteAccessDisable(void)
{
    MDR_IWDG->KR = KR_KEY_WriteDisable;
}

/**
  * @brief  Sets IWDG Prescaler value.
  * @note   Watchdog timer IWDG is clocked by the frequency of the LSI generator.
  *         Reading and writing of this register is allowed only if bit PVU=0 in SR register.
  * @param  IWDG_Prescaler_Value - @ref IWDG_Prescaler - specifies the IWDG Prescaler value.
  * @retval None
  */
void IWDG_SetPrescaler(IWDG_Prescaler IWDG_Prescaler_Value)
{
    /* Check the parameters */
    assert_param(IS_IWDG_PRESCALER(IWDG_Prescaler_Value));

    MDR_IWDG->PR = IWDG_Prescaler_Value;
}

/**
  * @brief  Gets IWDG Prescaler value.
  * @note   Reading and writing of this register is allowed only if bit PVU=0 in SR register.
  * @param  None
  * @retval @ref IWDG_Prescaler - Current IWDG Prescaler value.
  */
IWDG_Prescaler IWDG_GetPrescaler(void)
{
    return (IWDG_Prescaler)(MDR_IWDG->PR);
}

/**
  * @brief  Sets IWDG Reload value.
  * @note   Reading and writing of this register is allowed only if bit RVU=0 in SR register.
  * @param  Reload: specifies the IWDG Reload value.
  *         This parameter must be a number between 0 and 0x0FFF.
  * @retval None
  */
void IWDG_SetReload(uint32_t Reload)
{
    /* Check the parameters */
    assert_param(IS_IWDG_RELOAD(Reload));

    MDR_IWDG->RLR = Reload;
}

/**
  * @brief  Gets IWDG Reload value.
  * @note   Reading and writing of this register is allowed only if bit RVU=0 in SR register.
  * @param  Reload: specifies the IWDG Reload value.
  *         This parameter must be a number between 0 and 0x0FFF.
  * @retval None
  */
uint32_t IWDG_GetReload(void)
{
    return (MDR_IWDG->RLR & 0x0FFF);
}

/**
  * @brief  Reloads IWDG counter with value defined in the reload register
  *         (write access to IWDG_PR and IWDG_RLR registers disabled).
  * @param  None
  * @retval None
  */
void IWDG_ReloadCounter(void)
{
    MDR_IWDG->KR = KR_KEY_Reload;
}

/**
  * @brief  Enables IWDG (write access to IWDG_PR and IWDG_RLR registers disabled).
  * @param  None
  * @retval None
  */
void IWDG_Enable(void)
{
    MDR_IWDG->KR = KR_KEY_Enable;
}

/**
  * @brief  Checks whether the specified IWDG flag is set or not.
  * @param  IWDG_FLAG - @ref IWDG_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - The state of IWDG_FLAG (SET or RESET).
  */
FlagStatus IWDG_GetFlagStatus(IWDG_Flags IWDG_FLAG)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_IWDG_FLAG(IWDG_FLAG));

    if ((MDR_IWDG->SR & IWDG_FLAG) != (uint32_t)RESET)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    /* Return the flag status */
    return bitstatus;
}

/** @} */ /* End of group IWDG_Exported_Functions */

/** @} */ /* End of group IWDG */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_iwdg.c */


