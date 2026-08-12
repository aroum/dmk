/**
 * *****************************************************************************
 *  @file       bmcu_uart.c
 *  @author     Baikal electronics SDK team
 *  @brief      Universal Asynchronous Receiver-Transmitter (UART) module driver
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

#include "bmcu_uart.h"
#include "bmcu_def.h"
#include "bmcu_cru.h"

#define IS_UART_BAUDRATE(__VALUE__)             (((__VALUE__) > 0UL) && ((__VALUE__) <= 1500000UL))

#define IS_UART_PARITY(__VALUE__)               (((__VALUE__) == UART_PARITY_NONE) || \
                                                 ((__VALUE__) == UART_PARITY_ODD) || \
                                                 ((__VALUE__) == UART_PARITY_EVEN))

#define IS_UART_DATAWIDTH(__VALUE__)            (((__VALUE__) == UART_DATAWIDTH_5B) || \
                                                 ((__VALUE__) == UART_DATAWIDTH_6B) || \
                                                 ((__VALUE__) == UART_DATAWIDTH_7B) || \
                                                 ((__VALUE__) == UART_DATAWIDTH_8B))

#define IS_UART_STOPBITS(__VALUE__)             (((__VALUE__) == UART_STOP_1BIT) || \
                                                 ((__VALUE__) == UART_STOP_1_5BIT_OR_2BIT))

#define IS_UART_TRANSFER_9B(__VALUE__)          (((__VALUE__) == UART_TRANSFER_9B_DISABLE) || \
                                                 ((__VALUE__) == UART_TRANSFER_9B_HW_MODE) || \
                                                 ((__VALUE__) == UART_TRANSFER_9B_SW_MODE))

#define IS_UART_TX_FIFO_THRESHOLD(__VALUE__)    (((__VALUE__) == UART_TX_FIFO_EMPTY) || \
                                                 ((__VALUE__) == UART_TX_FIFO_CHAR_2) || \
                                                 ((__VALUE__) == UART_TX_FIFO_QUARTER_FULL) || \
                                                 ((__VALUE__) == UART_TX_FIFO_HALF_FULL))

#define IS_UART_RX_FIFO_THRESHOLD(__VALUE__)    (((__VALUE__) == UART_RX_FIFO_CHAR_1) || \
                                                 ((__VALUE__) == UART_RX_FIFO_QUARTER_FULL) || \
                                                 ((__VALUE__) == UART_RX_FIFO_HALF_FULL) || \
                                                 ((__VALUE__) == UART_RX_FIFO_FULL_2))

#define IS_UART_DMA_MODE(__VALUE__)             (((__VALUE__) == UART_DMA_MODE0) || \
                                                 ((__VALUE__) == UART_DMA_MODE1))

/**
 * @brief Deinitialize an UART instance.
 * @param UARTx The UART instance.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The UART instance have been deinitialized.
 *         - ERROR: The UART instance have not been deinitialized.
 */
ErrorStatus UART_DeInit(UART_TypeDef *UARTx)
{
    ErrorStatus status = SUCCESS;

    assert(IS_UART_ALL_INSTANCE(UARTx));

    /* Reset the UART instance */
    UART_Reset(UARTx);

    /* Wait for idle state */
    while (UART_IsActiveFlag(UARTx, UART_FLAG_BUSY) == SET);

    /* Read MSR register to prevent unwanted interrupts */
    READ_REG(UARTx->MSR);

    return status;
}

/**
 * @brief Initialize an UART instance.
 * @note UART_InitStruct structure should be initialized prior to calling this function.
 * @param UARTx The UART instance.
 * @param UART_InitStruct The pointer to UART_InitStruct_TypeDef structure.
 * @retval An ErrorStatus enumeration value:
 *         - SUCCESS: The UART instance have been initialized.
 *         - ERROR: The UART instance have not been initialized.
 */
ErrorStatus UART_Init(UART_TypeDef *UARTx, UART_InitStruct_TypeDef *UART_InitStruct)
{
    ErrorStatus status = ERROR;

    /* Check the parameters */
    assert(IS_UART_ALL_INSTANCE(UARTx));
    assert(IS_UART_BAUDRATE(UART_InitStruct->BaudRate));
    assert(IS_UART_DATAWIDTH(UART_InitStruct->DataWidth));
    assert(IS_UART_STOPBITS(UART_InitStruct->StopBits));
    assert(IS_UART_PARITY(UART_InitStruct->Parity));
    assert(IS_UART_TRANSFER_9B(UART_InitStruct->Transfer9b));
    assert(IS_FUNCTIONAL_STATE(UART_InitStruct->CtrlFIFO));
    assert(IS_UART_TX_FIFO_THRESHOLD(UART_InitStruct->TxFIFOThreshold));
    assert(IS_UART_RX_FIFO_THRESHOLD(UART_InitStruct->RxFIFOThreshold));
    assert(IS_FUNCTIONAL_STATE(UART_InitStruct->CtrlIrdaMode));
    assert(IS_FUNCTIONAL_STATE(UART_InitStruct->CtrlLoopbackMode));

    /* Wait for idle state */
    while (UART_IsActiveFlag(UARTx, UART_FLAG_BUSY) == SET);

    uint32_t pclk = 0UL;

    CRU_Clocks_TypeDef cru_clocks;
    CRU_GetSystemClocksFreq(&cru_clocks);

    if ((UARTx == UART0) ||
        (UARTx == UART1) ||
        (UARTx == UART2))
    {
        /* UART0, UART1 and UART2 */
        pclk = cru_clocks.PCLK0_Frequency;
    }
    else if ((UARTx == UART3) ||
             (UARTx == UART4) ||
             (UARTx == UART5))
    {
        /* UART3, UART4 and UART5 */
        pclk = cru_clocks.PCLK1_Frequency;
    }
    else
    {
        /* UART6 and UART7 */
        pclk = cru_clocks.PCLK2_Frequency;
    }

    if (pclk != 0UL)
    {
        /* Configure IrDA mode */
        if (UART_InitStruct->CtrlIrdaMode != DISABLE)
        {
            SET_BIT(UARTx->MCR, UART_MCR_SIRE);
        }

        /* Configure loopback mode */
        if (UART_InitStruct->CtrlLoopbackMode != DISABLE)
        {
            SET_BIT(UARTx->MCR, UART_MCR_LOOPBACK);
        }

        /* Configure UART baudrate */
        UART_SetBaudRate(UARTx, pclk, UART_InitStruct->BaudRate);

        /* Configure transfer characteristics */
        MODIFY_REG(UARTx->LCR,
                   (UART_LCR_EPS | UART_LCR_PEN | UART_LCR_STOP | UART_LCR_DLS),
                   (UART_InitStruct->Parity | UART_InitStruct->StopBits | UART_InitStruct->DataWidth));

        /* Configure 9-bit transfer mode */
        if (UART_InitStruct->Transfer9b == UART_TRANSFER_9B_HW_MODE)
        {
            /* 9-bit transfer in hardware address match mode */
            MODIFY_REG(UARTx->LCR_EXT,
                       (UART_LCR_EXT_TX_MODE | UART_LCR_EXT_SEND_ADDR),
                       (UART_LCR_EXT_ADDR_MATCH | UART_LCR_EXT_DLS_E));
        }
        else if (UART_InitStruct->Transfer9b == UART_TRANSFER_9B_SW_MODE)
        {
            /* 9-bit transfer in software address match mode */
            MODIFY_REG(UARTx->LCR_EXT,
                       (UART_LCR_EXT_SEND_ADDR | UART_LCR_EXT_ADDR_MATCH),
                       (UART_LCR_EXT_TX_MODE | UART_LCR_EXT_DLS_E));
        }
        else
        {
            /*!< 9-bit transfer disabled */
            WRITE_REG(UARTx->LCR_EXT, 0x0UL);
        }

        /* Configure FIFO mode */
        if (UART_InitStruct->CtrlFIFO != DISABLE)
        {
            MODIFY_REG(UARTx->FCR,
                       (UART_FCR_RT | UART_FCR_TET | UART_FCR_FIFOE),
                       (UART_InitStruct->RxFIFOThreshold | UART_InitStruct->TxFIFOThreshold |
                       UART_FCR_XFIFOR | UART_FCR_RFIFOR | UART_FCR_FIFOE_ENABLE));
        }

        /* Configure LSR clearing method, disable all interrupts */
        WRITE_REG(UARTx->IER, UART_IER_ELCOLR);

        status = SUCCESS;
    }

    return status;
}

/**
 * @brief Set each field of UART_InitStruct_TypeDef structure to default value.
 * @param UART_InitStruct The pointer to UART_InitStruct_TypeDef structure.
 */
void UART_StructInit(UART_InitStruct_TypeDef *UART_InitStruct)
{
    /* Set UART_InitStruct fields to default values */
    UART_InitStruct->BaudRate = 9600UL;
    UART_InitStruct->DataWidth = UART_DATAWIDTH_8B;
    UART_InitStruct->StopBits = UART_STOP_1BIT;
    UART_InitStruct->Parity = UART_PARITY_NONE;
    UART_InitStruct->Transfer9b = UART_TRANSFER_9B_DISABLE;
    UART_InitStruct->CtrlFIFO = DISABLE;
    UART_InitStruct->TxFIFOThreshold = UART_TX_FIFO_EMPTY;
    UART_InitStruct->RxFIFOThreshold = UART_RX_FIFO_CHAR_1;
    UART_InitStruct->CtrlIrdaMode = DISABLE;
    UART_InitStruct->CtrlLoopbackMode = DISABLE;
}
