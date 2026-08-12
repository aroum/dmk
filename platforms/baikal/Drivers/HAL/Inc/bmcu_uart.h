/**
 * *****************************************************************************
 *  @file       bmcu_uart.h
 *  @author     Baikal electronics SDK team
 *  @brief      Universal Asynchronous Receiver-Transmitter (UART) module
 *              driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BMCU_UART_H
#define __BMCU_UART_H

#include "bmcu_common.h"
#include "bmcu_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Line status flags */
#define UART_LINE_STATUS_DR                 UART_LSR_DR         /*!< Data Ready */
#define UART_LINE_STATUS_OE                 UART_LSR_OE         /*!< Overrun Error */
#define UART_LINE_STATUS_PE                 UART_LSR_PE         /*!< Parity Error */
#define UART_LINE_STATUS_FE                 UART_LSR_FE         /*!< Framing Error */
#define UART_LINE_STATUS_BI                 UART_LSR_BI         /*!< Break Interrupt */
#define UART_LINE_STATUS_THRE               UART_LSR_THRE       /*!< Transmitter Holding Register Empty */
#define UART_LINE_STATUS_TEMT               UART_LSR_TEMT       /*!< Transmitter Empty */
#define UART_LINE_STATUS_RFE                UART_LSR_RFE        /*!< Receiver FIFO Error */
#define UART_LINE_STATUS_ADDR_RCVD          UART_LSR_ADDR_RCVD  /*!< Address Received */

/** @brief Modem inputs */
#define UART_MODEM_INPUT_DCTS               UART_MSR_DCTS       /*!< Delta Clear to Send */
#define UART_MODEM_INPUT_DDSR               UART_MSR_DDSR       /*!< Delta Data Set Ready */
#define UART_MODEM_INPUT_TERI               UART_MSR_TERI       /*!< Trailing Edge of Ring Indicator */
#define UART_MODEM_INPUT_DDCD               UART_MSR_DDCD       /*!< Delta Data Carrier Detect */
#define UART_MODEM_INPUT_CTS                UART_MSR_CTS        /*!< Clear to Send */
#define UART_MODEM_INPUT_DSR                UART_MSR_DSR        /*!< Data Set Ready */
#define UART_MODEM_INPUT_RI                 UART_MSR_RI         /*!< Ring Indicator */
#define UART_MODEM_INPUT_DCD                UART_MSR_DCD        /*!< Data Carrier Detect */

/** @brief Modem outputs */
typedef enum {
    UART_MODEM_OUTPUT_DTR  = 0,     /*!< Data Terminal Ready */
    UART_MODEM_OUTPUT_RTS  = 1,     /*!< Request to Send */
    UART_MODEM_OUTPUT_OUT1 = 2,     /*!< User Output 1 */
    UART_MODEM_OUTPUT_OUT2 = 3      /*!< User Output 1 */
} UART_ModemOutput_TypeDef;

/** @brief Line status clear mode */
typedef enum {
    UART_LS_CLR_FIFO_OR_LSR = UART_IER_ELCOLR_DISABLE,  /*!< Line status bits are cleared either on reading Rx FIFO (RBR read) or on reading LSR register */
    UART_LS_CLR_LSR_ONLY    = UART_IER_ELCOLR_ENABLE    /*!< Line status bits are cleared only on reading LSR register */
} UART_LineStatusClearMode_TypeDef;

/** @brief Flags */
typedef enum {
    UART_FLAG_BUSY = 0,     /*!< UART Busy */
    UART_FLAG_TFNF = 1,     /*!< Transmit FIFO Not Full */
    UART_FLAG_TFE  = 2,     /*!< Transmit FIFO Empty */
    UART_FLAG_RFNE = 3,     /*!< Receive FIFO Not Empty */
    UART_FLAG_RFF  = 4      /*!< Receive FIFO Full */
} UART_Flag_TypeDef;

/** @brief Interrupts */
typedef enum {
    UART_IT_RDA  = 0,   /*!< Receive Data Available */
    UART_IT_THRE = 1,   /*!< Transmit Holding Register Empty */
    UART_IT_LS   = 2,   /*!< Line Status */
    UART_IT_MS   = 3    /*!< Modem Status */
} UART_IT_TypeDef;

/** @brief Interrupt ID */
typedef enum {
    UART_IID_MODEM_STATUS = 0x0,        /*!< Modem status */
    UART_IID_NO_INT_PENDING = 0x1,      /*!< No interrupt pending */
    UART_IID_THR_EMPTY = 0x2,           /*!< THR empty */
    UART_IID_RX_DATA_AVAILABLE = 0x4,   /*!< Received data available */
    UART_IID_RX_LINE_STATUS = 0x6,      /*!< Receiver line status */
    UART_IID_BUSY_DETECT = 0x7,         /*!< Busy detect */
    UART_IID_CHAR_TIMEOUT = 0xC         /*!< Character timeout */
} UART_IID_TypeDef;

/** @brief UART parity */
typedef enum {
    UART_PARITY_NONE = UART_LCR_PEN_DISABLE,                        /*!< Parity disabled */
    UART_PARITY_ODD  = (UART_LCR_PEN_ENABLE | UART_LCR_EPS_ODD),    /*!< Odd parity */
    UART_PARITY_EVEN = (UART_LCR_PEN_ENABLE | UART_LCR_EPS_EVEN)    /*!< Even parity */
} UART_Parity_TypeDef;

/** @brief UART data width */
typedef enum {
    UART_DATAWIDTH_5B = UART_LCR_DLS_CHAR_5BITS,    /*!< 5 data bits per character */
    UART_DATAWIDTH_6B = UART_LCR_DLS_CHAR_6BITS,    /*!< 6 data bits per character */
    UART_DATAWIDTH_7B = UART_LCR_DLS_CHAR_7BITS,    /*!< 7 data bits per character */
    UART_DATAWIDTH_8B = UART_LCR_DLS_CHAR_8BITS     /*!< 8 data bits per character */
} UART_DataWidth_TypeDef;

/** @brief UART stop bits */
typedef enum {
    UART_STOP_1BIT           = UART_LCR_STOP_1BIT,              /*!< 1 stop bit */
    UART_STOP_1_5BIT_OR_2BIT = UART_LCR_STOP_1_5BIT_OR_2BIT     /*!< 1.5 stop bits when data length is 5 bits, otherwise 2 stop bits */
} UART_StopBits_TypeDef;

/** @brief UART 9-bit transfer mode */
typedef enum {
    UART_TRANSFER_9B_DISABLE = 0,       /*!< 9-bit transfer disabled */
    UART_TRANSFER_9B_HW_MODE,           /*!< 9-bit transfer in hardware address match mode */
    UART_TRANSFER_9B_SW_MODE            /*!< 9-bit transfer in software address match mode */
} UART_Transfer9b_TypeDef;

/** @brief UART Tx FIFO threshold level */
typedef enum {
    UART_TX_FIFO_EMPTY        = UART_FCR_TET_FIFO_EMPTY,            /*!< FIFO empty */
    UART_TX_FIFO_CHAR_2       = UART_FCR_TET_FIFO_CHAR_2,           /*!< 2 characters in FIFO */
    UART_TX_FIFO_QUARTER_FULL = UART_FCR_TET_FIFO_QUARTER_FULL,     /*!< FIFO 1/4 full */
    UART_TX_FIFO_HALF_FULL    = UART_FCR_TET_FIFO_HALF_FULL         /*!< FIFO 1/2 full */
} UART_TxFIFOThreshold_TypeDef;

/** @brief UART Rx FIFO threshold level */
typedef enum {
    UART_RX_FIFO_CHAR_1       = UART_FCR_RT_FIFO_CHAR_1,            /*!< 1 character in FIFO */
    UART_RX_FIFO_QUARTER_FULL = UART_FCR_RT_FIFO_QUARTER_FULL,      /*!< FIFO 1/4 full */
    UART_RX_FIFO_HALF_FULL    = UART_FCR_RT_FIFO_HALF_FULL,         /*!< FIFO 1/2 full */
    UART_RX_FIFO_FULL_2       = UART_FCR_RT_FIFO_FULL_2             /*!< FIFO 2 less than full */
} UART_RxFIFOThreshold_TypeDef;

/** @brief UART DMA signalling mode */
typedef enum {
    UART_DMA_MODE0 = UART_FCR_DMAM_MODE0,   /*!< DMA Mode 0: Supports single DMA data transfers */
    UART_DMA_MODE1 = UART_FCR_DMAM_MODE1    /*!< DMA Mode 1: Supports multi DMA data transfers */
} UART_DMAMode_TypeDef;

/** @brief UART Init Structure definition */
typedef struct
{
    uint32_t BaudRate;                              /*!< UART communication baud rate */
    UART_DataWidth_TypeDef DataWidth;               /*!< The number of data bits in a character */
    UART_StopBits_TypeDef StopBits;                 /*!< The number of stop bits per character */
    UART_Parity_TypeDef Parity;                     /*!< Parity mode */
    UART_Transfer9b_TypeDef Transfer9b;             /*!< 9-bit transfer mode */
    FunctionalState CtrlFIFO;                       /*!< FIFO control */
    UART_TxFIFOThreshold_TypeDef TxFIFOThreshold;   /*!< Tx FIFO threshold level */
    UART_RxFIFOThreshold_TypeDef RxFIFOThreshold;   /*!< Rx FIFO threshold level */
    FunctionalState CtrlIrdaMode;                   /*!< IrDA mode control */
    FunctionalState CtrlLoopbackMode;               /*!< Loopback mode control */
} UART_InitStruct_TypeDef;

/**
 * @brief Writes a value in UART register.
 * @param INSTANCE The UART instance.
 * @param REG The register to be written.
 * @param VALUE The value to be written in the register.
 */
#define UART_WriteReg(INSTANCE, REG, VALUE)     WRITE_REG(INSTANCE->REG, (VALUE))

/**
 * @brief Reads a value in UART register.
 * @param INSTANCE The UART instance.
 * @param REG The register to be read.
 * @returns The register value.
 */
#define UART_ReadReg(INSTANCE, REG)             READ_REG(INSTANCE->REG)

/**
 * @brief Resets UART.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_Reset(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->SRR, UART_SRR_UR | UART_SRR_RFR | UART_SRR_XFR);
}

/**
 * @brief Enables IrDA mode.
 * @note In IrDA mode the number of data bits is fixed to 8 bits. No parity
 *       information can be supplied and only one stop bit is used in this mode.
 *       Trying to adjust the number of data bits or enable parity has no effect.
 * @note IrDA mode specifies the maximum baudrate of 115.2Kbaud.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_EnableIrda(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->MCR, UART_MCR_SIRE);
}

/**
 * @brief Disables IrDA mode.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_DisableIrda(UART_TypeDef *UARTx)
{
    CLEAR_BIT(UARTx->MCR, UART_MCR_SIRE);
}

/**
 * @brief Checks if IrDA mode is enabled.
 * @param UARTx The UART instance.
 * @retval 1 if IrDA mode is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t UART_IsEnabledIrda(UART_TypeDef *UARTx)
{
    return ((READ_BIT(UARTx->MCR, UART_MCR_SIRE) == UART_MCR_SIRE) ? 1UL : 0UL);
}

/**
 * @brief Enables loopback mode.
 * @note The loopback mode is used to put UART in diagnostic mode for test.
 * @note If operating in UART mode, the output is held high, while serial data
 *       output is looped back internally to the input. The modem control inputs
 *       are disconnected and the modem control outputs are looped back
 *       internally to the inputs.
 *       If operating in IrDA mode, the output is held low, while serial data
 *       output is inverted and looped back internally to the input.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_EnableLoopback(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->MCR, UART_MCR_LOOPBACK);
}

/**
 * @brief Disables loopback mode.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_DisableLoopback(UART_TypeDef *UARTx)
{
    CLEAR_BIT(UARTx->MCR, UART_MCR_LOOPBACK);
}

/**
 * @brief Checks if loopback mode is enabled.
 * @param UARTx The UART instance.
 * @retval 1 if loopback mode is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t UART_IsEnabledLoopback(UART_TypeDef *UARTx)
{
    return ((READ_BIT(UARTx->MCR, UART_MCR_LOOPBACK) == UART_MCR_LOOPBACK) ? 1UL : 0UL);
}

/**
 * @brief Sets the UART baudrate.
 * @note Can be set only in idle or inactive state.
 *       Use UART_IsActiveFlag() function with UART_FLAG_BUSY flag to check 
 *       if UART is busy or not.
 * @param UARTx The UART instance.
 * @param PeriphClk The peripheral clock frequency (Hz).
 * @param BaudRate The baudrate.
 */
__STATIC_INLINE void UART_SetBaudRate(UART_TypeDef *UARTx, uint32_t PeriphClk, uint32_t BaudRate)
{
    uint32_t div = PeriphClk / BaudRate;
    uint32_t div_int = div >> 4;
    uint32_t div_fract = div & 0xFUL;

    /* Set the fractional part of the divisor */
    /* It must be configured before DLL and DLH registers */
    WRITE_REG(UARTx->DLF, (div_fract << UART_DLF_Pos) & UART_DLF);

    /* Set the integer part of the divisor */
    SET_BIT(UARTx->LCR, UART_LCR_DLAB);

    WRITE_REG(UARTx->DLL, (div_int << UART_DLL_Pos) & UART_DLL);
    WRITE_REG(UARTx->DLH, ((div_int >> 8) << UART_DLH_Pos) & UART_DLH);

    CLEAR_BIT(UARTx->LCR, UART_LCR_DLAB);
}

/**
 * @brief Returns the current UART baudrate.
 * @note Can be set only in idle or inactive state.
 *       Use UART_IsActiveFlag() function with UART_FLAG_BUSY flag to check 
 *       if UART is busy or not.
 * @param UARTx The UART instance.
 * @param PeriphClk The peripheral clock frequency (Hz).
 * @returns The UART baudrate.
 */
__STATIC_INLINE uint32_t UART_GetBaudRate(UART_TypeDef *UARTx, uint32_t PeriphClk)
{
    uint32_t baudrate = 0UL;

    SET_BIT(UARTx->LCR, UART_LCR_DLAB);

    uint32_t div_int = (READ_BIT(UARTx->DLH, UART_DLH) >> UART_DLH_Pos);
    div_int <<= 8;
    div_int |= (READ_BIT(UARTx->DLL, UART_DLL) >> UART_DLL_Pos);

    CLEAR_BIT(UARTx->LCR, UART_LCR_DLAB);

    uint32_t div_fract = (READ_BIT(UARTx->DLF, UART_DLF) >> UART_DLF_Pos);

    if (div_int > 0UL)
    {
        baudrate = PeriphClk / (16UL * div_int + div_fract);
    }

    return baudrate;
}

/**
 * @brief Sets the parity mode.
 * @note Can be set only in idle or inactive state.
 *       Use UART_IsActiveFlag() function with UART_FLAG_BUSY flag to check 
 *       if UART is busy or not.
 * @param UARTx The UART instance.
 * @param Parity The parity mode.
 */
__STATIC_INLINE void UART_SetParity(UART_TypeDef *UARTx, UART_Parity_TypeDef Parity)
{
    MODIFY_REG(UARTx->LCR, UART_LCR_PEN | UART_LCR_EPS, (uint32_t)Parity);
}

/**
 * @brief Returns the parity mode.
 * @param UARTx The UART instance.
 * @returns The parity mode.
 */
__STATIC_INLINE UART_Parity_TypeDef UART_GetParity(UART_TypeDef *UARTx)
{
    return (UART_Parity_TypeDef)READ_BIT(UARTx->LCR, UART_LCR_PEN | UART_LCR_EPS);
}

/**
 * @brief Sets the number of data bits per character.
 * @note Can be set only in idle or inactive state.
 *       Use UART_IsActiveFlag() function with UART_FLAG_BUSY flag to check 
 *       if UART is busy or not.
 * @param UARTx The UART instance.
 * @param DataWidth The number of data bits.
 */
__STATIC_INLINE void UART_SetDataWidth(UART_TypeDef *UARTx, UART_DataWidth_TypeDef DataWidth)
{
    MODIFY_REG(UARTx->LCR, UART_LCR_DLS, (uint32_t)DataWidth);
}

/**
 * @brief Returns the number of data bits per character.
 * @param UARTx The UART instance.
 * @returns The number of data bits.
 */
__STATIC_INLINE UART_DataWidth_TypeDef UART_GetDataWidth(UART_TypeDef *UARTx)
{
    return (UART_DataWidth_TypeDef)READ_BIT(UARTx->LCR, UART_LCR_DLS);
}

/**
 * @brief Sets the number of stop bits per character.
 * @note Can be set only in idle or inactive state.
 *       Use UART_IsActiveFlag() function with UART_FLAG_BUSY flag to check 
 *       if UART is busy or not.
 * @param UARTx The UART instance.
 * @param StopBits The number of stop bits.
 */
__STATIC_INLINE void UART_SetStopBitsLength(UART_TypeDef *UARTx, UART_StopBits_TypeDef StopBits)
{
    MODIFY_REG(UARTx->LCR, UART_LCR_STOP, (uint32_t)StopBits);
}

/**
 * @brief Returns the number of stop bits per character.
 * @param UARTx The UART instance.
 * @returns The number of stop bits.
 */
__STATIC_INLINE UART_StopBits_TypeDef UART_GetStopBitsLength(UART_TypeDef *UARTx)
{
    return (UART_StopBits_TypeDef)READ_BIT(UARTx->LCR, UART_LCR_STOP);
}

/**
 * @brief Configures character frame format.
 * @note Can be set only in idle or inactive state.
 *       Use UART_IsActiveFlag() function with UART_FLAG_BUSY flag to check 
 *       if UART is busy or not.
 * @param UARTx The UART instance.
 * @param DataWidth The number of data bits.
 * @param Parity The parity mode.
 * @param StopBits The number of stop bits.
 */
__STATIC_INLINE void UART_ConfigCharacter(UART_TypeDef *UARTx,
                                          UART_DataWidth_TypeDef DataWidth,
                                          UART_Parity_TypeDef Parity,
                                          UART_StopBits_TypeDef StopBits)
{
    MODIFY_REG(UARTx->LCR,
               (UART_LCR_EPS | UART_LCR_PEN | UART_LCR_STOP | UART_LCR_DLS),
               (Parity | StopBits | DataWidth));
}

/**
 * @brief Asserts the modem output line.
 * @param UARTx The UART instance.
 * @param Output The output line. Can be one of UART_ModemOutput_TypeDef values.
 */
__STATIC_INLINE void UART_AssertModemOutput(UART_TypeDef *UARTx, UART_ModemOutput_TypeDef Output)
{
    SET_BIT(UARTx->MCR, (1UL << Output));
}

/**
 * @brief Deasserts the modem output line.
 * @param UARTx The UART instance.
 * @param Output The output line. Can be one of UART_ModemOutput_TypeDef values.
 */
__STATIC_INLINE void UART_DeassertModemOutput(UART_TypeDef *UARTx, UART_ModemOutput_TypeDef Output)
{
    CLEAR_BIT(UARTx->MCR, (1UL << Output));
}

/**
 * @brief Gets modem status.
 * @note Reading modem status clears Delta Clear to Send (DCTS),
 *       Delta Data Set Ready (DDSR), Trailing Edge of Ring Indicator (TERI)
 *       and Delta Data Carrier Detect (DDCD) status bits.
 * @param UARTx The UART instance.
 * @returns The bit mask combination of the next flags:
 *          - @ref UART_MODEM_INPUT_DCTS: Delta Clear to Send
 *          - @ref UART_MODEM_INPUT_DDSR: Delta Data Set Ready
 *          - @ref UART_MODEM_INPUT_TERI: Trailing Edge of Ring Indicator
 *          - @ref UART_MODEM_INPUT_DDCD: Delta Data Carrier Detect
 *          - @ref UART_MODEM_INPUT_CTS: Clear to Send
 *          - @ref UART_MODEM_INPUT_DSR: Data Set Ready
 *          - @ref UART_MODEM_INPUT_RI: Ring Indicator
 *          - @ref UART_MODEM_INPUT_DCD: Data Carrier Detect
 */
__STATIC_INLINE uint32_t UART_GetModemStatus(UART_TypeDef *UARTx)
{
    return READ_BIT(UARTx->MSR, (UART_MSR_DCD | UART_MSR_RI | UART_MSR_DSR |
                                 UART_MSR_CTS | UART_MSR_DDCD | UART_MSR_TERI |
                                 UART_MSR_DDSR | UART_MSR_DCTS));
}

/**
 * @brief Gets line status.
 * @note Reading line status clears Overrun Error (OE), Parity Error (PE),
 *       Framing Error (FE) and Break Interrupt (BI) status bits.
 * @note Receiver FIFO Error (RFE) is cleared on status read if the character
 *       with the error is at the top of the receiver FIFO and there are
 *       no subsequent errors in the FIFO.
 * @note Reading line status clears ADDR_RCVD status bit in 9-bit transfer mode.
 * @param UARTx The UART instance.
 * @returns The bit mask combination of the next flags:
 *          - @ref UART_LINE_STATUS_DR: Data Ready
 *          - @ref UART_LINE_STATUS_OE: Overrun Error
 *          - @ref UART_LINE_STATUS_PE: Parity Error
 *          - @ref UART_LINE_STATUS_FE: Framing Error
 *          - @ref UART_LINE_STATUS_BI: Break Interrupt
 *          - @ref UART_LINE_STATUS_THRE: Transmitter Holding Register Empty
 *          - @ref UART_LINE_STATUS_TEMT: Transmitter Empty
 *          - @ref UART_LINE_STATUS_RFE: Receiver FIFO Error
 *          - @ref UART_LINE_STATUS_ADDR_RCVD: Address Received
 */
__STATIC_INLINE uint32_t UART_GetLineStatus(UART_TypeDef *UARTx)
{
    return READ_BIT(UARTx->LSR, (UART_LSR_ADDR_RCVD | UART_LSR_RFE | UART_LSR_TEMT |
                                 UART_LSR_THRE | UART_LSR_BI | UART_LSR_FE |
                                 UART_LSR_PE | UART_LSR_OE | UART_LSR_DR));
}

/**
 * @brief Sets line status clear mode.
 * @note This is applicable only for Overrun Error, Parity Error, Framing Error
 *       and Break Interrupt status bits.
 * @param UARTx The UART instance.
 * @param Mode The mode. Can be one of UART_LineStatusClearMode_TypeDef values.
 */
__STATIC_INLINE void UART_SetLineStatusClearMode(UART_TypeDef *UARTx, UART_LineStatusClearMode_TypeDef Mode)
{
    MODIFY_REG(UARTx->IER, UART_IER_ELCOLR, (uint32_t)Mode);
}

/**
 * @brief Returns line status clear mode.
 * @note This is applicable only for Overrun Error, Parity Error, Framing Error
 *       and Break Interrupt status bits.
 * @param UARTx The UART instance.
 * @returns The mode. Can be one of UART_LineStatusClearMode_TypeDef values.
 */
__STATIC_INLINE UART_LineStatusClearMode_TypeDef UART_GetLineStatusClearMode(UART_TypeDef *UARTx)
{
    return ((UART_LineStatusClearMode_TypeDef)READ_BIT(UARTx->IER, UART_IER_ELCOLR));
}

/**
 * @brief Checks if flag is set.
 * @param UARTx The UART instance.
 * @param Flag The flag. Can be one of UART_Flag_TypeDef values.
 * @retval The state of flag (1 or 0).
 */
__STATIC_INLINE uint32_t UART_IsActiveFlag(UART_TypeDef *UARTx, UART_Flag_TypeDef Flag)
{
    return ((READ_BIT(UARTx->USR, (1UL << Flag)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables UART interrupt.
 * @param UARTx The UART instance.
 * @param IT The interrupt source. Can be one of UART_IT_TypeDef values.
 */
__STATIC_INLINE void UART_EnableIT(UART_TypeDef *UARTx, UART_IT_TypeDef IT)
{
    SET_BIT(UARTx->IER, (1UL << IT));
}

/**
 * @brief Disables UART interrupt.
 * @param UARTx The UART instance.
 * @param IT The interrupt source. Can be one of UART_IT_TypeDef values.
 */
__STATIC_INLINE void UART_DisableIT(UART_TypeDef *UARTx, UART_IT_TypeDef IT)
{
    CLEAR_BIT(UARTx->IER, (1UL << IT));
}

/**
 * @brief Checks if UART interrupt is enabled.
 * @param UARTx The UART instance.
 * @param IT The interrupt source. Can be one of UART_IT_TypeDef values.
 * @retval 1 if the interrupt is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t UART_IsEnabledIT(UART_TypeDef *UARTx, UART_IT_TypeDef IT)
{
    return ((READ_BIT(UARTx->IER, (1UL << IT)) != 0x0UL) ? 1UL : 0UL);
}

/**
 * @brief Enables Programmable THRE Interrupt Mode (PTIM).
 * @note When FIFO mode and THRE interrupts are enabled, the THRE interrupts
 *       are active at or below a programmed transmitter FIFO empty threshold
 *       level if PTIM is enabled, as opposed to FIFO empty.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_EnablePTIM(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->IER, UART_IER_PTIME);
}

/**
 * @brief Disables Programmable THRE Interrupt Mode (PTIM).
 * @note When FIFO mode and THRE interrupts are enabled, the THRE interrupts
 *       are active at or below a programmed transmitter FIFO empty threshold
 *       level if PTIM is enabled, as opposed to FIFO empty.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_DisablePTIM(UART_TypeDef *UARTx)
{
    CLEAR_BIT(UARTx->IER, UART_IER_PTIME);
}

/**
 * @brief Checks if Programmable THRE Interrupt Mode (PTIM) is enabled.
 * @note When FIFO mode and THRE interrupts are enabled, the THRE interrupts
 *       are active at or below a programmed transmitter FIFO empty threshold
 *       level if PTIM is enabled, as opposed to FIFO empty.
 * @param UARTx The UART instance.
 * @retval 1 if PTIM is enabled, otherwise 0.
 */
__STATIC_INLINE uint32_t UART_IsEnabledPTIM(UART_TypeDef *UARTx)
{
    return ((READ_BIT(UARTx->IER, UART_IER_PTIME) == UART_IER_PTIME) ? 1UL : 0UL);
}

/**
 * @brief Returns the highest priority pending interrupt.
 * @param UARTx The UART instance.
 * @retval The interrupt ID. Can be one of UART_IID_TypeDef values.
 */
__STATIC_INLINE UART_IID_TypeDef UART_GetIID(UART_TypeDef *UARTx)
{
    return ((UART_IID_TypeDef)READ_BIT(UARTx->IIR, UART_IIR_IID));
}

/**
 * @brief Enables the transmit and receive FIFOs.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_EnableFIFO(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->FCR, UART_FCR_FIFOE);
}

/**
 * @brief Disables the transmit and receive FIFOs.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_DisableFIFO(UART_TypeDef *UARTx)
{
    CLEAR_BIT(UARTx->FCR, UART_FCR_FIFOE);
}

/**
 * @brief Resets the transmit FIFO.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_ResetTxFIFO(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->FCR, UART_FCR_XFIFOR);
}

/**
 * @brief Resets the receive FIFO.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_ResetRxFIFO(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->FCR, UART_FCR_RFIFOR);
}

/**
 * @brief Sets the transmit FIFO threshold level.
 * @note This is used to select the empty threshold level at which
 *       THRE interrupt will be generated when FIFO mode is active.
 * @param UARTx The UART instance.
 * @param Level The transmit FIFO threshold level.
 */
__STATIC_INLINE void UART_SetTxFIFOThresholdLevel(UART_TypeDef *UARTx, UART_TxFIFOThreshold_TypeDef Level)
{
    MODIFY_REG(UARTx->FCR, UART_FCR_TET, (uint32_t)Level);
}

/**
 * @brief Sets the receive FIFO threshold level.
 * @note This is used to select the trigger level at which Received Data Available
 *       interrupt will be generated in FIFO mode.
 * @param UARTx The UART instance.
 * @param Level The receive FIFO threshold level.
 */
__STATIC_INLINE void UART_SetRxFIFOThresholdLevel(UART_TypeDef *UARTx, UART_RxFIFOThreshold_TypeDef Level)
{
    MODIFY_REG(UARTx->FCR, UART_FCR_RT, (uint32_t)Level);
}

/**
 * @brief Returns the number of data entries in the transmit FIFO.
 * @param UARTx The UART instance.
 * @returns The number of data entries.
 */
__STATIC_INLINE uint8_t UART_GetTxFIFOLevel(UART_TypeDef *UARTx)
{
    return (uint8_t)READ_BIT(UARTx->TFL, UART_TFL);
}

/**
 * @brief Returns the number of data entries in the receive FIFO.
 * @param UARTx The UART instance.
 * @returns The number of data entries.
 */
__STATIC_INLINE uint8_t UART_GetRxFIFOLevel(UART_TypeDef *UARTx)
{
    return (uint8_t)READ_BIT(UARTx->RFL, UART_RFL);
}

/**
 * @brief Sets DMA mode.
 * @note Mode 0: Supports single DMA data transfers at a time.
 *       Mode 1: Supports multi DMA data transfers, where multiple transfers
 *               are made continuously until the receiver FIFO has been emptied
 *               or the transmit FIFO has been filled.
 * @note Only DMA mode 0 is available when FIFOs are disabled.
 * @param UARTx The UART instance.
 * @param Mode The DMA mode.
 */
__STATIC_INLINE void UART_DMA_SetMode(UART_TypeDef *UARTx, UART_DMAMode_TypeDef Mode)
{
    MODIFY_REG(UARTx->FCR, UART_FCR_DMAM, (uint32_t)Mode);
}

/**
 * @brief Performs DMA software acknowledge.
 * @note DMA software acknowledge is used if a transfer needs to be terminated
 *       due to an error condition. For example, if DMA disables the channel,
 *       then UART should clear its request.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_DMA_Ack(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->DMASA, UART_DMASA);
}

/**
 * @brief Returns the address of the data register used for DMA transfer.
 * @note The address of the data register is valid for both transmit and
 *       receive transfers.
 * @param UARTx The UART instance.
 * @retval The address of the data register.
 */
__STATIC_INLINE uint32_t UART_DMA_GetRegAddr(UART_TypeDef *UARTx)
{
    return ((uint32_t)&(UARTx->RBR));
}

/**
 * @brief Reads Receive Buffer Register (8 bits).
 * @param UARTx The UART instance.
 * @returns The received data (8 bits).
 */
__STATIC_INLINE uint8_t UART_ReceiveData8b(UART_TypeDef *UARTx)
{
    return (uint8_t)READ_BIT(UARTx->RBR, UART_RBR_8BIT);
}

/**
 * @brief Reads Receive Buffer Register (9 bits).
 * @param UARTx The UART instance.
 * @returns The received data (9 bits).
 */
__STATIC_INLINE uint16_t UART_ReceiveData9b(UART_TypeDef *UARTx)
{
    return (uint16_t)READ_BIT(UARTx->RBR, UART_RBR_9BIT);
}

/**
 * @brief Writes in Transmit Holding Register (8 bits).
 * @param UARTx The UART instance.
 * @param Value The data to transmit (8 bits).
 */
__STATIC_INLINE void UART_TransmitData8b(UART_TypeDef *UARTx, uint8_t Value)
{
    WRITE_REG(UARTx->THR, (uint32_t)(Value & UART_THR_8BIT));
}

/**
 * @brief Writes in Transmit Holding Register (9 bits).
 * @param UARTx The UART instance.
 * @param Value The data to transmit (9 bits).
 */
__STATIC_INLINE void UART_TransmitData9b(UART_TypeDef *UARTx, uint16_t Value)
{
    WRITE_REG(UARTx->THR, (uint32_t)(Value & UART_THR_9BIT));
}

/**
 * @brief Sets the receive address in 9-bit transfer mode.
 * @param UARTx The UART instance.
 * @param NodeAddress The receive address.
 */
__STATIC_INLINE void UART_SetRxAddress9b(UART_TypeDef *UARTx, uint8_t NodeAddress)
{
    MODIFY_REG(UARTx->RAR, UART_RAR, NodeAddress);
}

/**
 * @brief Sets the transmit address in 9-bit transfer mode.
 * @param UARTx The UART instance.
 * @param NodeAddress The transmit address.
 */
__STATIC_INLINE void UART_SetTxAddress9b(UART_TypeDef *UARTx, uint8_t NodeAddress)
{
    MODIFY_REG(UARTx->TAR, UART_TAR, NodeAddress);
}

/**
 * @brief Returns the receive address in 9-bit transfer mode.
 * @param UARTx The UART instance.
 * @returns The receive address.
 */
__STATIC_INLINE uint8_t UART_GetRxAddress9b(UART_TypeDef *UARTx)
{
    return (uint8_t)READ_BIT(UARTx->RAR, UART_RAR);
}

/**
 * @brief Returns the transmit address in 9-bit transfer mode.
 * @param UARTx The UART instance.
 * @returns The transmit address.
 */
__STATIC_INLINE uint8_t UART_GetTxAddress9b(UART_TypeDef *UARTx)
{
    return (uint8_t)READ_BIT(UARTx->TAR, UART_TAR);
}

/**
 * @brief Sends the address during 9-bit transmit mode.
 * @note The address must be written into Transmit Address Register (TAR)
 *       prior to calling this function. Use UART_SetTxAddress9b() to write
 *       the address.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_TransmitAddress9b(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->LCR_EXT, UART_LCR_EXT_SEND_ADDR);
    while (READ_BIT(UARTx->LCR_EXT, UART_LCR_EXT_SEND_ADDR) == UART_LCR_EXT_SEND_ADDR);
}

/**
 * @brief Sets break condition on the serial output.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_SetBreak(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->LCR, UART_LCR_BC);
}

/**
 * @brief Releases serial output for data transmission.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_ClearBreak(UART_TypeDef *UARTx)
{
    CLEAR_BIT(UARTx->LCR, UART_LCR_BC);
}

/**
 * @brief Enables halt transmission.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_HaltTxEnable(UART_TypeDef *UARTx)
{
    SET_BIT(UARTx->HTX, UART_HTX);
}

/**
 * @brief Disables halt transmission.
 * @param UARTx The UART instance.
 */
__STATIC_INLINE void UART_HaltTxDisable(UART_TypeDef *UARTx)
{
    CLEAR_BIT(UARTx->HTX, UART_HTX);
}

ErrorStatus UART_DeInit(UART_TypeDef *UARTx);
ErrorStatus UART_Init(UART_TypeDef *UARTx, UART_InitStruct_TypeDef *UART_InitStruct);
void UART_StructInit(UART_InitStruct_TypeDef *UART_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_UART_H */
