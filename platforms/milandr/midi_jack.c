#include "config.h"

#ifdef MIDI_JACK
#include "FreeRTOS.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_uart.h"
#include "task.h"

static MDR_UART_TypeDef *midi_uart = MDR_UART2;

static void midi_rx_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        if (UART_GetFlagStatus(midi_uart, UART_FLAG_RXFE) == RESET) {
            uint8_t byte = UART_ReceiveData(midi_uart) & 0xFF;
#ifdef MIDI_THRU
            while (UART_GetFlagStatus(midi_uart, UART_FLAG_TXFF) == SET) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            UART_SendData(midi_uart, byte);
#endif
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void midi_jack_init(void) {
    RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);

    PORT_InitTypeDef PORT_InitStructure;
    PORT_StructInit(&PORT_InitStructure);
    PORT_InitStructure.PORT_Pin = PORT_Pin_1 | PORT_Pin_0;
    PORT_InitStructure.PORT_FUNC = PORT_FUNC_ALTER;
    PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
    PORT_InitStructure.PORT_SPEED = PORT_SPEED_MAXFAST;
    PORT_InitStructure.PORT_PD = PORT_PD_DRIVER;
    PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_OFF;
    PORT_Init(MDR_PORTF, &PORT_InitStructure);

    RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);
    UART_BRGInit(midi_uart, UART_HCLKdiv1);

    UART_InitTypeDef UART_InitStructure;
    UART_StructInit(&UART_InitStructure);
    UART_InitStructure.UART_BaudRate = 31250;
    UART_InitStructure.UART_WordLength = UART_WordLength8b;
    UART_InitStructure.UART_StopBits = UART_StopBits1;
    UART_InitStructure.UART_Parity = UART_Parity_No;
    UART_InitStructure.UART_FIFOMode = UART_FIFO_ON;
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

    UART_Init(midi_uart, &UART_InitStructure);
    UART_Cmd(midi_uart, ENABLE);

    xTaskCreate(midi_rx_task, "midi_rx", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void midi_jack_write(uint8_t byte) {
    while (UART_GetFlagStatus(midi_uart, UART_FLAG_TXFF) == SET) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    UART_SendData(midi_uart, byte);
}
#endif
