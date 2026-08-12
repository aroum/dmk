/**
  ******************************************************************************
  * @file    MDR32FxQI_arinc429r.c
  * @author  Milandr Application Team
  * @version V2.1.0i
  * @date    23/10/2024
  * @brief   This file contains all the ARINC429R firmware functions.
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
#include "MDR32FxQI_arinc429r.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup ARINC429R ARINC429R
  * @warning This module can be used only for microcontrollers MDR32F1QI, K1986VE1xI.
  * @{
  */

#if defined (USE_K1986VE1xI)

/** @defgroup ARINC429R_Exported_Functions ARINC429R Exported Functions
  * @{
  */

/**
  * @brief  Initializes the ARINC429R peripheral clock according to the
  *         specified parameters.
  * @param  ARINC429R_BRG: specifies the HCLK division factor.
  *         This parameter is number from 0 to 0x7F.
  * @retval None
  */
void ARINC429R_BRG_Init(uint32_t ARINC429R_BRG)
{
    uint32_t tmpreg_CONTROL1;
    uint32_t tmpreg_CONTROL2;

    /* Check the parameters */
    assert_param(IS_ARINC429R_BRG(ARINC429R_BRG));

    /* Read the control1 and control2 ARINC register */
    tmpreg_CONTROL1 = MDR_ARINC429R->CONTROL1;
    tmpreg_CONTROL2 = MDR_ARINC429R->CONTROL2;

    /* Clear the previous value of the divisor */
    tmpreg_CONTROL1 &= ~ARINC429R_CONTROL1_DIV_Msk;
    tmpreg_CONTROL2 &= ~ARINC429R_CONTROL2_DIV_6_4_Msk;

    /* Set the new value of the divisor */
    tmpreg_CONTROL1 |= ((ARINC429R_BRG & 0x0F ) << ARINC429R_CONTROL1_DIV_Pos);
    tmpreg_CONTROL2 |= (((ARINC429R_BRG & 0x70) >> 4) << ARINC429R_CONTROL2_DIV_6_4_Pos);

    /* Write the control1 and control2 ARINC register */
    MDR_ARINC429R->CONTROL1 = tmpreg_CONTROL1;
    MDR_ARINC429R->CONTROL2 = tmpreg_CONTROL2;
}

/**
  * @brief  Reset the ARINC429R peripheral registers to their default reset values.
  * @param  None.
  * @retval None.
  */
void ARINC429R_DeInit(void)
{
    MDR_ARINC429R->CONTROL1 = 0;
    MDR_ARINC429R->CONTROL2 = 0;
    MDR_ARINC429R->CONTROL3 = 0;
    MDR_ARINC429R->STATUS1  = 0;
    MDR_ARINC429R->STATUS2  = 0;
    MDR_ARINC429R->CONTROL4 = 0;
    MDR_ARINC429R->CONTROL5 = 0;
    MDR_ARINC429R->CHANNEL  = 0;
    MDR_ARINC429R->LABEL    = 0;
    MDR_ARINC429R->CONTROL8 = 0;
    MDR_ARINC429R->CONTROL9 = 0;
#if defined (USE_K1986VE1xI) && !defined (USE_MDR32F1QI)
    MDR_ARINC429R->CONTROL10 = 0;
    MDR_ARINC429R->CONTROL11 = 0;
#endif /* #if defined (USE_K1986VE1xI) && !defined (USE_MDR32F1QI) */
}

/**
  * @brief  Initializes the ARINC429R channelx peripheral according to the 
  *         specified parameters in the ARINC429R_InitChannelStruct.
  * @param  ARINC429R_CHANNELx - @ref ARINC429R_Channel - Select the ARINC429R channel.
  * @param  ARINC429R_InitChannelStruct: pointer to a @ref ARINC429R_InitChannelTypeDef
  *         structure that contains the configuration information for the specified ARINC429R channel.
  * @retval None
  */
void ARINC429R_ChannelInit(ARINC429R_Channel ARINC429R_CHANNELx, ARINC429R_InitChannelTypeDef * ARINC429R_InitChannelStruct)
{
    uint32_t tmpreg_CONTROL1;
    uint32_t tmpreg_CONTROL2;
    uint32_t tmpreg_CONTROL3;
    uint32_t tmpreg_CONTROL4;
    uint32_t tmpreg_CONTROL5;

    /* Check the parameters */
    assert_param(IS_ARINC429R_CHANNEL(ARINC429R_CHANNELx));
    assert_param(IS_ARINC429R_CLK(ARINC429R_InitChannelStruct->ARINC429R_CLK));
    assert_param(IS_FUNCTIONAL_STATE(ARINC429R_InitChannelStruct->ARINC429R_LB));
    assert_param(IS_FUNCTIONAL_STATE(ARINC429R_InitChannelStruct->ARINC429R_SD));
    assert_param(IS_BIT_STATUS(ARINC429R_InitChannelStruct->ARINC429R_SDI1));
    assert_param(IS_BIT_STATUS(ARINC429R_InitChannelStruct->ARINC429R_SDI2));
    assert_param(IS_ARINC429R_DIV(ARINC429R_InitChannelStruct->ARINC429R_DIV));


    /* Set the speed of receiving data for specified ARINC429R_CHANNELx */
    tmpreg_CONTROL1 = MDR_ARINC429R->CONTROL1;
    tmpreg_CONTROL1 &= ~(1 << (ARINC429R_CONTROL1_CLK1_Pos + ARINC429R_CHANNELx));
    tmpreg_CONTROL1 |= ARINC429R_InitChannelStruct->ARINC429R_CLK << (ARINC429R_CONTROL1_CLK1_Pos + ARINC429R_CHANNELx);
    MDR_ARINC429R->CONTROL1 = tmpreg_CONTROL1;

    /* Set resolution tag detection and resolution decoding 9 and 10 data bits */
    tmpreg_CONTROL2 = MDR_ARINC429R->CONTROL2;
    tmpreg_CONTROL2 &= ~( (1 << (ARINC429R_CONTROL2_LB_EN1_Pos + ARINC429R_CHANNELx))\
                         |(1 << (ARINC429R_CONTROL2_SD_EN1_Pos + ARINC429R_CHANNELx)));
    tmpreg_CONTROL2 |= ( (ARINC429R_InitChannelStruct->ARINC429R_LB << (ARINC429R_CONTROL2_LB_EN1_Pos + ARINC429R_CHANNELx))\
                        |(ARINC429R_InitChannelStruct->ARINC429R_SD << (ARINC429R_CONTROL2_SD_EN1_Pos + ARINC429R_CHANNELx)));
    MDR_ARINC429R->CONTROL2 = tmpreg_CONTROL2;

    /* Set bit comparison SDI1 and bit comparison SDI2 */
    tmpreg_CONTROL3 = MDR_ARINC429R->CONTROL3;
    tmpreg_CONTROL3 &= ~( (1 << (ARINC429R_CONTROL3_SDI1_1_Pos +ARINC429R_CHANNELx))\
                         |(1 << (ARINC429R_CONTROL3_SDI2_1_Pos +ARINC429R_CHANNELx)));
    tmpreg_CONTROL3 |= ( (ARINC429R_InitChannelStruct->ARINC429R_SDI1 << (ARINC429R_CONTROL3_SDI1_1_Pos +ARINC429R_CHANNELx))\
                        |(ARINC429R_InitChannelStruct->ARINC429R_SDI2 << (ARINC429R_CONTROL3_SDI2_1_Pos +ARINC429R_CHANNELx)));
    MDR_ARINC429R->CONTROL3 = tmpreg_CONTROL3;

    /* Set core frequency divider for frequency reference channel ARINC429R_CHANNELx */
    switch(ARINC429R_CHANNELx)
    {
        case ARINC429R_CHANNEL1:
        case ARINC429R_CHANNEL2:
        case ARINC429R_CHANNEL3:
        case ARINC429R_CHANNEL4:
            tmpreg_CONTROL4 = MDR_ARINC429R->CONTROL4;
            tmpreg_CONTROL4 &= ~(0xFFUL << (ARINC429R_CHANNELx * 8));
            tmpreg_CONTROL4 |= ARINC429R_InitChannelStruct->ARINC429R_DIV << (ARINC429R_CHANNELx * 8);
            MDR_ARINC429R->CONTROL4 = tmpreg_CONTROL4;
            break;
        case ARINC429R_CHANNEL5:
        case ARINC429R_CHANNEL6:
        case ARINC429R_CHANNEL7:
        case ARINC429R_CHANNEL8:
            tmpreg_CONTROL5 = MDR_ARINC429R->CONTROL5;
            tmpreg_CONTROL5 &= ~(0xFFUL << ((ARINC429R_CHANNELx - ARINC429R_CHANNEL5) * 8));
            tmpreg_CONTROL5 |= ARINC429R_InitChannelStruct->ARINC429R_DIV << ((ARINC429R_CHANNELx - ARINC429R_CHANNEL5) * 8);
            MDR_ARINC429R->CONTROL5 = tmpreg_CONTROL5;
            break;
    }
}

/**
  * @brief  Enables or disables the specified ARINC429R channel.
  * @param  ARINC429R_CHANNELx - @ref ARINC429R_Channel - Select the ARINC429R channel.
  * @param  NewState - @ref FunctionalState - new state of the ARINC429R channel.
  * @retval None
  */
void ARINC429R_ChannelCmd(ARINC429R_Channel ARINC429R_CHANNELx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ARINC429R_CHANNEL(ARINC429R_CHANNELx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        /* Enable the selected ARINC429R channel by setting the CH_ENx bit in the CONTROL1 register */
        MDR_ARINC429R->CONTROL1 |= 1 << (ARINC429R_CONTROL1_CH_EN1_Pos + ARINC429R_CHANNELx);
    }
    else
    {
        /* Disable the selected ARINC429R channel by clearing the CH_ENx bit in the CONTROL1 register */
        MDR_ARINC429R->CONTROL1 &= ~(1 << (ARINC429R_CONTROL1_CH_EN1_Pos + ARINC429R_CHANNELx));
    }
}

/**
  * @brief  Enables or disables the specified ARINC429R interrupts.
  * @param  ARINC429R_IT: specifies the ARINC429R interrupt sources to be enabled or disabled.
  *         This parameter can be any combination of @ref ARINC429R_IT_Def values.
  * @param  NewState - @ref FunctionalState - new state of the specified
  *         ARINC429R interrupts.
  * @retval None
  */
void ARINC429R_ITConfig(uint32_t ARINC429R_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ARINC429R_IT(ARINC429R_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_ARINC429R->CONTROL3 |= (ARINC429R_IT << 28);
    }
    else
    {
        MDR_ARINC429R->CONTROL3 &= ~(ARINC429R_IT << 28);
    }
}

/**
  * @brief   Enables or disables the specified ARINC429R interrupt mask(s).
  * @param   ARINC429R_CHANNELx - @ref ARINC429R_Channel - Select the ARINC429R channel.
  * @param   ARINC429R_IT_Mask: specifies the ARINC429R interrupt mask sources
  *          to be enabled or disabled.
  *          This parameter can be any of the @ref ARINC429R_ITMask values.
  * @retval  None.
  */
void ARINC429R_ITMaskConfig(ARINC429R_Channel ARINC429R_CHANNELx, uint32_t ARINC429R_IT_Mask, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ARINC429R_CHANNEL(ARINC429R_CHANNELx));
    assert_param(IS_ARINC429R_ITMask(ARINC429R_IT_Mask));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        MDR_ARINC429R->INTMASK |= ARINC429R_IT_Mask << (4 * ARINC429R_CHANNELx);
    }
    else
    {
        MDR_ARINC429R->INTMASK &= ~(ARINC429R_IT_Mask << (4 * ARINC429R_CHANNELx));
    }
}

/**
  * @brief  Reads the received data from individual buffer.
  * @param  ARINC429R_CHANNELx - @ref ARINC429R_Channel - Select the ARINC429R channel.
  * @retval The received data.
  */
uint32_t ARINC429R_ReceiveDataDirect(ARINC429R_Channel ARINC429R_CHANNELx)
{
    uint32_t tmp_channel;
    uint32_t arinc_data;

    /* Check the parameters */
    assert_param(IS_ARINC429R_CHANNEL(ARINC429R_CHANNELx));

    /* Remember the previous value of the channel. */
    tmp_channel = MDR_ARINC429R->CHANNEL;
    /* Set the direct access to the FIFO for each channel. */
    MDR_ARINC429R->CHANNEL = 0x0E;
    /* Read the received data. */
    arinc_data = MDR_ARINC429R->DATA_R_Direct[ARINC429R_CHANNELx];
    /* Restore the channel value. */
    MDR_ARINC429R->CHANNEL = tmp_channel;

    return (arinc_data);
}

/**
  * @brief  Checks whether the specified ARINC429R flag is set or not.
  * @param  ARINC429R_CHANNELx - @ref ARINC429R_Channel - Select the ARINC429R channel.
  * @param  ARINC429R_FLAG - @ref ARINC429R_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - The state of ARINC429R_FLAG (SET or RESET).
  */
FlagStatus ARINC429R_GetFlagStatus(ARINC429R_Channel ARINC429R_CHANNELx, ARINC429R_Flags ARINC429R_FLAG)
{
    FlagStatus bitstatus;
    /* Check the parameters */
    assert_param(IS_ARINC429R_CHANNEL(ARINC429R_CHANNELx));
    assert_param(IS_ARINC429R_FLAG(ARINC429R_FLAG));

    switch(ARINC429R_FLAG)
    {
        case ARINC429R_FLAG_DR:
            bitstatus = (MDR_ARINC429R->STATUS1 & (1 << (ARINC429R_STATUS1_DR1_Pos + ARINC429R_CHANNELx))) == 0 ? RESET : SET;
            break;
        case ARINC429R_FLAG_ERR:
            bitstatus = (MDR_ARINC429R->STATUS1 & (1 << (ARINC429R_STATUS1_ERR1_Pos + ARINC429R_CHANNELx))) == 0 ? RESET : SET;
            break;
        case ARINC429R_FLAG_HF:
            bitstatus = (MDR_ARINC429R->STATUS2 & (1 << (ARINC429R_STATUS2_HF1_Pos + ARINC429R_CHANNELx))) == 0 ? RESET : SET;
            break;
        case ARINC429R_FLAG_FF:
            bitstatus = (MDR_ARINC429R->STATUS2 & (1 << (ARINC429R_STATUS2_FF1_Pos + ARINC429R_CHANNELx))) == 0 ? RESET : SET;
            break;
    }

    return (bitstatus);
}

/**
  * @brief  Set the specified channel for the ARINC429R peripheral.
  * @param  ARINC429R_CHANNELx - @ref ARINC429R_Channel- Select the ARINC429R channel.
  * @retval None
  */
void ARINC429R_SetChannel(ARINC429R_Channel ARINC429R_CHANNELx)
{
    /* Check the parameters */
    assert_param(IS_ARINC429R_CHANNEL(ARINC429R_CHANNELx));

    /* Set new channel */
    MDR_ARINC429R->CHANNEL = ARINC429R_CHANNELx;
}

/**
  * @brief  Get label of the specified channel of ARINC429R peripheral, which
  *         compares the first received 8 bits, if set LB_EN bit corresponding
  *         to the specified channel.
  * @param  None
  * @retval Label of the specified channel ARINC429R peripheral.
  */
uint32_t ARINC429R_GetLabel(void)
{
    return (MDR_ARINC429R->LABEL);
}

/**
  * @brief  Set label of the specified channel ARINC429R peripheral, which
  *         compares the first received 8 bits, if set LB_EN bit corresponding
  *         to the specified channel.
  * @param  Label of the specified channel ARINC429R peripheral.
  * @retval None.
  */
void ARINC429R_SetLabel(uint8_t label)
{
    /* Check the parameters */
    assert_param(IS_ARINC429R_LABEL(label));

    MDR_ARINC429R->LABEL = label;
}

/**
  * @brief  Reads the received data from buffer.
  * @param  None
  * @retval The received data.
  */
uint32_t ARINC429R_ReceiveData(void)
{
    /* The received data */
    return (MDR_ARINC429R->DATA_R);
}

/** @} */ /* End of group ARINC429R_Exported_Functions */

#endif /* #if defined (USE_K1986VE1xI) */

/** @} */ /* End of group ARINC429R */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_arinc429r.c */

