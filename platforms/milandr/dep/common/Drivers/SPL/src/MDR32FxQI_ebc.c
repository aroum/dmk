/**
  ******************************************************************************
  * @file    MDR32FxQI_ebc.c
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    24/07/2024
  * @brief   This file contains all the ADC firmware functions.
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
#include "MDR32FxQI_ebc.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup EBC EBC
  * @{
  */

/** @defgroup EBC_Private_Defines EBC Private Defines
  * @{
  */

#define WAIT_STATES_COEF        1000000
#define WAIT_STATES_MIN         3
#define WAIT_STATES_MAX         18

#define NAND_SYCLES_COEF        1000000
#define NAND_SYCLES_MAX         15

/** @} */ /* End of group EBC_Private_Defines */


/** @defgroup EBC_Exported_Functions EBC Exported Functions
  * @{
  */

/**
  * @brief  Resets the EBC peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void EBC_DeInit(void)
{
    MDR_EBC->CONTROL     = 0;
    MDR_EBC->NAND_CYCLES = 0;

#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
    MDR_EBC->MEM_REGION[0] = 0;
    MDR_EBC->MEM_REGION[1] = 0;
    MDR_EBC->MEM_REGION[2] = 0;
    MDR_EBC->MEM_REGION[3] = 0;
#endif
}

/**
  * @brief  Initializes the EBC peripheral according to
  *         the specified parameters in the EBC_InitStruct.
  * @param  EBC_InitStruct: pointer to a @ref EBC_InitTypeDef structure that
  *         contains the configuration information for the specified EBC peripheral.
  * @retval None
  */
void EBC_Init(const EBC_InitTypeDef* EBC_InitStruct)
{
    uint32_t tmpreg_CONTROL;
    uint32_t tmpreg_CYCLES;

    /* Check the parameters */
    assert_param(IS_EBC_MODE(EBC_InitStruct->EBC_Mode));
    assert_param(IS_EBC_CPOL(EBC_InitStruct->EBC_Cpol));
    assert_param(IS_EBC_WAIT_STATE(EBC_InitStruct->EBC_WaitState));
    assert_param(IS_EBC_NAND_CYCLES(EBC_InitStruct->EBC_NandTrc));
    assert_param(IS_EBC_NAND_CYCLES(EBC_InitStruct->EBC_NandTwc));
    assert_param(IS_EBC_NAND_CYCLES(EBC_InitStruct->EBC_NandTrea));
    assert_param(IS_EBC_NAND_CYCLES(EBC_InitStruct->EBC_NandTwp));
    assert_param(IS_EBC_NAND_CYCLES(EBC_InitStruct->EBC_NandTwhr));
    assert_param(IS_EBC_NAND_CYCLES(EBC_InitStruct->EBC_NandTalea));
    assert_param(IS_EBC_NAND_CYCLES(EBC_InitStruct->EBC_NandTrr));

    /* Form new value for the EBC_CONTROL register */
    tmpreg_CONTROL = (EBC_InitStruct->EBC_Mode)
                   | (EBC_InitStruct->EBC_Cpol)
                   | (EBC_InitStruct->EBC_WaitState << EBC_CONTROL_WAIT_STATE_Pos);

#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
    /* Using the alignment data by LSB EBC */
    tmpreg_CONTROL |= EBC_InitStruct->EBC_DataAlignment;
#endif

    /* Form new value for the EBC_NAND_CYCLES register */
    tmpreg_CYCLES  = (EBC_InitStruct->EBC_NandTrc   << EBC_NAND_CYCLES_TRC_Pos)
                   | (EBC_InitStruct->EBC_NandTwc   << EBC_NAND_CYCLES_TWC_Pos)
                   | (EBC_InitStruct->EBC_NandTrea  << EBC_NAND_CYCLES_TREA_Pos)
                   | (EBC_InitStruct->EBC_NandTwp   << EBC_NAND_CYCLES_TWP_Pos)
                   | (EBC_InitStruct->EBC_NandTwhr  << EBC_NAND_CYCLES_TWHR_Pos)
                   | (EBC_InitStruct->EBC_NandTalea << EBC_NAND_CYCLES_TALEA_Pos)
                   | (EBC_InitStruct->EBC_NandTrr   << EBC_NAND_CYCLES_TRR_Pos);

    /* Configure EBC registers with new values */
    MDR_EBC->NAND_CYCLES = tmpreg_CYCLES;
    MDR_EBC->CONTROL = tmpreg_CONTROL;
}

/**
  * @brief  Fills each EBC_InitStruct member with its default value.
  * @param  EBC_InitStruct: pointer to a @ref EBC_InitTypeDef structure 
  *         which will be initialized.
  * @retval None
  */
void EBC_StructInit(EBC_InitTypeDef* EBC_InitStruct)
{
    /* Reset EBC initialization structure parameters values */
    EBC_InitStruct->EBC_Mode          = EBC_MODE_OFF;
    EBC_InitStruct->EBC_Cpol          = EBC_CPOL_POSITIVE;
    EBC_InitStruct->EBC_WaitState     = EBC_WAIT_STATE_3HCLK;
    EBC_InitStruct->EBC_NandTrc       = EBC_NAND_CYCLES_0HCLK;
    EBC_InitStruct->EBC_NandTwc       = EBC_NAND_CYCLES_0HCLK;
    EBC_InitStruct->EBC_NandTrea      = EBC_NAND_CYCLES_0HCLK;
    EBC_InitStruct->EBC_NandTwp       = EBC_NAND_CYCLES_0HCLK;
    EBC_InitStruct->EBC_NandTwhr      = EBC_NAND_CYCLES_0HCLK;
    EBC_InitStruct->EBC_NandTalea     = EBC_NAND_CYCLES_0HCLK;
    EBC_InitStruct->EBC_NandTrr       = EBC_NAND_CYCLES_0HCLK;
#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
    EBC_InitStruct->EBC_DataAlignment = EBC_EBC_DATA_ALIGNMENT_OFF;
    EBC_InitStruct->EBC_UseMemRegion  = EBC_USE_MEM_REGION_NONE;
#endif
}

/**
  * @brief  Calculates the Wait States number
  *         for selected HCLK frequency and time interval.
  * @param  HCLK_Frequency_KHz: specifies the HCLK frequency.
  * @param  Time_ns: specifies the time interval.
  * @retval The Wait States number in range 0..15
  *         or 0xFFFFFFFF if result is out of range 0..15.
  */
uint32_t EBC_CalcWaitStates(uint32_t HCLK_Frequency_KHz, uint32_t Time_ns)
{
    uint32_t Cycles;

    if (HCLK_Frequency_KHz == 0)
    {
        Cycles = 0;
    }
    else if (Time_ns > WAIT_STATES_MAX * WAIT_STATES_COEF / HCLK_Frequency_KHz)
    {
        Cycles = 0xFFFFFFFF;
    }
    else
    {
        Cycles = (HCLK_Frequency_KHz * Time_ns + WAIT_STATES_COEF - 1) / WAIT_STATES_COEF;
        if (Cycles > WAIT_STATES_MAX)
        {
            Cycles = 0xFFFFFFFF;
        }
        else if (Cycles >= WAIT_STATES_MIN)
        {
            Cycles -= WAIT_STATES_MIN;
        }
        else
        {
            Cycles = 0;
        }
    }

    return Cycles;
}

/**
  * @brief  Calculates the NAND Sycles number
  *         for selected HCLK frequency and time interval.
  * @param  HCLK_Frequency_KHz: specifies the HCLK frequency.
  * @param  Time_ns: specifies the time interval.
  * @retval The NAND Sycles number in range 0..15
  *         or 0xFFFFFFFF if result is out of range 0..15.
  */
uint32_t EBC_CalcNandCycles(uint32_t HCLK_Frequency_KHz, uint32_t Time_ns)
{
    uint32_t Cycles;

    if (HCLK_Frequency_KHz == 0)
    {
        Cycles = 0;
    }
    else if (Time_ns > NAND_SYCLES_MAX * NAND_SYCLES_COEF / HCLK_Frequency_KHz)
    {
        Cycles = 0xFFFFFFFF;
    }
    else
    {
        Cycles = (HCLK_Frequency_KHz * Time_ns + NAND_SYCLES_COEF - 1) / NAND_SYCLES_COEF;
        if (Cycles > NAND_SYCLES_MAX)
        {
            Cycles = 0xFFFFFFFF;
        }
    }

    return Cycles;
}

/**
  * @brief  Returns the BUSY status of the NAND Flash.
  * @param  None.
  * @retval @ref FlagStatus - The NAND Flash BUSY status
  *         (SET if NandFlashReady or RESET if NandFlashBusy).
  */
FlagStatus EBC_GetBusyStatus(void)
{
    FlagStatus tmpreg_BUSY_STS;

    if ((MDR_EBC->CONTROL & EBC_CONTROL_BUSY) == 0)
    {
        tmpreg_BUSY_STS = RESET;
    }
    else
    {
        tmpreg_BUSY_STS = SET;
    }

    return tmpreg_BUSY_STS;
}

#if defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI))
/**
  * @brief   Fills each EBC_MemRegionInitStruct member with its default value.
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI and K1986VE9xI
  * @param   EBC_MemRegionInitStruct: pointer to a @ref EBC_MemRegionInitTypeDef
  *          structure which will be initialized.
  * @retval  None
  */
void EBC_MemRegionStructInit(EBC_MemRegionInitTypeDef * EBC_MemRegionInitStruct)
{
    /* Reset EBC initialization structure parameters values */
    EBC_MemRegionInitStruct->WS_Active   = 0;
    EBC_MemRegionInitStruct->WS_Setup    = EBC_WS_SETUP_CYCLE_1HCLK;
    EBC_MemRegionInitStruct->WS_Hold     = EBC_WS_HOLD_CYCLE_1HCLK;
    EBC_MemRegionInitStruct->Use_Ready   = DISABLE;
    EBC_MemRegionInitStruct->Enable_Tune = DISABLE;
}

/**
  * @brief   Initializes the EBC Memory Region according to the specified
  *          parameters in the EBC_MemRegionInitStruct.
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI and K1986VE9xI
  * @param   EBC_MemRegionInitStruct: pointer to a @ref EBC_MemRegionInitTypeDef structure that
  *          contains the configuration information for the specified EBC Memory Region.
  * @param   EBC_MEM_REGIONx - @ref EBC_Mem_Region_Select - Select the specified memory region.
  * @retval  None
  */
void EBC_MemRegionInit(EBC_MemRegionInitTypeDef* EBC_MemRegionInitStruct, EBC_Mem_Region_Select EBC_MEM_REGIONx)
{
    uint32_t tmpreg_Mem_Region;

    /* Check the parameters */
    assert_param(IS_EBC_MEM_REGION(EBC_MEM_REGIONx));
    assert_param(IS_EBC_WS_SETUP_CYCLES(EBC_MemRegionInitStruct->WS_Setup));
    assert_param(IS_EBC_WS_HOLD_CYCLES(EBC_MemRegionInitStruct->WS_Hold));
    assert_param(IS_EBC_WS_ACTIVE(EBC_MemRegionInitStruct->WS_Active));

    /* Form new value for the RAM_CYCLESx register */
    tmpreg_Mem_Region = (EBC_MemRegionInitStruct->Use_Ready   << EBC_RAM_CYCLES_USE_READY_Pos)
                      | (EBC_MemRegionInitStruct->WS_Hold     << EBC_RAM_CYCLES_WS_HOLD_Pos)
                      | (EBC_MemRegionInitStruct->WS_Setup    << EBC_RAM_CYCLES_WS_SETUP_Pos)
                      | (EBC_MemRegionInitStruct->WS_Active   << EBC_RAM_CYCLES_WS_ACTIVE_Pos)
                      | (EBC_MemRegionInitStruct->Enable_Tune << EBC_RAM_CYCLES_ENABLE_TUNE_Pos);

    /* Configure EBC registers with new values */
    MDR_EBC->MEM_REGION[EBC_MEM_REGIONx] = tmpreg_Mem_Region;
}

/**
  * @brief   Enables or disables the specified EBC Memory Region settigs.
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI and K1986VE9xI
  * @param   EBC_MEM_REGIONx - @ref EBC_Mem_Region_Select - Select the specified memory region.
  * @param   NewState - @ref FunctionalState - new state of the EBC Memory Region settings.
  * @retval  None
  */
void EBC_MemRegionCMD(EBC_Mem_Region_Select EBC_MEM_REGIONx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_EBC_MEM_REGION(EBC_MEM_REGIONx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        /* Enable the selected EBC Memory Region settings */
        MDR_EBC->MEM_REGION[EBC_MEM_REGIONx] |= EBC_RAM_CYCLES_ENABLE_TUNE;
    }
    else
    {
        /* Disable the selected EBC Memory Region settings */
        MDR_EBC->MEM_REGION[EBC_MEM_REGIONx] &= ~EBC_RAM_CYCLES_ENABLE_TUNE;
    }
}

#endif /* defined (USE_K1986VE1xI) || (defined (USE_K1986VE9xI) && !defined(USE_MDR32F9xI)) */

/** @} */ /* End of group EBC_Exported_Functions */

/** @} */ /* End of group EBC */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_ebc.c */

