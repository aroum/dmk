/**
  ******************************************************************************
  * @file    MDR32FxQI_power.c
  * @author  Milandr Application Team
  * @version V2.2.1i
  * @date    24/07/2024
  * @brief   This file contains all the POWER firmware functions.
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
#include "MDR32FxQI_power.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup POWER POWER
  * @{
  */

/** @defgroup POWER_Private_Defines POWER Private Defines
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/* POWER registers bit address in the alias region */
#define POWER_OFFSET                (MDR_POWER_BASE - PERIPH_BASE)
#define BKP_OFFSET                  (MDR_BKP_BASE - PERIPH_BASE)

#define SFR_OFFSET(TP, MOD, SFR)    ((uint32_t)&((TP*)MOD)->SFR)
#define BB_ADDR(TP, MOD, SFR, BIT)  (PERIPH_BB_BASE + SFR_OFFSET(TP, MOD, SFR)*32 + BIT*4)
#define POWER_BB(SFR, BIT)          BB_ADDR(MDR_POWER_TypeDef, POWER_OFFSET, SFR, BIT)
#define BKP_BB(SFR, BIT)            BB_ADDR(MDR_BKP_TypeDef, BKP_OFFSET, SFR, BIT)

#define POWER_PVDEN_BB              POWER_BB(PVDCS, POWER_PVDCS_PVDEN_Pos)

#define BKP_FPOR_BB                 BKP_BB(REG_0E, BKP_REG_0E_FPOR_Pos)
#define BKP_STANDBY_BB              BKP_BB(REG_0F, BKP_REG_0F_STANDBY_Pos)

#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/* --------------------- POWER registers bit mask ------------------------ */

/* BKP_REG0E register bit mask */
#define DUcc_Mask         ((uint32_t)(BKP_REG_0E_LOW_Msk | BKP_REG_0E_SELECTRI_Msk))
#define DUccTrim_Mask     ((uint32_t)BKP_REG_0E_TRIM_Msk)

/** @} */ /* End of group POWER_Private_Defines */


/** @defgroup POWER_Exported_Functions POWER Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the POWER peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void POWER_DeInit(void)
{
    MDR_POWER->PVDCS = (uint32_t)0x00000000;
    MDR_POWER->PVDCS = (uint32_t)0x00000000;
}


/**
  * @brief  Enables or disables the Power Voltage Detectors:
  *             - PVD, PVBD for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             - PVD for MDR32F1QI, K1986VE1xI.
  * @param  NewState - @ref FunctionalState - new state of the PVDs.
  * @retval None
  */
void POWER_PVDenable(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) POWER_PVDEN_BB = (uint32_t) NewState;
#elif defined (USE_K1986VE1xI)

    if(NewState != DISABLE)
    {
        MDR_POWER->PVDCS |= POWER_PVDCS_PVDEN;
    }
    else
    {
        MDR_POWER->PVDCS &= ~POWER_PVDCS_PVDEN;
    }
#endif
}

#if defined (USE_K1986VE1xI)
/**
  * @brief  Enables or disables the Power Voltage Battery Detector (PVBD).
  * @note   This function can be used only for MDR32F1QI, K1986VE1xI microcontrollers.
  * @param  NewState - @ref FunctionalState - new state of the PVBD.
  * @retval None
  */
void POWER_PVBDenable(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_POWER->PVDCS |= POWER_PVDCS_PVDBEN;
    }
    else
    {
        MDR_POWER->PVDCS &= ~POWER_PVDCS_PVDBEN;
    }
}
#endif

/**
  * @brief  Configures the voltage threshold detected
  *         by the Power Voltage Detector(PVD).
  * @param  POWER_PVDlevel - @ref PWR_Voltage_Detector_Lvl - specifies the PVD detection level.
  * @retval None
  */
void POWER_PVDlevelConfig(PWR_Voltage_Detector_Lvl POWER_PVDlevel)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_POWER_PVD_LEVEL(POWER_PVDlevel));

    tmpreg = MDR_POWER->PVDCS;

    /* Clear PLS bits */
    tmpreg &= (uint32_t) ~POWER_PVDCS_PLS_Msk;
    /* Set PLS bits according to POWER_PVDlevel value */
    tmpreg |= ((uint32_t)POWER_PVDlevel & POWER_PVDCS_PLS_Msk);

    /* Store the new value */
    MDR_POWER->PVDCS = tmpreg;
}

/**
  * @brief  Configures the voltage threshold detected by the Power Battery Voltage Detector(PVBD).
  * @param  POWER_PVBDlevel - @ref PWR_Batt_Voltage_Detector_Lvl - specifies the PVBD detection level.
  * @retval None
  */
void POWER_PVBDlevelConfig(PWR_Batt_Voltage_Detector_Lvl POWER_PVBDlevel)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_POWER_PVBD_LEVEL(POWER_PVBDlevel));

    tmpreg = MDR_POWER->PVDCS;

    /* Clear PBLS bits */
    tmpreg &= (uint32_t) ~POWER_PVDCS_PBLS_Msk;
    /* Set PBLS bits according to POWER_PVBDlevel value */
    tmpreg |= ((uint32_t)POWER_PVBDlevel & POWER_PVDCS_PBLS_Msk);

    /* Store the new value */
    MDR_POWER->PVDCS = tmpreg;
}

/**
  * @brief  Checks whether the specified POWER detection flag is set or not.
  * @param  POWER_FLAG - @ref PWR_Voltage_Detector_Flags - specifies the flag to check.
  * @retval The state of POWER_FLAG (SET or RESET).
  */
FlagStatus POWER_GetFlagStatus(PWR_Voltage_Detector_Flags POWER_FLAG)
{
    FlagStatus status;

    /* Check the parameters */
    assert_param(IS_POWER_FLAG(POWER_FLAG));

    if ((MDR_POWER->PVDCS & (uint32_t)POWER_FLAG) != (uint32_t)RESET)
    {
        status = SET;
    }
    else
    {
        status = RESET;
    }

    /* Return the flag status */
    return status;
}

/**
  * @brief  Clears the POWER's pending flags.
  * @param  POWER_FLAG: specifies the flag to clear.
  *         This parameter can be any combination of the @ref PWR_Voltage_Detector_Flags values.
  * @retval None
  */
void POWER_ClearFlag(uint32_t POWER_FLAG)
{
    /* Check the parameters */
    assert_param(IS_POWER_FLAGS(POWER_FLAG));

    MDR_POWER->PVDCS &= (uint32_t) ~POWER_FLAG;
    MDR_POWER->PVDCS &= (uint32_t) ~POWER_FLAG;
}

/**
  * @brief  Enables or disables the specified POWER interrupts.
  * @param  POWER_IT: specifies the PVD interrupts sources to be enabled or disabled.
  *         This parameter can be any combination of the @ref PWR_Voltage_Detector_ITEnable values.
  * @param  NewState - @ref FunctionalState - new state of the specified POWER interrupts.
  * @retval None
  */
void POWER_PVD_ITconfig(uint32_t POWER_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_POWER_PVD_IT(POWER_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        MDR_POWER->PVDCS |= POWER_IT;
    }
    else
    {
        MDR_POWER->PVDCS &= (uint32_t)~POWER_IT;
    }
}

/**
  * @brief  Enables or disables inversion of the specified POWER voltage detection flags
  * @param  POWER_INV: specifies the PVD interrupts inversion to be enabled or disabled.
  *         This parameter can be any combination of the @ref PWR_Voltage_Detector_IT_Inversion values.
  * @param  NewState - @ref FunctionalState - new state of the specified POWER voltage flag inversion.
  * @retval None
  */
void POWER_PVD_INVconfig(uint32_t POWER_INV, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_POWER_PVD_INV(POWER_INV));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        MDR_POWER->PVDCS |= POWER_INV;
    }
    else
    {
        MDR_POWER->PVDCS &= (uint32_t)~POWER_INV;
    }
}

/** @} */ /* End of group POWER_Exported_Functions */

/** @} */ /* End of group POWER */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_power.c */


