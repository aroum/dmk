/**
  ******************************************************************************
  * @file    MDR32FxQI_adc.c
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
#include "MDR32FxQI_adc.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup ADC ADC
  * @{
  */

/** @defgroup ADC_Exported_Functions ADC Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the ADC peripheral registers to their default reset values.
  * @param  None.
  * @retval None.
  */
void ADC_DeInit(void)
{
    MDR_ADC->ADC1_CFG = 0;
    MDR_ADC->ADC2_CFG = 0;
    MDR_ADC->ADC1_H_LEVEL = 0;
    MDR_ADC->ADC1_L_LEVEL = 0;
    MDR_ADC->ADC1_RESULT;
    MDR_ADC->ADC1_STATUS = 0;
    MDR_ADC->ADC1_CHSEL = 0;

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    MDR_ADC->ADC2_H_LEVEL = 0;
    MDR_ADC->ADC2_L_LEVEL = 0;
    MDR_ADC->ADC2_RESULT;
    MDR_ADC->ADC2_STATUS = 0;
    MDR_ADC->ADC2_CHSEL = 0;
#endif

#if defined (USE_K1986VE1xI)
    MDR_ADC->ADC1_TRIM = 0;
#endif
}

/**
  * @brief  Initializes the ADC peripheral according to
  *         the specified parameters in the ADC_InitStruct.
  * @param  ADC_InitStruct: pointer to a @ref ADC_InitTypeDef structure that
  *         contains the configuration information for the specified ADC peripheral.
  * @retval None
  */
void ADC_Init(const ADC_InitTypeDef* ADC_InitStruct)
{
    uint32_t tmpreg_CFG;
    uint32_t tmpreg_MSK;
#if defined (USE_K1986VE1xI)
    uint32_t tmpreg_TRIM;
#endif /* #if defined (USE_K1986VE1xI) */

    /* Check the parameters */
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    assert_param(IS_ADC_SYNC_MODE(ADC_InitStruct->ADC_SynchronousMode));
    assert_param(IS_ADC_START_DELAY_VALUE(ADC_InitStruct->ADC_StartDelay));
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */
    assert_param(IS_ADC_TEMP_SENSOR_CONFIG(ADC_InitStruct->ADC_TempSensor));
    assert_param(IS_ADC_TEMP_SENSOR_AMP_CONFIG(ADC_InitStruct->ADC_TempSensorAmplifier));
    assert_param(IS_ADC_TEMP_SENSOR_CONVERSION_CONFIG(ADC_InitStruct->ADC_TempSensorConversion));
    assert_param(IS_ADC_VREF_CONVERSION_CONFIG(ADC_InitStruct->ADC_IntVRefConversion));
    assert_param(IS_ADC_VREF_TRIMMING_VALUE(ADC_InitStruct->ADC_IntVRefTrimming));
#if defined (USE_K1986VE1xI)
    assert_param(IS_ADC_INT_VREF_AMPLIFIER(ADC_InitStruct->ADC_IntVRefAmplifier));
#endif /* #if defined (USE_K1986VE1xI) */

    tmpreg_CFG = ADC_InitStruct->ADC_TempSensor
               | ADC_InitStruct->ADC_TempSensorAmplifier
               | ADC_InitStruct->ADC_TempSensorConversion
               | ADC_InitStruct->ADC_IntVRefConversion;

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    tmpreg_CFG |= (ADC_InitStruct->ADC_StartDelay << ADC1_CFG_DELAY_ADC_Pos)
               |   ADC_InitStruct->ADC_SynchronousMode
               |  (ADC_InitStruct->ADC_IntVRefTrimming << ADC1_CFG_TR_Pos);
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

    tmpreg_MSK = ADC1_CFG_DELAY_ADC_Msk
               | ADC1_CFG_TS_EN
               | ADC1_CFG_TS_BUF_EN
               | ADC1_CFG_SEL_TS
               | ADC1_CFG_SEL_VREF;
#if defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    tmpreg_MSK |= ADC1_CFG_TR_Msk
               |  ADC1_CFG_SYNC_CONVER;
#elif defined (USE_K1986VE1xI)
    tmpreg_TRIM = (ADC_InitStruct->ADC_IntVRefTrimming << ADC1_TRIM_TS_TRIM_Pos)
                |  ADC_InitStruct->ADC_IntVRefAmplifier;
    MDR_ADC->ADC1_TRIM  = tmpreg_TRIM;
#endif

    MDR_ADC->ADC1_CFG = (MDR_ADC->ADC1_CFG & ~tmpreg_MSK) | tmpreg_CFG;
}

/**
  * @brief  Fills each ADC_InitStruct member with its default value.
  * @param  ADC_InitStruct: pointer to a @ref ADC_InitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct)
{
#if defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    ADC_InitStruct->ADC_SynchronousMode     = ADC_SyncMode_Independent;
    ADC_InitStruct->ADC_StartDelay           = 0;
#endif
    ADC_InitStruct->ADC_TempSensor           = ADC_TEMP_SENSOR_Disable;
    ADC_InitStruct->ADC_TempSensorAmplifier  = ADC_TEMP_SENSOR_AMPLIFIER_Disable;
    ADC_InitStruct->ADC_TempSensorConversion = ADC_TEMP_SENSOR_CONVERSION_Disable;
    ADC_InitStruct->ADC_IntVRefConversion    = ADC_VREF_CONVERSION_Disable;
    ADC_InitStruct->ADC_IntVRefTrimming      = 0;

#if defined(USE_K1986VE1xI)
    ADC_InitStruct->ADC_IntVRefAmplifier     = ADC_INT_VREF_AMPLIFIER_Disable;
#endif
}


/**
  * @brief  Sets the Internal Voltage Reference trimming.
  * @param  Trim: trimming value.
  *         This parameter can be a value in range 0..7  for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *                                   and in range 0..31 for MDR32F1QI, K1986VE1xI.
  * @retval None
  */
void ADC_SetTrim(uint8_t Trim)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_ADC_VREF_TRIMMING_VALUE(Trim));

#if defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    tmpreg = MDR_ADC->ADC1_CFG & ~ADC1_CFG_TR_Msk;
    MDR_ADC->ADC1_CFG = tmpreg | ((uint32_t)Trim << ADC1_CFG_TR_Pos);

#elif defined (USE_K1986VE1xI)
    tmpreg = MDR_ADC->ADC1_TRIM;
    tmpreg &= ~ADC1_TRIM_TS_TRIM_Msk;
    tmpreg |= ((uint32_t)Trim << ADC1_TRIM_TS_TRIM_Pos);
    MDR_ADC->ADC1_TRIM = tmpreg;
#endif
}


/**
  * @brief  Initializes the ADC1 peripheral according to
  *         the specified parameters in the ADCx_InitStruct.
  * @param  ADCx_InitStruct: pointer to a @ref ADCx_InitTypeDef structure
  *         that contains the configuration information for the ADC1
  *         peripheral.
  * @retval None
  */
void ADC1_Init(const ADCx_InitTypeDef* ADCx_InitStruct)
{
    uint32_t tmpreg_CFG1;
    uint32_t tmpreg_CFG2;

    /* Check the parameters */
    assert_param(IS_ADC_CLOCK_SOURCE_CONFIG(ADCx_InitStruct->ADC_ClockSource));
    assert_param(IS_ADC_SAMPLING_MODE_CONFIG(ADCx_InitStruct->ADC_SamplingMode));
    assert_param(IS_ADC_CH_SWITCHING_CONFIG(ADCx_InitStruct->ADC_ChannelSwitching));
    assert_param(IS_ADC1_CH_NUM(ADCx_InitStruct->ADC_ChannelNumber));
    assert_param(IS_ADC1_CH_MASK(ADCx_InitStruct->ADC_Channels));
    assert_param(IS_ADC_LEVEL_CONTROL_CONFIG(ADCx_InitStruct->ADC_LevelControl));
    assert_param(IS_ADC_VALUE(ADCx_InitStruct->ADC_LowLevel));
    assert_param(IS_ADC_VALUE(ADCx_InitStruct->ADC_HighLevel));
    assert_param(IS_ADC_VREF_SOURCE_CONFIG(ADCx_InitStruct->ADC_VRefSource));
    assert_param(IS_ADC_INT_VREF_SOURCE_CONFIG(ADCx_InitStruct->ADC_IntVRefSource));
    assert_param(IS_ADC_CLK_div_VALUE(ADCx_InitStruct->ADC_Prescaler));
    assert_param(IS_ADC_DELAY_GO_VALUE(ADCx_InitStruct->ADC_DelayGo));

    tmpreg_CFG1 = MDR_ADC->ADC1_CFG;

    tmpreg_CFG1 &= ~(ADC1_CFG_REG_CLKS
                   | ADC1_CFG_REG_SAMPLE
                   | ADC1_CFG_REG_CHCH
                   | ADC1_CFG_REG_CHS_Msk
                   | ADC1_CFG_REG_RNGC
                   | ADC1_CFG_M_REF
                   | ADC1_CFG_REG_DIVCLK_Msk
                   | ADC1_CFG_DELAY_GO_Msk);

    tmpreg_CFG1 |= ADCx_InitStruct->ADC_ClockSource
                 | ADCx_InitStruct->ADC_SamplingMode
                 | ADCx_InitStruct->ADC_ChannelSwitching
                 | (ADCx_InitStruct->ADC_ChannelNumber << ADC1_CFG_REG_CHS_Pos)
                 | ADCx_InitStruct->ADC_LevelControl
                 | ADCx_InitStruct->ADC_VRefSource
                 | ADCx_InitStruct->ADC_Prescaler
                 | (ADCx_InitStruct->ADC_DelayGo << ADC1_CFG_DELAY_GO_Pos);

    tmpreg_CFG2 = MDR_ADC->ADC2_CFG;
    tmpreg_CFG2 &= ~ADC2_CFG_ADC1_OP;
    tmpreg_CFG2 |= (ADCx_InitStruct->ADC_IntVRefSource << ADC2_CFG_ADC1_OP_Pos);


    MDR_ADC->ADC1_L_LEVEL = ADCx_InitStruct->ADC_LowLevel;
    MDR_ADC->ADC1_H_LEVEL = ADCx_InitStruct->ADC_HighLevel;
    MDR_ADC->ADC1_CHSEL   = ADCx_InitStruct->ADC_Channels;

    MDR_ADC->ADC1_CFG = tmpreg_CFG1;
    MDR_ADC->ADC2_CFG = tmpreg_CFG2;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Initializes the ADC2 peripheral according to
  *          the specified parameters in the ADCx_InitStruct.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   ADCx_InitStruct: pointer to a @ref ADCx_InitTypeDef structure
  *          that contains the configuration information for the ADC2
  *          peripheral.
  * @retval  None
  */
void ADC2_Init(const ADCx_InitTypeDef* ADCx_InitStruct)
{
    uint32_t tmpreg_CFG2;

    /* Check the parameters */
    assert_param(IS_ADC_CLOCK_SOURCE_CONFIG(ADCx_InitStruct->ADC_ClockSource));
    assert_param(IS_ADC_SAMPLING_MODE_CONFIG(ADCx_InitStruct->ADC_SamplingMode));
    assert_param(IS_ADC_CH_SWITCHING_CONFIG(ADCx_InitStruct->ADC_ChannelSwitching));
    assert_param(IS_ADC2_CH_NUM(ADCx_InitStruct->ADC_ChannelNumber));
    assert_param(IS_ADC2_CH_MASK(ADCx_InitStruct->ADC_Channels));
    assert_param(IS_ADC_LEVEL_CONTROL_CONFIG(ADCx_InitStruct->ADC_LevelControl));
    assert_param(IS_ADC_VALUE(ADCx_InitStruct->ADC_LowLevel));
    assert_param(IS_ADC_VALUE(ADCx_InitStruct->ADC_HighLevel));
    assert_param(IS_ADC_VREF_SOURCE_CONFIG(ADCx_InitStruct->ADC_VRefSource));
    assert_param(IS_ADC_INT_VREF_SOURCE_CONFIG(ADCx_InitStruct->ADC_IntVRefSource));
    assert_param(IS_ADC_CLK_div_VALUE(ADCx_InitStruct->ADC_Prescaler));
    assert_param(IS_ADC_DELAY_GO_VALUE(ADCx_InitStruct->ADC_DelayGo));

    tmpreg_CFG2 = MDR_ADC->ADC2_CFG;

    tmpreg_CFG2 &= ~(ADC2_CFG_REG_CLKS
                   | ADC2_CFG_REG_SAMPLE
                   | ADC2_CFG_REG_CHCH
                   | ADC2_CFG_REG_CHS_Msk
                   | ADC2_CFG_REG_RNGC
                   | ADC2_CFG_M_REF
                   | ADC2_CFG_ADC2_OP
                   | ADC2_CFG_REG_DIVCLK_Msk
                   | ADC2_CFG_DELAY_GO_Msk);

    tmpreg_CFG2 |= ADCx_InitStruct->ADC_ClockSource
                 | ADCx_InitStruct->ADC_SamplingMode
                 | ADCx_InitStruct->ADC_ChannelSwitching
                 | (ADCx_InitStruct->ADC_ChannelNumber << ADC2_CFG_REG_CHS_Pos)
                 | ADCx_InitStruct->ADC_LevelControl
                 | ADCx_InitStruct->ADC_VRefSource
                 | (ADCx_InitStruct->ADC_IntVRefSource << ADC2_CFG_ADC2_OP_Pos)
                 | ADCx_InitStruct->ADC_Prescaler
                 | (ADCx_InitStruct->ADC_DelayGo << ADC2_CFG_DELAY_GO_Pos);

    MDR_ADC->ADC2_L_LEVEL = ADCx_InitStruct->ADC_LowLevel;
    MDR_ADC->ADC2_H_LEVEL = ADCx_InitStruct->ADC_HighLevel;
    MDR_ADC->ADC2_CHSEL   = ADCx_InitStruct->ADC_Channels;

    MDR_ADC->ADC2_CFG = tmpreg_CFG2;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Fills each ADCx_InitStruct member with its default value.
  * @param  ADCx_InitStruct: pointer to a @ref ADCx_InitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void ADCx_StructInit(ADCx_InitTypeDef* ADCx_InitStruct)
{
    ADCx_InitStruct->ADC_ClockSource      = ADC_CLOCK_SOURCE_CPU;
    ADCx_InitStruct->ADC_SamplingMode     = ADC_SAMPLING_MODE_SINGLE_CONV;
    ADCx_InitStruct->ADC_ChannelSwitching = ADC_CH_SWITCHING_Disable;
    ADCx_InitStruct->ADC_ChannelNumber    = ADC_CH_ADC0;
    ADCx_InitStruct->ADC_Channels         = 0;
    ADCx_InitStruct->ADC_LevelControl     = ADC_LEVEL_CONTROL_Disable;
    ADCx_InitStruct->ADC_LowLevel         = 0;
    ADCx_InitStruct->ADC_HighLevel        = 0;
    ADCx_InitStruct->ADC_VRefSource       = ADC_VREF_SOURCE_INTERNAL;
    ADCx_InitStruct->ADC_IntVRefSource    = ADC_INT_VREF_SOURCE_INEXACT;
    ADCx_InitStruct->ADC_Prescaler        = ADC_CLK_div_None;
    ADCx_InitStruct->ADC_DelayGo          = 0;
}

/**
  * @brief  Enables or disables the ADC1 peripheral.
  * @param  NewState - @ref FunctionalState - new state of the ADC1 peripheral.
  * @retval None
  */
void ADC1_Cmd(FunctionalState NewState)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CFG = MDR_ADC->ADC1_CFG;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable ADC1 by setting the Cfg_REG_ADON bit in the ADC1_CFG register */
        tmpreg_CFG |= ADC1_CFG_REG_ADON;
    }
    else
    {
        /* Disable ADC1 by resetting the Cfg_REG_ADON bit in the ADC1_CFG register */
        tmpreg_CFG &= ~ADC1_CFG_REG_ADON;
    }

    /* Configure ADC1_CFG register with new value */
    MDR_ADC->ADC1_CFG = tmpreg_CFG;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Enables or disables the ADC1 peripheral.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   NewState - @ref FunctionalState - new state of the ADC1 peripheral.
  * @retval  None
  */
void ADC2_Cmd(FunctionalState NewState)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CFG = MDR_ADC->ADC2_CFG;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable ADC2 by setting the Cfg_REG_ADON bit in the ADC2_CFG register */
        tmpreg_CFG |= ADC2_CFG_REG_ADON;
    }
    else
    {
        /* Disable ADC2 by resetting the Cfg_REG_ADON bit in the ADC2_CFG register */
        tmpreg_CFG &= ~ADC2_CFG_REG_ADON;
    }

    /* Configure ADC2_CFG register with new value */
    MDR_ADC->ADC2_CFG = tmpreg_CFG;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Selects the ADC1 Channel number for Single Channel Mode conversion.
  * @param  Channel - @ref ADCx_Channel_Number - specifies the ADC Channel number.
  * @retval None
  */
void ADC1_SetChannel(ADCx_Channel_Number Channel)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC1_CH_NUM(Channel));

    tmpreg_CFG = MDR_ADC->ADC1_CFG;
    tmpreg_CFG &= ~ADC1_CFG_REG_CHS_Msk;
    tmpreg_CFG |= ((uint32_t)Channel << ADC1_CFG_REG_CHS_Pos);
    MDR_ADC->ADC1_CFG = tmpreg_CFG;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Selects the ADC2 Channel number for Single Channel Mode conversion.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   Channel - @ref ADCx_Channel_Number - specifies the ADC Channel number.
  * @retval  None
  */
void ADC2_SetChannel(ADCx_Channel_Number Channel)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC2_CH_NUM(Channel));

    tmpreg_CFG = MDR_ADC->ADC2_CFG;
    tmpreg_CFG &= ~ADC2_CFG_REG_CHS_Msk;
    tmpreg_CFG |= ((uint32_t)Channel << ADC2_CFG_REG_CHS_Pos);
    MDR_ADC->ADC2_CFG = tmpreg_CFG;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Selects the ADC1 Channels for conversion with Channels switching.
  * @param  ChannelMask: specifies the ADC Channels Mask.
  *         This parameter can be any combination of @ref ADCx_Channels values.
  * @retval None
  */
void ADC1_SetChannels(uint32_t ChannelMask)
{
    /* Check the parameters */
    assert_param(IS_ADC1_CH_MASK(ChannelMask));

    MDR_ADC->ADC1_CHSEL = ChannelMask;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Selects the ADC2 Channels for conversion with Channels switching.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   ChannelMask: specifies the ADC Channels Mask.
  *          This parameter can be any combination of @ref ADCx_Channels values.
  * @retval  None
  */
void ADC2_SetChannels(uint32_t ChannelMask)
{
    /* Check the parameters */
    assert_param(IS_ADC2_CH_MASK(ChannelMask));

    MDR_ADC->ADC2_CHSEL = ChannelMask;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Sets the ADC1 operation mode.
  * @param  SamplingMode - @ref ADCx_Sampling_Mode - specifies the ADC1 sampling.
  * @param  SwitchingMode - @ref ADCx_Channel_Switching - specifies the ADC1 channel switching.
  * @retval None
  */
void ADC1_OperationModeConfig(ADCx_Sampling_Mode SamplingMode, ADCx_Channel_Switching SwitchingMode)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC_SAMPLING_MODE_CONFIG(SamplingMode));
    assert_param(IS_ADC_CH_SWITCHING_CONFIG(SwitchingMode));

    tmpreg_CFG = MDR_ADC->ADC1_CFG;
    tmpreg_CFG &= ~(ADC1_CFG_REG_SAMPLE | ADC1_CFG_REG_CHCH);
    tmpreg_CFG |= (uint32_t)SamplingMode | (uint32_t)SwitchingMode;
    MDR_ADC->ADC1_CFG = tmpreg_CFG;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Sets the ADC2 operation mode.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   SamplingMode - @ref ADCx_Sampling_Mode - specifies the ADC2 sampling.
  * @param   SwitchingMode - @ref ADCx_Channel_Switching - specifies the ADC2 channel switching.
  * @retval  None
  */
void ADC2_OperationModeConfig(ADCx_Sampling_Mode SamplingMode, ADCx_Channel_Switching SwitchingMode)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC_SAMPLING_MODE_CONFIG(SamplingMode));
    assert_param(IS_ADC_CH_SWITCHING_CONFIG(SwitchingMode));

    tmpreg_CFG = MDR_ADC->ADC2_CFG;
    tmpreg_CFG &= ~(ADC2_CFG_REG_SAMPLE | ADC2_CFG_REG_CHCH);
    tmpreg_CFG |= (uint32_t)SamplingMode | (uint32_t)SwitchingMode;
    MDR_ADC->ADC2_CFG = tmpreg_CFG;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Sets the ADC1 sampling mode.
  * @param  SamplingMode - @ref ADCx_Sampling_Mode - specifies the ADC1 sampling.
  * @retval None
  */
void ADC1_SamplingModeConfig(ADCx_Sampling_Mode SamplingMode)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC_SAMPLING_MODE_CONFIG(SamplingMode));

    tmpreg_CFG = MDR_ADC->ADC1_CFG;
    tmpreg_CFG &= ~ADC1_CFG_REG_SAMPLE;
    tmpreg_CFG |= (uint32_t)SamplingMode;
    MDR_ADC->ADC1_CFG = tmpreg_CFG;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Sets the ADC2 sampling mode.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   SamplingMode - @ref ADCx_Sampling_Mode - specifies the ADC2 sampling.
  * @retval  None
  */
void ADC2_SamplingModeConfig(ADCx_Sampling_Mode SamplingMode)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC_SAMPLING_MODE_CONFIG(SamplingMode));

    tmpreg_CFG = MDR_ADC->ADC2_CFG;
    tmpreg_CFG &= ~ADC2_CFG_REG_SAMPLE;
    tmpreg_CFG |= (uint32_t)SamplingMode;
    MDR_ADC->ADC2_CFG = tmpreg_CFG;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Sets the ADC1 channel switching mode.
  * @param  SwitchingMode - @ref ADCx_Channel_Switching - specifies the ADC1 channel switching.
  * @retval None
  */
void ADC1_ChannelSwithingConfig(ADCx_Channel_Switching SwitchingMode)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC_CH_SWITCHING_CONFIG(SwitchingMode));

    tmpreg_CFG = MDR_ADC->ADC1_CFG;
    tmpreg_CFG &= ~ADC1_CFG_REG_CHCH;
    tmpreg_CFG |= (uint32_t)SwitchingMode;
    MDR_ADC->ADC1_CFG = tmpreg_CFG;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Sets the ADC2 channel switching mode.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   SwitchingMode - @ref ADCx_Channel_Switching - specifies the ADC2 channel switching.
  * @retval  None
  */
void ADC2_ChannelSwithingConfig(ADCx_Channel_Switching SwitchingMode)
{
    uint32_t tmpreg_CFG;

    /* Check the parameters */
    assert_param(IS_ADC_CH_SWITCHING_CONFIG(SwitchingMode));

    tmpreg_CFG = MDR_ADC->ADC2_CFG;
    tmpreg_CFG &= ~ADC2_CFG_REG_CHCH;
    tmpreg_CFG |= (uint32_t)SwitchingMode;
    MDR_ADC->ADC2_CFG = tmpreg_CFG;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Configures the ADC1 threshould levels.
  * @param  LowLevel: specifies the ADC1 low level value.
  * @param  HighLevel: specifies the ADC1 high level value.
  * @param  NewState - @ref ADCx_Level_Control - enables or disables levels control.
  * @retval None
  */
void ADC1_LevelsConfig(uint32_t LowLevel, uint32_t HighLevel, ADCx_Level_Control NewState)
{
    /* Check the parameters */
    assert_param(IS_ADC_LEVEL_CONTROL_CONFIG(NewState));
    assert_param(IS_ADC_VALUE(LowLevel));
    assert_param(IS_ADC_VALUE(HighLevel));

    if (NewState != ADC_LEVEL_CONTROL_Disable) {
        MDR_ADC->ADC1_L_LEVEL = LowLevel;
        MDR_ADC->ADC1_H_LEVEL = HighLevel;
        MDR_ADC->ADC1_CFG |= ADC1_CFG_REG_RNGC;
    } else {
        MDR_ADC->ADC1_CFG &= ~ADC1_CFG_REG_RNGC;
        MDR_ADC->ADC1_L_LEVEL = LowLevel;
        MDR_ADC->ADC1_H_LEVEL = HighLevel;
    }
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Configures the ADC2 threshould levels.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   LowLevel: specifies the ADC2 low level value.
  * @param   HighLevel: specifies the ADC2 high level value.
  * @param   NewState - @ref ADCx_Level_Control - enables or disables levels control.
  * @retval  None
  */
void ADC2_LevelsConfig(uint32_t LowLevel, uint32_t HighLevel, ADCx_Level_Control NewState)
{
    /* Check the parameters */
    assert_param(IS_ADC_LEVEL_CONTROL_CONFIG(NewState));
    assert_param(IS_ADC_VALUE(LowLevel));
    assert_param(IS_ADC_VALUE(HighLevel));

    if (NewState != ADC_LEVEL_CONTROL_Disable) {
        MDR_ADC->ADC2_L_LEVEL = LowLevel;
        MDR_ADC->ADC2_H_LEVEL = HighLevel;
        MDR_ADC->ADC2_CFG |= ADC2_CFG_REG_RNGC;
    } else {
        MDR_ADC->ADC2_CFG &= ~ADC2_CFG_REG_RNGC;
        MDR_ADC->ADC2_L_LEVEL = LowLevel;
        MDR_ADC->ADC2_H_LEVEL = HighLevel;
    }
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Sets the ADC1 low level.
  * @param  LowLevel: specifies the ADC1 low level value.
  * @retval None
  */
void ADC1_SetLowLevel(uint32_t LowLevel)
{
    /* Check the parameters */
    assert_param(IS_ADC_VALUE(LowLevel));

    MDR_ADC->ADC1_L_LEVEL = LowLevel;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Sets the ADC2 low level.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   LowLevel: specifies the ADC2 low level value.
  * @retval  None
  */
void ADC2_SetLowLevel(uint32_t LowLevel)
{
    /* Check the parameters */
    assert_param(IS_ADC_VALUE(LowLevel));

    MDR_ADC->ADC2_L_LEVEL = LowLevel;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Sets the ADC1 high level.
  * @param  HighLevel: specifies the ADC1 high level value.
  * @retval None
  */
void ADC1_SetHighLevel(uint32_t HighLevel)
{
    /* Check the parameters */
    assert_param(IS_ADC_VALUE(HighLevel));

    MDR_ADC->ADC1_H_LEVEL = HighLevel;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Sets the ADC2 high level.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   HighLevel: specifies the ADC2 high level value.
  * @retval  None
  */
void ADC2_SetHighLevel(uint32_t HighLevel)
{
    /* Check the parameters */
    assert_param(IS_ADC_VALUE(HighLevel));

    MDR_ADC->ADC2_H_LEVEL = HighLevel;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Starts the ADC1 conversion.
  * @param  None.
  * @retval None.
  */
void ADC1_Start(void)
{
    MDR_ADC->ADC1_CFG |= ADC1_CFG_REG_GO;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Starts the ADC2 conversion.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   None.
  * @retval  None.
  */
void ADC2_Start(void)
{
    MDR_ADC->ADC2_CFG |= ADC2_CFG_REG_GO;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Returns the ADC1 result.
  * @param  None.
  * @retval ADC1 Result Register value.
  */
uint32_t ADC1_GetResult(void)
{
    return MDR_ADC->ADC1_RESULT;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Returns the ADC2 result.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   None.
  * @retval  ADC2 Result Register value.
  */
uint32_t ADC2_GetResult(void)
{
    return MDR_ADC->ADC2_RESULT;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Returns the ADC1, ADC2 Status Registers combined value.
  * @param  None.
  * @retval The ADC1_STATUS, ADC2_STATUS combined value.
  */
uint32_t ADC_GetStatus(void)
{
#if defined  (USE_K1986VE9xI)|| defined (USE_MDR32FG16S1QI)
    return MDR_ADC->ADC1_STATUS | (MDR_ADC->ADC2_STATUS << 16);
#elif defined (USE_K1986VE1xI)
    return MDR_ADC->ADC1_STATUS;
#endif
}

/**
  * @brief  Returns the ADC1 Status Register value.
  * @param  None.
  * @retval The ADC1_STATUS Register value.
  */
uint32_t ADC1_GetStatus(void)
{
    return MDR_ADC->ADC1_STATUS;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Returns the ADC2 Status Register value.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   None.
  * @retval  The ADC2_STATUS Register value.
  */
uint32_t ADC2_GetStatus(void)
{
    return MDR_ADC->ADC2_STATUS;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Checks whether the specified ADC1, ADC2 Status flag is set or not.
  * @param  Flags: Specifies the flags to check.
  *         This parameter could be any combination of @ref ADC_Flags values.
  * @retval @ref FlagStatus - Current Status flags state (SET or RESET).
  */
FlagStatus ADC_GetFlagStatus(uint32_t Flags)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_ADC_STATUS_FLAGS(Flags));

    if ((ADC_GetStatus() & Flags) == 0)
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
  * @brief  Checks whether the specified ADC1 Status flag is set or not.
  * @param  Flags: Specifies the flags to check.
  *         This parameter could be any combination of @ref ADCx_Flags values.
  * @retval @ref FlagStatus - Current Status flag state (SET or RESET).
  */
FlagStatus ADC1_GetFlagStatus(uint32_t Flags)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_ADCx_STATUS_FLAGS(Flags));

    if ((MDR_ADC->ADC1_STATUS & Flags) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Checks whether the specified ADC2 Status flag is set or not.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   Flags: Specifies the flags to check.
  *          This parameter could be any combination of @ref ADCx_Flags values.
  * @retval  @ref FlagStatus - Current Status flag state (SET or RESET).
  */
FlagStatus ADC2_GetFlagStatus(uint32_t Flags)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_ADCx_STATUS_FLAGS(Flags));

    if ((MDR_ADC->ADC2_STATUS & Flags) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Clears the ADC1 Overwrite flag.
  * @param  None.
  * @retval None
  */
void ADC1_ClearOverwriteFlag(void)
{
    MDR_ADC->ADC1_STATUS = (MDR_ADC->ADC1_STATUS | ADCx_FLAG_OUT_OF_RANGE) & ~ADCx_FLAG_OVERWRITE;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Clears the ADC2 Overwrite flag.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   None.
  * @retval  None
  */
void ADC2_ClearOverwriteFlag(void)
{
    MDR_ADC->ADC2_STATUS = (MDR_ADC->ADC2_STATUS | ADCx_FLAG_OUT_OF_RANGE) & ~ADCx_FLAG_OVERWRITE;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)) */

/**
  * @brief  Clears the ADC1 AWOIFEN flag.
  * @param  None.
  * @retval None
  */
void ADC1_ClearOutOfRangeFlag(void)
{
    MDR_ADC->ADC1_STATUS = (MDR_ADC->ADC1_STATUS | ADCx_FLAG_OVERWRITE) & ~ADCx_FLAG_OUT_OF_RANGE;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Clears the ADC2 AWOIFEN flag.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   None.
  * @retval  None
  */
void ADC2_ClearOutOfRangeFlag(void)
{
    MDR_ADC->ADC2_STATUS = (MDR_ADC->ADC2_STATUS | ADCx_FLAG_OVERWRITE) & ~ADCx_FLAG_OUT_OF_RANGE;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Enables or disables the ADC1, ADC2 interrupts.
  * @param  ADC_IT: Specifies the ADC interrupts sources to be enabled or disabled.
  *         This parameter could be any combination of @ref ADC_IT values.
  * @param  NewState - @ref FunctionalState - new state of the ADC interrupts (ENABLE or DISABLE).
  * @retval None
  */
void ADC_ITConfig(uint32_t ADC_IT, FunctionalState NewState)
{
    uint32_t tmpreg_ADCx_IE;

    /* Check the parameters */
    assert_param(IS_ADC_CONFIG_ITS(ADC_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_ADCx_IE = MDR_ADC->ADC1_STATUS | (ADCx_FLAG_OVERWRITE | ADCx_FLAG_OVERWRITE);

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the ADC Interrupt requests by setting bits in the ADCx_STATUS registers */
        tmpreg_ADCx_IE |= (ADC_IT & 0xFFFF);
    }
    else
    {
        /* Disable the ADC Interrupt requests by clearing bits in the ADCx_STATUS registers */
        tmpreg_ADCx_IE &= ~(ADC_IT & 0xFFFF);
    }

    /* Configure ADCx_STATUS registers with new value */
    MDR_ADC->ADC1_STATUS = tmpreg_ADCx_IE;

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)

    tmpreg_ADCx_IE = MDR_ADC->ADC2_STATUS | (ADCx_FLAG_OVERWRITE | ADCx_FLAG_OVERWRITE);

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the ADC Interrupt requests by setting bits in the ADCx_STATUS registers */
        tmpreg_ADCx_IE |= ADC_IT >> 16;
    }
    else
    {
        /* Disable the ADC Interrupt requests by clearing bits in the ADCx_STATUS registers */
        tmpreg_ADCx_IE &= ~(ADC_IT >> 16);
    }
    /* Configure ADCx_STATUS registers with new value */
    MDR_ADC->ADC2_STATUS = tmpreg_ADCx_IE;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */
}

/**
  * @brief  Enables or disables the ADC1 interrupts.
  * @param  ADCx_IT: Specifies the ADC1 interrupts sources to be enabled or disabled.
  *         This parameter could be any combination of @ref ADCx_IT values.
  * @param  NewState - @ref FunctionalState - new state of the ADC1 interrupts (ENABLE or DISABLE).
  * @retval None
  */
void ADC1_ITConfig(uint32_t ADCx_IT, FunctionalState NewState)
{
    uint32_t tmpreg_ADC1_IE;

    /* Check the parameters */
    assert_param(IS_ADCx_CONFIG_ITS(ADCx_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_ADC1_IE = MDR_ADC->ADC1_STATUS | (ADCx_FLAG_OVERWRITE | ADCx_FLAG_OVERWRITE);

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the ADC Interrupt requests by setting bits in the ADC1_STATUS register */
        tmpreg_ADC1_IE |= ADCx_IT;
    }
    else
    {
        /* Disable the ADC Interrupt requests by clearing bits in the ADC1_STATUS register */
        tmpreg_ADC1_IE &= ~ADCx_IT;
    }

    /* Configure ADC1_STATUS registers with new value */
    MDR_ADC->ADC1_STATUS = tmpreg_ADC1_IE;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Enables or disables the ADC2 interrupts.
  * @warning This function can be used only for microcontroller series
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param  ADCx_IT: Specifies the ADC2 interrupts sources to be enabled or disabled.
  *         This parameter could be any combination of @ref ADCx_IT values.
  * @param   NewState - @ref FunctionalState - new state of the ADC2 interrupts (ENABLE or DISABLE).
  * @retval  None
  */
void ADC2_ITConfig(uint32_t ADCx_IT, FunctionalState NewState)
{
    uint32_t tmpreg_ADC2_IE;

    /* Check the parameters */
    assert_param(IS_ADCx_CONFIG_ITS(ADCx_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_ADC2_IE = MDR_ADC->ADC2_STATUS | (ADCx_FLAG_OVERWRITE | ADCx_FLAG_OVERWRITE);

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the ADC Interrupt requests by setting bits in the ADC2_STATUS register */
        tmpreg_ADC2_IE |= ADCx_IT;
    }
    else
    {
        /* Disable the ADC Interrupt requests by clearing bits in the ADC2_STATUS register */
        tmpreg_ADC2_IE &= ~ADCx_IT;
    }

    /* Configure ADC2_STATUS registers with new value */
    MDR_ADC->ADC2_STATUS = tmpreg_ADC2_IE;
}
#endif /* #if defined  (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Checks whether the ADC1, ADC2 interrupt has occurred or not.
  * @param  ADC_IT: Specifies the ADC interrupt source to check.
  *         This parameter could be any combination of @ref ADC_IT_Def values.
  * @retval @ref ITStatus - Current state of the ADC_IT (SET or RESET).
  */
ITStatus ADC_GetITStatus(uint32_t ADC_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_ADC_CONFIG_ITS(ADC_IT));

    if ((ADC_GetStatus() & ADC_IT) == 0)
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
  * @brief  Checks whether the ADC1 interrupt has occurred or not.
  * @param  ADCx_IT: Specifies the ADC1 interrupt source to check.
  *         This parameter could be any combination of @ref ADCx_IT_Def values.
  * @retval @ref ITStatus - Current state of the ADCx_IT (SET or RESET).
  */
ITStatus ADC1_GetITStatus(uint32_t ADCx_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_ADCx_CONFIG_ITS(ADCx_IT));

    if ((MDR_ADC->ADC1_STATUS & ADCx_IT) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

  return bitstatus;
}

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
/**
  * @brief   Checks whether the ADC2 interrupt has occurred or not.
  * @warning This function can be used only for microcontroller
  *          MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI.
  * @param   ADCx_IT: Specifies the ADC2 interrupt source to check.
  *          This parameter could be any combination of @ref ADCx_IT_Def values.
  * @retval  @ref ITStatus - Current state of the ADCx_IT (SET or RESET).
  */
ITStatus ADC2_GetITStatus(uint32_t ADCx_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_ADCx_CONFIG_ITS(ADCx_IT));

    if ((MDR_ADC->ADC2_STATUS & ADCx_IT) == 0)
    {
        bitstatus = RESET;
    }
    else
    {
        bitstatus = SET;
    }

    return bitstatus;
}
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group ADC_Exported_Functions */

/** @} */ /* End of group ADC */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_adc.c */

