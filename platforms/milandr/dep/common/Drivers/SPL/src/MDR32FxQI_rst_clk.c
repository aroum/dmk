/**
  ******************************************************************************
  * @file    MDR32FxQI_rst_clk.c
  * @author  Milandr Application Team
  * @version V2.2.0i
  * @date    19/11/2024
  * @brief   This file contains all the RST_CLK firmware functions.
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
#include "MDR32FxQI_rst_clk.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup RST_CLK RST_CLK
  * @{
  */

/** @defgroup RST_CLK_Private_Constants RST_CLK Private Constants
  * @{
  */

/* RCC_CLK registers bit address in the alias region */
#define RST_CLK_OFFSET              (MDR_RST_CLK_BASE - PERIPH_BASE)
#define BKP_OFFSET                  (MDR_BKP_BASE - PERIPH_BASE)

#define SFR_OFFSET(TP, MOD, SFR)    ((uint32_t) & ((TP*)MOD)->SFR)
#define BB_ADDR(TP, MOD, SFR, BIT)  (PERIPH_BB_BASE + SFR_OFFSET(TP, MOD, SFR)*32 + BIT*4)
#define RST_CLK_BB(SFR, BIT)        BB_ADDR(MDR_RST_CLK_TypeDef, RST_CLK_OFFSET, SFR, BIT)
#define BKP_BB(SFR, BIT)            BB_ADDR(MDR_BKP_TypeDef, BKP_OFFSET, SFR, BIT)

#define FLAG_MASK                   ((uint8_t)0x1F)
#define FLAG_SFR_MASK               ((uint8_t)0xE0)

#define HSI_RDY_MASK                (1 << 23)
#define LSI_RDY_MASK                (1 << 21)
#define LSE_RDY_MASK                (1 << 13)
#define HSE_RDY_MASK                (1 << 2)
#define HSE2_RDY_MASK               (1 << 3)
#define PLLCPU_RDY_MASK             (1 << 1)
#define PLLUSB_RDY_MASK             (1 << 0)
#define PLLCPUMUL_OFFSET            8
#define PLLCPUmulclr                (~(uint32_t)(0x0F << PLLCPUMUL_OFFSET))
#define PLLUSBMUL_OFFSET            4
#define PLLUSBmulclr                (~(uint32_t)(0x0F << PLLUSBMUL_OFFSET))
#define CPU_C1_SELclr               (~(uint32_t)0x03)
#define CPU_C3_SELclr               (~(uint32_t)0x00000F0)
#define CPU_HCLK_SELclr             (~(uint32_t)(0x03 << 8))
#define ADC_C12_SELclr              (~(uint32_t)((0x03 << 4) | 0x03))
#define AUC_C12_SELclr              (~(uint32_t)((0x03 << 20) | (0x03 << 16)))
#define AUC_C3_SELclr               (~(uint32_t)((0xF<<24)))
#define ADC_C3_SEL_OFFSET           8
#define ADC_C3_SELclr               (~(uint32_t)(0x0F << ADC_C3_SEL_OFFSET))
#define HSI_SEL_OFFSET              4
#define HSI_SELclr                  (~(uint32_t)(0x0F << HSI_SEL_OFFSET))
#define HSE_SEL_OFFSET              0
#define HSE_SELclr                  (~(uint32_t)(0x0F << HSE_SEL_OFFSET))
#define USB_C1_SELclr               (~(uint32_t)0x03)

#define HSITRIM_OFFSET              24
#define HSITRIM_MASK                ((uint32_t)(0x3F << HSITRIM_OFFSET))

#define HSION_OFFSET                22
#define HSION_MASK                  ((uint32_t)(1 << HSION_OFFSET))

#define LSITRIM_OFFSET              16
#define LSITRIM_MASK                ((uint32_t)(0x1F << LSITRIM_OFFSET))

#define LSION_OFFSET                15
#define LSION_MASK                  ((uint32_t)(1 << LSION_OFFSET))

#if defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    #define PLLCPUON_BB                 RST_CLK_BB(PLL_CONTROL, 2)
    #define PLLCPURLD_BB                RST_CLK_BB(PLL_CONTROL, 3)
    #define PLLUSBON_BB                 RST_CLK_BB(PLL_CONTROL, 0)
    #define PLLUSBRLD_BB                RST_CLK_BB(PLL_CONTROL, 1)

    #define HSION_BB                    BKP_BB(REG_0F, HSION_OFFSET)
    #define LSION_BB                    BKP_BB(REG_0F, LSION_OFFSET)

    #define CPU_C2_SEL_BB               RST_CLK_BB(CPU_CLOCK, 2)
    #define USB_C2_SEL_BB               RST_CLK_BB(USB_CLOCK, 2)
    #define USB_C3_SEL_BB               RST_CLK_BB(USB_CLOCK, 4)
    #define USB_CLK_EN_BB               RST_CLK_BB(USB_CLOCK, 8)
    #define ADC_CLK_EN_BB               RST_CLK_BB(ADC_MCO_CLOCK, 13)
    #define HSI_RTC_EN_BB               RST_CLK_BB(RTC_CLOCK, 9)
    #define HSE_RTC_EN_BB               RST_CLK_BB(RTC_CLOCK, 8)
    #define CPU_C1_SEL0_BB              RST_CLK_BB(CPU_CLOCK, 0)
    #define CPU_C1_SEL1_BB              RST_CLK_BB(CPU_CLOCK, 1)
    #define USB_C1_SEL0_BB              RST_CLK_BB(USB_CLOCK, 0)
    #define USB_C1_SEL1_BB              RST_CLK_BB(USB_CLOCK, 1)
#endif /* #if defined(USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_MDR32FG16S1QI)
    #define PLL_DSP_RDY_BB              RST_CLK_BB(CLOCK_STATUS, RST_CLK_CLOCK_STATUS_PLL_DSP_RDY_Pos)
    #define PLL_DSP_ON_BB               RST_CLK_BB(PLL_CONTROL, RST_CLK_PLL_CONTROL_PLL_DSP_ON_Pos)
    #define PLL_DSP_RLD_BB              RST_CLK_BB(PLL_CONTROL, RST_CLK_PLL_CONTROL_PLL_DSP_RLD_Pos)

    #define DSP_CLK_EN_BB               RST_CLK_BB(DSP_CLOCK, RST_CLK_DSP_CLOCK_DSP_CLK_EN_Pos)
    #define DSP_CLK_C3_SEL_BB           RST_CLK_BB(DSP_CLOCK, RST_CLK_DSP_CLOCK_DSP_C3_SEL_Pos)
    #define DSP_CLK_C2_SEL_BB           RST_CLK_BB(DSP_CLOCK, RST_CLK_DSP_CLOCK_DSP_C2_SEL_Pos)
#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group RST_CLK_Private_Constants */

/** @defgroup RST_CLK_Exported_Functions RST_CLK Exported Functions
  * @{
  */

/**
  * @brief  Set the RST_CLK clock configuration to the default reset state.
  * @param  None
  * @retval None
  */
void RST_CLK_DeInit(void)
{

    uint32_t Reg_0E;

    MDR_RST_CLK->PER_CLOCK = (RST_CLK_PCLK_RST_CLK | RST_CLK_PCLK_BKP);
    Reg_0E = MDR_BKP->REG_0E;

    switch (Reg_0E & (BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_LOW_Msk))
    {
        case (4 | (4 << BKP_REG_0E_SELECTRI_Pos)): /* All clocks are switched off. */
        case (1 | (1 << BKP_REG_0E_SELECTRI_Pos)): /* System clock is up to 200 kHz. */
        case (2 | (2 << BKP_REG_0E_SELECTRI_Pos)): /* System clock is up to 500 kHz. */
        case (3 | (3 << BKP_REG_0E_SELECTRI_Pos)): /* System clock is up to 1 MHz. */
            /* Reset SELECTRI and LOW bits to default value (system clock is up to 10 MHz). */
            MDR_BKP->REG_0E = (Reg_0E & ~(BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_LOW_Msk));
            RST_CLK_WarmDeInit();
            break;

        case 0: /* System clock is up to 10 MHz. */
            RST_CLK_WarmDeInit();
            break;

        case (5 | (5 << BKP_REG_0E_SELECTRI_Pos)): /* System clock is up to 40 MHz. */
        case (6 | (6 << BKP_REG_0E_SELECTRI_Pos)): /* System clock is up to 80 MHz. */
        case (7 | (7 << BKP_REG_0E_SELECTRI_Pos)): /* System clock is over 80 MHz. */
            RST_CLK_WarmDeInit();
            /* Reset SELECTRI and LOW bits to default value (system clock is up to 10 MHz). */
            MDR_BKP->REG_0E = (Reg_0E & ~(BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_LOW_Msk));
            break;

        default: /* Forbidden value: SELECTRI != LOW. */
            /* Fallback to worst case - max core clock. */
            Reg_0E &= ~(BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_LOW_Msk);
#if defined (USE_K1986VE9xI)
            /* Max core clock is 80 MHz or lower. */
            Reg_0E |= ((6 << BKP_REG_0E_SELECTRI_Pos) | (6 << BKP_REG_0E_LOW_Pos));
#elif defined (USE_K1986VE1xI) || defined (USE_MDR32FG16S1QI)
            /* Max core clock is over 80 MHz. */
            Reg_0E |= ((7 << BKP_REG_0E_SELECTRI_Pos) | (7 << BKP_REG_0E_LOW_Pos));
#endif
            MDR_BKP->REG_0E = Reg_0E;

            RST_CLK_WarmDeInit();
            /* Reset SELECTRI and LOW bits to default value (system clock is up to 10 MHz). */
            MDR_BKP->REG_0E = (Reg_0E & ~(BKP_REG_0E_SELECTRI_Msk | BKP_REG_0E_LOW_Msk));
            break;
    }
}

/**
  * @brief  Set the RST_CLK clock configuration to the default reset state.
  * @note   This function doesn't modify RTC_CLOCK register as RTC may depend on it.
  * @param  None
  * @retval None
  */
void RST_CLK_WarmDeInit(void)
{
    /* Reset all clock but RST_CLK and BKP_CLK bits */
    MDR_RST_CLK->PER_CLOCK = (uint32_t)(RST_CLK_PCLK_RST_CLK | RST_CLK_PCLK_BKP);
    
    /* Prepare HSI clk */
    RST_CLK_HSIcmd(ENABLE);
    RST_CLK_HSIstatus();

    /* Reset all MDR_RST_CLK clock registers */
    MDR_RST_CLK->CPU_CLOCK     = (uint32_t)0x00000000;
    MDR_RST_CLK->PLL_CONTROL   = (uint32_t)0x00000000;
    MDR_RST_CLK->HS_CONTROL    = (uint32_t)0x00000000;
    MDR_RST_CLK->USB_CLOCK     = (uint32_t)0x00000000;
    MDR_RST_CLK->ADC_MCO_CLOCK = (uint32_t)0x00000000;

#if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI)
    MDR_RST_CLK->CAN_CLOCK     = (uint32_t)0x00000000;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_K1986VE1xI) */
    MDR_RST_CLK->TIM_CLOCK     = (uint32_t)0x00000000;
    MDR_RST_CLK->UART_CLOCK    = (uint32_t)0x00000000;
    MDR_RST_CLK->SSP_CLOCK     = (uint32_t)0x00000000;
#if defined (USE_K1986VE1xI)
    MDR_RST_CLK->ETH_CLOCK     = (uint32_t)0x00000000;
#endif /* #if defined (USE_K1986VE1xI) */
#if defined (USE_MDR32FG16S1QI)
    MDR_RST_CLK->DSP_CLOCK     = (uint32_t)0x00000000;
    MDR_RST_CLK->SPP2_CLOCK    = (uint32_t)0x00000000;
#endif /* #if defined (USE_MDR32FG16S1QI) */
}

/**
  * @brief  HSE (High Speed External) clock mode and source selection
  * @param  RST_CLK_HSE - @ref RST_CLK_HSE_Mode - mode selector.
  * @retval None
  */
void RST_CLK_HSEconfig(RST_CLK_HSE_Mode RST_CLK_HSE)
{
    /* Check the parameters */
    assert_param(IS_RST_CLK_HSE(RST_CLK_HSE));

    /* Reset HSEON and HSEBYP bits before configuring the HSE */
    MDR_RST_CLK->HS_CONTROL &= ~((uint32_t)(RST_CLK_HSE_ON | RST_CLK_HSE_Bypass));

    /* Configure HSE (RCC_HSE_OFF is already covered by the code section above) */
    switch (RST_CLK_HSE)
    {
        case RST_CLK_HSE_ON:
            /* Set HSEON bit */
            MDR_RST_CLK->HS_CONTROL |= RST_CLK_HSE_ON;
            break;
    
        case RST_CLK_HSE_Bypass:
            /* Set HSEBYP and HSEON bits */
            MDR_RST_CLK->HS_CONTROL |= RST_CLK_HSE_ON | RST_CLK_HSE_Bypass;
            break;

        default:
            break;
    }
}

/**
  * @brief  HSE clock status
  * @param  None
  * @retval @ref ErrorStatus - SUCCESS if HSE clock is ready, else ERROR
  */
ErrorStatus RST_CLK_HSEstatus(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until HSE is ready or time out is occure */
    do
    {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_HSERDY);
        startCounter++;
    } while ((startCounter < HSEonTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_HSERDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return state;
}


#if defined (USE_K1986VE1xI)
/**
  * @brief   HSE2 (High Speed External 2) clock mode and source selection
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI.
  * @param   RST_CLK_HSE2 - @ref RST_CLK_HSE2_Mode - mode selector.
  * @retval  None
  */
void RST_CLK_HSE2config(RST_CLK_HSE2_Mode RST_CLK_HSE2)
{
    /* Check the parameters */
    assert_param(IS_RST_CLK_HSE2(RST_CLK_HSE2));

    /* Reset HSEON and HSEBYP bits before configuring the HSE */
    MDR_RST_CLK->HS_CONTROL &= ~((uint32_t)(RST_CLK_HSE2_ON | RST_CLK_HSE2_Bypass));

    /* Configure HSE (RCC_HSE_OFF is already covered by the code section above) */
    switch (RST_CLK_HSE2)
    {
        case RST_CLK_HSE2_ON:
            /* Set HSEON bit */
            MDR_RST_CLK->HS_CONTROL |= RST_CLK_HSE2_ON;
            break;

        case RST_CLK_HSE2_Bypass:
            /* Set HSEBYP and HSEON bits */
            MDR_RST_CLK->HS_CONTROL |= RST_CLK_HSE2_ON | RST_CLK_HSE2_Bypass;
            break;

        default:
            break;
    }
}

/**
  * @brief   HSE2 clock status
  * @warning This function can be used only for MCU MDR32F1QI, K1986VE1xI.
  * @param   None
  * @retval  @ref ErrorStatus - SUCCESS if HSE2 clock is ready, else ERROR
  */
ErrorStatus RST_CLK_HSE2status(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until HSE2 is ready or time out is occure */
    do {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_HSE2RDY);
        startCounter++;
    } while ((startCounter < HSE2onTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_HSE2RDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return state;
}
#endif /* #if defined (USE_K1986VE1xI) */

/**
  * @brief  LSE (Low Speed External) clock mode and source selection
  * @param  RST_CLK_LSE - @ref RST_CLK_LSE_Mode - mode selector.
  * @retval None
  */
void RST_CLK_LSEconfig(RST_CLK_LSE_Mode RST_CLK_LSE)
{
    /* Check the parameters */
    assert_param(IS_RST_CLK_LSE(RST_CLK_LSE));

    /* Reset LSEON and LSEBYP bits before configuring the LSE */
    MDR_BKP->REG_0F &= ~((uint32_t)(RST_CLK_LSE_ON | RST_CLK_LSE_Bypass));

    switch (RST_CLK_LSE)
    {
        case RST_CLK_LSE_ON:
            /* Set LSEON bit */
            MDR_BKP->REG_0F |= RST_CLK_LSE_ON;
            break;

        case RST_CLK_LSE_Bypass:
            /* Set LSEBYP and LSEON bits */
            MDR_BKP->REG_0F |= (RST_CLK_LSE_ON | RST_CLK_LSE_Bypass);
            break;

        default:
            break;
    }
}

/**
  * @brief  LSE clock status
  * @param  None
  * @retval @ref ErrorStatus - SUCCESS if LSE clock is ready, else ERROR
  */
ErrorStatus RST_CLK_LSEstatus(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until LSE is ready or time out is occure */
    do {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_LSERDY);
        startCounter++;
    } while ((startCounter < LSEonTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_LSERDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return state;
}

/**
  * @brief  HSI (High Speed Internal) clock mode selection
  * @param  NewState - @ref FunctionalState - mode selector (switch off (DISABLE) or on (ENABLE)).
  * @retval None
  */
void RST_CLK_HSIcmd(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) HSION_BB = (uint32_t)NewState;
#endif

#if defined (USE_K1986VE1xI)

    temp = MDR_BKP->REG_0F;
    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable HSI clk by setting the HSE_ON bit in the BKP_REG_0F register */
        temp |= BKP_REG_0F_HSI_ON;
    }
    else
    {
        /* Disable HSI clk by resetting the HSE_ON bit in the BKP_REG_0F register */
        temp &= ~BKP_REG_0F_HSI_ON;
    }
    /* Configure REG_0F register with new value */
    MDR_BKP->REG_0F = temp;
#endif /* defined (USE_K1986VE1xI) */
}

/**
  * @brief  HSI clock frequency adjustment by HSITRIM setting
  * @param  HSItrimValue: HSI frequency trim value.
  *         This parameter must be a number between 0 and 0x3F.
  * @retval None
  */
void RST_CLK_HSIadjust(uint32_t HSItrimValue)
{
    uint32_t temp;
    /* Check the parameters */
    assert_param(IS_RCC_CLK_HSI_TRIM_VALUE(HSItrimValue));

    temp = MDR_BKP->REG_0F;

    /* Clear HSITRIM[5:0] bits */
    temp &= ~HSITRIM_MASK;

    /* Set the HSITRIM[5:0] bits according to HSItrimValue value */
    temp |= (uint32_t)HSItrimValue << HSITRIM_OFFSET;

    /* Store the new value */
    MDR_BKP->REG_0F = temp;
}

/**
  * @brief  HSI clock status
  * @param  None
  * @retval @ref ErrorStatus: SUCCESS if HSI clock is ready, else ERROR
  */
ErrorStatus RST_CLK_HSIstatus(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until HSI is ready or time out is occure */
    do {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_HSIRDY);
        startCounter++;
    } while ((startCounter < HSIonTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_HSIRDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return state;
}

/**
  * @brief  LSI (Low Speed Internal) clock mode selection
  * @param  NewState - @ref FunctionalState - mode selector (switch off (DISABLE) or on (ENABLE)).
  * @retval None
  */
void RST_CLK_LSIcmd(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) LSION_BB = (uint32_t) NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_BKP->REG_0F;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable LSI clk by setting the LSE_ON bit in the BKP_REG_0F register */
        temp |= BKP_REG_0F_LSI_ON;
    }
    else
    {
        /* Disable LSI clk by resetting the LSE_ON bit in the BKP_REG_0F register */
        temp &= ~BKP_REG_0F_LSI_ON;
    }

 /* Configure REG_0F register with new value */
    MDR_BKP->REG_0F = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  LSI clock frequency adjustment by LSITRIM setting
  * @param  LSItrimValue: LSI frequency trim value.
  *         This parameter must be a number between 0 and 0x1F.
  * @retval None
  */
void RST_CLK_LSIadjust(uint32_t LSItrimValue)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RCC_CLK_LSI_TRIM_VALUE(LSItrimValue));

    temp = MDR_BKP->REG_0F;

    /* Clear LSITRIM[4:0] bits */
    temp &= ~LSITRIM_MASK;

    /* Set the LSITRIM[4:0] bits according to LSItrimValue value */
    temp |= (uint32_t)LSItrimValue << LSITRIM_OFFSET;

    /* Store the new value */
    MDR_BKP->REG_0F = temp;
}

/**
  * @brief  LSI clock status
  * @param  None
  * @retval @ref ErrorStatus - SUCCESS if LSI clock is ready, else ERROR
  */
ErrorStatus RST_CLK_LSIstatus(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until LSI is ready or time out is occure */
    do {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_LSIRDY);
        startCounter++;
    } while ((startCounter < LSIonTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_LSIRDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return (state);
}

/**
  * @brief   Configures the CPU_PLL clock source and multiplication factor.
  * @param   RST_CLK_CPU_PLLsource - @ref RST_CLK_CPU_PLL_Source - specifies the PLL entry clock source.
  * @param   RST_CLK_CPU_PLLmul - @ref RST_CLK_CPU_PLL_Multiplier - specifies the PLL multiplication factor.
  * @warning For K1986VE9xI, K1986VE1xI:
  *          RST_CLK_CPU_PLLmul must not be equal to @arg RST_CLK_CPU_PLLmul2 (0x1), @arg RST_CLK_CPU_PLLmul3 (0x2).
  * @retval  None
  */
void RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLL_Source RST_CLK_CPU_PLLsource, uint32_t RST_CLK_CPU_PLLmul)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_CPU_PLL_SOURCE(RST_CLK_CPU_PLLsource));
    assert_param(IS_RST_CLK_CPU_PLL_MUL(RST_CLK_CPU_PLLmul));

    /* Select CPUPLL source */
    temp = MDR_RST_CLK->CPU_CLOCK;
    /* Clear CPU_C1_SEL bits */
    temp &= CPU_C1_SELclr;
    /* Set the CPU_C1_SEL bits */
    temp |= RST_CLK_CPU_PLLsource;
    /* Store the new value */MDR_RST_CLK->CPU_CLOCK = temp;

    /* Set CPUPLL multiplier */
    temp = MDR_RST_CLK->PLL_CONTROL;
    /* Clear PLLMUL[3:0] bits */
    temp &= PLLCPUmulclr;
    /* Set the PLLMUL[3:0] bits */
    temp |= (RST_CLK_CPU_PLLmul << PLLCPUMUL_OFFSET);
    /* Store the new value */
    MDR_RST_CLK->PLL_CONTROL = temp;

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    if (*(__IO uint32_t *) PLLCPUON_BB)
    {
        *(__IO uint32_t *) PLLCPURLD_BB = (uint32_t)0x01;
        *(__IO uint32_t *) PLLCPURLD_BB = (uint32_t)0x00;
    }
#elif defined (USE_K1986VE1xI)
    if( (MDR_RST_CLK->PLL_CONTROL & RST_CLK_PLL_CONTROL_PLL_CPU_ON) == RST_CLK_PLL_CONTROL_PLL_CPU_ON )
    {
        temp = MDR_RST_CLK->PLL_CONTROL;
        temp |= RST_CLK_PLL_CONTROL_PLL_CPU_RLD;
        MDR_RST_CLK->PLL_CONTROL = temp;
        temp &= ~RST_CLK_PLL_CONTROL_PLL_CPU_RLD;
        MDR_RST_CLK->PLL_CONTROL = temp;
    }
#endif
}

/**
  * @brief  Select the CPU_PLL output as input for CPU_C3_SEL or bypass the CPU_PLL.
  * @param  UsePLL - @ref FunctionalState - usage state of the CPU_PLL.
  * @retval None
  */
void RST_CLK_CPU_PLLuse(FunctionalState UsePLL)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(UsePLL));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) CPU_C2_SEL_BB = (uint32_t) UsePLL;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->CPU_CLOCK;

    /* Form new value */
    if (UsePLL != DISABLE)
    {
        /* Use the CPU PLL output as input for CPU_C3_SEL
        * by setting the CPU_C2_SEL bit in the CPU_CLOCK register */
        temp |= (1 << RST_CLK_CPU_CLOCK_CPU_C2_SEL_Pos);
    }
    else
    {
        /* Use the CPU_C1 output as input for CPU_C3_SEL
        * by resetting the CPU_C2_SEL bit in the CPU_CLOCK register */
        temp &= ~(1 << RST_CLK_CPU_CLOCK_CPU_C2_SEL_Pos);
    }
    /* Configure CPU_CLOCK register with new value */
    MDR_RST_CLK->CPU_CLOCK = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  Enables or disables the CPU_PLL.
  * @param  NewState - @ref FunctionalState - new state of the CPU_PLL.
  * @retval None
  */
void RST_CLK_CPU_PLLcmd(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) PLLCPUON_BB = (uint32_t) NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->PLL_CONTROL;

    if(NewState != DISABLE)
    {
        temp |= RST_CLK_PLL_CONTROL_PLL_CPU_ON;
    }
    else
    {
        temp &= ~RST_CLK_PLL_CONTROL_PLL_CPU_ON;
    }
    MDR_RST_CLK->PLL_CONTROL = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  CPU_PLL status
  * @param  None
  * @retval @ref ErrorStatus: SUCCESS if CPU_PLL output clock is ready, else ERROR
  */
ErrorStatus RST_CLK_CPU_PLLstatus(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until CPUPLL is ready or time out is occure */
    do {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_PLLCPURDY);
        startCounter++;
    } while ((startCounter < PLLCPUonTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_PLLCPURDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return state;
}

/**
  * @brief  Сonfigures the CPU_C3_SEL division factor switch.
  * @param  CPUclkDivValue - @ref RST_CLK_CPU_C3_Divisor - specifies the CPU_PLL clock division factor.
  * @retval None
  */
void RST_CLK_CPUclkPrescaler(RST_CLK_CPU_C3_Divisor CPUclkDivValue)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_CPUclkDIV(CPUclkDivValue));

    temp = MDR_RST_CLK->CPU_CLOCK;
    /* Clear CPU_C3_SEL bits */
    temp &= CPU_C3_SELclr;
    /* Set the CPU_C3_SEL bits */
    temp |= ((uint32_t)CPUclkDivValue << RST_CLK_CPU_CLOCK_CPU_C3_SEL_Pos);

    /* Store the new value */
    MDR_RST_CLK->CPU_CLOCK = temp;
}

/**
  * @brief  Select the CPU clock source.
  * @param  CPU_CLK - @ref RST_CLK_HCLK_Source - specifies the CPU clock source.
  * @retval None
  */
void RST_CLK_CPUclkSelection(RST_CLK_HCLK_Source CPU_CLK)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CPU_CLK(CPU_CLK));

    temp = MDR_RST_CLK->CPU_CLOCK;
    /* Clear CPU_C3_SEL bits */
    temp &= CPU_HCLK_SELclr;
    /* Set the CPU_C3_SEL bits */
    temp |= (uint32_t)CPU_CLK;

    /* Store the new value */
    MDR_RST_CLK->CPU_CLOCK = temp;
}

/**
  * @brief   Configures the USB_PLL clock source and multiplication factor.
  * @param   RST_CLK_USB_PLLsource - @ref RST_CLK_USB_PLL_Source - specifies the PLL entry clock source.
  * @param   RST_CLK_USB_PLLmul - @ref RST_CLK_USB_PLL_Multiplier - specifies the PLL multiplication factor.
  * @warning For K1986VE9xI, K1986VE1xI:
  *          RST_CLK_USB_PLLmul must not be equal to @arg RST_CLK_USB_PLLmul2 (0x1), @arg RST_CLK_USB_PLLmul3 (0x2).
  * @retval  None
  */
void RST_CLK_USB_PLLconfig(RST_CLK_USB_PLL_Source RST_CLK_USB_PLLsource, uint32_t RST_CLK_USB_PLLmul)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_USB_PLL_SOURCE(RST_CLK_USB_PLLsource));
    assert_param(IS_RST_CLK_USB_PLL_MUL(RST_CLK_USB_PLLmul));

    /* Select USBPLL source */
    temp = MDR_RST_CLK->USB_CLOCK;
    /* Clear USB_C1_SEL bits */
    temp &= USB_C1_SELclr;
    /* Set the USB_C1_SEL bits */
    temp |= RST_CLK_USB_PLLsource;
    /* Store the new value */
    MDR_RST_CLK->USB_CLOCK = temp;

    /* Set USBPLL multiplier */
    temp = MDR_RST_CLK->PLL_CONTROL;
    /* Clear PLLMUL[3:0] bits */
    temp &= PLLUSBmulclr;
    /* Set the PLLMUL[3:0] bits */
    temp |= ((uint32_t)RST_CLK_USB_PLLmul << PLLUSBMUL_OFFSET);
    /* Store the new value */
    MDR_RST_CLK->PLL_CONTROL = temp;

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    if (*(__IO uint32_t *) PLLUSBON_BB)
    {
        *(__IO uint32_t *) PLLUSBRLD_BB = (uint32_t) 0x01;
        *(__IO uint32_t *) PLLUSBRLD_BB = (uint32_t) 0x00;
    }
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    if ( MDR_RST_CLK->PLL_CONTROL & (RST_CLK_PLL_CONTROL_PLL_USB_ON) )
    {
        temp = MDR_RST_CLK->PLL_CONTROL;
        temp |= RST_CLK_PLL_CONTROL_PLL_USB_RLD;
        MDR_RST_CLK->PLL_CONTROL = temp;
        temp &= ~RST_CLK_PLL_CONTROL_PLL_USB_RLD;
        MDR_RST_CLK->PLL_CONTROL = temp;
    }
#endif /* #if defined (USE_K1986VE1xI)*/
}

/**
  * @brief  Select the USBPLL output as input for USB_C3_SEL or bypass the USB_PLL.
  * @param  UsePLL - @ref FunctionalState - usage state of the USB_PLL.
  * @retval None
  */
void RST_CLK_USB_PLLuse(FunctionalState UsePLL)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(UsePLL));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) USB_C2_SEL_BB = (uint32_t) UsePLL;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->USB_CLOCK;

    /* Form new value */
    if (UsePLL != DISABLE)
    {
        /* Use the USB PLL output as input for USB_C3_SEL
        * by setting the USB_C2_SEL bit in the USB_CLOCK register */
        temp |= (1 << RST_CLK_USB_CLOCK_USB_C2_SEL_Pos);
    }
    else
    {
        /* Use the USB_C1 output as input for USB_C3_SEL
        * by resetting the USB_C2_SEL bit in the USB_CLOCK register */
        temp &= ~(1 << RST_CLK_USB_CLOCK_USB_C2_SEL_Pos);
    }

    /* Configure USB_CLOCK register with new value */
    MDR_RST_CLK->USB_CLOCK = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  Enables or disables the USB_PLL.
  * @param  NewState - @ref FunctionalState - new state of the USB_PLL.
  * @retval None
  */
void RST_CLK_USB_PLLcmd(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));
#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) PLLUSBON_BB = (uint32_t) NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->PLL_CONTROL;

    /* Form a new value */
    if(NewState != DISABLE)
    {
        temp |= RST_CLK_PLL_CONTROL_PLL_USB_ON;
    }
    else
    {
        temp &= ~RST_CLK_PLL_CONTROL_PLL_USB_ON;
    }

    MDR_RST_CLK->PLL_CONTROL = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  USB_PLL status
  * @param  None
  * @retval @ref ErrorStatus: SUCCESS if USB_PLL output clock is ready, else ERROR
  */
ErrorStatus RST_CLK_USB_PLLstatus(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until USBPLL is ready or time out is occure */
    do {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_PLLUSBRDY);
        startCounter++;
    } while ((startCounter < PLLUSBonTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_PLLUSBRDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return state;
}

/**
  * @brief  Enables or disables the USB_CLK prescaler.
  * @param  NewState - @ref FunctionalState - new state of the USB_CLK prescaler.
  *             @arg ENABLE: USB_C2 divided by 2
  *             @arg DISABLE: USB_C2 not divided
  * @retval None
  */
void RST_CLK_USBclkPrescaler(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) USB_C3_SEL_BB = (uint32_t)NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->USB_CLOCK;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the USB prescaler by setting the USB_C3_SEL bit in the USB_CLOCK register */
        temp |= (1 << RST_CLK_USB_CLOCK_USB_C3_SEL_Pos);
    }
    else
    {
        /* Disable the USB prescaler by resetting the USB_C3_SEL bit in the USB_CLOCK register */
        temp &= ~(1 << RST_CLK_USB_CLOCK_USB_C3_SEL_Pos);
    }
    /* Configure USB_CLOCK register with new value */
    MDR_RST_CLK->USB_CLOCK = temp;
#endif /* #if defined (USE_K1986VE1xI) */

}

/**
  * @brief  Enables or disables the USB_CLK clock.
  * @param  NewState - @ref FunctionalState - new state of the USB_CLK.
  * @retval None
  */
void RST_CLK_USBclkEnable(FunctionalState NewState)
{
#if  defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) USB_CLK_EN_BB = (uint32_t) NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->USB_CLOCK;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the USB clk by setting the USB_CLK_EN bit in the USB_CLOCK register */
        temp |= (1 << RST_CLK_USB_CLOCK_USB_CLK_EN_Pos);
    }
    else
    {
        /* Disable the USB clk by resetting the USB_CLK_EN bit in the USB_CLOCK register */
        temp &= ~(1 << RST_CLK_USB_CLOCK_USB_CLK_EN_Pos);
    }

    /* Configure USB_CLOCK register with new value */
    MDR_RST_CLK->USB_CLOCK = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  Select the ADC clock source.
  * @param  ADC_CLK - @ref RST_CLK_ADC_Source - specifies the ADC_C2 clock source.
  * @retval None
  */
void RST_CLK_ADCclkSelection(RST_CLK_ADC_Source ADC_CLK)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_ADCclk(ADC_CLK));

    /* Select ADC_CLK source */
    temp = MDR_RST_CLK->ADC_MCO_CLOCK;
    /* Clear ADC_C1,C2 SEL bits */
    temp &= ADC_C12_SELclr;
    /* Set the ADC_C1,C2 SEL bits */
    temp |= ((uint32_t)ADC_CLK);

    /* Store the new value */
    MDR_RST_CLK->ADC_MCO_CLOCK = temp;
}

/**
  * @brief  Сonfigures the ADC_C3_SEL division factor switch.
  * @param  ADCclkDivValue - @ref RST_CLK_ADC_C3_Divisor - specifies the ADC_C2 clock division factor.
  * @retval None
  */
void RST_CLK_ADCclkPrescaler(RST_CLK_ADC_C3_Divisor ADCclkDivValue)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_ADCclkDivValue(ADCclkDivValue));

    temp = MDR_RST_CLK->ADC_MCO_CLOCK;
    /* Clear ADC_C3_SEL bits */
    temp &= ADC_C3_SELclr;
    /* Set the ADC_C3_SEL bits */
    temp |= (uint32_t)ADCclkDivValue << ADC_C3_SEL_OFFSET;

    /* Store the new value */
    MDR_RST_CLK->ADC_MCO_CLOCK = temp;
}

/**
  * @brief  Enables or disables the ADC_CLK clock.
  * @param  NewState - @ref FunctionalState - new state of the ADC_CLK.
  * @retval None
  */
void RST_CLK_ADCclkEnable(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) ADC_CLK_EN_BB = (uint32_t) NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->ADC_MCO_CLOCK;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable the ADC CLK by setting the ADC_CLK_EN bit in the ADC_MCO_CLOCK register */
        temp |= RST_CLK_ADC_MCO_CLOCK_ADC_CLK_EN;
    }
    else
    {
        /* Disable the ADC CLK by resetting the ADC_CLK_EN bit in the ADC_MCO_CLOCK register */
        temp &= ~RST_CLK_ADC_MCO_CLOCK_ADC_CLK_EN;
    }

    /* Configure CPU_CLOCK register with new value */
    MDR_RST_CLK->ADC_MCO_CLOCK = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  Configures the HSI_C1_SEL division factor switch.
  * @param  HSIclkDivValue - @ref RST_CLK_HSI_C1_Divisor - specifies the HSI clock division factor.
  * @retval None
  */
void RST_CLK_HSIclkPrescaler(RST_CLK_HSI_C1_Divisor HSIclkDivValue)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_HSIclkDivValue(HSIclkDivValue));

    temp = MDR_RST_CLK->RTC_CLOCK;
    /* Clear HSI_SEL bits */
    temp &= HSI_SELclr;
    /* Set the HSI_SEL bits */
    temp |= (uint32_t)HSIclkDivValue << HSI_SEL_OFFSET;

    /* Store the new value */
    MDR_RST_CLK->RTC_CLOCK = temp;
}

/**
  * @brief  Enables or disables the RTCHSI clock.
  * @param  NewState - @ref FunctionalState - new state of the HSI_RTC_EN.
  * @retval None
  */
void RST_CLK_RTC_HSIclkEnable(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) HSI_RTC_EN_BB = (uint32_t) NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->RTC_CLOCK;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable HSI RTC clk by setting the HSE_RTC_EN bit in the RTC_CLOCK register */
        temp |= RST_CLK_RTC_CLOCK_HSI_RTC_EN;
    }
    else
    {
        /* Disable HSI RTC clk by resetting the HSE_RTC_EN bit in the RTC_CLOCK register */
        temp &= ~RST_CLK_RTC_CLOCK_HSI_RTC_EN;
    }

    /* Configure RTC_CLOCK register with new value */
    MDR_RST_CLK->RTC_CLOCK = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  Configures the HSE_C1_SEL division factor switch.
  * @param  HSEclkDivValue - @ref RST_CLK_HSE_C1_Divisor - specifies the HSE clock division factor.
  * @retval None
  */
void RST_CLK_HSEclkPrescaler(RST_CLK_HSE_C1_Divisor HSEclkDivValue)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_HSEclkDivValue(HSEclkDivValue));

    temp = MDR_RST_CLK->RTC_CLOCK;
    /* Clear HSE_SEL bits */
    temp &= HSE_SELclr;
    /* Set the HSE_SEL bits */
    temp |= (uint32_t)HSEclkDivValue << HSE_SEL_OFFSET;

    /* Store the new value */
    MDR_RST_CLK->RTC_CLOCK = temp;
}

/**
  * @brief  Enables or disables the RTCHSE clock.
  * @param  NewState - @ref FunctionalState - new state of the HSE_RTC_EN.
  * @retval None
  */
void RST_CLK_RTC_HSEclkEnable(FunctionalState NewState)
{
#if defined (USE_K1986VE1xI)
    uint32_t temp = 0;
#endif /* #if defined (USE_K1986VE1xI) */
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

#if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI)
    *(__IO uint32_t *) HSE_RTC_EN_BB = (uint32_t)NewState;
#endif /* #if defined (USE_K1986VE9xI) || defined (USE_MDR32FG16S1QI) */

#if defined (USE_K1986VE1xI)
    temp = MDR_RST_CLK->RTC_CLOCK;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable HSI RTC clk by setting the HSE_RTC_EN bit in the RTC_CLOCK register */
        temp |= RST_CLK_RTC_CLOCK_HSE_RTC_EN;
    }
    else
    {
        /* Disable HSI RTC clk by resetting the HSE_RTC_EN bit in the RTC_CLOCK register */
        temp &= ~RST_CLK_RTC_CLOCK_HSE_RTC_EN;
    }

    /* Configure RTC_CLOCK register with new value */
    MDR_RST_CLK->RTC_CLOCK = temp;
#endif /* #if defined (USE_K1986VE1xI) */
}

/**
  * @brief  Select the CPU clock source.
  * @param  CPU_CLK - @ref RST_CLK_CPU_C1_Source - specifies the CPU clock source.
  * @retval None
  */
void RST_CLK_CPUclkSelectionC1(RST_CLK_CPU_C1_Source CPU_CLK)
{
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_RST_CLK_CPU_C1_SOURCE(CPU_CLK));

    temp = MDR_RST_CLK->CPU_CLOCK;
    /* Clear CPU_C3_SEL bits */
    temp &= CPU_C1_SELclr;
    /* Set the CPU_C3_SEL bits */
    temp |= (uint32_t)CPU_CLK;

    /* Store the new value */
    MDR_RST_CLK->CPU_CLOCK = temp;
}

/**
  * @brief  Enables or disables clock of peripherals.
  * @param  RST_CLK_PCLK: specifies the peripheral to gates its clock.
  *         This parameter can be any combination of the following values:
  *           @note for MDR32F9Q2I, K1986VE9xI:
  *           @arg RST_CLK_PCLK_RST_CLK, RST_CLK_PCLK_EEPROM, RST_CLK_PCLK_DMA,
  *                RST_CLK_PCLK_CAN1,    RST_CLK_PCLK_CAN2,
  *                RST_CLK_PCLK_UART1,   RST_CLK_PCLK_UART2,  RST_CLK_PCLK_SSP1, RST_CLK_PCLK_SSP2,
  *                RST_CLK_PCLK_USB,     RST_CLK_PCLK_I2C,    RST_CLK_PCLK_EBC,
  *                RST_CLK_PCLK_WWDG,    RST_CLK_PCLK_IWDG,   RST_CLK_PCLK_BKP,  RST_CLK_PCLK_POWER,
  *                RST_CLK_PCLK_TIMER1,  RST_CLK_PCLK_TIMER2, RST_CLK_PCLK_TIMER3,
  *                RST_CLK_PCLK_ADC,     RST_CLK_PCLK_DAC,    RST_CLK_PCLK_COMP,
  *                RST_CLK_PCLK_PORTA,   RST_CLK_PCLK_PORTB,  RST_CLK_PCLK_PORTC,
  *                RST_CLK_PCLK_PORTD,   RST_CLK_PCLK_PORTE,  RST_CLK_PCLK_PORTF.
  *           @note for MDR32FG16S1QI:
  *           @arg RST_CLK_PCLK_RST_CLK, RST_CLK_PCLK_EEPROM, RST_CLK_PCLK_DMA,
  *                RST_CLK_PCLK_UART1,   RST_CLK_PCLK_UART2,  RST_CLK_PCLK_UART3,
  *                RST_CLK_PCLK_SSP1,    RST_CLK_PCLK_SSP2,   RST_CLK_PCLK_SSP3, RST_CLK_PCLK_SSP4,
  *                RST_CLK_PCLK_USB,     RST_CLK_PCLK_I2C,    RST_CLK_PCLK_EBC,  RST_CLK_PCLK_SDIO,
  *                RST_CLK_PCLK_WWDG,    RST_CLK_PCLK_IWDG,   RST_CLK_PCLK_BKP,  RST_CLK_PCLK_POWER,
  *                RST_CLK_PCLK_TIMER1,  RST_CLK_PCLK_TIMER2, RST_CLK_PCLK_TIMER3,
  *                RST_CLK_PCLK_ADC,     RST_CLK_PCLK_DAC,    RST_CLK_PCLK_COMP, RST_CLK_PCLK_AUDIO_IP,
  *                RST_CLK_PCLK_PORTA,   RST_CLK_PCLK_PORTB,  RST_CLK_PCLK_PORTC,
  *                RST_CLK_PCLK_PORTD,   RST_CLK_PCLK_PORTE,  RST_CLK_PCLK_PORTF.
  *           @note for MDR32F1QI, K1986VE1xI:
  *           @arg RST_CLK_PCLK_RST_CLK,       RST_CLK_PCLK_EEPROM, RST_CLK_PCLK_DMA,
  *                RST_CLK_PCLK_CAN1,          RST_CLK_PCLK_CAN2,
  *                RST_CLK_PCLK_UART1,         RST_CLK_PCLK_UART2,
  *                RST_CLK_PCLK_SSP1,          RST_CLK_PCLK_SSP2,   RST_CLK_PCLK_SSP3,
  *                RST_CLK_PCLK_USB,           RST_CLK_PCLK_EBC,
  *                RST_CLK_PCLK_MIL_STD_15531, RST_CLK_PCLK_MIL_STD_15532,
  *                RST_CLK_PCLK_ARINC429R,     RST_CLK_PCLK_ARINC429T,
  *                RST_CLK_PCLK_WWDG,          RST_CLK_PCLK_IWDG,   RST_CLK_PCLK_BKP,    RST_CLK_PCLK_POWER,
  *                RST_CLK_PCLK_TIMER1,        RST_CLK_PCLK_TIMER2, RST_CLK_PCLK_TIMER3, RST_CLK_PCLK_TIMER4,
  *                RST_CLK_PCLK_ADC,           RST_CLK_PCLK_DAC,
  *                RST_CLK_PCLK_PORTA,         RST_CLK_PCLK_PORTB,  RST_CLK_PCLK_PORTC,
  *                RST_CLK_PCLK_PORTD,         RST_CLK_PCLK_PORTE,  RST_CLK_PCLK_PORTF.
  * @param  NewState - @ref FunctionalState - new state of the specified peripheral clock.
  * @retval None
  */
void RST_CLK_PCLKcmd(uint32_t RST_CLK_PCLK, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    assert_param(IS_RST_CLK_PCLK(RST_CLK_PCLK));

    if (NewState != DISABLE)
    {
        MDR_RST_CLK->PER_CLOCK |= RST_CLK_PCLK;
    }
    else
    {
        MDR_RST_CLK->PER_CLOCK &= ~RST_CLK_PCLK;
    }
}


#if defined (USE_MDR32FG16S1QI)
/**
  * @brief   Set the RST_CLK_AUC clock configuration to the default reset state.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   None.
  * @retval  None.
  */
void RST_CLK_AUCclkDeInit(void)
{
    MDR_RST_CLK->ADC_MCO_CLOCK &= ~(RST_CLK_ADC_MCO_CLOCK_AUC_C1_SEL_Msk |
                                    RST_CLK_ADC_MCO_CLOCK_AUC_C2_SEL_Msk |
                                    RST_CLK_ADC_MCO_CLOCK_AUC_C3_SEL_Msk |
                                    RST_CLK_ADC_MCO_CLOCK_AUC_CLK_EN);
}

/**
  * @brief   Select the AUC clock source.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   AUC_CLK - @ref RST_CLK_AUC_Source - specifies the AUC_C1 clock source.
  * @retval  None
  */
void RST_CLK_AUCclkSelection(RST_CLK_AUC_Source AUC_CLK)
{
    uint32_t temp = 0;

    /* Check the parameters */
    assert_param(IS_RST_CLK_AUCclk(AUC_CLK));

    /* Select ADC_CLK source */
    temp = MDR_RST_CLK->ADC_MCO_CLOCK;
    /* Clear AUC_C1,C2 SEL bits */
    temp &= AUC_C12_SELclr;
    /* Set the ADC_C1,C2 SEL bits */
    temp |= (uint32_t)AUC_CLK;

    /* Store the new value */
    MDR_RST_CLK->ADC_MCO_CLOCK = temp;
}

/**
  * @brief   Configures the AUC_C3_SEL division factor switch.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   AUCclkDIVValue - @ref RST_CLK_AUC_C3_Divisor - specifies the AUC_VIV clock division factor.
  * @retval  None
  */
void RST_CLK_AUCclkPrescaler(RST_CLK_AUC_C3_Divisor AUCclkDIVValue)
{
    uint32_t temp = 0;

    /* Check the parameters */
    assert_param(IS_RST_CLK_AUCclkDIV(AUCclkDIVValue));

    temp = MDR_RST_CLK->ADC_MCO_CLOCK;
    /* Clear CPU_C3_SEL bits */
    temp &= AUC_C3_SELclr;
    /* Set the CPU_C3_SEL bits */
    temp |= (uint32_t)AUCclkDIVValue;

    /* Store the new value */
    MDR_RST_CLK->ADC_MCO_CLOCK = temp;
}

/**
  * @brief   Enables or disables the AUC clk.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   NewState - @ref FunctionalState - new state of the AUC clk.
  * @retval  None
  */
void RST_CLK_AUCclkCMD(FunctionalState NewState)
{
    uint32_t temp = 0;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    temp = MDR_RST_CLK->ADC_MCO_CLOCK;

    /* Form new value */
    if (NewState != DISABLE)
    {
        /* Enable AUC clk by setting the AUC_CLK_EN bit in the ADC_MCO_CLOCK register */
        temp |= RST_CLK_ADC_MCO_CLOCK_AUC_CLK_EN;
    }
    else
    {
        /* Disable AUC clk by resetting the AUC_CLK_EN bit in the ADC_MCO_CLOCK register */
        temp &= ~RST_CLK_ADC_MCO_CLOCK_AUC_CLK_EN;
    }

    /* Configure ADC_MCO_CLOCK register with new value */
    MDR_RST_CLK->ADC_MCO_CLOCK = temp;
}
#endif /* #if defined (USE_MDR32FG16S1QI) */

/**
  * @brief  Put the frequencies of different on chip clocks in *RST_CLK_Clocks structure.
  * @param  RST_CLK_Clocks: pointer to a @ref RST_CLK_ClocksTypeDef structure that will hold
  *         the clocks frequencies.
  * @retval None
  */
void RST_CLK_GetClocksFreq(RST_CLK_FreqTypeDef* RST_CLK_Clocks)
{
    uint32_t cpu_c1_freq, cpu_c2_freq, cpu_c3_freq;
    uint32_t usb_c1_freq, usb_c2_freq, usb_c3_freq;
    uint32_t adc_c1_freq, adc_c2_freq, adc_c3_freq;
    uint32_t hsi_c1_freq, hse_c1_freq;
    uint32_t pll_mul;
    uint32_t temp;

    /* Check the parameter */
    assert_param(RST_CLK_Clocks != 0);

    /* Compute CPU_CLK, USB_CLK, ADC_CLK, RTCHSI, RTCHSE clocks frequencies */

    /* Compute CPU_CLK frequency */

    /* Determine CPU_C1 frequency */
    if( MDR_RST_CLK->CPU_CLOCK & (2 << RST_CLK_CPU_CLOCK_CPU_C1_SEL_Pos) )
    {
        cpu_c1_freq = HSE_Value;
    }
    else
    {
        cpu_c1_freq = HSI_Value;
    }

    if( MDR_RST_CLK->CPU_CLOCK & (1 << RST_CLK_CPU_CLOCK_CPU_C1_SEL_Pos) )
    {
        cpu_c1_freq /= 2;
    }

    /* Determine CPU_C2 frequency */
    cpu_c2_freq = cpu_c1_freq;

    if (MDR_RST_CLK->CPU_CLOCK & (1 << RST_CLK_CPU_CLOCK_CPU_C2_SEL_Pos))
    {
        /* Determine CPU PLL output frequency */
        pll_mul = ((MDR_RST_CLK->PLL_CONTROL >> RST_CLK_PLL_CONTROL_PLL_CPU_MUL_Pos) & (uint32_t) 0x0F) + 1;
        cpu_c2_freq *= pll_mul;
    }

    /*Select CPU_CLK from HSI, CPU_C3, LSE, LSI cases */
    switch ((MDR_RST_CLK->CPU_CLOCK >> RST_CLK_CPU_CLOCK_HCLK_SEL_Pos) & (uint32_t) 0x03)
    {
        case 0:
            /* HSI */
            RST_CLK_Clocks->CPU_CLK_Frequency = HSI_Value;
            break;
        case 1:
            /* CPU_C3 */
            /* Determine CPU_C3 frequency */
            if ((MDR_RST_CLK->CPU_CLOCK >> RST_CLK_CPU_CLOCK_CPU_C3_SEL_Pos & (uint32_t) 0x08) == 0x00)
            {
                cpu_c3_freq = cpu_c2_freq;
            }
            else
            {
                cpu_c3_freq = cpu_c2_freq / (1 << ((MDR_RST_CLK->CPU_CLOCK >> RST_CLK_CPU_CLOCK_CPU_C3_SEL_Pos & (uint32_t) 0x07) + 1));
            }
            RST_CLK_Clocks->CPU_CLK_Frequency = cpu_c3_freq;
            break;
        case 2:
            /* LSE */
            RST_CLK_Clocks->CPU_CLK_Frequency = LSE_Value;
            break;
        default: /* case 3 */
            /* LSI */
            RST_CLK_Clocks->CPU_CLK_Frequency = LSI_Value;
            break;
    }

    /* Compute USB_CLK frequency */

    /* Determine USB_C1 frequency */
    if( MDR_RST_CLK->USB_CLOCK & (2 << RST_CLK_USB_CLOCK_USB_C1_SEL_Pos) )
    {
        usb_c1_freq = HSE_Value;
    }
    else
    {
        usb_c1_freq = HSI_Value;
    }

    if( MDR_RST_CLK->USB_CLOCK & (1 << RST_CLK_USB_CLOCK_USB_C1_SEL_Pos) )
    {
        usb_c1_freq /= 2;
    }

    /* Determine USB_C2 frequency */
    usb_c2_freq = usb_c1_freq;

    if( MDR_RST_CLK->USB_CLOCK & (1 << RST_CLK_USB_CLOCK_USB_C2_SEL_Pos) )
    {
        /* Determine USB PLL output frequency */
        pll_mul = ((MDR_RST_CLK->PLL_CONTROL >> RST_CLK_PLL_CONTROL_PLL_USB_MUL_Pos) & (uint32_t) 0x0F) + 1;
        usb_c2_freq *= pll_mul;
    }

    /* Determine USB_C3 frequency */
    usb_c3_freq = usb_c2_freq;
    if(MDR_RST_CLK->USB_CLOCK & (1 << RST_CLK_USB_CLOCK_USB_C3_SEL_Pos))
    {
        usb_c3_freq /= 2;
    }

    RST_CLK_Clocks->USB_CLK_Frequency = usb_c3_freq;

    /* Compute RTCHSI_CLK frequency */
    temp = MDR_RST_CLK->RTC_CLOCK >> RST_CLK_RTC_CLOCK_HSI_SEL_Pos;
    if ((temp & 0x08U) == 0)
    {
        hsi_c1_freq = HSI_Value;
    }
    else
    {
        temp = 1 << ((temp & 0x07U) + 1);
        hsi_c1_freq = HSI_Value / temp;
    }
    RST_CLK_Clocks->RTCHSI_Frequency = hsi_c1_freq;

    /* Compute RTCHSE_CLK frequency */
    temp = MDR_RST_CLK->RTC_CLOCK >> RST_CLK_RTC_CLOCK_HSE_SEL_Pos;
    if ((temp & 0x08U) == 0)
    {
        hse_c1_freq = HSE_Value;
    }
    else
    {
        temp = 1 << ((temp & 0x07U) + 1);
        hse_c1_freq = HSE_Value / temp;
    }
    RST_CLK_Clocks->RTCHSE_Frequency = hse_c1_freq;

    /* Compute ADC_CLK frequency */

    /* Select ADC_C1 from CPU_C1, LSI, USB_C1, CPU_C2, USB_C2 cases */
    switch ((MDR_RST_CLK->ADC_MCO_CLOCK >> RST_CLK_ADC_MCO_CLOCK_ADC_C1_SEL_Pos) & (uint32_t) 0x03)
    {
        case 0:
            /* CPU_C1 */
            adc_c1_freq = cpu_c1_freq;
            break;
        case 1:
            /* USB_C1 */
            adc_c1_freq = usb_c1_freq;
            break;
        case 2:
            /* CPU_C2 */
            adc_c1_freq = cpu_c2_freq;
            break;
        default: /* case 3 */
            /* USB_C2 */
            adc_c1_freq = usb_c2_freq;
            break;
    }

    /* Select ADC_C2 from LSE, LSI, ADC_C1, HSI_C1 cases */
    switch ((MDR_RST_CLK->ADC_MCO_CLOCK >> RST_CLK_ADC_MCO_CLOCK_ADC_C2_SEL_Pos) & (uint32_t) 0x03)
    {
        case 0:
            /* LSE */
            adc_c2_freq = LSE_Value;
            break;
        case 1:
            /* LSI */
            adc_c2_freq = LSI_Value;
            break;
        case 2:
            /* ADC_C1 */
            adc_c2_freq = adc_c1_freq;
            break;
        default: /* case 3 */
            /* HSI_C1 */
            adc_c2_freq = hsi_c1_freq;
            break;
    }

    temp = MDR_RST_CLK->ADC_MCO_CLOCK >> RST_CLK_ADC_MCO_CLOCK_ADC_C3_SEL_Pos;
    if ((temp & 0x08U) == 0)
    {
        adc_c3_freq = adc_c2_freq;
    }
    else
    {
        temp = 1 << ((temp & 0x07U) + 1);
        adc_c3_freq = adc_c2_freq / temp;
    }

    RST_CLK_Clocks->ADC_CLK_Frequency = adc_c3_freq;
}

/**
  * @brief  Checks whether the specified RCC flag is set or not.
  * @param  RST_CLK_FLAG - @ref RST_CLK_Flags - specifies the flag to check.
  * @retval @ref FlagStatus - The state of RCC_FLAG (SET or RESET).
  */
FlagStatus RST_CLK_GetFlagStatus(RST_CLK_Flags RST_CLK_FLAG)
{
    uint32_t statusreg;
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_RST_CLK_FLAG(RST_CLK_FLAG));

    /* Get the register */
    if ((FLAG_SFR_MASK & (uint32_t)RST_CLK_FLAG) == (uint32_t)0x20)
    {
        /* The flag to check is in CLOCK_STATUS register */
        statusreg = MDR_RST_CLK->CLOCK_STATUS;
    }
    else
    {
        /* The flag to check is in REG_0F register */
        statusreg = MDR_BKP->REG_0F;
    }

    /* Get the flag status on proper position */
    if ((statusreg & ((uint32_t)1 << ((uint32_t)RST_CLK_FLAG & FLAG_MASK))) != (uint32_t)0x00)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    /* Return the flag status */
    return bitstatus;
}


#if defined (USE_MDR32FG16S1QI)
/**
  * @brief   Select the sourse of DSP_C1 CLOCK.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   DSP_CLK_Source - @ref RST_CLK_DSP_C1_Source - defines the the clock source of the DSP_C1.
  * @retval  None
  */
void RST_CLK_DSP_Selection(RST_CLK_DSP_C1_Source DSP_CLK_Source)
{
    uint32_t tmpreg;
    /* Check the parameters */
    assert_param(IS_DSP_C1_CLOCK(DSP_CLK_Source));

    tmpreg = MDR_RST_CLK->DSP_CLOCK & (~RST_CLK_DSP_CLOCK_DSP_C1_SEL_Msk);

    tmpreg |= (uint32_t)DSP_CLK_Source;

    MDR_RST_CLK->DSP_CLOCK = tmpreg;
}


/**
  * @brief   Configures the DSP_PLL multiplication factor.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   PLLMul - @ref RST_CLK_DSP_PLL_Multiplier - specifies the PLL multiplication factor.
  * @retval  None.
  */
void RST_CLK_DSPPLLConfig(RST_CLK_DSP_PLL_Multiplier DSP_PLL_Mul)
{
    uint32_t tmpreg;
    /* Check the parameters */
    assert_param(IS_DSP_PLL_MULL(DSP_PLL_Mul));

    tmpreg = MDR_RST_CLK->PLL_CONTROL & (~RST_CLK_PLL_CONTROL_PLL_DSP_MUL_Msk);

    tmpreg |= (uint32_t)DSP_PLL_Mul << RST_CLK_PLL_CONTROL_PLL_DSP_MUL_Pos;

    MDR_RST_CLK->PLL_CONTROL = tmpreg;

    if (*(__IO uint32_t *) PLL_DSP_ON_BB)
    {
        *(__IO uint32_t *) PLL_DSP_RLD_BB = (uint32_t)0x01;
        *(__IO uint32_t *) PLL_DSP_RLD_BB = (uint32_t)0x00;
    }
}

/**
  * @brief   Enable or disable the DSP PLL.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   NewState - @ref FunctionalState - the new state of the DSP PLL.
  * @retval  None
  */
void RST_CLK_DSPPLL_CMD(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    *(__IO uint32_t *)PLL_DSP_ON_BB = (uint32_t)NewState;
}

/**
  * @brief   Get DSP_PLL status
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   None
  * @retval  @ref ErrorStatus - SUCCESS if DSP_PLL output clock is ready, else ERROR.
  */
ErrorStatus RST_CLK_DSP_PLLStatus(void)
{
    __IO uint32_t startCounter = 0;
    ErrorStatus state;
    FlagStatus flag;

    /* Wait until DSPPLL is ready or time out is occure */
    do {
        flag = RST_CLK_GetFlagStatus(RST_CLK_FLAG_PLLDSPRDY);
        startCounter++;
    } while ((startCounter < PLLDSPonTimeOut) && (flag == RESET));

    if (RST_CLK_GetFlagStatus(RST_CLK_FLAG_PLLDSPRDY) != RESET)
    {
        state = SUCCESS;
    }
    else
    {
        state = ERROR;
    }

    return state;
}

/**
  * @brief   Select the DSP_PLL output as input for DSP_C3_SEL or bypass the CPU_PLL.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   UsePLL - @ref FunctionalState - usage state of the DSP_PLL.
  * @retval  None
  */
void RST_CLK_DSP_PLLUse(FunctionalState UsePLL)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(UsePLL));

    *(__IO uint32_t *) DSP_CLK_C2_SEL_BB = (uint32_t) UsePLL;
}


/**
  * @brief   Use DSP_C2  or DSP_C2/2 output as DSP_CLK.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   DSP_Prescaler - @ref RST_CLK_DSP_C3_Divisor - enables or disables halving the DSP_C2 output clock.
  * @retval  None
  */
void RST_CLK_DSPPrescaler(RST_CLK_DSP_C3_Divisor DSP_Prescaler)
{
    /* Check the parameters */
    assert_param(IS_DSP_PRESCALER(DSP_Prescaler));

    *(__IO uint32_t *) DSP_CLK_C3_SEL_BB = (uint32_t) DSP_Prescaler;
}

/**
  * @brief   Enable or disable DSP clock.
  * @warning This function can be used only for MCU MDR32FG16S1QI.
  * @param   NewState - @ref FunctionalState - New state of the DSP CLK.
  * @retval  None
  */
void RST_CLK_DSPCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    /* Set the new state of the DSP CLK bit */
    *(__IO uint32_t *) DSP_CLK_EN_BB = (uint32_t) NewState;
}
#endif /* #if defined (USE_MDR32FG16S1QI) */

#if defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI)
/**
 * @brief  Check whether the operation of the specified DMA channel is complete or not.
 * @param  Channel: Specify the flag to check.
 *         This parameter can be a value of the RST_CLK_DMA_DONE_STICK_CHANNEL_xxx definitions.
 * @return @ref FlagStatus - current state of dma_done flag for specified channel (SET or RESET).
 */
FlagStatus RST_CLK_DMADone_GetFlagStatus(uint32_t Channel)
{
    FlagStatus Status;

    /* Check the parameters. */
    assert_param(IS_RST_CLK_DMA_DONE_STICK_CHANNEL(Channel));

    if ((MDR_RST_CLK->DMA_DONE_STICK & (uint32_t)Channel) == 0) {
        Status = RESET;
    } else {
        Status = SET;
    }

    return Status;
}

/**
 * @brief  Return the dma_done combined value of all the channels.
 * @param  None.
 * @return dma_done status flags combined value.
 */
uint32_t RST_CLK_DMADone_GetStatus(void)
{
    return MDR_RST_CLK->DMA_DONE_STICK;
}

/**
 * @brief  Clear dma_done flags for the specified channels.
 * @param  Channels: Specify channels to clear.
 *         This parameter can be any combination of RST_CLK_DMA_DONE_STICK_CHANNEL_xxx definitions values.
 * @return None.
 */
void RST_CLK_DMADone_ClearFlags(uint32_t Channels)
{
    MDR_RST_CLK->DMA_DONE_STICK = ~Channels;
}
#endif /* #if defined(USE_K1986VE1xI) && !defined(USE_MDR32F1QI) */

/** @} */ /* End of group RST_CLK_Exported_Functions */

/** @} */ /* End of group RST_CLK */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_rst_clk.c */


