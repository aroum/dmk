#include "split.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hal_gpio.h"
#include "queue.h"
#include "task.h"
#include "task_internal.h"

#if defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

#ifndef SPLIT_CONNECTION_TYPE
#ifdef SPLIT_UART_HARDWARE
#define SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX
#else
#define SPLIT_CONNECTION_TYPE SOFT
#endif
#endif

extern volatile bool usb_mounted;

bool is_master(void) {
    return usb_mounted;
}

#if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX)
#error                                                                                                                 \
    "DMK Milandr Error: Milandr hardware UART does not support 1-wire HW_HALF_DUPLEX! Use SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX with 2 pins (TX+RX) or SPLIT_CONNECTION_TYPE SOFT for 1-wire."
#endif

#if (SPLIT_CONNECTION_TYPE == HW_FULL_DUPLEX)
#if (SPLIT_TX_PIN / 16) != (SPLIT_RX_PIN / 16)
#error                                                                                                                 \
    "DMK Milandr HW UART Error: SPLIT_TX_PIN and SPLIT_RX_PIN must be on the SAME GPIO port! (e.g. both on Port F: PF1 TX, PF0 RX; or Port A: PA7 TX, PA6 RX)."
#endif

#if ((SPLIT_TX_PIN / 16) == 5) // Port F
#if (SPLIT_TX_PIN != 80 && SPLIT_TX_PIN != 81) || (SPLIT_RX_PIN != 80 && SPLIT_RX_PIN != 81)
#error                                                                                                                 \
    "DMK Milandr HW UART Error: Invalid hardware UART pins on Port F! Supported pins for MDR_UART2 are PF1 (TX) and PF0 (RX)."
#endif
#elif ((SPLIT_TX_PIN / 16) == 0) // Port A
#if (SPLIT_TX_PIN != 6 && SPLIT_TX_PIN != 7) || (SPLIT_RX_PIN != 6 && SPLIT_RX_PIN != 7)
#error                                                                                                                 \
    "DMK Milandr HW UART Error: Invalid hardware UART pins on Port A! Supported pins for MDR_UART1 are PA7 (TX) and PA6 (RX)."
#endif
#else
#error                                                                                                                 \
    "DMK Milandr HW UART Error: Selected GPIO port does not support Hardware UART! Use Port F (PF1 TX, PF0 RX) or Port A (PA7 TX, PA6 RX), or switch to SPLIT_CONNECTION_TYPE SOFT."
#endif

#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_uart.h"

#ifndef SPLIT_UART_GPIO_PORT
#if (SPLIT_TX_PIN / 16) == 0
#define SPLIT_UART_GPIO_PORT MDR_PORTA
#elif (SPLIT_TX_PIN / 16) == 1
#define SPLIT_UART_GPIO_PORT MDR_PORTB
#elif (SPLIT_TX_PIN / 16) == 2
#define SPLIT_UART_GPIO_PORT MDR_PORTC
#elif (SPLIT_TX_PIN / 16) == 3
#define SPLIT_UART_GPIO_PORT MDR_PORTD
#elif (SPLIT_TX_PIN / 16) == 4
#define SPLIT_UART_GPIO_PORT MDR_PORTE
#else
#define SPLIT_UART_GPIO_PORT MDR_PORTF
#endif
#endif

#ifndef SPLIT_UART_PORT
#if (SPLIT_TX_PIN / 16) == 0 || (SPLIT_TX_PIN / 16) == 1
#define SPLIT_UART_PORT MDR_UART1
#else
#define SPLIT_UART_PORT MDR_UART2
#endif
#endif

#ifndef SPLIT_UART_PIN_FUNC
#define SPLIT_UART_PIN_FUNC PORT_FUNC_ALTER
#endif

#ifndef SPLIT_UART_OPEN_DRAIN
#define SPLIT_UART_OPEN_DRAIN 1
#endif

#ifndef SPLIT_UART_PULLUP
#define SPLIT_UART_PULLUP 1
#endif

void split_send_event(matrix_event_t *event) {
    split_packet_t pkt;
    pkt.header = 0xA5;
    pkt.row = event->row;
    pkt.col = event->col;
    pkt.pressed = event->pressed;

    uint8_t *pkt_ptr = (uint8_t *)&pkt;
    for (size_t i = 0; i < sizeof(split_packet_t); i++) {
        while (UART_GetFlagStatus(SPLIT_UART_PORT, UART_FLAG_TXFF) == SET) {
            vTaskDelay(1);
        }
        UART_SendData(SPLIT_UART_PORT, pkt_ptr[i]);
    }
}

void split_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        if (is_master()) {
            if (UART_GetFlagStatus(SPLIT_UART_PORT, UART_FLAG_RXFE) == RESET) {
                uint8_t byte = UART_ReceiveData(SPLIT_UART_PORT) & 0xFF;
                split_process_received_byte(byte);
            } else {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void split_init(void) {
    uint16_t tx_mask;
    uint16_t rx_mask;

    if (SPLIT_TX_PIN >= 0x0001 && (SPLIT_TX_PIN & (SPLIT_TX_PIN - 1)) == 0) {
        tx_mask = SPLIT_TX_PIN;
    } else {
        tx_mask = (1 << (SPLIT_TX_PIN % 16));
    }

    if (SPLIT_RX_PIN >= 0x0001 && (SPLIT_RX_PIN & (SPLIT_RX_PIN - 1)) == 0) {
        rx_mask = SPLIT_RX_PIN;
    } else {
        rx_mask = (1 << (SPLIT_RX_PIN % 16));
    }

    RST_CLK_PCLKcmd(PCLK_BIT(SPLIT_UART_GPIO_PORT), ENABLE);

    PORT_InitTypeDef PORT_InitStructure;
    PORT_StructInit(&PORT_InitStructure);
    PORT_InitStructure.PORT_Pin = tx_mask | rx_mask;
    PORT_InitStructure.PORT_FUNC = SPLIT_UART_PIN_FUNC;
    PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
    PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;

    if (SPLIT_UART_OPEN_DRAIN) {
        PORT_InitStructure.PORT_PD = PORT_PD_OPEN;
    } else {
        PORT_InitStructure.PORT_PD = PORT_PD_DRIVER;
    }

    if (SPLIT_UART_PULLUP) {
        PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_ON;
    } else {
        PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
    }

    PORT_Init(SPLIT_UART_GPIO_PORT, &PORT_InitStructure);

    RST_CLK_PCLKcmd(PCLK_BIT(SPLIT_UART_PORT), ENABLE);
    UART_BRGInit(SPLIT_UART_PORT, UART_HCLKdiv1);

    UART_InitTypeDef UART_InitStructure;
    UART_StructInit(&UART_InitStructure);
    UART_InitStructure.UART_BaudRate = SPLIT_UART_BAUD;
    UART_InitStructure.UART_WordLength = UART_WordLength8b;
    UART_InitStructure.UART_StopBits = UART_StopBits1;
    UART_InitStructure.UART_Parity = UART_Parity_No;
    UART_InitStructure.UART_FIFOMode = UART_FIFO_ON;
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

    UART_Init(SPLIT_UART_PORT, &UART_InitStructure);
    UART_Cmd(SPLIT_UART_PORT, ENABLE);

    xTaskCreate(split_task, "split", 512, NULL, configMAX_PRIORITIES - 1, NULL);
}

#else // Bit-bang (Software UART) mode (SOFT)

void split_send_event(matrix_event_t *event) {
    split_soft_send_event(event);
}

void split_init(void) {
    split_soft_init();
}

#endif // SOFT

#endif // defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)
