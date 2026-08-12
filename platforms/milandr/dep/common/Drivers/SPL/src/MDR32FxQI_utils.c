/**
  ******************************************************************************
  * @file    MDR32FxQI_utils.c
  * @author  Milandr Application Team
  * @version V1.1.1i
  * @date    24/07/2024
  * @brief   This file contains all utility functions of the firmware library.
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
#include "MDR32FxQI_utils.h"
#include "MDR32FxQI_rst_clk.h"
#if defined (_USE_DEBUG_UART_)
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_uart.h"
#include <stdio.h>
#endif

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup UTILS UTILS
  * @{
  */

/** @defgroup UTILS_Private_Variables UTILS Private Variables
  * @{
  */

/** @defgroup UTILS_DELAY_Private_Variables UTILS DELAY
  * @{
  */

static uint32_t DelayMode = DELAY_MODE_PROGRAM;
static MDR_TIMER_TypeDef* DelayTimer = MDR_TIMER1;

static uint32_t DelayProgramConstUs = DELAY_PROGRAM_GET_CONST_US(HSI_Value);
static uint32_t DelayProgramConstMs = DELAY_PROGRAM_GET_CONST_MS(HSI_Value);

static uint32_t DelayCounterConstUs = DELAY_COUNTER_GET_CONST_US(HSI_Value);
static uint32_t DelayCounterConstMs = DELAY_COUNTER_GET_CONST_MS(HSI_Value);

/** @} */ /* End of group UTILS_DELAY_Private_Variables */

/** @defgroup UTILS_STDIO_Private_Variables UTILS STDIO
  * @{
  */

#if defined (_USE_DEBUG_UART_)
#if defined(__ARMCC_VERSION) /* ARM Compiler */
#if defined(__CC_ARM)        /* ARM Compiler 5 */
struct __FILE {
    int handle;
};
#endif /* __CC_ARM */
FILE __stdout;
FILE __stdin;
#endif    /* __ARMCC_VERSION */
#endif    /* _USE_DEBUG_UART_ */

/** @} */ /* End of group UTILS_STDIO_Private_Variables */

/** @} */ /* End of group UTILS_Private_Variables */

/** @defgroup UTILS_Private_Defines UTILS Private Defines
  * @{
  */

/** @defgroup UTILS_DELAY_Private_Defines UTILS DELAY
  * @{
  */

#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
    #define IS_DELAY_TIMER_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_TIMER1) || \
                                               ((PERIPH) == MDR_TIMER2) || \
                                               ((PERIPH) == MDR_TIMER3))
#elif (defined (USE_K1986VE1xI))
    #define IS_DELAY_TIMER_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_TIMER1) || \
                                               ((PERIPH) == MDR_TIMER2) || \
                                               ((PERIPH) == MDR_TIMER3) || \
                                               ((PERIPH) == MDR_TIMER4))
#endif

/** @} */ /* End of group UTILS_DELAY_Private_Defines */

/** @defgroup UTILS_STDIO_Private_Defines UTILS STDIO
  * @{
  */

#define STDIO_PUTCHAR                int fputc(int ch, FILE* f)
#define STDIO_GETCHAR                int fgetc(FILE* f)

#if defined (USE_K1986VE9xI) || defined(USE_K1986VE1xI)
    #define IS_STDIO_UART_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_UART1) || \
                                              ((PERIPH) == MDR_UART2))
#elif defined (USE_MDR32FG16S1QI)
    #define IS_STDIO_UART_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_UART1) || \
                                              ((PERIPH) == MDR_UART2) || \
                                              ((PERIPH) == MDR_UART3))
#endif

#if defined (USE_K1986VE9xI)
#define IS_STDIO_PORT_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_PORTA) || \
                                          ((PERIPH) == MDR_PORTB) || \
                                          ((PERIPH) == MDR_PORTD) || \
                                          ((PERIPH) == MDR_PORTF))
#elif defined (USE_MDR32FG16S1QI)
#define IS_STDIO_PORT_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_PORTA) || \
                                          ((PERIPH) == MDR_PORTB) || \
                                          ((PERIPH) == MDR_PORTC) || \
                                          ((PERIPH) == MDR_PORTD) || \
                                          ((PERIPH) == MDR_PORTE) || \
                                          ((PERIPH) == MDR_PORTF))
#elif defined (USE_K1986VE1xI)
#define IS_STDIO_PORT_ALL_PERIPH(PERIPH) (((PERIPH) == MDR_PORTC) || \
                                          ((PERIPH) == MDR_PORTD))
#endif

/** @} */ /* End of group UTILS_STDIO_Private_Defines */

/** @} */ /* End of group UTILS_Private_Defines */

/** @defgroup UTILS_Private_Types UTILS Private Types
  * @{
  */

/** @defgroup UTILS_STDIO_Private_Types UTILS STDIO
  * @{
  */

#if defined (_USE_DEBUG_UART_)
/**
  * @brief  STDIO UART Init Structure definition
  */
typedef struct
{
    uint32_t                 STDIO_UART_BaudRate;   /*!< This member configures the UART communication baud rate.
                                                         The baud rate is computed using the following formula:
                                                          - IntegerDivider = ((UARTCLK) / (16 * (UART_BaudRate)))
                                                          - FractionalDivider = ((IntegerDivider - ((u32) IntegerDivider)) * 64) + 0.5 */
    UART_Word_Length_TypeDef STDIO_UART_WordLength; /*!< Specifies the number of data bits transmitted or received in a frame.
                                                         This parameter can be a value of @ref UART_Word_Length_TypeDef */
    UART_Stop_Bits_TypeDef   STDIO_UART_StopBits;   /*!< Specifies the number of stop bits transmitted.
                                                         This parameter can be a value of @ref UART_Stop_Bits_TypeDef */
    UART_Parity_TypeDef      STDIO_UART_Parity;     /*!< Specifies the parity mode.
                                                         This parameter can be a value of @ref UART_Parity_TypeDef */
    MDR_PORT_TypeDef*        STDIO_UART_PORTx;      /*!< Select the MDR_PORTx peripheral with UART function. x can be:
                                                          - (A, B, D, F) for MDR32F9Q2I, K1986VE9xI;
                                                          - (A..F) for MDR32FG16S1QI;
                                                          - (C, D) for MDR32F1QI, K1986VE1xI. */
    uint16_t                 STDIO_UART_PORT_Pin;   /*!< Specifies PORT pins with UART function to be configured.
                                                         This parameter is a mask of @ref PORT_Pin_TypeDef values. */
    PORT_FUNC_TypeDef        STDIO_UART_PORT_Func;  /*!< Specifies function UART for the selected pins.
                                                         This parameter is one of @ref PORT_FUNC_TypeDef values. */
} STDIO_UART_InitTypeDef;
#endif

/** @} */ /* End of group UTILS_STDIO_Private_Types */

/** @} */ /* End of group UTILS_Private_Types */

/** @defgroup UTILS_Private_Function_Prototypes UTILS Private Functions Prototypes
  * @{
  */

/** @defgroup UTILS_DELAY_Private_Function_Prototypes UTILS DELAY
  * @{
  */

void DELAY_SYSTICK_Init(void);
void DELAY_SYSTICK_WaitTicks(uint32_t Ticks);
void DELAY_TIMER_Init(MDR_TIMER_TypeDef* TIMERx);
void DELAY_TIMER_WaitTicks(MDR_TIMER_TypeDef* TIMERx, uint32_t Ticks);
#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
void DELAY_DWT_Init(void);
void DELAY_DWT_WaitTicks(uint32_t Ticks);
#endif

/** @} */ /* End of group UTILS_DELAY_Private_Function_Prototypes */

/** @defgroup UTILS_STDIO_Private_Function_Prototypes UTILS STDIO
  * @{
  */

#if defined (_USE_DEBUG_UART_)
ErrorStatus STDIO_UartInit(MDR_UART_TypeDef* STDIO_UARTx, STDIO_UART_InitTypeDef* STDIO_UART_InitStruct);
#endif

/** @} */ /* End of group UTILS_STDIO_Private_Function_Prototypes */

/** @} */ /* End of group UTILS_Private_Function_Prototypes */

/** @addtogroup UTILS_Exported_Functions UTILS Exported Functions
  * @{
  */

/** @addtogroup UTILS_DELAY_Exported_Functions UTILS DELAY
  * @{
  */

/**
  * @brief  Initializes the DELAY module according to the specified mode.
  * @note   The core clock from SystemCoreClock is used to calculate the delay time.
  *         Before calling DELAY_Init(), you must configure the required core clock and call SystemCoreClockUpdate().
  * @param  Mode - @ref DELAY_Mode_TypeDef - specifies the Delay mode.
  * @retval None.
  */
void DELAY_Init(DELAY_Mode_TypeDef Mode)
{
    assert_param(IS_DELAY_MODE(Mode));

    DelayMode = Mode;

    if(DelayMode == DELAY_MODE_PROGRAM)
    {
        DelayProgramConstUs = DELAY_PROGRAM_GET_CONST_US(SystemCoreClock);
        DelayProgramConstMs = DELAY_PROGRAM_GET_CONST_MS(SystemCoreClock);
    }
    else
    {
        DelayCounterConstUs = DELAY_COUNTER_GET_CONST_US(SystemCoreClock);
        DelayCounterConstMs = DELAY_COUNTER_GET_CONST_MS(SystemCoreClock);

        switch (DelayMode)
        {
            case DELAY_MODE_SYSTICK:
                DELAY_SYSTICK_Init();
                break;
            case DELAY_MODE_TIMER1:
                DELAY_TIMER_Init(MDR_TIMER1);
                DelayTimer = MDR_TIMER1;
                break;
            case DELAY_MODE_TIMER2:
                DELAY_TIMER_Init(MDR_TIMER2);
                DelayTimer = MDR_TIMER2;
                break;
            case DELAY_MODE_TIMER3:
                DELAY_TIMER_Init(MDR_TIMER3);
                DelayTimer = MDR_TIMER3;
                break;
#if (defined (USE_K1986VE1xI))
            case DELAY_MODE_TIMER4:
                DELAY_TIMER_Init(MDR_TIMER4);
                DelayTimer = MDR_TIMER4;
                break;
#elif ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
            case DELAY_MODE_DWT:
                DELAY_DWT_Init();
                break;
#endif
            default:
                break;
        }
    }
}

/**
  * @brief  Performs a blocking delay in microseconds.
  * @param  Us: specifies the delay time in microseconds.
  *         This parameter can be a value:
  *         - DELAY_MODE_PROGRAM:
  *                               Us <= 16784000 / CPU_CLK (MHz) for MDR32FG16S1QI;
  *                               Us <= 25177000 / CPU_CLK (MHz) for MDR32F9Q2I, K1986VE9xI;
  *                               Us <= 33569000 / CPU_CLK (MHz) for MDR32F1QI, K1986VE1xI;
  *         - DELAY_MODE_SYSTICK: Us <= (2^32-1) / CPU_CLK (MHz);
  *         - DELAY_MODE_TIMERx:  Us <= (2^32-1) / CPU_CLK (MHz);
  *         - DELAY_MODE_DWT:     Us <= (2^32-1) / CPU_CLK (MHz).
  * @retval None.
  */
void DELAY_WaitUs(uint32_t Us)
{
    switch (DelayMode)
    {
        case DELAY_MODE_PROGRAM:
            DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_US_LOOPS(Us, DelayProgramConstUs));
            break;
        case DELAY_MODE_SYSTICK:
            DELAY_SYSTICK_WaitTicks(DELAY_COUNTER_GET_TICKS(Us, DelayCounterConstUs));
            break;
        case DELAY_MODE_TIMER1:
        case DELAY_MODE_TIMER2:
        case DELAY_MODE_TIMER3:
#if (defined (USE_K1986VE1xI))
        case DELAY_MODE_TIMER4:
#endif
            DELAY_TIMER_WaitTicks(DelayTimer, DELAY_COUNTER_GET_TICKS(Us, DelayCounterConstUs));
            break;
#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
        case DELAY_MODE_DWT:
            DELAY_DWT_WaitTicks(DELAY_COUNTER_GET_TICKS(Us, DelayCounterConstUs));
            break;
#endif
    }
}

/**
  * @brief  Performs a blocking delay in milliseconds.
  * @param  Ms: specifies the delay time in milliseconds.
  *         This parameter can be a value:
  *         - DELAY_MODE_PROGRAM:
  *                               Ms <= 17179000 / CPU_CLK (MHz) for MDR32FG16S1QI;
  *                               Ms <= 25769000 / CPU_CLK (MHz) for MDR32F9Q2I, K1986VE9xI;
  *                               Ms <= 34359000 / CPU_CLK (MHz) for MDR32F1QI, K1986VE1xI;
  *         - DELAY_MODE_SYSTICK: Ms <= ((2^32-1) / 1000) / CPU_CLK (MHz);
  *         - DELAY_MODE_TIMERx:  Ms <= ((2^32-1) / 1000) / CPU_CLK (MHz);
  *         - DELAY_MODE_DWT:     Ms <= ((2^32-1) / 1000) / CPU_CLK (MHz).
  * @retval None.
  */
void DELAY_WaitMs(uint32_t Ms)
{
    switch (DelayMode)
    {
        case DELAY_MODE_PROGRAM:
            DELAY_PROGRAM_WaitLoopsAsm(DELAY_PROGRAM_GET_MS_LOOPS(Ms, DelayProgramConstMs));
            break;
        case DELAY_MODE_SYSTICK:
            DELAY_SYSTICK_WaitTicks(DELAY_COUNTER_GET_TICKS(Ms, DelayCounterConstMs));
            break;
        case DELAY_MODE_TIMER1:
        case DELAY_MODE_TIMER2:
        case DELAY_MODE_TIMER3:
#if (defined (USE_K1986VE1xI))
        case DELAY_MODE_TIMER4:
#endif
            DELAY_TIMER_WaitTicks(DelayTimer, DELAY_COUNTER_GET_TICKS(Ms, DelayCounterConstMs));
            break;
#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
        case DELAY_MODE_DWT:
            DELAY_DWT_WaitTicks(DELAY_COUNTER_GET_TICKS(Ms, DelayCounterConstMs));
            break;
#endif
    }
}

/** @} */ /* End of group UTILS_DELAY_Exported_Functions */

/** @addtogroup UTILS_STDIO_Exported_Functions UTILS STDIO
  * @{
  */

#if defined (_USE_DEBUG_UART_)
/**
  * @brief  Initializes the UARTx peripheral selected in MDR32FxQI_config.h for standard input/output.
  * @param  None.
  * @retval @ref ErrorStatus - The UARTx peripheral initialization status.
  */
ErrorStatus STDIO_Init(void)
{
    ErrorStatus Status = ERROR;
    STDIO_UART_InitTypeDef STDIO_UART_InitStruct;

    STDIO_UART_InitStruct.STDIO_UART_BaudRate       = DEBUG_BAUD_RATE;
    STDIO_UART_InitStruct.STDIO_UART_WordLength     = DEBUG_WORD_LENGTH;
    STDIO_UART_InitStruct.STDIO_UART_StopBits       = DEBUG_STOP_BITS;
    STDIO_UART_InitStruct.STDIO_UART_Parity         = DEBUG_BAUD_PARITY;
    STDIO_UART_InitStruct.STDIO_UART_PORTx          = DEBUG_UART_PORT;
    STDIO_UART_InitStruct.STDIO_UART_PORT_Pin       = DEBUG_UART_PINS;
    STDIO_UART_InitStruct.STDIO_UART_PORT_Func      = DEBUG_UART_PINS_FUNCTION;

    Status = STDIO_UartInit(DEBUG_UART, &STDIO_UART_InitStruct);

    return Status;
}

/**
  * @brief  Writes the character specified by ch (converted to uint8_t) to the UART FIFOTX.
  * @param  ch: Character.
  * @param  f:  Stream handle.
  * @retval ch: The character written.
  */
STDIO_PUTCHAR
{
    /* Wait until there is enough space in TX buffer */
    while (UART_GetFlagStatus(DEBUG_UART, UART_FLAG_TXFF) == SET) {}
    UART_SendData(DEBUG_UART, (uint8_t)ch);

    return ch;
}

/**
  * @brief  Obtains the next character (converted to uint8_t) from the UART FIFORX.
  * @param  f:  Stream handle.
  * @retval ch: The next character from the UART FIFORX.
  */
STDIO_GETCHAR
{
    /* Wait until a character is received */
    while (UART_GetFlagStatus(DEBUG_UART, UART_FLAG_RXFE) == SET) {}

    return UART_Data(UART_ReceiveData(DEBUG_UART));
}
#endif    /* _USE_DEBUG_UART_ */

/** @} */ /* End of group UTILS_STDIO_Exported_Functions */

/** @} */ /* End of group UTILS_Exported_Functions */

/** @defgroup UTILS_Private_Functions UTILS Private Functions
  * @{
  */

/** @defgroup UTILS_DELAY_Private_Functions UTILS DELAY
  * @{
  */

/**
  * @brief  Initializes and starts the System Tick Timer. The System Tick Timer is clocked at the core clock.
  * @param  None.
  * @retval None.
  */
void DELAY_SYSTICK_Init(void)
{
    SysTick->LOAD = SysTick_LOAD_RELOAD_Msk;
    SysTick->VAL  = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief  Performs a blocking delay with a System Tick Timer in ticks.
  * @param  Ticks: specifies the delay time in ticks.
  * @retval None.
  */
void DELAY_SYSTICK_WaitTicks(uint32_t Ticks)
{
    uint32_t NumOfReloads;

    SysTick->LOAD = Ticks;
    SysTick->VAL  = 0UL;
    NumOfReloads  = Ticks >> 24;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) { }

    if (NumOfReloads)
    {
        SysTick->LOAD = SysTick_LOAD_RELOAD_Msk;
        SysTick->VAL  = 0UL;
        while (NumOfReloads--)
        {
            while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) { }
        }
    }
}

/**
  * @brief  Initializes and starts the TIMERx peripheral. The TIMERx is clocked at the core clock.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @retval None.
  */
void DELAY_TIMER_Init(MDR_TIMER_TypeDef* TIMERx)
{
    /* Check the parameters */
    assert_param(IS_DELAY_TIMER_ALL_PERIPH(TIMERx));

    MDR_RST_CLK->PER_CLOCK |= PCLK_BIT(TIMERx);

    if(TIMERx == MDR_TIMER1)
    {
        MDR_RST_CLK->TIM_CLOCK &= ~RST_CLK_TIM_CLOCK_TIM1_BRG_Msk;
        MDR_RST_CLK->TIM_CLOCK |= RST_CLK_TIM_CLOCK_TIM1_CLK_EN;
    }
    else if(TIMERx == MDR_TIMER2)
    {
        MDR_RST_CLK->TIM_CLOCK &= ~RST_CLK_TIM_CLOCK_TIM2_BRG_Msk;
        MDR_RST_CLK->TIM_CLOCK |= RST_CLK_TIM_CLOCK_TIM2_CLK_EN;
    }
    else if(TIMERx == MDR_TIMER3)
    {
        MDR_RST_CLK->TIM_CLOCK &= ~RST_CLK_TIM_CLOCK_TIM3_BRG_Msk;
        MDR_RST_CLK->TIM_CLOCK |= RST_CLK_TIM_CLOCK_TIM3_CLK_EN;
    }
#if (defined (USE_K1986VE1xI))
    else if(TIMERx == MDR_TIMER4)
    {
        MDR_RST_CLK->UART_CLOCK &= ~RST_CLK_UART_CLOCK_TIM4_BRG_Msk;
        MDR_RST_CLK->UART_CLOCK |= RST_CLK_UART_CLOCK_TIM4_CLK_EN;
    }
#endif

    TIMERx->CNTRL = 0UL;
#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
    TIMERx->ARR = 0xFFFF;
#elif (defined (USE_K1986VE1xI))
    TIMERx->ARR = 0xFFFFFFFF;
#endif
    TIMERx->PSG = 0UL;
    TIMERx->STATUS = 0UL;
    TIMERx->CNTRL = TIMER_CNTRL_CNT_EN;
}

/**
  * @brief  Performs a blocking delay with a TIMERx in ticks.
  * @param  TIMERx: select the TIMER peripheral.
  *         This parameter can be one of the MDR_TIMERx values, where x is a number:
  *             1, 2, 3 for MDR32F9Q2I, K1986VE9xI and MDR32FG16S1QI;
  *             1, 2, 3, 4 for MDR32F1QI, K1986VE1xI.
  * @param  Ticks: specifies the delay time in ticks.
  * @retval None.
  */
void DELAY_TIMER_WaitTicks(MDR_TIMER_TypeDef* TIMERx, uint32_t Ticks)
{
#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
    uint32_t NumOfReloads;

    TIMERx->CNT = 0UL;
    TIMERx->ARR = Ticks;
    TIMERx->STATUS = 0UL;

    NumOfReloads = Ticks >> 16;
    while((TIMERx->STATUS & TIMER_STATUS_CNT_ARR_EVENT) == 0UL){}
    
    if(NumOfReloads)
    {
        TIMERx->ARR = 0xFFFF;
        while(NumOfReloads--)
        {
            TIMERx->STATUS = 0UL;
            while((TIMERx->STATUS & TIMER_STATUS_CNT_ARR_EVENT) == 0UL){}
        }
    }
#elif (defined (USE_K1986VE1xI))
    uint32_t StartTicks;

    StartTicks = TIMERx->CNT;
    while((TIMERx->CNT - StartTicks) < Ticks){}
#endif
}

#if ((defined (USE_K1986VE9xI)) || (defined (USE_MDR32FG16S1QI)))
/**
  * @brief  Initializes and starts the DWT clock cycle counter (CYCCNT).
  * @param  None.
  * @retval None.
  */
void DELAY_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  Performs a blocking delay with a DWT clock cycle counter (CYCCNT) in ticks.
  * @param  Ticks: specifies the delay time in ticks.
  * @retval None.
  */
void DELAY_DWT_WaitTicks(uint32_t Ticks)
{
    uint32_t StartTicks;

    StartTicks = DWT->CYCCNT;
    while((DWT->CYCCNT - StartTicks) < Ticks){}
}
#endif

/** @} */ /* End of group UTILS_DELAY_Private_Functions */

/** @defgroup UTILS_STDIO_Private_Functions UTILS STDIO
 * @{
 */

#if defined (_USE_DEBUG_UART_)
/**
 * @brief  Initializes the UARTx peripheral according to the specified
 *         parameters _USE_DEBUG_UART_ in MDR32FxQI_config.h for standard input/output.
 * @param  STDIO_UARTx: Select the UART peripheral.
  *        This parameter can be one of the MDR_UARTx values, where x is a number:
  *            1, 2 for MDR32F9Q2I, K1986VE9xI and MDR32F1QI, K1986VE1xI;
  *            1, 2, 3 for MDR32FG16S1QI.
 * @param  STDIO_UART_InitStruct: pointer to a @ref STDIO_UART_InitTypeDef structure
 *         that contains the configuration information for the specified UART peripheral.
 * @retval @ref ErrorStatus - The UARTx peripheral initialization status.
 */
ErrorStatus STDIO_UartInit(MDR_UART_TypeDef* STDIO_UARTx, STDIO_UART_InitTypeDef* STDIO_UART_InitStruct)
{
    PORT_InitTypeDef PORT_InitStructure;
    UART_InitTypeDef UART_InitStructure;
    ErrorStatus      Status = ERROR;

    /* Check the parameters */
    assert_param(IS_STDIO_UART_ALL_PERIPH(STDIO_UARTx));
    assert_param(IS_UART_BAUDRATE(STDIO_UART_InitStruct->STDIO_UART_BaudRate));
    assert_param(IS_UART_WORD_LENGTH(STDIO_UART_InitStruct->STDIO_UART_WordLength));
    assert_param(IS_UART_STOPBITS(STDIO_UART_InitStruct->STDIO_UART_StopBits));
    assert_param(IS_UART_PARITY(STDIO_UART_InitStruct->STDIO_UART_Parity));
    assert_param(IS_STDIO_PORT_ALL_PERIPH(STDIO_UART_InitStruct->STDIO_UART_PORTx));
    assert_param(IS_PORT_PIN(STDIO_UART_InitStruct->STDIO_UART_PORT_Pin));
    assert_param(IS_PORT_FUNC(STDIO_UART_InitStruct->STDIO_UART_PORT_Func));

    /* Configure STDIO UART Pins */
    RST_CLK_PCLKcmd(PCLK_BIT(STDIO_UART_InitStruct->STDIO_UART_PORTx), ENABLE);

    PORT_StructInit(&PORT_InitStructure);
    PORT_InitStructure.PORT_Pin      = STDIO_UART_InitStruct->STDIO_UART_PORT_Pin;
    PORT_InitStructure.PORT_MODE     = PORT_MODE_DIGITAL;
    PORT_InitStructure.PORT_FUNC     = STDIO_UART_InitStruct->STDIO_UART_PORT_Func;
    PORT_InitStructure.PORT_SPEED    = PORT_SPEED_MAXFAST;

    PORT_Init(STDIO_UART_InitStruct->STDIO_UART_PORTx, &PORT_InitStructure);

    /* Configure STDIO UART */
    RST_CLK_PCLKcmd(PCLK_BIT(STDIO_UARTx), ENABLE);
    UART_BRGInit(STDIO_UARTx, UART_HCLKdiv1);

    UART_DeInit(STDIO_UARTx);

    UART_StructInit(&UART_InitStructure);
    UART_InitStructure.UART_BaudRate            = STDIO_UART_InitStruct->STDIO_UART_BaudRate;
    UART_InitStructure.UART_WordLength          = STDIO_UART_InitStruct->STDIO_UART_WordLength;
    UART_InitStructure.UART_StopBits            = STDIO_UART_InitStruct->STDIO_UART_StopBits;
    UART_InitStructure.UART_Parity              = STDIO_UART_InitStruct->STDIO_UART_Parity;
    UART_InitStructure.UART_FIFOMode            = UART_FIFO_ON;
    UART_InitStructure.UART_HardwareFlowControl = (UART_HardwareFlowControl_RXE |
                                                   UART_HardwareFlowControl_TXE);

    Status = UART_Init(STDIO_UARTx, &UART_InitStructure);

    if (Status == SUCCESS)
    {
        UART_Cmd(STDIO_UARTx, ENABLE);
    }

    return (ErrorStatus)Status;
}
#endif    /* _USE_DEBUG_UART_ */

/** @} */ /* End of group UTILS_STDIO_Private_Functions */

/** @} */ /* End of group UTILS_Private_Functions */

/** @} */ /* End of group UTILS */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_utils.c */

