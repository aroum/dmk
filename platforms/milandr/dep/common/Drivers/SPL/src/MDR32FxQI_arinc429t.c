/**
  ******************************************************************************
  * @file    MDR32FxQI_arinc429t.c
  * @author  Milandr Application Team
  * @version V2.1.0i
  * @date    23/10/2024
  * @brief   This file contains all the ARINC429T firmware functions.
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
#include "MDR32FxQI_arinc429t.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup ARINC429T ARINC429T
  * @warning This module can be used only for microcontrollers MDR32F1QI, K1986VE9xI.
  * @{
  */

#if defined (USE_K1986VE1xI)
/** @defgroup ARINC429T_Exported_Funstions ARINC429T Exported Funstions
  * @{
  */

/**
  * @brief  Initializes the ARINC429T peripheral Clock according to the
  *         specified parameters.
  * @param  ARINC429T_BRG: specifies the HCLK division factor.
  *         This parameter is number from 0 to 0x7F.
  * @retval None
  */
void ARINC429T_BRG_Init(uint32_t ARINC429T_BRG)
{
    uint32_t tmpreg_CONTROL1;

    /* Check the parameters */
    assert_param(IS_ARINC429T_HCLKdiv(ARINC429T_BRG));

    /* Read the control1 ARINC register */
    tmpreg_CONTROL1 = MDR_ARINC429T->CONTROL1;

    /* Clear the previous value of the divisor */
    tmpreg_CONTROL1 &= ~ARINC429T_HCLKdiv_MASK;

    /* Set the new value of the divisor */
    tmpreg_CONTROL1 |= (ARINC429T_BRG << ARINC429T_CONTROL1_DIV_Pos);

    /* Write the control1 ARINC register */
    MDR_ARINC429T->CONTROL1 = tmpreg_CONTROL1;
}

/**
  * @brief  Reset the ARINC429T peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void ARINC429T_DeInit(void)
{
    MDR_ARINC429T->CONTROL1 = 0;
    MDR_ARINC429T->CONTROL2 = 0;
    MDR_ARINC429T->CONTROL3 = 0;
    MDR_ARINC429T->CONTROL4 = 0;
#if defined (USE_K1986VE1xI) && !defined (USE_MDR32F1QI)
    MDR_ARINC429T->CONTROL5 = 0;
#endif
    MDR_ARINC429T->STATUS   = 0;
}

/**
  * @brief  Initializes the ARINC429T channelx peripheral according to the specified
  *         sparameters in the ARINC429T_InitChannelStruct.
  * @param  ARINC429T_CHANNELx - @ref ARINC429T_Channel - Select the ARINC429T channel.
  * @param  ARINC429T_InitChanelStruct - @ref ARINC429T_InitChannelTypeDef - pointer
  *         to a ARINC429T_InitChannelTypeDef structure that contains
  *         the configuration information for the specified ARINC429T channel.
  * @retval None
  */
void ARINC429T_ChannelInit(ARINC429T_Channel ARINC429T_CHANNELx, ARINC429T_InitChannelTypeDef * ARINC429T_InitChannelStruct)
{
    uint32_t tmpreg_CONTROL1;
    uint32_t tmpreg_CONTROL2;
    uint32_t tmpreg_CONTROL3;

    /* Check the parameters */
    assert_param(IS_ARINC429T_CHANNEL(ARINC429T_CHANNELx));
    assert_param(IS_ARINC429T_CLK(ARINC429T_InitChannelStruct->ARINC429T_CLK));
    assert_param(IS_ARINC429T_DIV_CH(ARINC429T_InitChannelStruct->ARINC429T_DIV));
    assert_param(IS_FUNCTIONAL_STATE(ARINC429T_InitChannelStruct->ARINC429T_EN_PAR));
    assert_param(IS_ARINC429T_ODD(ARINC429T_InitChannelStruct->ARINC429T_ODD));

    tmpreg_CONTROL3 = MDR_ARINC429T->CONTROL3;
    tmpreg_CONTROL3 &= ~(0xFF << (ARINC429T_CHANNELx * 8));
    tmpreg_CONTROL3 |= (ARINC429T_InitChannelStruct->ARINC429T_DIV << (ARINC429T_CHANNELx * 8));
    MDR_ARINC429T->CONTROL3 = tmpreg_CONTROL3;

    switch(ARINC429T_CHANNELx)
    {
        case ARINC429T_CHANNEL1:
        case ARINC429T_CHANNEL2:
            tmpreg_CONTROL1 = MDR_ARINC429T->CONTROL1;
            tmpreg_CONTROL1 &= ~( (1 << (ARINC429T_CONTROL1_CLK1_Pos + ARINC429T_CHANNELx * 4)) \
                                 |(1 << (ARINC429T_CONTROL1_EN_PAR1_Pos + ARINC429T_CHANNELx * 4)) \
                                 |(1 << (ARINC429T_CONTROL1_ODD1_Pos + ARINC429T_CHANNELx * 4)));
            tmpreg_CONTROL1 |= ((ARINC429T_InitChannelStruct->ARINC429T_CLK << (ARINC429T_CONTROL1_CLK1_Pos + ARINC429T_CHANNELx * 4)) \
                               |(ARINC429T_InitChannelStruct->ARINC429T_EN_PAR << (ARINC429T_CONTROL1_EN_PAR1_Pos + ARINC429T_CHANNELx * 4)) \
                               |(ARINC429T_InitChannelStruct->ARINC429T_ODD << (ARINC429T_CONTROL1_ODD1_Pos + ARINC429T_CHANNELx * 4)));
            MDR_ARINC429T->CONTROL1 = tmpreg_CONTROL1;
            break;
        case ARINC429T_CHANNEL3:
        case ARINC429T_CHANNEL4:
            tmpreg_CONTROL2 = MDR_ARINC429T->CONTROL2;
            tmpreg_CONTROL2 &= ~( (1 << (ARINC429T_CONTROL2_CLK3_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4)) \
                                 |(1 << (ARINC429T_CONTROL1_EN_PAR1_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4)) \
                                 |(1 << (ARINC429T_CONTROL1_ODD1_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4)));
            tmpreg_CONTROL2 |= ((ARINC429T_InitChannelStruct->ARINC429T_CLK << (ARINC429T_CONTROL1_CLK1_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4))\
                               |(ARINC429T_InitChannelStruct->ARINC429T_EN_PAR << (ARINC429T_CONTROL1_EN_PAR1_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4))\
                               |(ARINC429T_InitChannelStruct->ARINC429T_ODD << (ARINC429T_CONTROL1_ODD1_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4)));
            MDR_ARINC429T->CONTROL2 = tmpreg_CONTROL2;
            break;
    }
}

/**
  * @brief  Enables or disables the specified ARINC429T channel.
  * @param  ARINC429T_CHANNELx - @ref ARINC429T_Channel - Select the ARINC429T channel.
  * @param  NewState - @ref FunctionalState - new state of the ARINC429T channel.
  * @retval None
  */
void ARINC429T_ChannelCmd(ARINC429T_Channel ARINC429T_CHANNELx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ARINC429T_CHANNEL(ARINC429T_CHANNELx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        if(ARINC429T_CHANNELx <= ARINC429T_CHANNEL2)
        {
            MDR_ARINC429T->CONTROL1 |= 1 << (ARINC429T_CONTROL1_CH_EN1_Pos + ARINC429T_CHANNELx * 4);
        }
        else
        {
            MDR_ARINC429T->CONTROL2 |= 1 << (ARINC429T_CONTROL2_CH_EN3_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4);
        }
    }
    else
    {
        if(ARINC429T_CHANNELx <= ARINC429T_CHANNEL2)
        {
            MDR_ARINC429T->CONTROL1 &= ~(1 << (ARINC429T_CONTROL1_CH_EN1_Pos + ARINC429T_CHANNELx * 4));
        }
        else
        {
            MDR_ARINC429T->CONTROL2 &= ~(1 << (ARINC429T_CONTROL2_CH_EN3_Pos + (ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 4));
        }
    }
}

/**
  * @brief  Enables or disables the specified ARINC429T interrupts
  *         for the specifies channel.
  * @param  ARINC429T_CHANNELx - @ref ARINC429T_Channel - Select the ARINC429T channel.
  * @param  ARINC429T_IT - @ref ARINC429T_IT_Def - specifies the ARINC429T
  *         interrupt source to be enabled or disabled.
  * @param  NewState - @ref FunctionalState - new state of the specified ARINC429T interrupts.
  * @retval None
  */
void ARINC429T_ITConfig(ARINC429T_Channel ARINC429T_CHANNELx, ARINC429T_IT_Def ARINC429T_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ARINC429T_CHANNEL(ARINC429T_CHANNELx));
    assert_param(IS_ARINC429T_IT(ARINC429T_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if(NewState != DISABLE)
    {
        if(ARINC429T_CHANNELx < ARINC429T_CHANNEL3)
        {
            MDR_ARINC429T->CONTROL1 |= ARINC429T_IT << (ARINC429T_CONTROL1_INTE_FFT1_Pos + (ARINC429T_CHANNELx * 3));
        }
        else
        {
            MDR_ARINC429T->CONTROL2 |= ARINC429T_IT << (ARINC429T_CONTROL2_INTE_FFT3_Pos + ((ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 3));
        }
    }
    else
    {
        if(ARINC429T_CHANNELx < ARINC429T_CHANNEL3)
        {
            MDR_ARINC429T->CONTROL1 &= ~(ARINC429T_IT << (ARINC429T_CONTROL1_INTE_FFT1_Pos + (ARINC429T_CHANNELx * 3)));
        }
        else
        {
            MDR_ARINC429T->CONTROL2 &= ~(ARINC429T_IT << (ARINC429T_CONTROL2_INTE_FFT3_Pos + ((ARINC429T_CHANNELx - ARINC429T_CHANNEL3) * 3)));
        }
    }
}

/**
  * @brief  Checks whether the specified ARINC429T flag is set or not.
  * @param  ARINC429T_CHANNELx - @ref ARINC429T_Channel - Select the ARINC429T channel.
  * @param  ARINC429T_FLAG - @ref ARINC429T_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - The state of ARINC429T_FLAG (SET or RESET).
  */
FlagStatus ARINC429T_GetFlagStatus(ARINC429T_Channel ARINC429T_CHANNELx, ARINC429T_Flags ARINC429T_FLAG)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_ARINC429T_CHANNEL(ARINC429T_CHANNELx));
    assert_param(IS_ARINC429T_FLAG(ARINC429T_FLAG));

    switch(ARINC429T_FLAG)
    {
        case ARINC429T_FLAG_TX_R:
            bitstatus = (MDR_ARINC429T->STATUS & (1 << (ARINC429T_STATUS_TX_R1_Pos + ARINC429T_CHANNELx * 3 \
                      + 2 * (ARINC429T_CHANNELx >= ARINC429T_CHANNEL3)))) == 0 ? RESET : SET;
            break;
        case ARINC429T_FLAG_FFT:
            bitstatus = (MDR_ARINC429T->STATUS & (1 << (ARINC429T_STATUS_FFT1_Pos + ARINC429T_CHANNELx * 3 \
                      + 2 * (ARINC429T_CHANNELx >= ARINC429T_CHANNEL3)))) == 0 ? RESET : SET;
            break;
        case ARINC429T_FLAG_HFT:
            bitstatus = (MDR_ARINC429T->STATUS & (1 << (ARINC429T_STATUS_HFT1_Pos + ARINC429T_CHANNELx * 3 \
                      + 2 * (ARINC429T_CHANNELx >= ARINC429T_CHANNEL3)))) == 0 ? RESET : SET;
            break;
    }

    return (bitstatus);
}

/**
  * @brief  Transmits single data through the specified ARINC429T channel.
  * @param  ARINC429T_CHANNELx - @ref ARINC429T_Channel - Select the ARINC429T channel.
  * @param  Data: the data to transmit.
  * @retval one
  */
void ARINC429T_SendData(ARINC429T_Channel ARINC429T_CHANNELx, uint32_t Data)
{
    /* Check the parameters */
    assert_param(IS_ARINC429T_CHANNEL(ARINC429T_CHANNELx));

    switch(ARINC429T_CHANNELx)
    {
        case ARINC429T_CHANNEL1:
            MDR_ARINC429T->DATA1_T = Data;
            break;
        case ARINC429T_CHANNEL2:
            MDR_ARINC429T->DATA2_T = Data;
            break;
        case ARINC429T_CHANNEL3:
            MDR_ARINC429T->DATA3_T = Data;
            break;
        case ARINC429T_CHANNEL4:
            MDR_ARINC429T->DATA4_T = Data;
            break;
    }
}

/** @} */ /* End of group ARINC429T_Exported_Funstions */

#endif /* #if defined (USE_K1986VE1xI) */

/** @} */ /* End of group ARINC429T */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_arinc429t.c */

