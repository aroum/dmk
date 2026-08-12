/**
  ******************************************************************************
  * @file    MDR32FxQI_audio.c
  * @author  Milandr Application Team
  * @version V2.1.0i
  * @date    29/06/2023
  * @brief   This file contains all the AUDIO_IP firmware functions.
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
#include "MDR32FxQI_audio.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
/** @defgroup AUDIO_IP AUDIO_IP
  * @warning This module can be used only for MCU MDR32FG16S1QI.
  * @{
  */

/** @defgroup AUDIO_IP_Exported_Functions AUDIO_IP Exported Functions
  * @{
  */

/**
  * @brief  Reset the AUDIO_IP registers to their default reset values.
  * @param  None.
  * @retval None.
  */
void AUDIO_IP_DeInit(void)
{
    MDR_AUDIO_IP->POWCTL  = 0x000001C0;
    MDR_AUDIO_IP->ADCCTL  = 0x0000002A;
    MDR_AUDIO_IP->DACCTL  = 0x0000002A;
    MDR_AUDIO_IP->MASKCTL = 0x0000000F;
    MDR_AUDIO_IP->IRQFLAG = 0x00000000;
}

/**
  * @brief  Fills each AUDIO_IP_InitStuct member with its default value.
  * @param  AUDIO_IP_InitStuct: pointer to a @ref AUDIO_IP_InitTypeDef structure
  *         which will be initialized.
  * @retval None.
  */
void AUDIO_IP_StructInit(AUDIO_IP_InitTypeDef* AUDIO_IP_InitStruct)
{
    /* General codec control */
    AUDIO_IP_InitStruct->FilterMode                 = AUDIO_IP_FILTER_MODE_FIR_FILTER;
    AUDIO_IP_InitStruct->AnalogLoopBackMode         = DISABLE;
    AUDIO_IP_InitStruct->DigitalLoopBackMode        = DISABLE;

    /* ADC control */
    AUDIO_IP_InitStruct->ADGain                     = AUDIO_IP_ADGain_0dB;
    AUDIO_IP_InitStruct->ADCInputAmplifier          = AUDIO_IP_ADC_INPUT_AMPLIFIER_LEVEL_0_DB;
    AUDIO_IP_InitStruct->ADCAnalogInput             = AUDIO_IP_ANALOG_INPUT_SOURCE_INP1;
    AUDIO_IP_InitStruct->ADCInputControl            = DISABLE;

    /* DAC control */
    AUDIO_IP_InitStruct->DAGain                     = AUDIO_IP_DAGain_0dB;
    AUDIO_IP_InitStruct->DACMute                    = DISABLE;
    AUDIO_IP_InitStruct->DACOutputAmplifier         = DISABLE;
    AUDIO_IP_InitStruct->DACBIASOffsetSchemeState   = DISABLE;
    AUDIO_IP_InitStruct->DACAnalogState             = DISABLE;
    AUDIO_IP_InitStruct->DACOverCurrentBufferDet    = DISABLE;
    AUDIO_IP_InitStruct->DACOverCurrentNetOffsetDet = DISABLE;
    AUDIO_IP_InitStruct->DACSideToneLoopBack        = AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK_3dB;
    AUDIO_IP_InitStruct->DACState                   = DISABLE;
}

/**
  * @brief  Initializes the AUDIO_IP peripheral according to the specified
  *         parameters in the AUDIO_IP_InitStuct.
  * @param  AUDIO_IP_InitStuct: pointer to a @ref AUDIO_IP_InitTypeDef structure
  *         which will be initialized.
  * @retval None.
  */
void AUDIO_IP_Init(AUDIO_IP_InitTypeDef* AUDIO_IP_InitStruct)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_AUDIO_IP_FILTER_MODE(AUDIO_IP_InitStruct->FilterMode));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->AnalogLoopBackMode));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DigitalLoopBackMode));
    assert_param(IS_AUDIO_IP_AD_GAIN(AUDIO_IP_InitStruct->ADGain));
    assert_param(IS_AUDIO_IP_INPUT_AMPLIFIER_LEVEL(AUDIO_IP_InitStruct->ADCInputAmplifier));
    assert_param(IS_AUDIO_IP_DA_GAIN(AUDIO_IP_InitStruct->DAGain));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DACMute));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DACOutputAmplifier));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DACBIASOffsetSchemeState));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DACAnalogState));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DACOverCurrentBufferDet));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DACOverCurrentNetOffsetDet));
    assert_param(IS_AUDIO_IP_DAC_SIDE_TONE_LOOP_BACK(AUDIO_IP_InitStruct->DACSideToneLoopBack));
    assert_param(IS_FUNCTIONAL_STATE(AUDIO_IP_InitStruct->DACState));

    /* Configure POWCTL register */
    tmpreg = (AUDIO_IP_InitStruct->FilterMode          << AUDIO_IP_POWCTL_IIREN_Pos) |
             (AUDIO_IP_InitStruct->AnalogLoopBackMode  << AUDIO_IP_POWCTL_ALB_Pos  ) |
             (AUDIO_IP_InitStruct->DigitalLoopBackMode << AUDIO_IP_POWCTL_DLB_Pos  );

    MDR_AUDIO_IP->POWCTL = tmpreg;

    /* Configure ADCCTL register */
    tmpreg = (AUDIO_IP_InitStruct->ADGain)                                          |
             (AUDIO_IP_InitStruct->ADCInputAmplifier << AUDIO_IP_ADCCTL_INBG_Pos)   |
             (AUDIO_IP_InitStruct->ADCAnalogInput    << AUDIO_IP_ADCCTL_AINSEL_Pos) |
             (AUDIO_IP_InitStruct->ADCInputControl   << AUDIO_IP_ADCCTL_ICONT_Pos);

    MDR_AUDIO_IP->ADCCTL = tmpreg;

    /* Configure DACCTL register */
    tmpreg = (AUDIO_IP_InitStruct->DAGain)                                                    |
             (AUDIO_IP_InitStruct->DACMute                    << AUDIO_IP_DACCTL_MUTE1_Pos)   |
             (AUDIO_IP_InitStruct->DACOutputAmplifier         << AUDIO_IP_DACCTL_ODAMP_Pos)   |
             (AUDIO_IP_InitStruct->DACBIASOffsetSchemeState   << AUDIO_IP_DACCTL_ODBIAS_Pos)  |
             (AUDIO_IP_InitStruct->DACAnalogState             << AUDIO_IP_DACCTL_ODDAC_Pos)   |
             (AUDIO_IP_InitStruct->DACOverCurrentBufferDet    << AUDIO_IP_DACCTL_OVECBA_Pos)  |
             (AUDIO_IP_InitStruct->DACOverCurrentNetOffsetDet << AUDIO_IP_DACCTL_OVECBS_Pos)  |
             (AUDIO_IP_InitStruct->DACSideToneLoopBack        << AUDIO_IP_DACCTL_SIDETONE_Pos)|
             (AUDIO_IP_InitStruct->DACState                   << AUDIO_IP_DACCTL_DACRES_Pos);

    MDR_AUDIO_IP->DACCTL = tmpreg;
}

/**
  * @brief  Enables or disables the AUDIO_IP ADC channel.
  * @param  NewState - @ref FunctionalState - new state of the AUDIO_IP ADC channel.
  * @retval None.
  */
void AUDIO_IP_ADCCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_AUDIO_IP->POWCTL |= AUDIO_IP_POWCTL_ADCEN;
    }
    else
    {
        MDR_AUDIO_IP->POWCTL &= ~AUDIO_IP_POWCTL_ADCEN;
    }
}

/**
  * @brief  Enables or disables the AUDIO_IP DAC channel.
  * @param  NewState - @ref FunctionalState - new state of the AUDIO_IP DAC channel.
  * @retval None.
  */
void AUDIO_IP_DACCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_AUDIO_IP->POWCTL |= AUDIO_IP_POWCTL_DACEN;
    }
    else
    {
        MDR_AUDIO_IP->POWCTL &= ~AUDIO_IP_POWCTL_DACEN;
    }
}

/**
  * @brief  Set the new value of the ADC gain.
  * @param  GainValue - @ref AUDIO_IP_ADGain - new value of of the ADC gain.
  * @retval None.
  */
void AUDIO_IP_SetADGain(AUDIO_IP_ADGain GainValue)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_AUDIO_IP_AD_GAIN(GainValue));

    tmpreg = MDR_AUDIO_IP->ADCCTL & (~AUDIO_IP_ADCCTL_ADGAIN_Msk);
    tmpreg |= GainValue & AUDIO_IP_ADCCTL_ADGAIN_Msk;
    MDR_AUDIO_IP->ADCCTL = tmpreg;
}

/**
  * @brief  Set the new value of the DAC gain.
  * @param  GainValue - @ref AUDIO_IP_DAGain - new value of of the DAC gain.
  * @retval None.
  */
void AUDIO_IP_SetDAGain(AUDIO_IP_DAGain GainValue)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_AUDIO_IP_DA_GAIN(GainValue));

    tmpreg = MDR_AUDIO_IP->ADCCTL & (~AUDIO_IP_DACCTL_DAGAIN_Msk);
    tmpreg |= GainValue & AUDIO_IP_DACCTL_DAGAIN_Msk;
    MDR_AUDIO_IP->DACCTL = tmpreg;
}

/**
  * @brief  Sets the new value of the preamplifier ADC.
  * @param  InputAmplifierLevel - @ref AUDIO_IP_ADC_Input_Ampl -  the new value of the preamplifier.
  * @retval None.
  */
void AUDIO_IP_SetADCInputAmplifierLevel(AUDIO_IP_ADC_Input_Ampl InputAmplifierLevel)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_AUDIO_IP_INPUT_AMPLIFIER_LEVEL(InputAmplifierLevel));

    tmpreg = MDR_AUDIO_IP->ADCCTL & (~AUDIO_IP_ADCCTL_INBG_Msk);
    tmpreg |= (uint32_t)InputAmplifierLevel << AUDIO_IP_ADCCTL_INBG_Pos;

    MDR_AUDIO_IP->ADCCTL = tmpreg;
}

/**
  * @brief  Selects the source of the analog input signal.
  * @param  ADCInputSource - @ref AUDIO_IP_ADC_Input_Src - the new value
  *         of the analog input signal source.
  * @retval None.
  */
void AUDIO_IP_SetADCInputSource(AUDIO_IP_ADC_Input_Src ADCInputSource)
{
    uint32_t tmpreg;
    /* Check the parameters */
    assert_param(IS_AUDIO_IP_ANALOG_INPUT_SOURCE(ADCInputSource));

    tmpreg = MDR_AUDIO_IP->ADCCTL & (~AUDIO_IP_ADCCTL_AINSEL_Msk);
    tmpreg |= (uint32_t)ADCInputSource << AUDIO_IP_ADCCTL_AINSEL_Pos;

    MDR_AUDIO_IP->ADCCTL = tmpreg;
}

/**
  * @brief  Enable or disable mute on the DAC output.
  * @param  NewState - @ref FunctionalState - new state of the DAC mute output.
  * @retval None.
  */
void AUDIO_IP_DACMuteCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_AUDIO_IP->DACCTL |= AUDIO_IP_DACCTL_MUTE1;
    }
    else
    {
        MDR_AUDIO_IP->DACCTL &= ~AUDIO_IP_DACCTL_MUTE1;
    }
}

/**
  * @brief  Enable or disable DAC output amplifier.
  * @param  NewState - @ref FunctionalState - new state of the DAC output.
  * @retval None.
  */
void AUDIO_IP_DACOutputAmplifierCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE )
    {
        MDR_AUDIO_IP->DACCTL |= AUDIO_IP_DACCTL_ODAMP;
    }
    else
    {
        MDR_AUDIO_IP->DACCTL &= ~AUDIO_IP_DACCTL_ODAMP;
    }
}

/**
  * @brief  Enables or disables the detection circuit short circuit.
  * @param  ShortCircuitDetection - @ref AUDIO_IP_Short_Circ_Detect_Out_Buff -
  *         type of circuit a short circuit.
  * @param  NewState - @ref FunctionalState - new state of the circuit.
  * @retval None.
  */
void AUDIO_IP_OverCurCmd(AUDIO_IP_Short_Circ_Detect_Out_Buff ShortCircuitDetection, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    assert_param(IS_AUDIO_IP_SHORT_CIRCUIT_DETECTION(ShortCircuitDetection));

    if(NewState != DISABLE)
    {
        MDR_AUDIO_IP->DACCTL |= (uint32_t)ShortCircuitDetection;
    }
    else
    {
        MDR_AUDIO_IP->DACCTL &= ~(uint32_t)ShortCircuitDetection;
    }
}

/**
  * @brief  Enables or disables the specified AUDIO_IP interrupts.
  * @param  AUDIO_IT - @ref AUDIO_IP_ITMask - specifies the AUDIO_IP
  *         interrupt sources to be enabled or disabled.
  * @retval None.
  */
void AUDIO_IP_ITConfig(AUDIO_IP_ITMask AUDIO_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_AUDIO_IP_IT(AUDIO_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_AUDIO_IP->MASKCTL |= (uint32_t)AUDIO_IT;
    }
    else
    {
        MDR_AUDIO_IP->MASKCTL &= ~((uint32_t)AUDIO_IT);
    }
}

/**
  * @brief  Checks whether the specified AUDIO_IP interrupt has occurred or not.
  * @param  AUDIO_IP_FLAG - @ref AUDIO_IP_Flags - specifies
  *         the AUDIO_IP interrupt source to check.
  * @retval @ITStatus - Current state of the AUDIO_IP_FLAG (SET or RESET).
  */
ITStatus AUDIO_IP_GetITStatus(AUDIO_IP_Flags AUDIO_IP_FLAG)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_AUDIO_IP_FLAG(AUDIO_IP_FLAG));

    if(MDR_AUDIO_IP->IRQFLAG & (uint32_t)AUDIO_IP_FLAG)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    return (bitstatus);
}

/**
  * @brief  Clears the AUDIO_IP's interrupt pending bits.
  * @param  AUDIO_IT - @ref AUDIO_IP_ITMask - specifies the interrupt pending bit to clear.
  *         This parameter can be one of the following values:
  *             @arg AUDIO_IP_IT_DAOVFM
  *             @arg AUDIO_IP_IT_ADCVFM
  * @retval None.
  */
void AUDIO_IP_ClearITFlag(AUDIO_IP_ITMask AUDIO_IT)
{
    /* Check the parameters */
    assert_param(IS_AUDIO_IP_CLEAR_IT(AUDIO_IT));
    MDR_AUDIO_IP->IRQFLAG |= (uint32_t)AUDIO_IT;
}


/**
  * @brief  Returns the new sample of the ADC AUDIO_IP.
  * @param  None.
  * @retval New sample of the ADC AUDIO_IP.
  */
uint16_t AUDIO_IP_ReceiveData(void)
{
    return (MDR_AUDIO_IP->ADCREG & 0x0000FFFF);
}

/**
  * @brief  Sets the AUDIO_IP DAC output data.
  * @param  AudioData: specifies the DAC output data.
  * @retval None.
  */
void AUDIO_IP_SendData(uint16_t AudioData)
{
    MDR_AUDIO_IP->DACREG = AudioData & 0x0000FFFF;
}

/** @} */ /* End of group AUDIO_IP_Exported_Functions */

/** @} */ /* End of group AUDIO_IP */
#endif /* defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_audio.c */

