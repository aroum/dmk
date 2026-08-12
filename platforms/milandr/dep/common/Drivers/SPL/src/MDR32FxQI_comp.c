/**
  ******************************************************************************
  * @file    MDR32FxQI_comp.c
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    23/07/2024
  * @brief   This file contains all the COMP firmware functions.
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
#include "MDR32FxQI_comp.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/** @defgroup COMP COMP
  * @warning This module can be used only for MCU MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @{
  */

/** @defgroup COMP_Exported_Functions COMP Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the COMP peripheral registers to their default reset values.
  * @param  None.
  * @retval None.
  */
void COMP_DeInit(void)
{
    MDR_COMP->CFG = 0;
    MDR_COMP->RESULT_LATCH;
}

/**
  * @brief  Initializes the COMP peripheral according to
  *         the specified parameters in the COMP_InitStruct.
  * @param  COMP_InitStruct: pointer to a @ref COMP_InitTypeDef structure that
  *         contains the configuration information for the specified COMP peripheral.
  * @retval None
  */
void COMP_Init(const COMP_InitTypeDef* COMP_InitStruct)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_COMP_PLUS_INPUT_CONFIG(COMP_InitStruct->COMP_PlusInputSource));
    assert_param(IS_COMP_MINUS_INPUT_CONFIG(COMP_InitStruct->COMP_MinusInputSource));
    assert_param(IS_COMP_OUT_INV_CONFIG(COMP_InitStruct->COMP_OutInversion));

    tmpreg_CFG = MDR_COMP->CFG;
    tmpreg_CFG &= ~(COMP_CFG_CREF | COMP_CFG_CCH_Msk | COMP_CFG_INV);
    tmpreg_CFG += COMP_InitStruct->COMP_PlusInputSource
                + COMP_InitStruct->COMP_MinusInputSource
                + COMP_InitStruct->COMP_OutInversion;

    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Fills each COMP_InitStruct member with its default value.
  * @param  COMP_InitStruct: pointer to a @ref COMP_InitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void COMP_StructInit(COMP_InitTypeDef* COMP_InitStruct)
{
    COMP_InitStruct->COMP_PlusInputSource  = COMP_PlusInput_IN1;
    COMP_InitStruct->COMP_MinusInputSource = COMP_MinusInput_IN2;
    COMP_InitStruct->COMP_OutInversion     = COMP_OUT_INV_Disable;
}

/**
  * @brief  Enables or disables the COMP peripheral.
  * @param  NewState - @ref FunctionalState - new state of the COMP peripheral.
  * @retval None
  */
void COMP_Cmd(FunctionalState NewState)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CFG = MDR_COMP->CFG;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable COMP by setting the COMP_CFG_ON bit in the COMP_CFG register */
        tmpreg_CFG |= COMP_CFG_ON;
    }
    else
    {
        /* Disable COMP by resetting the COMP_CFG_ON bit in the COMP_CFG register */
        tmpreg_CFG &= ~COMP_CFG_ON;
    }

    /* Configure COMP_CFG register with new value */
    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Checks whether the specified COMP Status flag is set or not.
  * @param  Flag - @ref COMP_CFG_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - Current Status flag state (SET or RESET).
  */
FlagStatus COMP_GetCfgFlagStatus(COMP_CFG_Flags Flag)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_COMP_CFG_FLAG(Flag));

    if ((MDR_COMP->CFG & Flag) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * @brief  Configures non-inverting comparator input.
  * @param  Source - @ref COMP_Plus_Inp_Src - specifies the non-inverting comparator input source.
  * @retval None.
  */
void COMP_PInpSourceConfig(COMP_Plus_Inp_Src Source)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_COMP_PLUS_INPUT_CONFIG(Source));

    tmpreg_CFG = MDR_COMP->CFG;
    tmpreg_CFG &= ~COMP_CFG_CREF;
    tmpreg_CFG += Source;

    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Configures inverting comparator input.
  * @param  Source - @ref COMP_Minus_Inp_Src - specifies the inverting comparator input source.
  * @retval None.
  */
void COMP_MInpSourceConfig(COMP_Minus_Inp_Src Source)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_COMP_MINUS_INPUT_CONFIG(Source));

    tmpreg_CFG = MDR_COMP->CFG;
    tmpreg_CFG &= ~COMP_CFG_CCH_Msk;
    tmpreg_CFG += Source;

    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Configures comparator output inversion.
  * @param  Inversion - @ref COMP_Out_Inv_State - enables or disables inversion on the comparator output.
  * @retval None.
  */
void COMP_OutInversionConfig(COMP_Out_Inv_State Inversion)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_COMP_OUT_INV_CONFIG(Inversion));

    tmpreg_CFG = MDR_COMP->CFG;
    tmpreg_CFG &= ~COMP_CFG_INV;
    tmpreg_CFG += Inversion;

    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Initializes the COMP reference voltage source according to
  *         the specified parameters in the COMP_CVRefInitStruct.
  * @param  COMP_CVRefInitStruct: pointer to a @ref COMP_CVRefInitTypeDef structure
  *         that contains the configuration information for the COMP reference voltage source.
  * @retval None
  */
void COMP_CVRefInit(const COMP_CVRefInitTypeDef* COMP_CVRefInitStruct)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_COMP_CVREF_SOURCE_CONFIG(COMP_CVRefInitStruct->COMP_CVRefSource));
    assert_param(IS_COMP_CVREF_SCALE_CONFIG(COMP_CVRefInitStruct->COMP_CVRefScale));

    tmpreg_CFG = MDR_COMP->CFG;
    tmpreg_CFG &= ~(COMP_CFG_CVRSS | COMP_CFG_CVRR | COMP_CFG_CVR_Msk);

    tmpreg_CFG += COMP_CVRefInitStruct->COMP_CVRefSource
                + COMP_CVRefInitStruct->COMP_CVRefScale;

    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Fills each COMP_CVRefInitStruct member with its default value.
  * @param  COMP_CVRefInitStruct: pointer to a @ref COMP_CVRefInitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void COMP_CVRefStructInit(COMP_CVRefInitTypeDef* COMP_CVRefInitStruct)
{
    COMP_CVRefInitStruct->COMP_CVRefSource = COMP_CVREF_SOURCE_AVdd;
    COMP_CVRefInitStruct->COMP_CVRefScale  = COMP_CVREF_SCALE_0_div_24;
}

/**
  * @brief  Enables or disables the COMP reference voltage source.
  * @param  NewState - @ref FunctionalState - new state of the COMP reference voltage source.
  * @retval None
  */
void COMP_CVRefCmd(FunctionalState NewState)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CFG = MDR_COMP->CFG;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable COMP reference by setting the COMP_CFG_CVREN bit in the COMP_CFG register */
        tmpreg_CFG |= COMP_CFG_CVREN;
    }
    else
    {
        /* Disable COMP reference by resetting the COMP_CFG_CVREN bit in the COMP_CFG register */
        tmpreg_CFG &= ~COMP_CFG_CVREN;
    }

    /* Configure COMP_CFG register with new value */
    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Configures the COMP reference voltage source.
  * @param  Source - @ref COMP_CVRef_Src - specifies the COMP reference voltage source.
  * @retval None.
  */
void COMP_CVRefSourceConfig(COMP_CVRef_Src Source)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_COMP_CVREF_SOURCE_CONFIG(Source));

    tmpreg_CFG = MDR_COMP->CFG;
    tmpreg_CFG &= ~COMP_CFG_CVRSS;
    tmpreg_CFG += Source;

    MDR_COMP->CFG = tmpreg_CFG;
}


/**
  * @brief  Configures the COMP reference voltage scale.
  * @param  Scale - @ref COMP_CVRef_Scale - specifies the COMP reference voltage scale.
  * @retval None.
  */
void COMP_CVRefScaleConfig(COMP_CVRef_Scale Scale)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_COMP_CVREF_SCALE_CONFIG(Scale));

    tmpreg_CFG = MDR_COMP->CFG;
    tmpreg_CFG &= ~(COMP_CFG_CVR_Msk | COMP_CFG_CVRR);
    tmpreg_CFG += Scale;

    MDR_COMP->CFG = tmpreg_CFG;
}

/**
  * @brief  Returns and clears the COMP Result Latch register value.
  * @param  None.
  * @retval The Result Latch register value.
  */
uint32_t COMP_GetResultLatch(void)
{
    return MDR_COMP->RESULT_LATCH;
}

/**
  * @brief  Returns the COMP Result register value.
  * @param  None.
  * @retval The Result register value.
  */
uint32_t COMP_GetStatus(void)
{
    return MDR_COMP->RESULT;
}

/**
  * @brief  Checks whether the specified COMP Status flag is set or not.
  * @param  Flag - @ref COMP_STATUS_Flags - specifies the flag to check
  * @retval @ref FlagStatus - Current Status flag state (SET or RESET).
  */
FlagStatus COMP_GetFlagStatus(COMP_STATUS_Flags Flag)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_COMP_STATUS_FLAG(Flag));

    if ((COMP_GetStatus() & Flag) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * @brief  Enables or disables the COMP interrupt.
  * @param  NewState - @ref FunctionalState - new state of the COMP interrupt.
  * @retval None
  */
void COMP_ITConfig(FunctionalState NewState)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CFG = MDR_COMP->CFG;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the COMP Interrupt requests by setting bits in the CFG registers */
        tmpreg_CFG |= COMP_CFG_CMPIE;
    }
    else
    {
        /* Disable the COMP Interrupt requests by clearing bits in the CFG registers */
        tmpreg_CFG &= ~COMP_CFG_CMPIE;
    }

    /* Configure CFG registers with new value */
    MDR_COMP->CFG = tmpreg_CFG;
}

/** @} */ /* End of group COMP_Exported_Functions */

/** @} */ /* End of group COMP */
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_comp.c */


