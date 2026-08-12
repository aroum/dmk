/**
 * *****************************************************************************
 *  @file       BSP/Components/serial/serial.c
 *  @author     Baikal electronics SDK team
 *  @brief      Serial interface driver source file
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

/* Includes ------------------------------------------------------------------*/
#include "serial.h"
#include "bmcu_cru.h"
#include "bmcu_uart.h"

#if defined(EVU_BA_1_2)
#include "../../EVU_BA_1_2/bsp.h"
#elif defined(EVU_BA_2_0)
#include "../../EVU_BA_2_0/bsp.h"
#elif defined(EVU_BA_2_1)
#include "../../EVU_BA_2_1/bsp.h"
#elif defined(EVU_BA_2_3)
#include "../../EVU_BA_2_3/bsp.h"
#elif defined(EVU_BA_2_5)
#include "../../EVU_BA_2_5/bsp.h"
#elif defined(EVU_LI_2_0)
#include "../../EVU_LI_2_0/bsp.h"
#else
#error "The selected development board does not support serial interface"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void bsp_serial_init(void) {
#if defined(BMCU_U)
    BSP_SERIAL_CLK_EN_FN(BSP_SERIAL_CLK_PERIPH);
    BSP_SERIAL_GPIO_CLK_EN_FN(BSP_SERIAL_GPIO_CLK_PERIPH);
#endif

    CRU_SetPinAF(BSP_SERIAL_TX_CRU_PORT, BSP_SERIAL_TX_CRU_PIN, BSP_SERIAL_CRU_PIN_AF);
    CRU_SetPinAF(BSP_SERIAL_RX_CRU_PORT, BSP_SERIAL_RX_CRU_PIN, BSP_SERIAL_CRU_PIN_AF);

    UART_DeInit(BSP_SERIAL_INSTANCE);

    UART_InitStruct_TypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    UART_InitStruct.BaudRate = 115200UL;
    UART_InitStruct.DataWidth = UART_DATAWIDTH_8B;
    UART_InitStruct.StopBits = UART_STOP_1BIT;
    UART_InitStruct.Parity = UART_PARITY_NONE;
    UART_InitStruct.Transfer9b = UART_TRANSFER_9B_DISABLE;
    UART_InitStruct.CtrlFIFO = DISABLE;

    UART_Init(BSP_SERIAL_INSTANCE, &UART_InitStruct);
}

void bsp_serial_deinit(void) {
    CRU_SetPinAF(BSP_SERIAL_TX_CRU_PORT, BSP_SERIAL_TX_CRU_PIN, CRU_PIN_AF_0);
    CRU_SetPinAF(BSP_SERIAL_RX_CRU_PORT, BSP_SERIAL_RX_CRU_PIN, CRU_PIN_AF_0);

    UART_DeInit(BSP_SERIAL_INSTANCE);
}

int bsp_serial_putchar(int ch) {
    while ((UART_GetLineStatus(BSP_SERIAL_INSTANCE) & UART_LINE_STATUS_THRE) == 0UL)
        ;
    UART_TransmitData8b(BSP_SERIAL_INSTANCE, (uint8_t)ch);

    return ch;
}

int bsp_serial_getchar(void) {
    int ch;

    while ((UART_GetLineStatus(BSP_SERIAL_INSTANCE) & UART_LINE_STATUS_DR) == 0UL)
        ;
    ch = (int)UART_ReceiveData8b(BSP_SERIAL_INSTANCE);

    return ch;
}
