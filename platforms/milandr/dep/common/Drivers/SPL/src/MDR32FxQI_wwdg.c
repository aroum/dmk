/**
  ******************************************************************************
  * @file    MDR32FxQI_wwdg.c
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    24/07/2024
  * @brief   This file contains all the WWDG firmware functions.
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
#include "MDR32FxQI_wwdg.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup WWDG WWDG
  * @{
  */

/** @defgroup WWDG_Private_Defines WWDG Private Defines
  * @{
  */
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    /* WWDG registers bit address in the alias region */
    #define WWDG_OFFSET                 (MDR_WWDG_BASE - PERIPH_BASE)

    #define SFR_OFFSET(TP, MOD, SFR)    ((uint32_t)&((TP*)MOD)->SFR)
    #define BB_ADDR(TP, MOD, SFR, BIT)  (PERIPH_BB_BASE + SFR_OFFSET(TP, MOD, SFR)*32 + BIT*4)
    #define WWDG_BB(SFR, BIT)           BB_ADDR(MDR_WWDG_TypeDef, WWDG_OFFSET, SFR, BIT)

    #define EWI_BB                      WWDG_BB(CFR, WWDG_CFR_EWI_Pos)
#endif /* #if defined  (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/* --------------------- WWDG registers bit mask ------------------------ */

/* CR register bit mask */
#define WDGA_Mask         ((uint32_t)0x00000080)

/* CFR register bit mask */
#define WDGTB_Mask        ((uint32_t)0xFFFFFE7F)
#define WIN_Mask          ((uint32_t)0x7F)
#define CTR_Mask          ((uint32_t)0x7F)

/** @} */ /* End of group WWDG_Private_Defines */


/** @defgroup WWDG_Exported_Functions WWDG Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the WWDG peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void WWDG_DeInit(void)
{
    MDR_WWDG->CR = 0;
    MDR_WWDG->CFR = 0;
    MDR_WWDG->SR = 0;
}

/**
  * @brief  Sets the WWDG Prescaler.
  * @param  WWDG_Prescaler_Value - @ref WWDG_Prescaler - specifies the WWDG Prescaler.
  * @retval None
  */
void WWDG_SetPrescaler(WWDG_Prescaler WWDG_Prescaler_Value)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_WWDG_PRESCALER(WWDG_Prescaler_Value));

    /* Clear WDGTB[1:0] bits */
    tmpreg = MDR_WWDG->CFR & WDGTB_Mask;

    /* Set WDGTB[1:0] bits according to WWDG_Prescaler_Value */
    tmpreg |= (uint32_t)WWDG_Prescaler_Value;

    /* Store the new value */
    MDR_WWDG->CFR = tmpreg;
}

/**
  * @brief  Gets the WWDG Prescaler.
  * @param  None
  * @retval @ref WWDG_Prescaler - Current WWDG prescaler
  */
WWDG_Prescaler WWDG_GetPrescaler(void)
{
    return (WWDG_Prescaler)(MDR_WWDG->CFR & ~WDGTB_Mask);
}

/**
  * @brief  Sets the WWDG window value.
  * @param  WindowValue: specifies the window value to be compared to the downcounter.
  *         This parameter must be a number between 0x40 and 0x7F.
  * @retval None
  */
void WWDG_SetWindowValue(uint32_t WindowValue)
{
    __IO uint32_t tmpreg = 0;

    /* Check the parameters */
    assert_param(IS_WWDG_WINDOW_VALUE(WindowValue));

    /* Clear W[6:0] bits */
    tmpreg = MDR_WWDG->CFR & (uint32_t) (~WIN_Mask);

    /* Set W[6:0] bits according to WindowValue value */
    tmpreg |= WindowValue & (uint32_t) WIN_Mask;

    /* Store the new value */
    MDR_WWDG->CFR = tmpreg;
}

/**
  * @brief  Gets the WWDG window value.
  * @param  None
  * @retval Current window value.
  */
uint32_t WWDG_GetWindowValue(void)
{
    return (MDR_WWDG->CFR & WIN_Mask);
}


/**
  * @brief  Enables the WWDG Early Wakeup interrupt(EWI).
  * @param  None
  * @retval None
  */
void WWDG_EnableIT(void)
{
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) EWI_BB = (uint32_t) ENABLE;
#elif defined (USE_K1986VE1xI)
    MDR_WWDG->CFR |= WWDG_CFR_EWI;
#endif
}

/**
  * @brief  Sets the WWDG counter value.
  * @param  Counter: specifies the watchdog counter value.
  *         This parameter must be a number between 0x40 and 0x7F.
  * @retval None
  */
void WWDG_SetCounter(uint32_t Counter)
{
    /* Check the parameters */
    assert_param(IS_WWDG_COUNTER(Counter));

    /* Write to T[6:0] bits to configure the counter value, no need to do
       a read-modify-write; writing a 0 to WDGA bit does nothing */
    MDR_WWDG->CR = Counter & CTR_Mask;
}

/**
  * @brief  Gets the WWDG counter value.
  * @param  None
  * @retval Current counter value
  */
uint32_t WWDG_GetCounter(void)
{
    return (MDR_WWDG->CR & CTR_Mask);
}

/**
  * @brief  Enables WWDG and load the counter value.
  * @param  Counter: specifies the watchdog counter value.
  *         This parameter must be a number between 0x40 and 0x7F.
  * @retval None
  */
void WWDG_Enable(uint32_t Counter)
{
    /* Check the parameters */
    assert_param(IS_WWDG_COUNTER(Counter));
    
    MDR_WWDG->CR = WDGA_Mask | Counter;
}

/**
  * @brief  Checks whether the Early Wakeup interrupt flag is set or not.
  * @param  None
  * @retval @ref FlagStatus - Current state of the Early Wakeup interrupt flag (SET or RESET)
  */
FlagStatus WWDG_GetFlagStatus(void)
{
    return (FlagStatus)(MDR_WWDG->SR);
}

/**
  * @brief  Clears Early Wakeup interrupt flag.
  * @param  None
  * @retval None
  */
void WWDG_ClearFlag(void)
{
    MDR_WWDG->SR = (uint32_t)RESET;
}

/** @} */ /* End of group WWDG_Exported_Functions */

/** @} */ /* End of group WWDG */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_wwdg.c */


