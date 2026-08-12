/**
  ******************************************************************************
  * @file    MDR32FxQI_ssp.c
  * @author  Milandr Application Team
  * @version V2.1.1i
  * @date    24/07/2024
  * @brief   This file contains all the SSP firmware functions.
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
#include "MDR32FxQI_ssp.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup SSP SSP
  * @{
  */

/** @defgroup SSP_Private_Defines SSP Private Defines
  * @{
  */

#if defined (USE_K1986VE9xI)
    #define IS_SSP_ALL_PERIPH(PERIPH)   (((PERIPH) == MDR_SSP1) || \
                                         ((PERIPH) == MDR_SSP2))
#elif defined (USE_K1986VE1xI)
    #define IS_SSP_ALL_PERIPH(PERIPH)   (((PERIPH) == MDR_SSP1) || \
                                         ((PERIPH) == MDR_SSP2) || \
                                         ((PERIPH) == MDR_SSP3))
#elif defined (USE_MDR32FG16S1QI)
    #define IS_SSP_ALL_PERIPH(PERIPH)   (((PERIPH) == MDR_SSP1) || \
                                         ((PERIPH) == MDR_SSP2) || \
                                         ((PERIPH) == MDR_SSP3) || \
                                         ((PERIPH) == MDR_SSP4))
#endif


#define CR1_EN_Set              ((uint16_t)0x0002)  /*!< SSP Enable Mask */
#define CR1_EN_Reset            ((uint16_t)0xFFFD)  /*!< SSP Disable Mask */


#if defined (USE_MDR32FG16S1QI)
    #define SSP4_BRG_Mask       RST_CLK_SPP2_CLOCK_SSP4_BRG_Msk
    #define SSP4_BRG_Pos        RST_CLK_SPP2_CLOCK_SSP4_BRG_Pos
    #define SSP4_CLK_EN         RST_CLK_SPP2_CLOCK_SSP4_CLK_EN
#endif

/** @} */ /* End of group SSP_Private_Defines */

/** @defgroup SSP_Exported_Functions SSP Exported Functions
  * @{
  */

/**
  * @brief  Resets the SSPx peripheral registers to their default reset values.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @retval None
  */
void SSP_DeInit(MDR_SSP_TypeDef* SSPx)
{
    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));

    SSPx->CR0   = 0;
    SSPx->CR1   = 0;
    SSPx->CPSR  = 0;
    SSPx->IMSC  = 0;
    SSPx->DMACR = 0;

    /* Clear SSP ICR[RTIC] and SSP ICR[RORIC] bits */
    SSPx->ICR = SSP_IT_RT | SSP_IT_ROR;
}

/**
  * @brief  Initializes the SSPx peripheral according to the specified
  *         parameters in the SSP_InitStruct.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_InitStruct: pointer to a @ref SSP_InitTypeDef structure that contains
  *         the configuration information for the specified SSP peripheral.
  * @retval None
  */
void SSP_Init(MDR_SSP_TypeDef* SSPx, const SSP_InitTypeDef* SSP_InitStruct)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_SPEED_FACTOR(SSP_InitStruct->SSP_SCR));
    assert_param(IS_SSP_SPEED_DIVIDER(SSP_InitStruct->SSP_CPSDVSR));
    assert_param(IS_SSP_MODE(SSP_InitStruct->SSP_Mode));
    assert_param(IS_SSP_WORD_LENGTH(SSP_InitStruct->SSP_WordLength));
    assert_param(IS_SSP_SPH(SSP_InitStruct->SSP_SPH));
    assert_param(IS_SSP_SPO(SSP_InitStruct->SSP_SPO));
    assert_param(IS_SSP_FRF(SSP_InitStruct->SSP_FRF));
    assert_param(IS_SSP_HARDWARE_FLOW_CONTROL(SSP_InitStruct->SSP_HardwareFlowControl));

    /* SSPx CPSR Configuration */
    SSPx->CPSR = SSP_InitStruct->SSP_CPSDVSR;

    /* SSPx CR0 Configuration */
    tmpreg = (SSP_InitStruct->SSP_SCR << SSP_CR0_SCR_Pos)
            + SSP_InitStruct->SSP_SPH
            + SSP_InitStruct->SSP_SPO
            + SSP_InitStruct->SSP_FRF
            + SSP_InitStruct->SSP_WordLength;
    SSPx->CR0 = tmpreg;

    /* SSPx CR1 Configuration */
    tmpreg = SSP_InitStruct->SSP_HardwareFlowControl + SSP_InitStruct->SSP_Mode;
    SSPx->CR1 = tmpreg;
}

/**
  * @brief  Fills each SSP_InitStruct member with its default value.
  * @param  SSP_InitStruct: pointer to a @ref SSP_InitTypeDef structure
  *         that is to be initialized.
  * @retval None
  */
void SSP_StructInit(SSP_InitTypeDef* SSP_InitStruct)
{
    /* SSP_InitStruct members default value */
    /* Initialize the SPI_Mode member */
    SSP_InitStruct->SSP_Mode = SSP_ModeMaster;
    /* initialize the SSP_WordLength member */
    SSP_InitStruct->SSP_WordLength = SSP_WordLength8b;
    /* Initialize the SSP_SPO member */
    SSP_InitStruct->SSP_SPO = SSP_SPO_Low;
    /* Initialize the SSP_SPH member */
    SSP_InitStruct->SSP_SPH = SSP_SPH_1Edge;
    /* Initialize the SSP_FRF member */
    SSP_InitStruct->SSP_FRF = SSP_FRF_SPI_Motorola;
    /* Initialize the SSP max speed */
    SSP_InitStruct->SSP_CPSDVSR = 2;
    SSP_InitStruct->SSP_SCR = 0;
    /* Initialize the hardware flow control */
    SSP_InitStruct->SSP_HardwareFlowControl = SSP_HardwareFlowControl_None;
}

/**
  * @brief  Enables or disables the specified SSP peripheral.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  NewState - @ref FunctionalState - new state of the SSPx peripheral.
  * @retval None
  */
void SSP_Cmd(MDR_SSP_TypeDef* SSPx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Enable the selected SSP by setting the SSE bit in the CR1 register */
        SSPx->CR1 |= CR1_EN_Set;
    }
    else
    {
        /* Disable the selected SSP by clearing the SSE bit in the CR1 register */
        SSPx->CR1 &= CR1_EN_Reset;
    }
}

/**
  * @brief  Enables or disables the specified SSP interrupts.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_IT: specifies the SSP interrupt sources to be enabled or disabled.
  *         This parameter can be any combination of @ref SSP_IT_TypeDef values.
  * @param  NewState - @ref FunctionalState - new state of the specified SSPx interrupts.
  * @retval None
  */
void SSP_ITConfig(MDR_SSP_TypeDef* SSPx, uint32_t SSP_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_CONFIG_IT(SSP_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        SSPx->IMSC |= SSP_IT;
    }
    else
    {
        SSPx->IMSC &= ~SSP_IT;
    }
}

/**
  * @brief  Checks whether the specified SSP interrupt has occurred or not.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_IT - @ref SSP_IT_TypeDef - specifies the SSP interrupt source to check.
  * @retval @ref ITStatus - The state of SSP_IT (SET or RESET).
  */
ITStatus SSP_GetITStatus(MDR_SSP_TypeDef* SSPx, SSP_IT_TypeDef SSP_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_CONFIG_IT(SSP_IT));

    if ((SSPx->RIS & ((uint32_t)SSP_IT)))
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
  * @brief  Checks whether the specified SSP interrupt (masked) has occurred or not.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_IT - @ref SSP_IT_TypeDef - specifies the SSP interrupt source to check.
  * @retval @ref ITStatus - The state of SSP_IT (SET or RESET).
  */
ITStatus SSP_GetITStatusMasked(MDR_SSP_TypeDef* SSPx, SSP_IT_TypeDef SSP_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_CONFIG_IT(SSP_IT));

    if (SSPx->MIS & ((uint32_t)SSP_IT))
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
  * @brief  Clears the SSPx's interrupt pending bits.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_IT - @ref SSP_IT_TypeDef - specifies the interrupt pending bit to clear.
  *         This parameter can be one of the following values:
  *           @arg SSP_IT_RT
  *           @arg SSP_IT_ROR
  * @retval None
  */
void SSP_ClearITPendingBit(MDR_SSP_TypeDef* SSPx, SSP_IT_TypeDef SSP_IT)
{
    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_RESET_IT(SSP_IT));

    SSPx->ICR |= ((uint32_t)SSP_IT);
}

/**
  * @brief  Enables or disables the SSP's DMA interface.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_DMAReq: specifies the DMA request.
  *         This parameter can be any combination of @ref SSP_DMA_Req_TypeDef values.
  * @param  NewState - @ref FunctionalState - new state of the DMA Request sources.
  * @note   The DMA mode is not available for SSP5.
  * @retval None
  */
void SSP_DMACmd(MDR_SSP_TypeDef* SSPx, uint32_t SSP_DMAReq, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_DMAREQ(SSP_DMAReq));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Enable the DMA transfer for selected requests in the SSP DMACR register */
        SSPx->DMACR |= SSP_DMAReq;
    }
    else
    {
        /* Disable the DMA transfer for selected requests in the SSP DMACR register */
        SSPx->DMACR &= (uint16_t)~SSP_DMAReq;
    }
}

/**
  * @brief  Transmits single data through the SSPx peripheral.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  Data: the data to transmit.
  * @retval None
  */
void SSP_SendData(MDR_SSP_TypeDef* SSPx, uint16_t Data)
{
    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));

    /* Transmit Data */
    SSPx->DR = Data;
}

/**
  * @brief  Returns the most recent received data by the SSPx peripheral.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @retval The received data.
  */
uint16_t SSP_ReceiveData(MDR_SSP_TypeDef* SSPx)
{
    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));

    /* Receive Data */
    return ((uint16_t)(SSPx->DR));
}

/**
  * @brief  Checks whether the specified SSP flag is set or not.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_FLAG - @ref SSP_Flags_TypeDef - specifies the flag to check.
  * @retval @ref FlagStatus - The state of SSP_FLAG (SET or RESET).
  */
FlagStatus SSP_GetFlagStatus(MDR_SSP_TypeDef* SSPx, SSP_Flags_TypeDef SSP_FLAG)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_FLAG(SSP_FLAG));

    if (SSPx->SR & ((uint32_t)SSP_FLAG))
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
  * @brief  Initializes the SSPx peripheral clock according to the
  *         specified parameters.
  * @param  SSPx: Select the SSP peripheral.
  *         This parameter can be one of the MDR_SSPy values, where y is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3 for MDR32F1QI, K1986VE1xI;
  *             1, 2, 3, 4 for MDR32FG16S1QI.
  * @param  SSP_BRG - @ref SSP_Clock_BRG_TypeDef - specifies the HCLK division factor.
  * @retval None
  */
void SSP_BRGInit(MDR_SSP_TypeDef* SSPx, SSP_Clock_BRG_TypeDef SSP_BRG)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_SSP_ALL_PERIPH(SSPx));
    assert_param(IS_SSP_CLOCK_BRG(SSP_BRG));

#if defined (USE_MDR32FG16S1QI)
    if(SSPx == MDR_SSP4)
    {
        tmpreg = MDR_RST_CLK->SPP2_CLOCK;
    }
    else
#endif
    {
        tmpreg = MDR_RST_CLK->SSP_CLOCK;
    }

    if(SSPx == MDR_SSP1)
    {
        tmpreg |= RST_CLK_SSP_CLOCK_SSP1_CLK_EN;
        tmpreg &= ~RST_CLK_SSP_CLOCK_SSP1_BRG_Msk;
        tmpreg |= ((uint32_t)SSP_BRG);
    }
    else if(SSPx == MDR_SSP2)
    {
        tmpreg |= RST_CLK_SSP_CLOCK_SSP2_CLK_EN;
        tmpreg &= ~RST_CLK_SSP_CLOCK_SSP2_BRG_Msk;
        tmpreg |= (((uint32_t)SSP_BRG) << 8);
    }
#if defined (USE_K1986VE1xI) || defined (USE_MDR32FG16S1QI)
    else if(SSPx == MDR_SSP3)
    {
        tmpreg |= RST_CLK_SSP_CLOCK_SSP3_CLK_EN;
        tmpreg &= ~RST_CLK_SSP_CLOCK_SSP3_BRG_Msk;
        tmpreg |= (((uint32_t)SSP_BRG) << RST_CLK_SSP_CLOCK_SSP3_BRG_Pos);
    }
#endif
#if defined (USE_MDR32FG16S1QI)
    else if(SSPx == MDR_SSP4)
    {
        tmpreg |= SSP4_CLK_EN;
        tmpreg &= ~SSP4_BRG_Mask;
        tmpreg |= (((uint32_t)SSP_BRG) << SSP4_BRG_Pos);
    }
#endif

#if defined (USE_MDR32FG16S1QI)
    if(SSPx == MDR_SSP4)
    {
        MDR_RST_CLK->SPP2_CLOCK = tmpreg;
    }
    else
#endif
    {
        MDR_RST_CLK->SSP_CLOCK = tmpreg;
    }
}

/** @} */ /* End of group SSP_Exported_Functions */

/** @} */ /* End of group SSP */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_ssp.c */


