/**
  ******************************************************************************
  * @file    MDR32FxQI_bkp.c
  * @author  Milandr Application Team
  * @version V2.2.1i
  * @date    24/09/2024
  * @brief   This file contains all the BKP firmware functions.
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
#include "MDR32FxQI_bkp.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup BKP BKP
  * @{
  */

/** @defgroup BKP_Private_Defines BKP Private Defines
  * @{
  */

/* BKP registers bit address in the alias region */
#define BKP_OFFSET                  (MDR_BKP_BASE - PERIPH_BASE)

#define SFR_OFFSET(TP, MOD, SFR)    ((uint32_t)&((TP*)MOD)->SFR)
#define BB_ADDR(TP, MOD, SFR, BIT)  (PERIPH_BB_BASE + SFR_OFFSET(TP, MOD, SFR)*32 + BIT*4)
#define BKP_BB(SFR, BIT)            BB_ADDR(MDR_BKP_TypeDef, BKP_OFFSET, SFR, BIT)

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    #define BKP_JTAGA_BB            BKP_BB(REG_0E, BKP_REG_0E_JTAGA_Pos)
    #define BKP_JTAGB_BB            BKP_BB(REG_0E, BKP_REG_0E_JTAGB_Pos)
    #define BKP_FPOR_BB             BKP_BB(REG_0E, BKP_REG_0E_FPOR_Pos)
    #define BKP_STANDBY_BB          BKP_BB(REG_0F, BKP_REG_0F_STANDBY_Pos)
    #define RTC_ENABLE_BB           BKP_BB(REG_0F, BKP_REG_0F_RTC_EN_Pos)
    #define RTC_RESET_BB            BKP_BB(REG_0F, BKP_REG_0F_RTC_RESET_Pos)
    #define RTC_WEC_BB              BKP_BB(RTC_CS, BKP_RTC_CS_WEC_Pos)
#endif
/* --------------------- BKP registers bit mask ------------------------ */

/* BKP_REG0E register bit mask */
#define DUcc_Mask                   ((uint32_t)(BKP_REG_0E_LOW_Msk | BKP_REG_0E_SELECTRI_Msk))
#define DUccTrim_Mask               ((uint32_t)BKP_REG_0E_TRIM_Msk)

/* CFR register bit mask */
#define WDGTB_Mask                  ((uint32_t)0xFFFFFE7F)
#define WIN_Mask                    ((uint32_t)0x7F)
#define CTR_Mask                    ((uint32_t)0x7F)

/** @} */ /* End of group BKP_Private_Defines */


/** @defgroup BKP_Exported_Functions BKP Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the BKP peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void BKP_DeInit(void)
{
    #define BKP_RTC_CS_NUM  ((uint32_t)(SFR_OFFSET(MDR_BKP_TypeDef, BKP_OFFSET, RTC_CS) - BKP_OFFSET)/4)
    #define BKP_REG_0F_NUM  ((uint32_t)(SFR_OFFSET(MDR_BKP_TypeDef, BKP_OFFSET, REG_0F) - BKP_OFFSET)/4)
    #define BKP_REG_0E_NUM  ((uint32_t)(SFR_OFFSET(MDR_BKP_TypeDef, BKP_OFFSET, REG_0E) - BKP_OFFSET)/4)
    #define BKP_RTC_CNT_NUM ((uint32_t)(SFR_OFFSET(MDR_BKP_TypeDef, BKP_OFFSET, RTC_CNT) - BKP_OFFSET)/4)

    uint32_t * _bkp = (uint32_t *) MDR_BKP_BASE;
    uint32_t i;

    for (i = 0; i < BKP_REG_0E_NUM; i++)
    {
        _bkp[i] = 0;
    }

    for (i = BKP_RTC_CNT_NUM; i <= BKP_RTC_CS_NUM; i++)
    {
        _bkp[i] = 0;
    }

    MDR_BKP->REG_0E |= BKP_REG_0E_FPOR;
#if (defined(USE_MDR32F9xI) && defined(USE_K1986VE9xI)) || defined(USE_MDR32FG16S1QI)
    MDR_BKP->REG_0E &= ~(BKP_REG_0E_LOW_Msk | BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_TRIM_Msk);
#elif (defined(USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
    MDR_BKP->REG_0E &= ~(BKP_REG_0E_LOW_Msk | BKP_REG_0E_SELECTRI_Msk);
#elif (defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI))
    MDR_BKP->REG_0E &= ~(BKP_REG_0E_LOW_Msk | BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_ILIMEN);
#elif (defined(USE_K1986VE1xI) && defined(USE_MDR32F1QI))
    MDR_BKP->REG_0E &= ~(BKP_REG_0E_LOW_Msk | BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_TRIM_Msk | BKP_REG_0E_ILIMEN);
#endif

#if (defined(USE_K1986VE9xI) || defined(USE_K1986VE1xI)) && !(defined(USE_MDR32F9xI) || defined(USE_MDR32F1QI))
    MDR_BKP->REG_0F |= (uint32_t)(BKP_REG_0F_LSI_ON | BKP_REG_0F_HSI_ON);
#else
    MDR_BKP->REG_0F  = (uint32_t)(BKP_REG_0F_LSI_ON | (0x10 << BKP_REG_0F_LSI_TRIM_Pos) |
                                  BKP_REG_0F_HSI_ON | (0x20 << BKP_REG_0F_HSI_TRIM_Pos));
#endif
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Enables or disables the JTAG A.
  * @warning This function can be used only for MCU MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI
  * @param   NewState - @ref FunctionalState - new state of the JTAG A (ENABLE or DISABLE).
  * @retval  None
  */
void BKP_JTAGA_CMD(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *) BKP_JTAGA_BB = (uint32_t)NewState;
}


/**
  * @brief   Enables or disables the JTAG B.
  * @warning This function can be used only for MCU MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI
  * @param   NewState - @ref FunctionalState - new state of the JTAG B (ENABLE or DISABLE).
  * @retval  None
  */
void BKP_JTAGB_CMD(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *) BKP_JTAGB_BB = (uint32_t)NewState;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Configures the RTC clock source.
  * @param  RTC_CLK - @ref BKP_RTC_CLK - specifies the RTC clock source.
  * @retval None
  */
void BKP_RTCclkSource(BKP_RTC_CLK RTC_CLK)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_RTC_CLK_SOURCE(RTC_CLK));
    /* Clear BKP_REG0F[3:2] bits */
    tmpreg = MDR_BKP->REG_0F & (uint32_t) (~BKP_REG_0F_RTC_SEL_Msk );
    /* Set BKP_REG0F[3:2] bits according to RTC clock source*/
    tmpreg |= BKP_REG_0F_RTC_SEL_Msk & (uint32_t)RTC_CLK;
    MDR_BKP ->REG_0F = tmpreg;
}


/**
  * @brief  Enables or disables the RTC.
  * @param  NewState - @ref FunctionalState - new state of the RTC (ENABLE or DISABLE).
  * @retval None
  */
void BKP_RTC_WorkPermit(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) RTC_ENABLE_BB = (uint32_t)NewState;
#elif defined (USE_K1986VE1xI)
    if(NewState != DISABLE)
    {
        MDR_BKP->REG_0F |= BKP_REG_0F_RTC_EN;
    }
    else
    {
        MDR_BKP->REG_0F &= ~BKP_REG_0F_RTC_EN;
    }
#endif
}

/**
  * @brief  Configures the RTC clock source.
  * @param  RTC_Calibration: specifies the tact drop count.
  *         @arg 0 <= RTC_Calibration <= 255.
  * @retval None
  */
void BKP_RTC_Calibration(uint32_t RTC_Calibration)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_RTC_CALIBRATION(RTC_Calibration));

    /* Clear BKP_REG0F[12:5] bits */
    tmpreg = MDR_BKP -> REG_0F & (uint32_t) (~BKP_REG_0F_CAL_Msk);

    /* Set BKP_REG0F[12:5] bits according to RTC clock source*/
    tmpreg |= BKP_REG_0F_CAL_Msk & (RTC_Calibration << BKP_REG_0F_CAL_Pos);

    MDR_BKP -> REG_0F = tmpreg;
}


/**
  * @brief  Reset the RTC.
  * @param  NewState - @ref FunctionalState - new state of the RTC (ENABLE or DISABLE).
  * @retval None
  */
void BKP_RTC_Reset(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) RTC_RESET_BB = (uint32_t) NewState;
#elif defined (USE_K1986VE1xI)
    if(NewState != DISABLE)
    {
        MDR_BKP->REG_0F |= BKP_REG_0F_RTC_RESET;
    }
    else
    {
        MDR_BKP->REG_0F &= ~BKP_REG_0F_RTC_RESET;
    }
#endif
}

/**
  * @brief  Enables or disables the specified RTC interrupts.
  * @param  BKP_RTC_IT_Source: Specifies the RTC interrupts sources to be enabled or disabled.
  *         This parameter can be any combination of @ref BKP_RTC_IT values.
  * @param  NewState - @ref FunctionalState - new state of the specified RTC interrupts (ENABLE or DISABLE).
  * @retval None
  */
void BKP_RTC_ITConfig(uint32_t BKP_RTC_IT_Source, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_RTC_CLK_IT(BKP_RTC_IT_Source));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        MDR_BKP->RTC_CS |= BKP_RTC_IT_Source;
    }
    else
    {
        MDR_BKP->RTC_CS &= ~BKP_RTC_IT_Source;
    }
}

/**
  * @brief  Gets the RTC counter value.
  * @param  None
  * @retval RTC counter value.
  */
uint32_t BKP_RTC_GetCounter(void)
{
    return MDR_BKP->RTC_CNT;
}

/**
  * @brief  Sets the RTC counter value.
  * @param  CounterValue: RTC counter new value.
  * @retval None
  */
void BKP_RTC_SetCounter(uint32_t CounterValue)
{
    MDR_BKP->RTC_CNT = CounterValue;
}

/**
  * @brief  Sets the RTC alarm value.
  * @param  AlarmValue: RTC alarm new value.
  * @retval None
  */
void BKP_RTC_SetAlarm(uint32_t AlarmValue)
{
    MDR_BKP->RTC_ALRM = AlarmValue;
}

/**
  * @brief  Gets the RTC divider value.
  * @param  None
  * @retval RTC Divider value.
  */
uint32_t BKP_RTC_GetDivider(void)
{
  return MDR_BKP->RTC_DIV;
}

/**
  * @brief  Sets the RTC prescaler (RTC_DIV) preloaded value.
  * @param  PrescalerValue: RTC prescaler new value.
  *         This parameter should be less or equal then 0x000FFFFF
  * @retval None
  */
void BKP_RTC_SetPrescaler(uint32_t PrescalerValue)
{
  /* Check the parameter */
  assert_param(IS_BKP_PRESCALER_VALUE(PrescalerValue));

  MDR_BKP->RTC_PRL = PrescalerValue;
}

/**
  * @brief  Waits until last write operation on RTC registers has finished.
  * @note   This function must be called before any write to RTC registers.
  * @param  None
  * @retval None
  */
void BKP_RTC_WaitForUpdate(void)
{
    /* Loop until WEC flag is set */
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    while (*(__IO uint32_t *) RTC_WEC_BB != 0);
#elif defined (USE_K1986VE1xI)
    while((MDR_BKP->RTC_CS & BKP_RTC_CS_WEC) == BKP_RTC_CS_WEC);
#endif

}

/**
  * @brief  Checks whether the specified RTC flag is set or not.
  * @param  BKP_RTC_FLAG - @ref BKP_RTC_FLAGS - specifies the flag to check.
  * @retval @ref FlagStatus - Current state of BKP_RTC_FLAG (SET or RESET).
  */
FlagStatus BKP_RTC_GetFlagStatus(BKP_RTC_FLAGS BKP_RTC_FLAG)
{
    FlagStatus status;

    /* Check the parameters */
    assert_param(IS_BKP_RTC_GET_FLAG(BKP_RTC_FLAG));

    if ((MDR_BKP->RTC_CS & BKP_RTC_FLAG) != 0)
    {
        status = SET;
    }
    else
    {
        status = RESET;
    }

    return status;
}

/**
  * @brief  Clears the specified RTC flag(s).
  * @param  BKP_RTC_FLAG: specifies the flag to clear.
  *         This parameter can be any combination of @ref BKP_RTC_FLAGS values.
  * @retval None.
  */
void BKP_RTC_ClearFlagStatus(uint32_t BKP_RTC_FLAG)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_BKP_RTC_CLEAR_FLAG(BKP_RTC_FLAG));

    tmpreg = MDR_BKP->RTC_CS & (~BKP_RTC_CS_Msk);
    tmpreg |= BKP_RTC_FLAG;

    MDR_BKP->RTC_CS = tmpreg;
}

/**
  * @brief  Select the internal voltage regulator mode
  * @param  DUccMode - @ref BKP_DUcc_Mode - specifies the DUcc mode.
  * @retval None
  */
void BKP_DUccMode(BKP_DUcc_Mode DUccMode)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_BKP_DUCC_MODE(DUccMode));

    /* Clear BKP_REG0E[5:0] bits */
    tmpreg  = MDR_BKP -> REG_0E & (uint32_t) (~DUcc_Mask);

    /* Set BKP_REG0E[5:0] bits according to DUcc mode */
    tmpreg |= DUcc_Mask & DUccMode;

    MDR_BKP -> REG_0E = tmpreg;
}

/**
  * @brief  Set the internal voltage regulator trim
  * @param  DUccTrim - @ref BKP_DUcc_Trim - specifies the DUcc trim.
  * @retval None
  */
void BKP_DUccTrim(BKP_DUcc_Trim DUccTrim)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_BKP_DUCC_TRIM(DUccTrim));

    /* Clear BKP_REG0E[5:0] bits */
    tmpreg  = MDR_BKP -> REG_0E & (uint32_t) (~DUccTrim_Mask);

    /* Set BKP_REG0E[5:0] bits according to DUcc mode */
    tmpreg |= DUccTrim_Mask & (uint32_t)DUccTrim;

    MDR_BKP -> REG_0E = tmpreg;
}

/**
  * @brief  Set power on reset flag (FPOR).
  * @param  None
  * @retval None
  */
void BKP_SetFlagPOR(void)
{
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) BKP_FPOR_BB = (uint32_t) 0x01;
#elif defined (USE_K1986VE1xI)
    MDR_BKP->REG_0E |= BKP_REG_0E_FPOR;
#endif
}

/**
  * @brief  Power on reset flag (FPOR) status
  * @param  None
  * @retval @ref ErrorStatus - SUCCESS if FPOR is zero, else ERROR
  */
ErrorStatus BKP_FlagPORstatus(void)
{
    ErrorStatus state = ERROR;
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    if (*(__IO uint32_t *) BKP_FPOR_BB == 0)
    {
        state = SUCCESS;
    }
#elif defined (USE_K1986VE1xI)
    if (( MDR_BKP->REG_0E & BKP_REG_0E_FPOR ) == BKP_REG_0E_FPOR)
    {
        state = SUCCESS;
    }
#endif
    return state;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Enters STOP mode.
  * @warning This function can be used only for MCU MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI
  * @param   BKP_Regulator_state - @ref FunctionalState - specifies the regulator state in STOP mode.
  *              @arg ENABLE:  STOP mode with regulator ON
  *              @arg DISABLE: STOP mode with regulator in low power mode
  * @param   BKP_STOPentry - @ref BKP_Stop_Entry_Mode - specifies if
  *          STOP mode in entered with WFI or WFE instruction.
  * @retval  None
  */
void BKP_EnterSTOPMode(FunctionalState BKP_Regulator_state, BKP_Stop_Entry_Mode BKP_STOPentry)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(BKP_Regulator_state));
    assert_param(IS_BKP_STOP_ENTRY(BKP_STOPentry));

    if(BKP_Regulator_state == DISABLE)
    {
        MDR_BKP->REG_0F |= BKP_REG_0F_STANDBY;
    }
    else
    {
        /* Set SLEEPDEEP bit of System Control Register */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

        /* Select STOP mode entry */
        if(BKP_STOPentry == BKP_STOPentry_WFE)
        {
            /* Request Wait For Event */
            __WFE();
        }
        else
        {
            /* Request Wait For Interrupt */
            __WFI();
        }
    }
}

#elif defined (USE_K1986VE1xI)
/**
  * @brief   Shifting core controller into a low power consumption. In this mode
  *          the clock frequency is applied only to the selected peripheral
  *          blocks, which interrupt the supply resumes clock on the core.
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI
  * @param   None
  * @retval  None
  */
void BKP_EnterSLEEPMode(void)
{
    /* Enter in SLEEP mode */
    MDR_RST_CLK->ETH_CLOCK |= RST_CLK_ETH_CLOCK_SLEEP;
    __DSB();
}

/**
  * @brief   Adjustment coefficient of the reference voltage
  *          integrated voltage regulator DUcc roughly.
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI
  * @param   trim - @ref BKP_TRIM - coefficient of the reference voltage.
  * @return  None.
  */
void BKP_SetTrim(BKP_TRIM trim)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_BKP_TRIM(trim));

    tmpreg = MDR_BKP->REG_0E & (~(3 << BKP_REG_0E_TRIM_34_Pos));
    tmpreg = tmpreg | (uint32_t)trim;

    MDR_BKP->REG_0E = tmpreg;
}

/**
  * @brief   Controls current limit protection 150mA
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI
  * @param   NewState - @ref FunctionalState - new state of ilimen bit (ENABLE or DISABLE).
  * @return  None.
  */
void BKP_CurrentLimitProtection150mA(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState == DISABLE)
    {
        MDR_BKP->REG_0E &= ~(1 << BKP_REG_0E_ILIMEN_Pos);
    }
    else
    {
        MDR_BKP->REG_0E |= (1 << BKP_REG_0E_ILIMEN_Pos);
    }
}

/**
  * @brief   Enter StandAlone mode
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI
  * @param   None.
  * @return  None.
  */
void BKP_EnterStandAloneMode(void)
{
    MDR_BKP->REG_0E |= (1 << BKP_REG_0E_SANDALONE_Pos);
}
#endif /* #if defined (USE_K1986VE1xI) */

/**
  * @brief   Enters STANDBY mode.
  * @warning For MDR32F1QI, K1986VE1xI: Recommended not to use.
  * @param   None.
  * @retval  None.
  */
void BKP_EnterSTANDBYMode(void)
{
    /* Select STANDBY mode */
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) BKP_STANDBY_BB = (uint32_t) 0x01;
#elif defined (USE_K1986VE1xI)
    MDR_BKP->REG_0F |= BKP_REG_0F_STANDBY;
#endif
}

/** @} */ /* End of group BKP_Exported_Functions */

/** @} */ /* End of group BKP */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_bkp.c */

