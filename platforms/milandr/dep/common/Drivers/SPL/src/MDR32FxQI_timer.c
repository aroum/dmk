/**
  ******************************************************************************
  * @file    MDR32FxQI_timer.c
  * @author  Milandr Application Team
  * @version V2.2.1i
  * @date    24/07/2024
  * @brief   This file contains all the TIMER firmware functions.
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
#include "MDR32FxQI_timer.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup TIMER TIMER
  * @{
  */

/** @defgroup TIMER_Private_Defines TIMER Private Defines
  * @{
  */

#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
    #define IS_TIMER_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_TIMER1) || \
                                         ((PERIPH) == MDR_TIMER2) || \
                                         ((PERIPH) == MDR_TIMER3))
#elif defined (USE_K1986VE1xI)
    #define IS_TIMER_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_TIMER1) || \
                                         ((PERIPH) == MDR_TIMER2) || \
                                         ((PERIPH) == MDR_TIMER3) || \
                                         ((PERIPH) == MDR_TIMER4))
#endif

/** @} */ /* End of group TIMER_Private_Defines */

/** @defgroup TIMER_Exported_Functions TIMER Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the TIMERx peripheral registers to their default reset values.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval None
  */
void TIMER_DeInit(MDR_TIMER_TypeDef* TIMERx)
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    TIMERx->CNTRL = 0;
    TIMERx->CNT   = 0;
    TIMERx->PSG   = 0;
    TIMERx->ARR   = 0;

    TIMERx->CH1_CNTRL  = 0;
    TIMERx->CH2_CNTRL  = 0;
    TIMERx->CH3_CNTRL  = 0;
    TIMERx->CH4_CNTRL  = 0;
    TIMERx->CH1_CNTRL1 = 0;
    TIMERx->CH2_CNTRL1 = 0;
    TIMERx->CH3_CNTRL1 = 0;
    TIMERx->CH4_CNTRL1 = 0;
    TIMERx->CH1_CNTRL2 = 0;
    TIMERx->CH2_CNTRL2 = 0;
    TIMERx->CH3_CNTRL2 = 0;
    TIMERx->CH4_CNTRL2 = 0;

    TIMERx->CCR1         = 0;
    TIMERx->CCR2         = 0;
    TIMERx->CCR3         = 0;
    TIMERx->CCR4         = 0;
    TIMERx->CCR11        = 0;
    TIMERx->CCR21        = 0;
    TIMERx->CCR31        = 0;
    TIMERx->CCR41        = 0;
    TIMERx->CH1_DTG      = 0;
    TIMERx->CH2_DTG      = 0;
    TIMERx->CH3_DTG      = 0;
    TIMERx->CH4_DTG      = 0;
    TIMERx->BRKETR_CNTRL = 0;
    TIMERx->STATUS       = 0;
    TIMERx->IE           = 0;
    TIMERx->DMA_RE       = 0;
#if defined (USE_K1986VE1xI)
    TIMERx->DMA_REChx[0] = TIMERx->DMA_REChx[1] =
            TIMERx->DMA_REChx[2] = TIMERx->DMA_REChx[3] = 0;
#endif /* #if  defined (USE_K1986VE1xI) */
}

/**
  * @brief  Initializes the TIMERx Counter peripheral according to
  *         the specified parameters in the TIMER_CntInitStruct.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  TIMER_CntInitStruct: pointer to a @ref TIMER_CntInitTypeDef structure
  *         that contains the configuration information for the specified TIMER peripheral.
  * @retval None
  */
void TIMER_CntInit(MDR_TIMER_TypeDef* TIMERx, const TIMER_CntInitTypeDef* TIMER_CntInitStruct)
{
    uint32_t tmpreg_CNTRL;
    uint32_t tmpreg_BRKETR_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_COUNTER_MODE(TIMER_CntInitStruct->TIMER_CounterMode));
    assert_param(IS_TIMER_COUNTER_DIR(TIMER_CntInitStruct->TIMER_CounterDirection));
    assert_param(IS_TIMER_EVENT_SOURCE(TIMER_CntInitStruct->TIMER_EventSource));
    assert_param(IS_TIMER_FILTER_SAMPLING(TIMER_CntInitStruct->TIMER_FilterSampling));
    assert_param(IS_TIMER_ARR_UPDATE_MODE(TIMER_CntInitStruct->TIMER_ARR_UpdateMode));
    assert_param(IS_TIMER_FILTER_CONF(TIMER_CntInitStruct->TIMER_ETR_FilterConf));
    assert_param(IS_TIMER_ETR_PRESCALER(TIMER_CntInitStruct->TIMER_ETR_Prescaler));
    assert_param(IS_TIMER_ETR_POLARITY(TIMER_CntInitStruct->TIMER_ETR_Polarity));
    assert_param(IS_TIMER_BRK_POLARITY(TIMER_CntInitStruct->TIMER_BRK_Polarity));

    TIMERx->CNT = TIMER_CntInitStruct->TIMER_IniCounter;
    TIMERx->PSG = TIMER_CntInitStruct->TIMER_Prescaler;
    TIMERx->ARR = TIMER_CntInitStruct->TIMER_Period;

    /* Form new value for the CNTRL register */
    tmpreg_CNTRL = TIMER_CntInitStruct->TIMER_CounterMode
                 | TIMER_CntInitStruct->TIMER_CounterDirection
                 | TIMER_CntInitStruct->TIMER_EventSource
                 | TIMER_CntInitStruct->TIMER_FilterSampling
                 | TIMER_CntInitStruct->TIMER_ARR_UpdateMode;

    /* Configure CNTRL register with new value */
    TIMERx->CNTRL = tmpreg_CNTRL;

    /* Form new value for the BRKETR_CNTRL register */
    tmpreg_BRKETR_CNTRL = (TIMER_CntInitStruct->TIMER_ETR_FilterConf << TIMER_BRKETR_CNTRL_ETR_FILTER_Pos)
                         | TIMER_CntInitStruct->TIMER_ETR_Prescaler
                         | TIMER_CntInitStruct->TIMER_ETR_Polarity
                         | TIMER_CntInitStruct->TIMER_BRK_Polarity;

    /* Configure BRKETR_CNTRL register with new value */
    TIMERx->BRKETR_CNTRL = tmpreg_BRKETR_CNTRL;
}

/**
  * @brief  Fills each TIMER_CntInitStruct member with its default value.
  * @param  TIMER_CntInitStruct: pointer to a @ref TIMER_CntInitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void TIMER_CntStructInit(TIMER_CntInitTypeDef* TIMER_CntInitStruct)
{
    TIMER_CntInitStruct->TIMER_IniCounter       = 0;
    TIMER_CntInitStruct->TIMER_Prescaler        = 0;
    TIMER_CntInitStruct->TIMER_Period           = 0;
    TIMER_CntInitStruct->TIMER_CounterMode      = TIMER_CntMode_ClkFixedDir;
    TIMER_CntInitStruct->TIMER_CounterDirection = TIMER_CntDir_Up;
    TIMER_CntInitStruct->TIMER_EventSource      = TIMER_EvSrc_TIM_CLK;
    TIMER_CntInitStruct->TIMER_FilterSampling   = TIMER_FDTS_TIMER_CLK_div_1;
    TIMER_CntInitStruct->TIMER_ARR_UpdateMode   = TIMER_ARR_Update_Immediately;
    TIMER_CntInitStruct->TIMER_ETR_FilterConf   = TIMER_Filter_1FF_at_TIMER_CLK;
    TIMER_CntInitStruct->TIMER_ETR_Prescaler    = TIMER_ETR_Prescaler_None;
    TIMER_CntInitStruct->TIMER_ETR_Polarity     = TIMER_ETRPolarity_NonInverted;
    TIMER_CntInitStruct->TIMER_BRK_Polarity     = TIMER_BRKPolarity_NonInverted;
}

/**
  * @brief  Enables or disables the specified TIMER peripheral.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  NewState - @ref FunctionalState - new state of the TIMERx peripheral
  * @retval None
  */
void TIMER_Cmd(MDR_TIMER_TypeDef* TIMERx, FunctionalState NewState)
{
    uint32_t tmpreg_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CNTRL = TIMERx->CNTRL;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable TIMERx by setting the CNT_EN bit in the CNTRL register */
        tmpreg_CNTRL |= TIMER_CNTRL_CNT_EN;
    }
    else
    {
        /* Disable TIMERx by clearing the CNT_EN bit in the CNTRL register */
        tmpreg_CNTRL &= ~TIMER_CNTRL_CNT_EN;
    }

    /* Configure CNTRL register with new value */
    TIMERx->CNTRL = tmpreg_CNTRL;
}

/**
  * @brief  Sets the TIMERx Counter Register value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Counter: specifies the Counter register new value.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_SetCounter(MDR_TIMER_TypeDef* TIMERx, uint32_t Counter)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_SetCounter(MDR_TIMER_TypeDef* TIMERx, uint16_t Counter)
#endif
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    TIMERx->CNT = Counter;
}

/**
  * @brief  Returns the TIMERx Counter value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval Counter Register value.
  */
#if defined (USE_K1986VE1xI)
uint32_t TIMER_GetCounter(MDR_TIMER_TypeDef* TIMERx)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
uint16_t TIMER_GetCounter(MDR_TIMER_TypeDef* TIMERx)
#endif
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    return TIMERx->CNT;
}


/**
  * @brief  Sets the TIMERx Prescaler.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Prescaler: specifies the Prescaler Register value.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_SetCntPrescaler(MDR_TIMER_TypeDef* TIMERx, uint32_t Prescaler)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_SetCntPrescaler(MDR_TIMER_TypeDef* TIMERx, uint16_t Prescaler)
#endif
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    TIMERx->PSG = Prescaler;
}

/**
  * @brief  Gets the TIMERx Prescaler.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval Prescaler Register value
  */
#if defined (USE_K1986VE1xI)
uint32_t TIMER_GetCntPrescaler(MDR_TIMER_TypeDef* TIMERx)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
uint16_t TIMER_GetCntPrescaler(MDR_TIMER_TypeDef* TIMERx)
#endif
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    return (TIMERx->PSG);
}

/**
  * @brief  Sets the TIMERx Autoreload Register value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Autoreload: specifies the Autoreload Register value.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_SetCntAutoreload(MDR_TIMER_TypeDef* TIMERx, uint32_t Autoreload)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_SetCntAutoreload(MDR_TIMER_TypeDef* TIMERx, uint16_t Autoreload)
#endif
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    TIMERx->ARR = Autoreload;
}

/**
  * @brief  Sets the TIMERx Autoreload Register (ARR) value and specifies ARR update mode.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Autoreload: specifies the Autoreload Register value.
  * @param  UpdateMode - @ref TIMER_ARR_Update_Mode_TypeDef - specifies the Autoreload Register Update mode.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_CntAutoreloadConfig(MDR_TIMER_TypeDef* TIMERx, uint32_t Autoreload, TIMER_ARR_Update_Mode_TypeDef UpdateMode)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_CntAutoreloadConfig(MDR_TIMER_TypeDef* TIMERx, uint16_t Autoreload, TIMER_ARR_Update_Mode_TypeDef UpdateMode)
#endif
{
    uint32_t tmpreg_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_ARR_UPDATE_MODE(UpdateMode));

    tmpreg_CNTRL = TIMERx->CNTRL;
    tmpreg_CNTRL &= ~TIMER_CNTRL_ARRB_EN;
    tmpreg_CNTRL |= UpdateMode;

    TIMERx->CNTRL = tmpreg_CNTRL;

    TIMERx->ARR = Autoreload;
}

/**
  * @brief  Gets the TIMERx Autoreload Register value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval Autoreload Register value.
  */
#if defined (USE_K1986VE1xI)
uint32_t TIMER_GetCntAutoreload(MDR_TIMER_TypeDef* TIMERx)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
uint16_t TIMER_GetCntAutoreload(MDR_TIMER_TypeDef* TIMERx)
#endif
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    return (TIMERx->ARR);
}

/**
  * @brief  Configures the TIMERx Counter Event source.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  EventSource - @ref TIMER_Event_Src_TypeDef - specifies the Event source.
  * @retval None
  */
void TIMER_CntEventSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Event_Src_TypeDef EventSource)
{
    uint32_t tmpreg_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_EVENT_SOURCE(EventSource));

    tmpreg_CNTRL = TIMERx->CNTRL;
    tmpreg_CNTRL &= ~TIMER_CNTRL_EVENT_SEL_Msk;
    tmpreg_CNTRL |= (uint32_t)EventSource;

    TIMERx->CNTRL = tmpreg_CNTRL;
}

/**
  * @brief  Configures the TIMERx Filter Sampling clock.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Prescaler - @ref TIMER_Filter_Sampl_Clk_TypeDef - specifies the Filter Sampling clock.
  * @retval None
  */
void TIMER_FilterSamplingConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Filter_Sampl_Clk_TypeDef Prescaler)
{
    uint32_t tmpreg_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_FILTER_SAMPLING(Prescaler));

    tmpreg_CNTRL = TIMERx->CNTRL;
    tmpreg_CNTRL &= ~TIMER_CNTRL_FDTS_Msk;
    tmpreg_CNTRL |= (uint32_t)Prescaler;

    TIMERx->CNTRL = tmpreg_CNTRL;
}

/**
  * @brief  Configures the TIMERx count mode.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Mode - @ref TIMER_Counter_Mode_TypeDef - specifies the Timer count mode.
  * @retval None
  */
void TIMER_CounterModeConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Counter_Mode_TypeDef Mode)
{
    uint32_t tmpreg_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_COUNTER_MODE(Mode));

    tmpreg_CNTRL = TIMERx->CNTRL;
    tmpreg_CNTRL &= ~TIMER_CNTRL_CNT_MODE_Msk;
    tmpreg_CNTRL |= (uint32_t)Mode;

    TIMERx->CNTRL = tmpreg_CNTRL;
}

/**
  * @brief  Configures the TIMERx count direction.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Direction - @ref TIMER_Counter_Dir_TypeDef - specifies the Timer count direction.
  * @retval None
  */
void TIMER_SetCounterDirection(MDR_TIMER_TypeDef* TIMERx, TIMER_Counter_Dir_TypeDef Direction)
{
    uint32_t tmpreg_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_COUNTER_DIR(Direction));

    tmpreg_CNTRL = TIMERx->CNTRL;
    tmpreg_CNTRL &= ~TIMER_CNTRL_DIR;
    tmpreg_CNTRL |= (uint32_t)Direction;

    TIMERx->CNTRL = tmpreg_CNTRL;
}

/**
  * @brief  Configures the TIMERx ETR input.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Prescaler - @ref TIMER_ETR_Prescaler_TypeDef - specifies the ETR signal prescaler value.
  * @param  Polarity - @ref TIMER_ETR_Polarity_TypeDef - enables or disables inversion on ETR input.
  * @param  Filter - @ref TIMER_Filter_Config_TypeDef - specifies the ETR Filter configuration.
  * @retval None
  */
void TIMER_ETRInputConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_ETR_Prescaler_TypeDef Prescaler, TIMER_ETR_Polarity_TypeDef Polarity, TIMER_Filter_Config_TypeDef Filter)
{
    uint32_t tmpreg_BRKETR_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_ETR_PRESCALER(Prescaler));
    assert_param(IS_TIMER_ETR_POLARITY(Polarity));
    assert_param(IS_TIMER_FILTER_CONF(Filter));

    tmpreg_BRKETR_CNTRL = TIMERx->BRKETR_CNTRL;
    tmpreg_BRKETR_CNTRL &= ~(TIMER_BRKETR_CNTRL_ETR_PSC_Msk | TIMER_BRKETR_CNTRL_ETR_INV | TIMER_BRKETR_CNTRL_ETR_FILTER_Msk);
    tmpreg_BRKETR_CNTRL |= (uint32_t)Prescaler | (uint32_t)Polarity | ((uint32_t)Filter << TIMER_BRKETR_CNTRL_ETR_FILTER_Pos);

    TIMERx->BRKETR_CNTRL = tmpreg_BRKETR_CNTRL;
}

/**
  * @brief  Configures the TIMERx ETR Filter peripheral.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Filter - @ref TIMER_Filter_Config_TypeDef - specifies the ETR Filter configuration.
  * @retval None
  */
void TIMER_ETRFilterConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Filter_Config_TypeDef Filter)
{
    uint32_t tmpreg_BRKETR_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_FILTER_CONF(Filter));

    tmpreg_BRKETR_CNTRL = TIMERx->BRKETR_CNTRL;
    tmpreg_BRKETR_CNTRL &= ~TIMER_BRKETR_CNTRL_ETR_FILTER_Msk;
    tmpreg_BRKETR_CNTRL |= (uint32_t)Filter << TIMER_BRKETR_CNTRL_ETR_FILTER_Pos;

    TIMERx->BRKETR_CNTRL = tmpreg_BRKETR_CNTRL;
}

/**
  * @brief  Configures the TIMERx ETR signal prescaler value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Prescaler - @ref TIMER_ETR_Prescaler_TypeDef - specifies the ETR signal prescaler value.
  * @retval None
  */
void TIMER_ETRPrescalerConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_ETR_Prescaler_TypeDef Prescaler)
{
    uint32_t tmpreg_BRKETR_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_ETR_PRESCALER(Prescaler));

    tmpreg_BRKETR_CNTRL = TIMERx->BRKETR_CNTRL;
    tmpreg_BRKETR_CNTRL &= ~TIMER_BRKETR_CNTRL_ETR_PSC_Msk;
    tmpreg_BRKETR_CNTRL |= (uint32_t)Prescaler;

    TIMERx->BRKETR_CNTRL = tmpreg_BRKETR_CNTRL;
}

/**
  * @brief  Configures the TIMERx ETR input inversion.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Polarity - @ref TIMER_ETR_Polarity_TypeDef - enables or disables inversion on ETR input.
  * @retval None
  */
void TIMER_ETRPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_ETR_Polarity_TypeDef Polarity)
{
    uint32_t tmpreg_BRKETR_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_ETR_POLARITY(Polarity));

    tmpreg_BRKETR_CNTRL = TIMERx->BRKETR_CNTRL;
    tmpreg_BRKETR_CNTRL &= ~TIMER_BRKETR_CNTRL_ETR_INV;
    tmpreg_BRKETR_CNTRL |= (uint32_t)Polarity;

    TIMERx->BRKETR_CNTRL = tmpreg_BRKETR_CNTRL;
}

/**
  * @brief  Configures the TIMERx BRK input inversion.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Polarity - @ref TIMER_BRK_Polarity_TypeDef - enables or disables inversion on BRK input.
  * @retval None
  */
void TIMER_BRKPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_BRK_Polarity_TypeDef Polarity)
{
    uint32_t tmpreg_BRKETR_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_BRK_POLARITY(Polarity));

    tmpreg_BRKETR_CNTRL = TIMERx->BRKETR_CNTRL;
    tmpreg_BRKETR_CNTRL &= ~TIMER_BRKETR_CNTRL_BRK_INV;
    tmpreg_BRKETR_CNTRL |= (uint32_t)Polarity;

    TIMERx->BRKETR_CNTRL = tmpreg_BRKETR_CNTRL;
}

/**
  * @brief  Returns the TIMERx count direction.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval @ref TIMER_Counter_Dir_TypeDef - The current Timer count direction.
  */
TIMER_Counter_Dir_TypeDef TIMER_GetCounterDirection(MDR_TIMER_TypeDef* TIMERx)
{
    TIMER_Counter_Dir_TypeDef bitstatus;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    if ((TIMERx->CNTRL & TIMER_CNTRL_DIR) == 0)
    {
        bitstatus = TIMER_CntDir_Up;
    }
    else
    {
        bitstatus = TIMER_CntDir_Dn;
    }

    return bitstatus;
}

/**
  * @brief  Checks whether the TIMERx WR_CMPL flag is set or not.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval @ref FlagStatus - The WR_CMPL flag current state.
  */
FlagStatus TIMER_GetCntWriteComplete(MDR_TIMER_TypeDef* TIMERx)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    if ((TIMERx->CNTRL & TIMER_CNTRL_WR_CMPL) == 0)
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
  * @brief  Initializes the TIMERx Channel peripheral according to
  *         the specified parameters in the TIMER_ChnInitStruct.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  TIMER_ChnInitStruct: pointer to a @ref TIMER_ChnInitTypeDef structure
  *         that contains the configuration information for the specified
  *         TIMER Channel peripheral.
  * @retval None
  */
void TIMER_ChnInit(MDR_TIMER_TypeDef* TIMERx, const TIMER_ChnInitTypeDef* TIMER_ChnInitStruct)
{
    uint32_t tmpreg_CH_Number;
    uint32_t tmpreg_CH_CNTRL;
    uint32_t tmpreg_CH_CNTRL2;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(TIMER_ChnInitStruct->TIMER_CH_Number));
    assert_param(IS_TIMER_CHANNEL_MODE(TIMER_ChnInitStruct->TIMER_CH_Mode));
    assert_param(IS_FUNCTIONAL_STATE(TIMER_ChnInitStruct->TIMER_CH_ETR_Ena));
    assert_param(IS_TIMER_CHANNEL_ETR_RESET_CONFIG(TIMER_ChnInitStruct->TIMER_CH_ETR_Reset));
    assert_param(IS_TIMER_CHANNEL_BRK_RESET_CONFIG(TIMER_ChnInitStruct->TIMER_CH_BRK_Reset));
    assert_param(IS_TIMER_CHANNEL_REF_FORMAT(TIMER_ChnInitStruct->TIMER_CH_REF_Format));
    assert_param(IS_TIMER_CHANNEL_PRESCALER(TIMER_ChnInitStruct->TIMER_CH_Prescaler));
    assert_param(IS_TIMER_CHANNEL_EVENT_SOURCE(TIMER_ChnInitStruct->TIMER_CH_EventSource));
    assert_param(IS_TIMER_FILTER_CONF(TIMER_ChnInitStruct->TIMER_CH_FilterConf));
    assert_param(IS_TIMER_CHANNEL_CCR_UPDATE_MODE(TIMER_ChnInitStruct->TIMER_CH_CCR_UpdateMode));
    assert_param(IS_FUNCTIONAL_STATE(TIMER_ChnInitStruct->TIMER_CH_CCR1_Ena));
    assert_param(IS_TIMER_CHANNEL_CCR1_EVENT_SOURCE(TIMER_ChnInitStruct->TIMER_CH_CCR1_EventSource));
#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)
    assert_param(IS_FUNCTIONAL_STATE(TIMER_ChnInitStruct->TIMER_CH_EventDelay));
#endif

    tmpreg_CH_CNTRL = TIMER_ChnInitStruct->TIMER_CH_Mode
                    | TIMER_ChnInitStruct->TIMER_CH_ETR_Reset
                    | TIMER_ChnInitStruct->TIMER_CH_BRK_Reset
                    | TIMER_ChnInitStruct->TIMER_CH_REF_Format
                    | (TIMER_ChnInitStruct->TIMER_CH_Prescaler << TIMER_CH_CNTRL_CHPSC_Pos)
                    | TIMER_ChnInitStruct->TIMER_CH_EventSource
                    | (TIMER_ChnInitStruct->TIMER_CH_FilterConf << TIMER_CH_CNTRL_CHFLTR_Pos);

    if (TIMER_ChnInitStruct->TIMER_CH_ETR_Ena != DISABLE)
    {
        tmpreg_CH_CNTRL |= TIMER_CH_CNTRL_OCCE;
    }

    tmpreg_CH_Number = TIMER_ChnInitStruct->TIMER_CH_Number;

    *(&TIMERx->CH1_CNTRL + tmpreg_CH_Number) = tmpreg_CH_CNTRL;

    tmpreg_CH_CNTRL2 = TIMER_ChnInitStruct->TIMER_CH_CCR_UpdateMode
                     | TIMER_ChnInitStruct->TIMER_CH_CCR1_EventSource;

    if (TIMER_ChnInitStruct->TIMER_CH_CCR1_Ena != DISABLE)
    {
        tmpreg_CH_CNTRL2 |= TIMER_CH_CNTRL2_CCR1_EN;
    }
    
#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)
    if (TIMER_ChnInitStruct->TIMER_CH_EventDelay != DISABLE)
    {
        tmpreg_CH_CNTRL2 |= TIMER_CH_CNTRL2_EV_DELAY;
    }
#endif

    *(&TIMERx->CH1_CNTRL2 + tmpreg_CH_Number) = tmpreg_CH_CNTRL2;
}

/**
  * @brief  Fills each TIMER_ChnInitStruct member with its default value.
  * @param  TIMER_ChnInitStruct: pointer to a @ref TIMER_ChnInitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void TIMER_ChnStructInit(TIMER_ChnInitTypeDef* TIMER_ChnInitStruct)
{
    TIMER_ChnInitStruct->TIMER_CH_Number           = TIMER_CHANNEL1;
    TIMER_ChnInitStruct->TIMER_CH_Mode             = TIMER_CH_MODE_PWM;
    TIMER_ChnInitStruct->TIMER_CH_ETR_Ena          = DISABLE;
    TIMER_ChnInitStruct->TIMER_CH_ETR_Reset        = TIMER_CH_ETR_RESET_Disable;
    TIMER_ChnInitStruct->TIMER_CH_BRK_Reset        = TIMER_CH_BRK_RESET_Disable;
    TIMER_ChnInitStruct->TIMER_CH_REF_Format       = TIMER_CH_REF_Format0;
    TIMER_ChnInitStruct->TIMER_CH_Prescaler        = TIMER_CH_Prescaler_None;
    TIMER_ChnInitStruct->TIMER_CH_EventSource      = TIMER_CH_EvSrc_PE;
    TIMER_ChnInitStruct->TIMER_CH_FilterConf       = TIMER_Filter_1FF_at_TIMER_CLK;
    TIMER_ChnInitStruct->TIMER_CH_CCR_UpdateMode   = TIMER_CH_CCR_Update_Immediately;
    TIMER_ChnInitStruct->TIMER_CH_CCR1_Ena         = DISABLE;
    TIMER_ChnInitStruct->TIMER_CH_CCR1_EventSource = TIMER_CH_CCR1EvSrc_PE;
#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)
    TIMER_ChnInitStruct->TIMER_CH_EventDelay       = ENABLE;
#endif
}

/**
  * @brief  Sets the TIMERx Channel Capture Compare Register (CCR) value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Compare: specifies the Capture Compare Register (CCR) new value.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_SetChnCompare(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_SetChnCompare(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare)
#endif
{
    __IO uint32_t *tmpreg_CCRx;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));

    tmpreg_CCRx = &TIMERx->CCR1 + (uint32_t)Channel;
    *tmpreg_CCRx = Compare;
}

/**
  * @brief  Sets the TIMERx Channel Capture Compare Register (CCR) value and specifies CCR update mode.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Compare: specifies the Capture Compare Register (CCR) new value.
  * @param  UpdateMode - @ref TIMER_CH_CCR_Update_Mode_TypeDef - specifies the Capture Compare Register Update mode.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_ChnCompareConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_ChnCompareConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode)
#endif
{
    __IO uint32_t *tmpreg_CNTRL2x;
    __IO uint32_t *tmpreg_CCRx;
    uint32_t tmpreg_CNTRL2;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_ARR_UPDATE_MODE(UpdateMode));

    tmpreg_CNTRL2x = &TIMERx->CH1_CNTRL2 + (uint32_t)Channel;

    tmpreg_CNTRL2 = *tmpreg_CNTRL2x;
    tmpreg_CNTRL2 &= ~TIMER_CH_CNTRL2_CCRRLD;
    tmpreg_CNTRL2 |= (uint32_t)UpdateMode;
    *tmpreg_CNTRL2x = tmpreg_CNTRL2;

    tmpreg_CCRx = &TIMERx->CCR1 + Channel;

    *tmpreg_CCRx = Compare;
}

/**
  * @brief  Returns the TIMERx Channel Capture Compare Register (CCR) value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @retval Capture Compare Register (CCR) value.
  */
#if defined (USE_K1986VE1xI)
uint32_t TIMER_GetChnCapture(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
uint16_t TIMER_GetChnCapture(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel)
#endif
{
    __IO uint32_t *tmpreg_CCRx;
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));

    tmpreg_CCRx = &TIMERx->CCR1 + (uint32_t)Channel;
    tmpreg = *tmpreg_CCRx;

    return tmpreg;
}

/**
  * @brief  Sets the TIMERx Channel Capture Compare Register1 (CCR1) value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Compare: specifies the Capture Compare Register1 (CCR1) new value.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_SetChnCompare1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_SetChnCompare1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare)
#endif
{
    __IO uint32_t *tmpreg_CCR1x;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));

    tmpreg_CCR1x = &TIMERx->CCR11 + (uint32_t)Channel;
    *tmpreg_CCR1x = Compare;
}

/**
  * @brief  Sets the TIMERx Channel Capture Compare Register1 (CCR1) value and specifies CCR1 update mode.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Compare: specifies the Capture Compare Register1 (CCR1) new value.
  * @param  UpdateMode - @ref TIMER_CH_CCR_Update_Mode_TypeDef - specifies the Capture Compare Register1 Update mode.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_ChnCompare1Config(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_ChnCompare1Config(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint16_t Compare, TIMER_CH_CCR_Update_Mode_TypeDef UpdateMode)
#endif
{
    __IO uint32_t *tmpreg_CNTRL2x;
    __IO uint32_t *tmpreg_CCR1x;
    uint32_t tmpreg_CNTRL2;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_CCR_UPDATE_MODE(UpdateMode));

    tmpreg_CNTRL2x = &TIMERx->CH1_CNTRL2 + (uint32_t)Channel;

    tmpreg_CNTRL2 = *tmpreg_CNTRL2x;
    tmpreg_CNTRL2 &= ~TIMER_CH_CNTRL2_CCRRLD;
    tmpreg_CNTRL2 |= (uint32_t)UpdateMode;
    *tmpreg_CNTRL2x = tmpreg_CNTRL2;

    tmpreg_CCR1x = &TIMERx->CCR11 + (uint32_t)Channel;

    *tmpreg_CCR1x = Compare;
}

/**
  * @brief  Returns the TIMERx Channel Capture Compare Register1 (CCR1) value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @retval Capture Compare Register1 (CCR1) value.
  */
#if defined (USE_K1986VE1xI)
uint32_t TIMER_GetChnCapture1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel)
#elif defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
uint16_t TIMER_GetChnCapture1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel)
#endif
{
    __IO uint32_t *tmpreg_CCR1x;
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));

    tmpreg_CCR1x = &TIMERx->CCR11 + (uint32_t)Channel;
    tmpreg = *tmpreg_CCR1x;

    return tmpreg;
}

/**
  * @brief  Enables or disables the TIMERx Channel ETR input.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  NewState - @ref FunctionalState - new state of the ETR enable (OCCE) bit.
  * @retval None
  */
void TIMER_ChnETR_Cmd(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, FunctionalState NewState)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    uint32_t tmpreg_CH_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    tmpreg_CH_CNTRL = *tmpreg_CH_CNTRLx;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the TIMERx Channel ETR input by setting the OCCE bit in the CHy_CNTRL register */
        tmpreg_CH_CNTRL |= TIMER_CH_CNTRL_OCCE;
    }
    else
    {
        /* Disable the TIMERx Channel ETR input by clearing the OCCE bit in the CHy_CNTRL register */
        tmpreg_CH_CNTRL &= ~TIMER_CH_CNTRL_OCCE;
    }

    /* Configure CHy_CNTRL register with new value */
    *tmpreg_CH_CNTRLx = tmpreg_CH_CNTRL;
}

/**
  * @brief  Enables or disables TIMERx Channel ETR Reset.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  NewState - @ref TIMER_CH_ETR_RESET_TypeDef - new state of the ETR Reset enable.
  * @retval None
  */
void TIMER_ChnETRResetConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_ETR_RESET_TypeDef NewState)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    uint32_t tmpreg_CH_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_ETR_RESET_CONFIG(NewState));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    tmpreg_CH_CNTRL = *tmpreg_CH_CNTRLx;
    tmpreg_CH_CNTRL &= ~TIMER_CH_CNTRL_ETREN;
    tmpreg_CH_CNTRL |= (uint32_t)NewState;
    *tmpreg_CH_CNTRLx = tmpreg_CH_CNTRL;
}

/**
  * @brief  Enables or disables TIMERx Channel BRK Reset.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  NewState - @ref TIMER_CH_BRK_RESET_TypeDef - new state of the BRK Reset enable.
  * @retval None
  */
void TIMER_ChnBRKResetConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_BRK_RESET_TypeDef NewState)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    uint32_t tmpreg_CH_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_BRK_RESET_CONFIG(NewState));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    tmpreg_CH_CNTRL = *tmpreg_CH_CNTRLx;
    tmpreg_CH_CNTRL &= ~TIMER_CH_CNTRL_BRKEN;
    tmpreg_CH_CNTRL |= (uint32_t)NewState;
    *tmpreg_CH_CNTRLx = tmpreg_CH_CNTRL;
}

/**
  * @brief  Configures the TIMERx REF signal format.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Format - @ref TIMER_CH_REF_Format_TypeDef - new value of the TIMERx REF format.
  * @retval None
  */
void TIMER_ChnREFFormatConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_REF_Format_TypeDef Format)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    uint32_t tmpreg_CH_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_REF_FORMAT(Format));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    tmpreg_CH_CNTRL = *tmpreg_CH_CNTRLx;
    tmpreg_CH_CNTRL &= ~TIMER_CH_CNTRL_OCCM_Msk;
    tmpreg_CH_CNTRL |= (uint32_t)Format;
    *tmpreg_CH_CNTRLx = tmpreg_CH_CNTRL;
}

/**
  * @brief  Configures the TIMERx Capture prescaler value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Prescaler - @ref TIMER_CH_Prescaler_TypeDef - specifies the Capture signal prescaler value.
  * @retval None
  */
void TIMER_ChnCapturePrescalerConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_Prescaler_TypeDef Prescaler)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    uint32_t tmpreg_CH_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_PRESCALER(Prescaler));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    tmpreg_CH_CNTRL = *tmpreg_CH_CNTRLx;
    tmpreg_CH_CNTRL &= ~TIMER_CH_CNTRL_CHPSC_Msk;
    tmpreg_CH_CNTRL |= (uint32_t)Prescaler << TIMER_CH_CNTRL_CHPSC_Pos;
    *tmpreg_CH_CNTRLx = tmpreg_CH_CNTRL;
}

/**
  * @brief  Configures the TIMERx Channel Capture Event source.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  EventSource - @ref TIMER_CH_Event_Src_TypeDef - specifies the Event source.
  * @retval None
  */
void TIMER_ChnEventSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_Event_Src_TypeDef EventSource)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    uint32_t tmpreg_CH_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_EVENT_SOURCE(EventSource));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    tmpreg_CH_CNTRL = *tmpreg_CH_CNTRLx;
    tmpreg_CH_CNTRL &= ~TIMER_CH_CNTRL_CHSEL_Msk;
    tmpreg_CH_CNTRL |= (uint32_t)EventSource;
    *tmpreg_CH_CNTRLx = tmpreg_CH_CNTRL;
}

#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)
/**
  * @brief  Enables or disables synchronization between the capture event and the update of registers CCR and CCR1.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  NewState - @ref FunctionalState - new state of the EV_DELAY bit.
  * @retval None
  */
void TIMER_ChnEventDelay_Cmd(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, FunctionalState NewState)
{
    __IO uint32_t *tmpreg_CH_CNTRL2x;
    uint32_t tmpreg_CH_CNTRL2;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CH_CNTRL2x = &TIMERx->CH1_CNTRL2 + (uint32_t)Channel;

    tmpreg_CH_CNTRL2 = *tmpreg_CH_CNTRL2x;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the TIMERx Channel event delay by setting the EV_DELAY bit in the CHy_CNTRL2 register */
        tmpreg_CH_CNTRL2 |= TIMER_CH_CNTRL2_EV_DELAY;
    }
    else
    {
        /* Disable the TIMERx Channel event delay by clearing the EV_DELAY bit in the CHy_CNTRL2 register */
        tmpreg_CH_CNTRL2 &= ~TIMER_CH_CNTRL2_EV_DELAY;
    }

    /* Configure CHy_CNTRL2 register with new value */
    *tmpreg_CH_CNTRL2x = tmpreg_CH_CNTRL2;
}
#endif

/**
  * @brief  Configures the TIMERx Channel Filter peripheral.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Filter - @ref TIMER_Filter_Config_TypeDef - specifies the Channel Filter configuration.
  * @retval None
  */
void TIMER_ChnFilterConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_Filter_Config_TypeDef Filter)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    uint32_t tmpreg_CH_CNTRL;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_FILTER_CONF(Filter));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    tmpreg_CH_CNTRL = *tmpreg_CH_CNTRLx;
    tmpreg_CH_CNTRL &= ~TIMER_CH_CNTRL_CHFLTR_Msk;
    tmpreg_CH_CNTRL |= (uint32_t)Filter << TIMER_CH_CNTRL_CHFLTR_Pos;
    *tmpreg_CH_CNTRLx = tmpreg_CH_CNTRL;
}

/**
  * @brief  Checks whether the TIMERx CCR write operation is complete or not.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @retval @ref FlagStatus - The TIMERx_CHy_CNTRL.WR_CMPL flag current state (SET or RESET).
  */
FlagStatus TIMER_GetChnWriteComplete(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    if ((*tmpreg_CH_CNTRLx & TIMER_CH_CNTRL_WR_CMPL) == 0)
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
  * @brief  Checks whether the TIMERx CCR1 write operation is complete or not.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @retval @ref FlagStatus - The TIMERx_CHy_CNTRL.WR_CMPL1 flag current state (SET or RESET).
  */
FlagStatus TIMER_GetChnWriteComplete1(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel)
{
    __IO uint32_t *tmpreg_CH_CNTRLx;
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));

    tmpreg_CH_CNTRLx = &TIMERx->CH1_CNTRL + (uint32_t)Channel;

    if ((*tmpreg_CH_CNTRLx & TIMER_CH_CNTRL_WR_CMPL1) == 0)
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
  * @brief  Configures the TIMERx Channel Capture1 Event source.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  EventSource - @ref TIMER_CH_CCR1_Event_Src_TypeDef - specifies the Event source.
  * @retval None
  */
void TIMER_ChnCCR1_EventSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_CCR1_Event_Src_TypeDef EventSource)
{
    __IO uint32_t *tmpreg_CH_CNTRL2x;
    uint32_t tmpreg_CH_CNTRL2;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_CCR1_EVENT_SOURCE(EventSource));

    tmpreg_CH_CNTRL2x = &TIMERx->CH1_CNTRL2 + (uint32_t)Channel;

    tmpreg_CH_CNTRL2 = *tmpreg_CH_CNTRL2x;
    tmpreg_CH_CNTRL2 &= ~TIMER_CH_CNTRL2_CHSEL1_Msk;
    tmpreg_CH_CNTRL2 |= (uint32_t)EventSource;
    *tmpreg_CH_CNTRL2x = tmpreg_CH_CNTRL2;
}

/**
  * @brief  Enables or disables the TIMERx Channel CCR1 register.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  NewState - @ref FunctionalState - new state of the CCR1_EN bit.
  * @retval None
  */
void TIMER_ChnCCR1_Cmd(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, FunctionalState NewState)
{
    __IO uint32_t *tmpreg_CH_CNTRL2x;
    uint32_t tmpreg_CH_CNTRL2;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_CH_CNTRL2x = &TIMERx->CH1_CNTRL2 + (uint32_t)Channel;

    tmpreg_CH_CNTRL2 = *tmpreg_CH_CNTRL2x;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the TIMERx Channel CCR1 register by setting the CCR1_EN bit in the CHy_CNTRL2 register */
        tmpreg_CH_CNTRL2 |= TIMER_CH_CNTRL2_CCR1_EN;
    }
    else
    {
        /* Disable the TIMERx Channel CCR1 register by clearing the CCR1_EN bit in the CHy_CNTRL2 register */
        tmpreg_CH_CNTRL2 &= ~TIMER_CH_CNTRL2_CCR1_EN;
    }

    /* Configure CHy_CNTRL2 register with new value */
    *tmpreg_CH_CNTRL2x = tmpreg_CH_CNTRL2;
}

/**
  * @brief  Initializes the TIMERx Channel Output peripheral according to
  *         the specified parameters in the TIMER_ChnOutInitStruct.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  TIMER_ChnOutInitStruct: pointer to a @ref TIMER_ChnOutInitTypeDef structure
  *         that contains the configuration information for the specified
  *         TIMER Channel peripheral.
  * @retval None
  */
void TIMER_ChnOutInit(MDR_TIMER_TypeDef* TIMERx, const TIMER_ChnOutInitTypeDef* TIMER_ChnOutInitStruct)
{
    uint32_t tmpreg_CH_Number;
    uint32_t tmpreg_CH_CNTRL1;
    uint32_t tmpreg_CH_DTG;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(TIMER_ChnOutInitStruct->TIMER_CH_Number));
    assert_param(IS_TIMER_CHO_POLARITY(TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Polarity));
    assert_param(IS_TIMER_CHO_SOURCE(TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Source));
    assert_param(IS_TIMER_CHO_MODE(TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Mode));
    assert_param(IS_TIMER_CHO_POLARITY(TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Polarity));
    assert_param(IS_TIMER_CHO_SOURCE(TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Source));
    assert_param(IS_TIMER_CHO_MODE(TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Mode));
    assert_param(IS_TIMER_CHANNEL_DTG_MAIN_PRESCALER(TIMER_ChnOutInitStruct->TIMER_CH_DTG_MainPrescaler));
    assert_param(IS_TIMER_CHANNEL_DTG_AUX_PRESCALER(TIMER_ChnOutInitStruct->TIMER_CH_DTG_AuxPrescaler));
    assert_param(IS_TIMER_CHANNEL_DTG_CLK_SOURCE(TIMER_ChnOutInitStruct->TIMER_CH_DTG_ClockSource));

    tmpreg_CH_CNTRL1 = (TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Polarity << TIMER_CH_CNTRL1_INV_Pos)
                     | (TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Source   << TIMER_CH_CNTRL1_SELO_Pos)
                     | (TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Mode     << TIMER_CH_CNTRL1_SELOE_Pos)
                     | (TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Polarity << TIMER_CH_CNTRL1_NINV_Pos)
                     | (TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Source   << TIMER_CH_CNTRL1_NSELO_Pos)
                     | (TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Mode     << TIMER_CH_CNTRL1_NSELOE_Pos);

    tmpreg_CH_Number = TIMER_ChnOutInitStruct->TIMER_CH_Number;

    *(&TIMERx->CH1_CNTRL1 + tmpreg_CH_Number) = tmpreg_CH_CNTRL1;

    tmpreg_CH_DTG = (TIMER_ChnOutInitStruct->TIMER_CH_DTG_MainPrescaler << TIMER_CH_DTG_Pos)
                  | (TIMER_ChnOutInitStruct->TIMER_CH_DTG_AuxPrescaler  << TIMER_CH_DTGX_Pos)
                  |  TIMER_ChnOutInitStruct->TIMER_CH_DTG_ClockSource;

    *(&TIMERx->CH1_DTG + tmpreg_CH_Number) = tmpreg_CH_DTG;
}

/**
  * @brief  Fills each TIMER_ChnOutInitStruct member with its default value.
  * @param  TIMER_ChnOutInitStruct: pointer to a @ref TIMER_ChnOutInitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void TIMER_ChnOutStructInit(TIMER_ChnOutInitTypeDef* TIMER_ChnOutInitStruct)
{
    TIMER_ChnOutInitStruct->TIMER_CH_Number            = TIMER_CHANNEL1;
    TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Polarity   = TIMER_CHOPolarity_NonInverted;
    TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Source     = TIMER_CH_OutSrc_Only_0;
    TIMER_ChnOutInitStruct->TIMER_CH_DirOut_Mode       = TIMER_CH_OutMode_Input;
    TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Polarity   = TIMER_CHOPolarity_NonInverted;
    TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Source     = TIMER_CH_OutSrc_Only_0;
    TIMER_ChnOutInitStruct->TIMER_CH_NegOut_Mode       = TIMER_CH_OutMode_Input;
    TIMER_ChnOutInitStruct->TIMER_CH_DTG_MainPrescaler = 0;
    TIMER_ChnOutInitStruct->TIMER_CH_DTG_AuxPrescaler  = 0;
    TIMER_ChnOutInitStruct->TIMER_CH_DTG_ClockSource   = TIMER_CH_DTG_ClkSrc_TIMER_CLK;
}

/**
  * @brief  Configures the TIMERx Channel output.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  OutSource - @ref TIMER_CH_OUT_Src_TypeDef - specifies the Channel Output source.
  * @param  Mode - @ref TIMER_CH_OUT_Mode_TypeDef - specifies the Channel Output Enable source.
  * @param  Polarity - @ref TIMER_CH_OUT_Polarity_TypeDef - enables or disables the Channel Output inversion.
  * @retval None
  */
void TIMER_ChnOutConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource, TIMER_CH_OUT_Mode_TypeDef Mode, TIMER_CH_OUT_Polarity_TypeDef Polarity)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_SOURCE(OutSource));
    assert_param(IS_TIMER_CHO_MODE(Mode));
    assert_param(IS_TIMER_CHO_POLARITY(Polarity));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + (uint32_t)Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~(TIMER_CH_CNTRL1_INV | TIMER_CH_CNTRL1_SELO_Msk | TIMER_CH_CNTRL1_SELOE_Msk);
    tmpreg_CH_CNTRL1 |= ((uint32_t)Polarity  << TIMER_CH_CNTRL1_INV_Pos)
                      | ((uint32_t)OutSource << TIMER_CH_CNTRL1_SELO_Pos)
                      | ((uint32_t)Mode      << TIMER_CH_CNTRL1_SELOE_Pos);

    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx Channel Output source.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  OutSource - @ref TIMER_CH_OUT_Src_TypeDef - specifies the Channel Output source.
  * @retval None
  */
void TIMER_ChnOutSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_SOURCE(OutSource));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + (uint32_t)Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~TIMER_CH_CNTRL1_SELO_Msk;
    tmpreg_CH_CNTRL1 |= (uint32_t)OutSource << TIMER_CH_CNTRL1_SELO_Pos;
    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx Channel Output mode.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Mode - @ref TIMER_CH_OUT_Mode_TypeDef - specifies the Channel Output Enable source.
  * @retval None
  */
void TIMER_ChnOutModeConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Mode_TypeDef Mode)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_MODE(Mode));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + (uint32_t)Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~TIMER_CH_CNTRL1_SELOE_Msk;
    tmpreg_CH_CNTRL1 |= (uint32_t)Mode << TIMER_CH_CNTRL1_SELOE_Pos;
    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx Channel Output polarity.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Polarity - @ref TIMER_CH_OUT_Polarity_TypeDef - enables or disables the Channel Output inversion.
  * @retval None
  */
void TIMER_ChnOutPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Polarity_TypeDef Polarity)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_POLARITY(Polarity));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + (uint32_t)Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~TIMER_CH_CNTRL1_INV;
    tmpreg_CH_CNTRL1 |= (uint32_t)Polarity << TIMER_CH_CNTRL1_INV_Pos;
    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx NChannel output.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  OutSource - @ref TIMER_CH_OUT_Src_TypeDef - specifies the NChannel Output source.
  * @param  Mode - @ref TIMER_CH_OUT_Mode_TypeDef - specifies the NChannel Output Enable source.
  * @param  Polarity - @ref TIMER_CH_OUT_Polarity_TypeDef - enables or disables the NChannel Output inversion.
  * @retval None
  */
void TIMER_ChnNOutConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource, TIMER_CH_OUT_Mode_TypeDef Mode, TIMER_CH_OUT_Polarity_TypeDef Polarity)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_SOURCE(OutSource));
    assert_param(IS_TIMER_CHO_MODE(Mode));
    assert_param(IS_TIMER_CHO_POLARITY(Polarity));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + (uint32_t)Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~(TIMER_CH_CNTRL1_NINV | TIMER_CH_CNTRL1_NSELO_Msk | TIMER_CH_CNTRL1_NSELOE_Msk);
    tmpreg_CH_CNTRL1 |= ((uint32_t)Polarity  << TIMER_CH_CNTRL1_NINV_Pos)
                      | ((uint32_t)OutSource << TIMER_CH_CNTRL1_NSELO_Pos)
                      | ((uint32_t)Mode      << TIMER_CH_CNTRL1_NSELOE_Pos);
    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx NChannel Output source.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  OutSource - @ref TIMER_CH_OUT_Src_TypeDef - specifies the NChannel Output source.
  * @retval None
  */
void TIMER_ChnNOutSourceConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Src_TypeDef OutSource)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_SOURCE(OutSource));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + (uint32_t)Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~TIMER_CH_CNTRL1_NSELO_Msk;
    tmpreg_CH_CNTRL1 |= (uint32_t)OutSource << TIMER_CH_CNTRL1_NSELO_Pos;
    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx NChannel Output mode.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Mode - @ref TIMER_CH_OUT_Mode_TypeDef - specifies the NChannel Output Enable source.
  * @retval None
  */
void TIMER_ChnNOutModeConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Mode_TypeDef Mode)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_MODE(Mode));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + (uint32_t)Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~TIMER_CH_CNTRL1_NSELOE_Msk;
    tmpreg_CH_CNTRL1 |= (uint32_t)Mode << TIMER_CH_CNTRL1_NSELOE_Pos;
    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx NChannel Output polarity.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  Polarity - @ref TIMER_CH_OUT_Polarity_TypeDef - enables or disables the NChannel Output inversion.
  * @retval None
  */
void TIMER_ChnNOutPolarityConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, TIMER_CH_OUT_Polarity_TypeDef Polarity)
{
    __IO uint32_t *tmpreg_CH_CNTRL1x;
    uint32_t tmpreg_CH_CNTRL1;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHO_POLARITY(Polarity));

    tmpreg_CH_CNTRL1x = &TIMERx->CH1_CNTRL1 + Channel;

    tmpreg_CH_CNTRL1 = *tmpreg_CH_CNTRL1x;
    tmpreg_CH_CNTRL1 &= ~TIMER_CH_CNTRL1_NINV;
    tmpreg_CH_CNTRL1 |= (uint32_t)Polarity << TIMER_CH_CNTRL1_NINV_Pos;
    *tmpreg_CH_CNTRL1x = tmpreg_CH_CNTRL1;
}

/**
  * @brief  Configures the TIMERx Dead Time Generator (DTG) peripheral.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Channel - @ref TIMER_Channel_Number_TypeDef - specifies the Timer Channel number.
  * @param  MainPrescaler: specifies the Main Prescaler value.
  *         This parameter can be a number between 0x0000 and 0x00FF.
  * @param  AuxPrescaler: specifies the Auxiliary Prescaler value.
  *         This parameter can be a number between 0x0000 and 0x000F.
  * @param  ClockSource - @ref TIMER_CH_DTG_Clk_Src_TypeDef - specifies the DTG clock source.
  * @retval None
  */
void TIMER_ChnOutDTGConfig(MDR_TIMER_TypeDef* TIMERx, TIMER_Channel_Number_TypeDef Channel, uint32_t MainPrescaler, uint32_t AuxPrescaler, TIMER_CH_DTG_Clk_Src_TypeDef ClockSource)
{
    uint32_t tmpreg_CH_DTG;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CHANNEL_NUMBER(Channel));
    assert_param(IS_TIMER_CHANNEL_DTG_MAIN_PRESCALER(MainPrescaler));
    assert_param(IS_TIMER_CHANNEL_DTG_AUX_PRESCALER(AuxPrescaler));
    assert_param(IS_TIMER_CHANNEL_DTG_CLK_SOURCE(ClockSource));

    tmpreg_CH_DTG = (MainPrescaler << TIMER_CH_DTG_Pos)
                  | (AuxPrescaler  << TIMER_CH_DTGX_Pos)
                  | (uint32_t)ClockSource;

    *(&TIMERx->CH1_DTG + (uint32_t)Channel) = tmpreg_CH_DTG;
}

/**
  * @brief  Returns the TIMERx Status Register value.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval The TIMERx_STATUS Register value.
  */
uint32_t TIMER_GetStatus(MDR_TIMER_TypeDef* TIMERx)
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));

    return (TIMERx->STATUS);
}

/**
  * @brief  Checks whether the specified TIMERx Status flag is set or not.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Flag - @ref TIMER_Status_Flags_TypeDef - specifies the flag to check.
  * @retval @ref FlagStatus - Current Status flag state (SET or RESET).
  */
FlagStatus TIMER_GetFlagStatus(MDR_TIMER_TypeDef* TIMERx, TIMER_Status_Flags_TypeDef Flag)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_STATUS_FLAG(Flag));

    if ((TIMERx->STATUS & Flag) == 0)
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
  * @brief  Clears the TIMERx's pending flags.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Flags: specifies the flag bit mask to clear.
  *         This parameter can be any combination of @ref TIMER_Status_Flags_TypeDef values.
  * @retval None
  */
void TIMER_ClearFlag(MDR_TIMER_TypeDef* TIMERx, uint32_t Flags)
{
    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_STATUS(Flags));

    TIMERx->STATUS = ~Flags;
}

/**
  * @brief  Enables or disables the TIMERx's DMA Requests.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  TIMER_DMASource - @ref TIMER_Status_Flags_TypeDef - specifies the DMA Request sources.
  * @param  TIMER_DMAChannel - @ref TIMER_DMA_Channel_TypeDef - specifies the DMA channel.
  *         @note TIMER_DMAChannel available only for MDR32F1QI, K1986VE1xI
  * @param  NewState - @ref FunctionalState - new state of the DMA Request sources.
  * @retval None
  */
#if defined (USE_K1986VE1xI)
void TIMER_DMACmd(MDR_TIMER_TypeDef* TIMERx, uint32_t TIMER_DMASource, TIMER_DMA_Channel_TypeDef TIMER_DMAChannel, FunctionalState NewState)
#endif
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
void TIMER_DMACmd(MDR_TIMER_TypeDef* TIMERx, uint32_t TIMER_DMASource, FunctionalState NewState)
#endif
{
    uint32_t tmpreg_DMA_RE;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_STATUS(TIMER_DMASource));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
#if defined (USE_K1986VE1xI)
    assert_param(IS_TIMER_DMA_Channel(TIMER_DMAChannel));

    if(TIMER_DMAChannel == TIMER_DMA_Channel0)
    {
#endif
        tmpreg_DMA_RE = TIMERx->DMA_RE;

        /* Form new value */
        if (NewState != DISABLE)
        {
            /* Enable the TIMERx DMA Requests by setting bits in the DMA_RE register */
            tmpreg_DMA_RE |= TIMER_DMASource;
        }
        else
        {
            /* Disable the TIMERx DMA Requests by clearing bits in the DMA_RE register */
            tmpreg_DMA_RE &= ~TIMER_DMASource;
        }

        /* Configure DMA_RE register with new value */
        TIMERx->DMA_RE = tmpreg_DMA_RE;
#if defined (USE_K1986VE1xI)
    }
    else
    {
        tmpreg_DMA_RE = TIMERx->DMA_REChx[TIMER_DMAChannel];

        /* Form new value */
        if (NewState != DISABLE)
        {
            /* Enable the TIMERx DMA Requests by setting bits in the DMA_REx register */
            tmpreg_DMA_RE |= TIMER_DMASource;
        }
        else
        {
            /* Disable the TIMERx DMA Requests by clearing bits in the DMA_REx register */
            tmpreg_DMA_RE &= ~TIMER_DMASource;
        }

        /* Configure DMA_REx register with new value */
        TIMERx->DMA_REChx[TIMER_DMAChannel] = tmpreg_DMA_RE;

    }
#endif
}

/**
  * @brief  Enables or disables the specified TIMER interrupts.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  TIMER_IT: specifies the TIMER interrupts sources to be enabled or disabled.
  *         This parameter can be any combination of @ref TIMER_Status_Flags_TypeDef values.
  * @param  NewState - @ref FunctionalState - new state of the TIMER interrupts.
  * @retval None
  */
void TIMER_ITConfig(MDR_TIMER_TypeDef* TIMERx, uint32_t TIMER_IT, FunctionalState NewState)
{
    uint32_t tmpreg_IE;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_STATUS(TIMER_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    tmpreg_IE = TIMERx->IE;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the TIMERx Interrupt requests by setting bits in the IE register */
        tmpreg_IE |= TIMER_IT;
    }
    else
    {
        /* Disable the TIMERx Interrupt requests by clearing bits in the IE register */
        tmpreg_IE &= ~TIMER_IT;
    }

    /* Configure IE register with new value */
    TIMERx->IE = tmpreg_IE;
}

/**
  * @brief  Checks whether the TIMERx interrupt has occurred or not.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  TIMER_IT - @ref TIMER_Status_Flags_TypeDef - specifies the TIMER interrupt source to check.
  * @retval @ref ITStatus - The state of the TIMER_IT (SET or RESET).
  */
ITStatus TIMER_GetITStatus(MDR_TIMER_TypeDef* TIMERx, TIMER_Status_Flags_TypeDef TIMER_IT)
{
    ITStatus bitstatus;
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_STATUS_FLAG(TIMER_IT));

    tmpreg = TIMERx->IE;
    tmpreg = TIMERx->STATUS & tmpreg & (uint32_t)TIMER_IT;

    if (tmpreg == 0)
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
  * @brief  Initializes the TIMERx peripheral Clock according to the
  *         specified parameters.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  TIMER_BRG - @ref TIMER_Clock_BRG_TypeDef - specifies the HCLK division factor.
  * @retval None
  */
void TIMER_BRGInit(MDR_TIMER_TypeDef* TIMERx, TIMER_Clock_BRG_TypeDef TIMER_BRG)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_TIMER_ALL_PERIPH(TIMERx));
    assert_param(IS_TIMER_CLOCK_BRG(TIMER_BRG));

#if defined (USE_K1986VE1xI)
    if(TIMERx == MDR_TIMER4)
        tmpreg = MDR_RST_CLK->UART_CLOCK;
    else
#endif /* #if defined (USE_K1986VE1xI) */
        tmpreg = MDR_RST_CLK->TIM_CLOCK;

    if(TIMERx == MDR_TIMER1)
    {
        tmpreg &= ~RST_CLK_TIM_CLOCK_TIM1_BRG_Msk;
        tmpreg |= TIMER_BRG << RST_CLK_TIM_CLOCK_TIM1_BRG_Pos;
        tmpreg |= RST_CLK_TIM_CLOCK_TIM1_CLK_EN;
    }
    else if(TIMERx == MDR_TIMER2)
    {
        tmpreg &= ~RST_CLK_TIM_CLOCK_TIM2_BRG_Msk;
        tmpreg |= TIMER_BRG << RST_CLK_TIM_CLOCK_TIM2_BRG_Pos;
        tmpreg |= RST_CLK_TIM_CLOCK_TIM2_CLK_EN;
    }
    else if(TIMERx == MDR_TIMER3)
    {
        tmpreg &= ~RST_CLK_TIM_CLOCK_TIM3_BRG_Msk;
        tmpreg |= TIMER_BRG << RST_CLK_TIM_CLOCK_TIM3_BRG_Pos;
        tmpreg |= RST_CLK_TIM_CLOCK_TIM3_CLK_EN;
    }
#if defined (USE_K1986VE1xI)
    else if(TIMERx == MDR_TIMER4)
    {
        tmpreg &= ~RST_CLK_UART_CLOCK_TIM4_BRG_Msk;
        tmpreg |= TIMER_BRG << RST_CLK_UART_CLOCK_TIM4_BRG_Pos;
        tmpreg |= RST_CLK_UART_CLOCK_TIM4_CLK_EN;
    }

    if(TIMERx == MDR_TIMER4)
        MDR_RST_CLK->UART_CLOCK = tmpreg;
    else
#endif /* #if defined (USE_K1986VE1xI) */
        MDR_RST_CLK->TIM_CLOCK = tmpreg;
}

/** @} */ /* End of group TIMER_Exported_Functions */

/** @} */ /* End of group TIMER */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_timer.c */


