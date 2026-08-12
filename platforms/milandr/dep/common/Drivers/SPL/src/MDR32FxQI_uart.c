/**
  ******************************************************************************
  * @file    MDR32FxQI_uart.c
  * @author  Milandr Application Team
  * @version V2.2.1i
  * @date    24/07/2024
  * @brief   This file contains all the UART firmware functions.
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
#include "MDR32FxQI_uart.h"
#include "MDR32FxQI_rst_clk.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @defgroup UART UART
  * @{
  */

/** @defgroup UART_Private_Defines UART Private Defines
  * @{
  */

#if defined (USE_K1986VE9xI) || defined(USE_K1986VE1xI)
    #define IS_UART_ALL_PERIPH(PERIPH)      (((PERIPH) == MDR_UART1) || \
                                             ((PERIPH) == MDR_UART2))
#elif defined (USE_MDR32FG16S1QI)
    #define IS_UART_ALL_PERIPH(PERIPH)      (((PERIPH) == MDR_UART1) || \
                                             ((PERIPH) == MDR_UART2) || \
                                             ((PERIPH) == MDR_UART3))
#endif

#define CR_EN_Set                 ((uint16_t)0x0001)  /*!< UART Enable Mask */
#define CR_EN_Reset               ((uint16_t)0xFFFE)  /*!< UART Disable Mask */

#define CR_SIREN_Set              ((uint16_t)0x0002)  /*!< UART IrDA mode Enable Mask */
#define CR_SIREN_Reset            ((uint16_t)0xFFFD)  /*!< UART IrDA mode Disable Mask */

#define CR_FC_Mask                ((uint16_t)0xFF80)  /*!< UART CR Flow control Bits Mask */

#define LCR_H_BRK_Set             ((uint16_t)0x0001)  /*!< UART Break Line Set Mask */
#define LCR_H_BRK_Reset           ((uint16_t)0xFFFE)  /*!< UART Break Line Reset Mask */
#define LCR_H_Clear_Mask          ((uint16_t)0x00FF)  /*!< UART LCR_H Mask */

#define FBRD_Fract_Mask           ((uint16_t)0x003F)  /*!< Fractional divider Mask */

#define IrLPBaud16                ((uint32_t)1843200) /*!< F_IrLPBaud16 nominal frequency Hz */

#define ICR_IRQ_Mask            ((uint32_t)UART_ICR_RIMIC  | UART_ICR_CTSMIC | \
                                           UART_ICR_DCDMIC | UART_ICR_DSRMIC | \
                                           UART_ICR_RXIC   | UART_ICR_TXIC   | \
                                           UART_ICR_RTIC   | UART_ICR_FEIC   | \
                                           UART_ICR_PEIC   | UART_ICR_BEIC   | \
                                           UART_ICR_OEIC) /*!< UART interrupt reset mask */

#define UART1_BRG_Mask            ((uint32_t)0x0007)  /*!< UART1 clock divider Mask */
#define UART2_BRG_Mask            ((uint32_t)0x0700)  /*!< UART2 clock divider Mask */
#define UART2_BRG_Offs            ((uint32_t)0x0008)  /*!< UART2 clock divider Offset */

#if defined (USE_MDR32FG16S1QI)
    #define UART3_BRG_Mask            ((uint32_t)0x70000) /*!< UART3 clock divider Mask */
    #define UART3_BRG_Offs            ((uint32_t)0x0010)  /*!< UART3 clock divider Offset */
#endif /* #if defined (USE_MDR32FG16S1QI) */

/** @} */ /* End of group UART_Private_Defines */


/** @defgroup UART_Exported_Functions UART Exported Functions
  * @{
  */

/**
  * @brief  Resets the UARTx peripheral registers to their default reset values.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @retval None
  */
void UART_DeInit(MDR_UART_TypeDef* UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    /* Clear UART CR */
    UARTx->CR      = 0;
    UARTx->RSR_ECR = 0;
    UARTx->ILPR    = 0;
    UARTx->IBRD    = 0;
    UARTx->FBRD    = 0;
    UARTx->LCR_H   = 0;
    UARTx->IFLS    = ((UART_IT_FIFO_LVL_8words << UART_IFLS_RXIFLSEL_Pos) | \
                      UART_IT_FIFO_LVL_8words);
    UARTx->IMSC    = 0;
    UARTx->ICR     = ICR_IRQ_Mask;
    UARTx->DMACR   = 0;

    /* Set UART CR[RXE] and UART CR[TXE] bits */
    UARTx->CR = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;
}

/**
  * @brief  Initializes the UARTx peripheral according to the specified
  *         parameters in the UART_InitStruct.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_InitStruct: pointer to a @ref UART_InitTypeDef structure
  *         that contains the configuration information for the specified UART peripheral.
  * @retval @ref ErrorStatus - The Baud Rate status.
  */
ErrorStatus UART_Init(MDR_UART_TypeDef* UARTx, UART_InitTypeDef* UART_InitStruct)
{
    uint32_t tmpreg, cpuclock;
    uint32_t realspeed;
    uint32_t divider;
    uint32_t integerdivider;
    uint32_t fractionaldivider;
    int32_t speederror;
    RST_CLK_FreqTypeDef RST_CLK_Clocks;

    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_BAUDRATE(UART_InitStruct->UART_BaudRate));
    assert_param(IS_UART_WORD_LENGTH(UART_InitStruct->UART_WordLength));
    assert_param(IS_UART_STOPBITS(UART_InitStruct->UART_StopBits));
    assert_param(IS_UART_PARITY(UART_InitStruct->UART_Parity));
    assert_param(IS_UART_FIFOMODE(UART_InitStruct->UART_FIFOMode));
    assert_param(IS_UART_HARDWARE_FLOW_CONTROL(UART_InitStruct->UART_HardwareFlowControl));

    /* Write to UART CR */
    UARTx->CR = UART_InitStruct->UART_HardwareFlowControl;

    /* Configure the UART Baud Rate */
    RST_CLK_GetClocksFreq(&RST_CLK_Clocks);
    cpuclock = RST_CLK_Clocks.CPU_CLK_Frequency;

    tmpreg = MDR_RST_CLK->UART_CLOCK;

    if (UARTx == MDR_UART1)
    {
        cpuclock /= (1 << (tmpreg & UART1_BRG_Mask));
    }
    else if (UARTx == MDR_UART2)
    {
        cpuclock /= (1 << ((tmpreg & UART2_BRG_Mask) >> UART2_BRG_Offs));
    }
#if defined (USE_MDR32FG16S1QI)
    else if (UARTx == MDR_UART3)
    {
        cpuclock /= (1 << ((tmpreg & UART3_BRG_Mask) >> UART3_BRG_Offs));
    }
#endif /* #if defined (USE_MDR32FG16S1QI) */

    /* Determine the integer part */
    divider = cpuclock / (UART_InitStruct->UART_BaudRate >> 2);
    integerdivider = divider >> 6;
    /* Determine the fractional part */
    fractionaldivider = (divider & FBRD_Fract_Mask);
    /* Determine the speed error */
    realspeed = (cpuclock * 4) / ((integerdivider * 64) + fractionaldivider);
    speederror = ((realspeed - UART_InitStruct->UART_BaudRate) * 128)
                / UART_InitStruct->UART_BaudRate;

    if ((speederror > 2) || (speederror < -2))
    {
        return ERROR;
    }

    /* Write UART Baud Rate */
    UARTx->IBRD = integerdivider;
    UARTx->FBRD = fractionaldivider;

    /* UART LCR_H configuration */
    /* Set the WLEN bits according to UART_WordLength value */
    /* Set STP2 bit according to UART_StopBits value */
    /* Set PEN, EPS and SPS bits according to UART_Parity value */
    /* Set FEN bit according to UART_FIFOMode value */
    tmpreg = UART_InitStruct->UART_WordLength | UART_InitStruct->UART_StopBits
           | UART_InitStruct->UART_Parity     | UART_InitStruct->UART_FIFOMode;
    UARTx->LCR_H = tmpreg;

    return SUCCESS;
}

/**
  * @brief  Fills each UART_InitStruct member with its default value.
  * @param  UART_InitStruct: pointer to a @ref UART_InitTypeDef structure
  *         that is to be initialized.
  * @retval None
  */
void UART_StructInit(UART_InitTypeDef* UART_InitStruct)
{
    /* UART_InitStruct members default value */
    UART_InitStruct->UART_BaudRate            = 9600;
    UART_InitStruct->UART_WordLength          = UART_WordLength8b;
    UART_InitStruct->UART_StopBits            = UART_StopBits1;
    UART_InitStruct->UART_Parity              = UART_Parity_No;
    UART_InitStruct->UART_FIFOMode            = UART_FIFO_OFF;
    UART_InitStruct->UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;
}

/**
  * @brief  Enables or disables the specified UART peripheral.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  NewState - @ref FunctionalState - new state of the UARTx peripheral.
  * @retval None
  */
void UART_Cmd(MDR_UART_TypeDef* UARTx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Enable the selected UART by setting the UARTEN bit in the CR register */
        UARTx->CR |= CR_EN_Set;
    }
    else
    {
        /* Disable the selected UART by clearing the UARTEN bit in the CR register */
        UARTx->CR &= CR_EN_Reset;
    }
}

/**
  * @brief  Enables or disables the specified UART interrupts.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_IT: specifies the UART interrupt sources to be enabled or disabled.
  *         This parameter can be any combination of @ref UART_IT_TypeDef values.
  * @param  NewState - @ref FunctionalState - new state of the specified UARTx interrupts.
  * @retval None
  */
void UART_ITConfig(MDR_UART_TypeDef* UARTx, uint32_t UART_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CONFIG_ITS(UART_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        UARTx->IMSC |= UART_IT;
    }
    else
    {
        UARTx->IMSC &= ~UART_IT;
    }
}

/**
  * @brief  Checks whether the specified UART interrupt has occurred or not.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_IT - @ref UART_IT_TypeDef - specifies the UART interrupt source to check.
  * @retval @ref ITStatus - The state of UART_IT (SET or RESET).
  */
ITStatus UART_GetITStatus(MDR_UART_TypeDef* UARTx, UART_IT_TypeDef UART_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CONFIG_IT(UART_IT));

    if ((UARTx->RIS & UART_IT) == UART_IT)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    return bitstatus;
}

/**
  * @brief  Checks whether the specified UART interrupt (masked) has occurred or not.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_IT - @ref UART_IT_TypeDef - specifies the UART interrupt source to check.
  * @retval @ref ITStatus - The state of UART_IT (SET or RESET).
  */
ITStatus UART_GetITStatusMasked(MDR_UART_TypeDef* UARTx, UART_IT_TypeDef UART_IT)
{
    ITStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CONFIG_IT(UART_IT));

    if (UARTx->MIS & UART_IT)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    return bitstatus;
}

/**
  * @brief  Clears the UARTx's interrupt pending bits.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_IT: specifies the interrupt pending bit to clear.
  *         This parameter can be any combination of @ref UART_IT_TypeDef values.
  * @retval None
  */
void UART_ClearITPendingBit(MDR_UART_TypeDef* UARTx, uint32_t UART_IT)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CONFIG_ITS(UART_IT));

    UARTx->ICR = UART_IT;
}

/**
  * @brief  Specified the UART DMA buffer interrupt level.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_IT_RB_LVL - @ref UART_IT_FIFO_TypeDef - specifies the receiver buffer.
  * @param  UART_IT_TB_LVL - @ref UART_IT_FIFO_TypeDef - specifies the transmitter buffer.
  * @retval None
  */
void UART_DMAConfig(MDR_UART_TypeDef* UARTx, UART_IT_FIFO_TypeDef UART_IT_RB_LVL, UART_IT_FIFO_TypeDef UART_IT_TB_LVL)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_IT_FIFO_LVL(UART_IT_RB_LVL));
    assert_param(IS_UART_IT_FIFO_LVL(UART_IT_TB_LVL));

    UARTx->IFLS = (((uint32_t)UART_IT_RB_LVL) << 3) | ((uint32_t)UART_IT_TB_LVL);
}

/**
  * @brief  Enables or disables the UART's DMA interface.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_DMAReq: specifies the DMA request.
  *         This parameter can be any combination of @ref UART_DMA_Req_TypeDef values.
  * @param  NewState - @ref FunctionalState - new state of the DMA Request sources.
  * @retval None
  */
void UART_DMACmd(MDR_UART_TypeDef* UARTx, uint32_t UART_DMAReq, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_DMAREQS(UART_DMAReq));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Enable the DMA transfer for selected requests and DMAONERR bit
        in the UART DMACR register */
        UARTx->DMACR |= UART_DMAReq;
    }
    else
    {
        /* Disable the DMA transfer for selected requests and DMAONERR bit
        in the UART DMACR register */
        UARTx->DMACR &= (uint16_t)~UART_DMAReq;
    }
}

/**
  * @brief  Transmits single data through the UARTx peripheral.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  Data: the data to transmit.
  * @retval None
  */
void UART_SendData(MDR_UART_TypeDef* UARTx, uint8_t Data)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    /* Transmit Data */
    UARTx->DR = (uint32_t)Data;
}

/**
  * @brief  Returns the most recent received data by the UARTx peripheral.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART1, UART2.
  * @retval The received data (7:0) and error flags (11:8).
  */
uint16_t UART_ReceiveData(MDR_UART_TypeDef* UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));

    /* Receive Data */
    return (uint16_t)(UARTx->DR);
}

/**
  * @brief  Break transmit.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  NewState - @ref FunctionalState - new state of the Line.
  * @retval None
  */
void UART_BreakLine(MDR_UART_TypeDef* UARTx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Set BRK bit in the UART LCR_H register */
        UARTx->LCR_H |= LCR_H_BRK_Set;
    }
    else
    {
        /* Reset BRK bit in the UART LCR_H register */
        UARTx->LCR_H &= LCR_H_BRK_Reset;
    }
}

/**
  * @brief  Configures the UART's IrDA interface.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_IrDAMode - @ref UART_IrDA_Mode_TypeDef - specifies the IrDA mode.
  * @retval None
  */
void UART_IrDAConfig(MDR_UART_TypeDef* UARTx, UART_IrDA_Mode_TypeDef UART_IrDAMode)
{
    uint32_t uartclock, tmpreg;
    RST_CLK_FreqTypeDef RST_CLK_Clocks;

    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_IRDA_MODE(UART_IrDAMode));

    if (UART_IrDAMode == UART_IrDAMode_LowPower)
    {
        /* Configure the UART ILPR */
        RST_CLK_GetClocksFreq(&RST_CLK_Clocks);
        
        tmpreg = MDR_RST_CLK->UART_CLOCK;
        
        if (UARTx == MDR_UART1)
        {
            uartclock = RST_CLK_Clocks.CPU_CLK_Frequency / (1 << (tmpreg & UART1_BRG_Mask));
        }
        else if (UARTx == MDR_UART2)
        {
            uartclock = RST_CLK_Clocks.CPU_CLK_Frequency / (1 << ((tmpreg & UART2_BRG_Mask) >> UART2_BRG_Offs));
        }
#if defined (USE_MDR32FG16S1QI)
        else if (UARTx == MDR_UART3)
        {
            uartclock = RST_CLK_Clocks.CPU_CLK_Frequency / (1 << ((tmpreg & UART3_BRG_Mask) >> UART3_BRG_Offs));
        }
#endif /* #if defined (USE_MDR32FG16S1QI) */
        
        UARTx->ILPR = uartclock / IrLPBaud16;
    }

    UARTx->CR |= UART_IrDAMode;
}

/**
  * @brief  Enables or disables the UART's IrDA interface.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  NewState - @ref FunctionalState - new state of the IrDA mode.
  * @retval None
  */
void UART_IrDACmd(MDR_UART_TypeDef* UARTx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        /* Set SIREN bit in the UART CR register */
        UARTx->CR |= CR_SIREN_Set;
    }
    else
    {
        /* Reset SIREN bit in the UART CR register */
        UARTx->CR &= CR_SIREN_Reset;
    }
}

/**
  * @brief  Checks whether the specified UART flag is set or not.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_FLAG - @ref UART_Flags_TypeDef - specifies the flag to check.
  * @retval @ref FlagStatus - The state of UART_FLAG (SET or RESET).
  */
FlagStatus UART_GetFlagStatus(MDR_UART_TypeDef* UARTx, UART_Flags_TypeDef UART_FLAG)
{
    FlagStatus bitstatus;

    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_FLAG(UART_FLAG));

    if (UARTx->FR & UART_FLAG)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }

    return bitstatus;
}

/**
  * @brief  Initializes the UARTx peripheral Clock according to the
  *         specified parameters.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the MDR_UARTx values, where x is a number:
  *             1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *             1, 2, 3 for MDR32FG16S1QI.
  * @param  UART_BRG - @ref UART_Clock_BRG_TypeDef - specifies the HCLK division factor.
  * @retval None
  */
void UART_BRGInit(MDR_UART_TypeDef* UARTx, UART_Clock_BRG_TypeDef UART_BRG)
{
    uint32_t tmpreg;

    /* Check the parameters */
    assert_param(IS_UART_ALL_PERIPH(UARTx));
    assert_param(IS_UART_CLOCK_BRG(UART_BRG));

    tmpreg = MDR_RST_CLK->UART_CLOCK;


    if (UARTx == MDR_UART1)
    {
        tmpreg |= RST_CLK_UART_CLOCK_UART1_CLK_EN;
        tmpreg &= ~RST_CLK_UART_CLOCK_UART1_BRG_Msk;
        tmpreg |= ((uint32_t)UART_BRG);
    }
    else if (UARTx == MDR_UART2)
    {
        tmpreg |= RST_CLK_UART_CLOCK_UART2_CLK_EN;
        tmpreg &= ~RST_CLK_UART_CLOCK_UART2_BRG_Msk;
        tmpreg |= (((uint32_t)UART_BRG) << 8);
    }
#if defined (USE_MDR32FG16S1QI)
    else if( UARTx == MDR_UART3)
    {
        tmpreg |= RST_CLK_UART_CLOCK_UART3_CLK_EN;
        tmpreg &= ~ RST_CLK_UART_CLOCK_UART3_BRG_Msk;
        tmpreg |= (((uint32_t)UART_BRG) << RST_CLK_UART_CLOCK_UART3_BRG_Pos);
    }
#endif
    MDR_RST_CLK->UART_CLOCK = tmpreg;
}

/** @} */ /* End of group UART_Exported_Functions */

/** @} */ /* End of group UART */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_uart.c */


