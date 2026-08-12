/**
  ******************************************************************************
  * @file    MDR32FxQI_uart.h
  * @author  Milandr Application Team
  * @version V2.0.2i
  * @date    14/03/2022
  * @brief   This file contains all the functions prototypes for the UART
  *          firmware library.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MDR32FxQI_UART_H
#define __MDR32FxQI_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32FxQI_config.h"

/** @addtogroup __MDR32FxQI_StdPeriph_Driver MDR32FxQI Standard Peripherial Driver
  * @{
  */

/** @addtogroup UART
  * @{
  */

/** @defgroup UART_Exported_Types UART Exported Types
  * @{
  */

/**
  * @brief UART Word Length
  */
typedef enum
{
    UART_WordLength5b = ((uint16_t)0x00),
    UART_WordLength6b = ((uint16_t)0x20),
    UART_WordLength7b = ((uint16_t)0x40),
    UART_WordLength8b = ((uint16_t)0x60)
} UART_Word_Length_TypeDef;

#define IS_UART_WORD_LENGTH(LENGTH)     (((LENGTH) == UART_WordLength5b) || \
                                         ((LENGTH) == UART_WordLength6b) || \
                                         ((LENGTH) == UART_WordLength7b) || \
                                         ((LENGTH) == UART_WordLength8b))

/**
  * @brief UART Stop Bits
  */
typedef enum
{
    UART_StopBits1 = ((uint16_t)0x00),
    UART_StopBits2 = ((uint16_t)0x08)
} UART_Stop_Bits_TypeDef;

#define IS_UART_STOPBITS(STOPBITS)      (((STOPBITS) == UART_StopBits1) || \
                                         ((STOPBITS) == UART_StopBits2))

/**
  * @brief UART Parity
  */
typedef enum
{
    UART_Parity_No   = ((uint16_t)0x00),
    UART_Parity_Even = ((uint16_t)0x06),
    UART_Parity_Odd  = ((uint16_t)0x02),
    UART_Parity_1    = ((uint16_t)0x82),
    UART_Parity_0    = ((uint16_t)0x86)
} UART_Parity_TypeDef;

#define IS_UART_PARITY(PARITY)      (((PARITY) == UART_Parity_No  ) || \
                                     ((PARITY) == UART_Parity_Even) || \
                                     ((PARITY) == UART_Parity_Odd ) || \
                                     ((PARITY) == UART_Parity_1   ) || \
                                     ((PARITY) == UART_Parity_0   ))

/**
  * @brief UART FIFO Mode
  */
typedef enum
{
    UART_FIFO_OFF = ((uint16_t)0x00),
    UART_FIFO_ON  = ((uint16_t)0x10)
} UART_FIFO_Mode_TypeDef;

#define IS_UART_FIFOMODE(FIFOMODE)      (((FIFOMODE) == UART_FIFO_ON ) || \
                                         ((FIFOMODE) == UART_FIFO_OFF))

/**
  * @brief UART Hardware Flow Control
  */
typedef enum
{
    UART_HardwareFlowControl_None  = ((uint16_t)0x0000),
    UART_HardwareFlowControl_CTSEn = ((uint16_t)0x8000),
    UART_HardwareFlowControl_RTSEn = ((uint16_t)0x4000),
    UART_HardwareFlowControl_Out2  = ((uint16_t)0x2000),
    UART_HardwareFlowControl_Out1  = ((uint16_t)0x1000),
    UART_HardwareFlowControl_RTS   = ((uint16_t)0x0800),
    UART_HardwareFlowControl_DTR   = ((uint16_t)0x0400),
    UART_HardwareFlowControl_RXE   = ((uint16_t)0x0200),
    UART_HardwareFlowControl_TXE   = ((uint16_t)0x0100),
    UART_HardwareFlowControl_LBE   = ((uint16_t)0x0080)
} UART_Hardware_Flow_Control_TypeDef;

#define UART_HARDWARE_FLOW_CONTROL_MASK         ((uint16_t)0xFF80)

#define IS_UART_HARDWARE_FLOW_CONTROL(CONTROL)  (((CONTROL) & (~(UART_HARDWARE_FLOW_CONTROL_MASK))) == 0)

/**
  * @brief UART Interrupt definition
  */
typedef enum
{
    UART_IT_OE  = ((uint32_t)0x0400), /*!< Buffer overflow interrupt (UARTOEINTR) */
    UART_IT_BE  = ((uint32_t)0x0200), /*!< Line break interrupt (UARTBEINTR) */
    UART_IT_PE  = ((uint32_t)0x0100), /*!< Parity error interrupt (UARTPEINTR) */
    UART_IT_FE  = ((uint32_t)0x0080), /*!< Frame structure error interrupt (UARTFEINTR) */
    UART_IT_RT  = ((uint32_t)0x0040), /*!< Data input timeout interrupt (UARTRTINTR) */
    UART_IT_TX  = ((uint32_t)0x0020), /*!< Transmitter interrupt (UARTTXINTR) */
    UART_IT_RX  = ((uint32_t)0x0010), /*!< Receiver interrupt (UARTRXINTR) */
    UART_IT_DSR = ((uint32_t)0x0008), /*!< Line nUARTDSR change interrupt (UARTDSRINTR) */
    UART_IT_DCD = ((uint32_t)0x0004), /*!< Line nUARTDCD change interrupt (UARTDCDINTR) */
    UART_IT_CTS = ((uint32_t)0x0002), /*!< Line nUARTCTS change interrupt (UARTCTSINTR) */
    UART_IT_RI  = ((uint32_t)0x0001)  /*!< Line nUARTRI change interrupt (UARTRIINTR) */
} UART_IT_TypeDef;

#define IS_UART_CONFIG_IT(IT)   (((IT) == UART_IT_OE ) || ((IT) == UART_IT_BE ) || \
                                 ((IT) == UART_IT_PE ) || ((IT) == UART_IT_FE ) || \
                                 ((IT) == UART_IT_RT ) || ((IT) == UART_IT_TX ) || \
                                 ((IT) == UART_IT_RX ) || ((IT) == UART_IT_DSR) || \
                                 ((IT) == UART_IT_DCD) || ((IT) == UART_IT_CTS) || \
                                 ((IT) == UART_IT_RI))

#define IS_UART_CONFIG_ITS(ITS)  (((ITS) <= 0x07FF) && ((ITS) >= 0x1))

/**
  * @brief UART DMA interrupt definition
  */
typedef enum
{
    UART_IT_FIFO_LVL_2words  = ((uint32_t)0x00), /*!< Interrupt on 1/8 buffer filled */
    UART_IT_FIFO_LVL_4words  = ((uint32_t)0x01), /*!< Interrupt on 1/4 buffer filled */
    UART_IT_FIFO_LVL_8words  = ((uint32_t)0x02), /*!< Interrupt on 1/2 buffer filled */
    UART_IT_FIFO_LVL_12words = ((uint32_t)0x03), /*!< Interrupt on 3/4 buffer filled */
    UART_IT_FIFO_LVL_14words = ((uint32_t)0x04)  /*!< Interrupt on 7/8 buffer filled */
} UART_IT_FIFO_TypeDef;

#define IS_UART_IT_FIFO_LVL(IT_FIFO_LVL)    (((IT_FIFO_LVL) == UART_IT_FIFO_LVL_2words ) || \
                                             ((IT_FIFO_LVL) == UART_IT_FIFO_LVL_4words ) || \
                                             ((IT_FIFO_LVL) == UART_IT_FIFO_LVL_8words ) || \
                                             ((IT_FIFO_LVL) == UART_IT_FIFO_LVL_12words) || \
                                             ((IT_FIFO_LVL) == UART_IT_FIFO_LVL_14words))

/**
  * @brief UART DMA request
  */
typedef enum
{
    UART_DMA_RXE   = ((uint32_t)0x01),
    UART_DMA_TXE   = ((uint32_t)0x02),
    UART_DMA_ONERR = ((uint32_t)0x04)
} UART_DMA_Req_TypeDef;

#define IS_UART_DMAREQ(DMAREQ)              (((DMAREQ) == UART_DMA_RXE  ) || \
                                             ((DMAREQ) == UART_DMA_TXE  ) || \
                                             ((DMAREQ) == UART_DMA_ONERR))

#define IS_UART_DMAREQS(DMAREQ)             ((DMAREQ) <= 0x07)

/**
  * @brief UART IrDA Low Power
  */
typedef enum
{
    UART_IrDAMode_LowPower = ((uint32_t)0x04),
    UART_IrDAMode_Normal   = ((uint32_t)0x00)
} UART_IrDA_Mode_TypeDef;

#define IS_UART_IRDA_MODE(MODE)     (((MODE) == UART_IrDAMode_LowPower) || \
                                     ((MODE) == UART_IrDAMode_Normal  ))

/**
  * @brief UART Flags
  */
typedef enum
{
    UART_FLAG_RI   = ((uint32_t)0x0100),
    UART_FLAG_TXFE = ((uint32_t)0x0080),
    UART_FLAG_RXFF = ((uint32_t)0x0040),
    UART_FLAG_TXFF = ((uint32_t)0x0020),
    UART_FLAG_RXFE = ((uint32_t)0x0010),
    UART_FLAG_BUSY = ((uint32_t)0x0008),
    UART_FLAG_DCD  = ((uint32_t)0x0004),
    UART_FLAG_DSR  = ((uint32_t)0x0002),
    UART_FLAG_CTS  = ((uint32_t)0x0001)
} UART_Flags_TypeDef;

#define IS_UART_FLAG(FLAG)  (((FLAG) == UART_FLAG_RI  ) || ((FLAG) == UART_FLAG_TXFE) || \
                             ((FLAG) == UART_FLAG_RXFF) || ((FLAG) == UART_FLAG_TXFF) || \
                             ((FLAG) == UART_FLAG_RXFE) || ((FLAG) == UART_FLAG_BUSY) || \
                             ((FLAG) == UART_FLAG_DCD ) || ((FLAG) == UART_FLAG_DSR ) || \
                             ((FLAG) == UART_FLAG_CTS ))

/**
  * @brief UART Clock BRG
  */
typedef enum
{
    UART_HCLKdiv1   = ((uint32_t)0x00),
    UART_HCLKdiv2   = ((uint32_t)0x01),
    UART_HCLKdiv4   = ((uint32_t)0x02),
    UART_HCLKdiv8   = ((uint32_t)0x03),
    UART_HCLKdiv16  = ((uint32_t)0x04),
    UART_HCLKdiv32  = ((uint32_t)0x05),
    UART_HCLKdiv64  = ((uint32_t)0x06),
    UART_HCLKdiv128 = ((uint32_t)0x07)
} UART_Clock_BRG_TypeDef;

#define IS_UART_CLOCK_BRG(BRG)  (((BRG) == UART_HCLKdiv1)  || ((BRG) == UART_HCLKdiv2)  || \
                                 ((BRG) == UART_HCLKdiv4)  || ((BRG) == UART_HCLKdiv8)  || \
                                 ((BRG) == UART_HCLKdiv16) || ((BRG) == UART_HCLKdiv32) || \
                                 ((BRG) == UART_HCLKdiv64) || ((BRG) == UART_HCLKdiv128))

/**
  * @brief  UART Init Structure definition
  */
typedef struct
{
    uint32_t UART_BaudRate;            /*!< This member configures the UART communication baud rate.
                                            The baud rate is computed using the following formula:
                                             - IntegerDivider = ((UARTCLK) / (16 * (UART_InitStruct->UART_BaudRate)))
                                             - FractionalDivider = ((IntegerDivider - ((u32) IntegerDivider)) * 64) + 0.5 */
    uint16_t UART_WordLength;          /*!< Specifies the number of data bits transmitted or received in a frame.
                                            This parameter is one of @ref UART_Word_Length_TypeDef values. */
    uint16_t UART_StopBits;            /*!< Specifies the number of stop bits transmitted.
                                            This parameter is one of @ref UART_Stop_Bits_TypeDef values. */
    uint16_t UART_Parity;              /*!< Specifies the parity mode.
                                            This parameter is one of @ref UART_Parity_TypeDef values. */
    uint16_t UART_FIFOMode;            /*!< Specifies the FIFO mode.
                                            This parameter is one of @ref UART_FIFO_Mode_TypeDef values. */
    uint16_t UART_HardwareFlowControl; /*!< Specifies whether the hardware flow control mode is enabled or disabled.
                                            This parameter is one of @ref UART_Hardware_Flow_Control_TypeDef values. */
} UART_InitTypeDef;

/** @} */ /* End of group UART_Exported_Types */


/** @defgroup UART_Exported_Constants UART Exported Constants
  * @{
  */

/** @defgroup UART_BAUDRATE UART Baudrate
  * @{
  */

#define IS_UART_BAUDRATE(BAUDRATE) (((BAUDRATE) > 0) && ((BAUDRATE) <= 921600))

/** @} */ /* End of group UART_BAUDRATE */

/** @defgroup UART_Data_Status UART Data Status
  * @{
  */

#define UART_Data_OE                        0x0B
#define UART_Data_BE                        0x0A
#define UART_Data_PE                        0x09
#define UART_Data_FE                        0x08

#define UART_Data_OE_Msk                    ((uint32_t)(1 << UART_Data_OE))
#define UART_Data_BE_Msk                    ((uint32_t)(1 << UART_Data_BE))
#define UART_Data_PE_Msk                    ((uint32_t)(1 << UART_Data_FE))
#define UART_Data_FE_Msk                    ((uint32_t)(1 << UART_Data_PE))

#define UART_Data(DATA)                     ((uint8_t)(DATA))
#define UART_Flags(DATA)                    ((uint8_t)((DATA) >> 8))
#define UART_CheckDataStatus(DATA, BIT)     ((((uint32_t)(DATA)) << (31 - (BIT))) >> 31)

#define IS_UART_DATA(DATA)                  ((DATA) <= 0xFF)

/** @} */ /* End of group UART_Data_Status */

/** @} */ /* End of group UART_Exported_Constants */


/** @defgroup UART_Exported_Functions UART Exported Functions
  * @{
  */

void UART_DeInit(MDR_UART_TypeDef* UARTx);

ErrorStatus UART_Init(MDR_UART_TypeDef* UARTx, UART_InitTypeDef* UART_InitStruct);

void UART_StructInit(UART_InitTypeDef* UART_InitStruct);

void UART_Cmd(MDR_UART_TypeDef* UARTx, FunctionalState NewState);

void UART_ITConfig(MDR_UART_TypeDef* UARTx, uint32_t UART_IT, FunctionalState NewState);
ITStatus UART_GetITStatus(MDR_UART_TypeDef* UARTx, UART_IT_TypeDef UART_IT);
ITStatus UART_GetITStatusMasked(MDR_UART_TypeDef* UARTx, UART_IT_TypeDef UART_IT);
void UART_ClearITPendingBit(MDR_UART_TypeDef* UARTx, uint32_t UART_IT);

void UART_DMAConfig(MDR_UART_TypeDef* UARTx, UART_IT_FIFO_TypeDef UART_IT_RB_LVL, UART_IT_FIFO_TypeDef UART_IT_TB_LVL);
void UART_DMACmd(MDR_UART_TypeDef* UARTx, uint32_t UART_DMAReq, FunctionalState NewState);

void UART_SendData(MDR_UART_TypeDef* UARTx, uint8_t Data);
uint16_t UART_ReceiveData(MDR_UART_TypeDef* UARTx);
void UART_BreakLine(MDR_UART_TypeDef* UARTx, FunctionalState NewState);

void UART_IrDAConfig(MDR_UART_TypeDef* UARTx, UART_IrDA_Mode_TypeDef UART_IrDAMode);
void UART_IrDACmd(MDR_UART_TypeDef* UARTx, FunctionalState NewState);

FlagStatus UART_GetFlagStatus(MDR_UART_TypeDef* UARTx, UART_Flags_TypeDef UART_FLAG);

void UART_BRGInit(MDR_UART_TypeDef* UARTx, UART_Clock_BRG_TypeDef UART_BRG);

/** @} */ /* End of group UART_Exported_Functions */

/** @} */ /* End of group UART */

/** @} */ /* End of group __MDR32FxQI_StdPeriph_Driver */

#ifdef __cplusplus
} // extern "C" block end
#endif

#endif /* __MDR32FxQI_UART_H */

/*********************** (C) COPYRIGHT 2025 Milandr ****************************
*
* END OF FILE MDR32FxQI_uart.h */


