#include "midi_jack.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hooks.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

static uart_inst_t *s_midi_uart = NULL;

static void hal_midi_jack_hw_init(void) {
    int tx_pin = PIN_MIDI_JACK_OUT;
    if (tx_pin == 0 || tx_pin == 12 || tx_pin == 16 || tx_pin == 1 || tx_pin == 13 || tx_pin == 17) {
        s_midi_uart = uart0;
    } else {
        s_midi_uart = uart1;
    }

    uart_init(s_midi_uart, MIDI_JACK_BAUD);
    gpio_set_function(PIN_MIDI_JACK_OUT, GPIO_FUNC_UART);
    gpio_set_function(PIN_MIDI_JACK_IN, GPIO_FUNC_UART);

    uart_set_hw_flow(s_midi_uart, false, false);
    uart_set_format(s_midi_uart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(s_midi_uart, true);
}

static inline bool hal_midi_jack_readable(void) {
    return s_midi_uart ? uart_is_readable(s_midi_uart) : false;
}

static inline uint8_t hal_midi_jack_getc(void) {
    return s_midi_uart ? uart_getc(s_midi_uart) : 0;
}

static inline void hal_midi_jack_putc(uint8_t byte) {
    if (s_midi_uart) {
        uart_putc(s_midi_uart, byte);
    }
}

#elif defined(MCU_milandr)
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_uart.h"

static MDR_UART_TypeDef *s_midi_uart = MDR_UART2;

static void hal_midi_jack_hw_init(void) {
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
    UART_BRGInit(s_midi_uart, UART_HCLKdiv1);

    UART_InitTypeDef UART_InitStructure;
    UART_StructInit(&UART_InitStructure);
    UART_InitStructure.UART_BaudRate = MIDI_JACK_BAUD;
    UART_InitStructure.UART_WordLength = UART_WordLength8b;
    UART_InitStructure.UART_StopBits = UART_StopBits1;
    UART_InitStructure.UART_Parity = UART_Parity_No;
    UART_InitStructure.UART_FIFOMode = UART_FIFO_ON;
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;

    UART_Init(s_midi_uart, &UART_InitStructure);
    UART_Cmd(s_midi_uart, ENABLE);
}

static inline bool hal_midi_jack_readable(void) {
    return (UART_GetFlagStatus(s_midi_uart, UART_FLAG_RXFE) == RESET);
}

static inline uint8_t hal_midi_jack_getc(void) {
    return (uint8_t)(UART_ReceiveData(s_midi_uart) & 0xFF);
}

static inline void hal_midi_jack_putc(uint8_t byte) {
    while (UART_GetFlagStatus(s_midi_uart, UART_FLAG_TXFF) == SET) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    UART_SendData(s_midi_uart, byte);
}

#else
// Generic fallback stub for platforms without dedicated hardware UART implementation
static void hal_midi_jack_hw_init(void) {}
static inline bool hal_midi_jack_readable(void) {
    return false;
}
static inline uint8_t hal_midi_jack_getc(void) {
    return 0;
}
static inline void hal_midi_jack_putc(uint8_t byte) {
    (void)byte;
}
#endif

static bool s_initialized = false;

static void midi_rx_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        if (hal_midi_jack_readable()) {
            uint8_t byte = hal_midi_jack_getc();
#ifdef MIDI_THRU
            hal_midi_jack_putc(byte);
#endif
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void midi_jack_init(void) {
    if (s_initialized) {
        return;
    }
    s_initialized = true;
    hal_midi_jack_hw_init();
    xTaskCreate(midi_rx_task, "midi_rx", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void midi_jack_write(uint8_t byte) {
    if (!s_initialized) {
        midi_jack_init();
    }
    hal_midi_jack_putc(byte);
}

__attribute__((weak)) void hook_early_init(void) {
    midi_jack_init();
}

void hook_midi_send(const uint8_t *msg, uint8_t len) {
    if (!s_initialized) {
        midi_jack_init();
    }
    for (uint8_t i = 0; i < len; i++) {
        hal_midi_jack_putc(msg[i]);
    }
}
