/**
  ******************************************************************************
  * @file    MDR32FxQI_dsp.c
  * @author  Milandr Application Team
  * @version V2.0.4i
  * @date    22/07/2024
  * @brief   This file contains all the DSP firmware functions for MCU MDR32FG16S1QI.
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
#include "MDR32FxQI_dsp.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

#if defined (USE_MDR32FG16S1QI)
/** @defgroup DSP DSP
  * @{
  */

/** @defgroup DSP_Private_Constant DSP Private Constant
  * @{
  */

/* RCC_CLK registers bit address in the alias region */
#define RST_CLK_OFFSET              (MDR_RST_CLK_BASE - PERIPH_BASE)

#define SFR_OFFSET(TP, MOD, SFR)    ((uint32_t)&((TP*)MOD)->SFR)
#define BB_ADDR(TP, MOD, SFR, BIT)  (PERIPH_BB_BASE + SFR_OFFSET(TP, MOD, SFR)*32 + BIT*4)
#define RST_CLK_BB(SFR, BIT)        BB_ADDR(MDR_RST_CLK_TypeDef, RST_CLK_OFFSET, SFR, BIT)

#define RST_DSP_BB                  RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_RST_DSP_Pos)
#define RST_DSP_CPU_BB              RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_RST_DSP_CPU_Pos)
#define RST_DSP_MEM_BB              RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_RST_DSP_MEM_Pos)
#define RST_DSP_PER_BB              RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_RST_DSP_PER_Pos)
#define RST_DSP_HOLD_BB             RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_HOLD_Pos)
#define RST_DSP_BIO_BB              RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_BIO_Pos)
#define RST_DSP_XF_BB               RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_XF_Pos)
#define RST_DSP_HOLDA_BB            RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_HOLDA_Pos)
#define RST_DSP_BRTRD_BB            RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_BRTRD_Pos)
#define RST_DSP_RD_BUF_EN_BB        RST_CLK_BB(DSP_CONTROL_STATUS, RST_CLK_DSP_CONTROL_STATUS_RD_BUFF_EN_Pos)

/** @} */ /* End of group DSP_Private_Constant */


/** @defgroup DSP_Exported_Functions DSP Exported Functions
  * @{
  */

/**
  * @brief  Disable all clock of the DSP core.
  * @param  None
  * @retval None
  */
void DSP_DeInit(void)
{
    *(__IO uint32_t *)RST_DSP_RD_BUF_EN_BB   = (uint32_t)DISABLE;
    *(__IO uint32_t *)RST_DSP_PER_BB         = (uint32_t)ENABLE;
    *(__IO uint32_t *)RST_DSP_MEM_BB         = (uint32_t)ENABLE;
    *(__IO uint32_t *)RST_DSP_CPU_BB         = (uint32_t)ENABLE;
    *(__IO uint32_t *)RST_DSP_BB             = (uint32_t)ENABLE;
}

/**
  * @brief  Config the DSP core state.
  * @param  NewState - @ref FunctionalState - the new state of the DSP core clock.
  * @retval None
  */
void DSP_RST_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *) RST_DSP_BB = (uint32_t)!NewState;
}

/**
  * @brief  Enable or Disable the DSP core.
  * @param  NewState - @ref FunctionalState - the new state of the DSP CPU core.
  * @retval None.
  */
void DSP_CPU_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *) RST_DSP_CPU_BB = (uint32_t)!NewState;
}

/**
  * @brief  Enable or disable the DSP memory block.
  * @param  NewState - @ref FunctionalState - the new state of the DSP memory block.
  * @retval None.
  */
void DSP_MEM_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *) RST_DSP_MEM_BB = (uint32_t)!NewState;
}

/**
  * @brief  Config the peripheral DSP block.
  * @param  NewState - @ref FunctionalState - the new state of the peripheral DSP block.
  * @retval None.
  */
void DSP_PER_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *) RST_DSP_PER_BB = (uint32_t)!NewState;
}

/**
  * @brief  Config read prefetch address space DSP.
  * @param  NewState - @ref FunctionalState - the new state of the RD_Buff_EN bit.
  * @retval None.
  */
void DSP_ReadBufferCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *) RST_DSP_RD_BUF_EN_BB = (uint32_t)NewState;
}

/**
  * @brief  Get DSP flag status.
  * @param  flag - @ref DSP_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - Current state of the DSP checked flag (SET or RESET).
  */
FlagStatus DSP_GetStatus(DSP_Flags flag)
{
    FlagStatus state;
    /* Check the parameters */
    assert_param(IS_DSP_FLAG(flag));

    switch (flag)
    {
        case DSP_FLAG_BIO:
            state = (FlagStatus) *(__IO uint32_t *)RST_DSP_BIO_BB;
            break;
        case DSP_FLAG_XF:
            state = (FlagStatus) *(__IO uint32_t *)RST_DSP_XF_BB;
            break;
        case DSP_FLAG_HOLDA:
            state = (FlagStatus) *(__IO uint32_t *)RST_DSP_HOLDA_BB;
            break;
        case DSP_FLAG_BRTRD:
            state = (FlagStatus) *(__IO uint32_t *)RST_DSP_BRTRD_BB;
            break;
        default:
            state = RESET;
            break;
    }

    return(state);
}

/**
  * @brief  Enables or disables the DSP peripheral modules clock.
  * @param  DSP_CLK_PCLK: specifies the peripheral to gates its clock.
  *         This parameter can be any combination of the @ref DSP_CLK_PCLKx values.
  * @param  NewState - @ref FunctionalState - new state of the specified peripheral clock.
  * @retval None.
  */
void DSP_CLKPCLKCmd(uint32_t DSP_CLK_PCLK, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    assert_param(IS_DSP_CLK_PCLK(DSP_CLK_PCLK));

    if(NewState == ENABLE)
    {
        MDR_DSP_CORE->CLKCMD |= (uint32_t)DSP_CLK_PCLK;
    }
    else
    {
        MDR_DSP_CORE->CLKCMD &= ~((uint32_t)DSP_CLK_PCLK);
    }
}

/**
  * @brief  Config the interrupt from RISC to DSP.
  * @param  Interrupt: specifies the peripheral interrupt, that should be set or reset.
  *         This parameter can be any combination of the @ref DSP_AIRQ values.
  * @param  NewStatus - @ref FlagStatus - new state of the specified peripheral clock.
  * @retval None.
  */
void DSP_RISCToDSPInterruptCmd(uint16_t Interrupt, FlagStatus NewStatus)
{
    /* Check the parameters */
    assert_param(IS_BIT_STATUS(NewStatus));
    assert_param(IS_DSP_AIRQ(Interrupt));

    MDR_DSP_IRQ->AIR = Interrupt | ((NewStatus == SET) << DSP_CORE_AIR_SNR_Pos);
}

/**
  * @brief  Config the DMA events from RISC to DSP.
  * @param  ADMA: specifies the DMA events, that should be set or reset.
  *         This parameter can be any combination of the @ref DSP_ADMA values.
  * @param  NewStatus - @ref FlagStatus - the new status of the specified event.
  * @retval None.
  */
void DSP_DMA_RISC_Cmd(uint16_t ADMA, FlagStatus NewStatus)
{
    /* Check the parameters */
    assert_param(IS_BIT_STATUS(NewStatus));
    assert_param(IS_DSP_ADMA(ADMA));

    MDR_DSP_IRQ->AIR = ADMA | ((NewStatus == SET) << DSP_CORE_AIR_SNR_Pos)
                            | DSP_CORE_AIR_SID;
}


/**
  * @brief  Config the interrupts form DSP to RISC.
  * @param  Interrupt: specifies the DSP interrupt, that should be ste or reset.
  *         This parameter can be any combination of the @ref DSP_DIR values.
  * @param  NewStatus - @ref FlagStatus - the new status of the specified interrupt.
  * @retval None.
  */
void DSP_DSPToRISCInterruptCmd(uint16_t Interrupt, FlagStatus NewStatus)
{
    /* Check the parameters */
    assert_param(IS_BIT_STATUS(NewStatus));
    assert_param(IS_DSP_DIR(Interrupt));

    MDR_DSP_IRQ->DIR = Interrupt | ((NewStatus == SET) << DSP_CORE_DIR_SNR_Pos);
}

/** @} */ /* End of group DSP_Exported_Functions */

/** @} */ /* End of group DSP */
#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_dsp.c */

