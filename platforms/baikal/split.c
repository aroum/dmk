#include "split.h"
#include "FreeRTOS.h"
#include "bmcu_cru.h"
#include "bmcu_gpio.h"
#include "bmcu_uart.h"
#include "config.h"
#include "pin_defs.h"
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

#if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX)
#error                                                                                                                 \
    "DMK Baikal Error: Baikal hardware UART does not support 1-wire HW_HALF_DUPLEX! Use SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX with 2 pins (TX+RX) or SPLIT_CONNECTION_TYPE SOFT for 1-wire."
#endif

#if (SPLIT_CONNECTION_TYPE == HW_FULL_DUPLEX)

// Determine UART instance, clocks, ports, and alternate functions based on SPLIT_TX_PIN / SPLIT_RX_PIN
#if (SPLIT_TX_PIN == PC6 && SPLIT_RX_PIN == PC7)
#define SPLIT_UART_DEV UART6
#define SPLIT_UART_TX_PORT CRU_PORT_C
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_6
#define SPLIT_UART_TX_AF CRU_PIN_AF_2
#define SPLIT_UART_RX_PORT CRU_PORT_C
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_7
#define SPLIT_UART_RX_AF CRU_PIN_AF_2
#define SPLIT_UART_CLK_ENABLE() CRU_APB2_EnableClock(CRU_APB2_PERIPH_UART6)

#elif (SPLIT_TX_PIN == PC8 && SPLIT_RX_PIN == PC9)
#define SPLIT_UART_DEV UART7
#define SPLIT_UART_TX_PORT CRU_PORT_C
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_8
#define SPLIT_UART_TX_AF CRU_PIN_AF_4
#define SPLIT_UART_RX_PORT CRU_PORT_C
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_9
#define SPLIT_UART_RX_AF CRU_PIN_AF_4
#define SPLIT_UART_CLK_ENABLE() CRU_APB2_EnableClock(CRU_APB2_PERIPH_UART7)

#elif (SPLIT_TX_PIN == PA6 && SPLIT_RX_PIN == PA7)
#define SPLIT_UART_DEV UART0
#define SPLIT_UART_TX_PORT CRU_PORT_A
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_6
#define SPLIT_UART_TX_AF CRU_PIN_AF_1
#define SPLIT_UART_RX_PORT CRU_PORT_A
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_7
#define SPLIT_UART_RX_AF CRU_PIN_AF_1
#define SPLIT_UART_CLK_ENABLE() CRU_APB0_EnableClock(CRU_APB0_PERIPH_UART0)

#elif (SPLIT_TX_PIN == PA2 && SPLIT_RX_PIN == PA3)
#define SPLIT_UART_DEV UART0
#define SPLIT_UART_TX_PORT CRU_PORT_A
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_2
#define SPLIT_UART_TX_AF CRU_PIN_AF_5
#define SPLIT_UART_RX_PORT CRU_PORT_A
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_3
#define SPLIT_UART_RX_AF CRU_PIN_AF_5
#define SPLIT_UART_CLK_ENABLE() CRU_APB0_EnableClock(CRU_APB0_PERIPH_UART0)

#elif (SPLIT_TX_PIN == PA4 && SPLIT_RX_PIN == PA5)
#define SPLIT_UART_DEV UART1
#define SPLIT_UART_TX_PORT CRU_PORT_A
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_4
#define SPLIT_UART_TX_AF CRU_PIN_AF_5
#define SPLIT_UART_RX_PORT CRU_PORT_A
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_5
#define SPLIT_UART_RX_AF CRU_PIN_AF_5
#define SPLIT_UART_CLK_ENABLE() CRU_APB0_EnableClock(CRU_APB0_PERIPH_UART1)

#elif (SPLIT_TX_PIN == PA10 && SPLIT_RX_PIN == PA11)
#define SPLIT_UART_DEV UART1
#define SPLIT_UART_TX_PORT CRU_PORT_A
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_10
#define SPLIT_UART_TX_AF CRU_PIN_AF_2
#define SPLIT_UART_RX_PORT CRU_PORT_A
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_11
#define SPLIT_UART_RX_AF CRU_PIN_AF_2
#define SPLIT_UART_CLK_ENABLE() CRU_APB0_EnableClock(CRU_APB0_PERIPH_UART1)

#elif (SPLIT_TX_PIN == PA12 && SPLIT_RX_PIN == PA13)
#define SPLIT_UART_DEV UART2
#define SPLIT_UART_TX_PORT CRU_PORT_A
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_12
#define SPLIT_UART_TX_AF CRU_PIN_AF_2
#define SPLIT_UART_RX_PORT CRU_PORT_A
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_13
#define SPLIT_UART_RX_AF CRU_PIN_AF_2
#define SPLIT_UART_CLK_ENABLE() CRU_APB0_EnableClock(CRU_APB0_PERIPH_UART2)

#elif (SPLIT_TX_PIN == PB6 && SPLIT_RX_PIN == PB7)
#define SPLIT_UART_DEV UART3
#define SPLIT_UART_TX_PORT CRU_PORT_B
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_6
#define SPLIT_UART_TX_AF CRU_PIN_AF_1
#define SPLIT_UART_RX_PORT CRU_PORT_B
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_7
#define SPLIT_UART_RX_AF CRU_PIN_AF_1
#define SPLIT_UART_CLK_ENABLE() CRU_APB1_EnableClock(CRU_APB1_PERIPH_UART3)

#elif (SPLIT_TX_PIN == PB2 && SPLIT_RX_PIN == PB3)
#define SPLIT_UART_DEV UART3
#define SPLIT_UART_TX_PORT CRU_PORT_B
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_2
#define SPLIT_UART_TX_AF CRU_PIN_AF_5
#define SPLIT_UART_RX_PORT CRU_PORT_B
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_3
#define SPLIT_UART_RX_AF CRU_PIN_AF_5
#define SPLIT_UART_CLK_ENABLE() CRU_APB1_EnableClock(CRU_APB1_PERIPH_UART3)

#elif (SPLIT_TX_PIN == PB4 && SPLIT_RX_PIN == PB5)
#define SPLIT_UART_DEV UART4
#define SPLIT_UART_TX_PORT CRU_PORT_B
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_4
#define SPLIT_UART_TX_AF CRU_PIN_AF_5
#define SPLIT_UART_RX_PORT CRU_PORT_B
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_5
#define SPLIT_UART_RX_AF CRU_PIN_AF_5
#define SPLIT_UART_CLK_ENABLE() CRU_APB1_EnableClock(CRU_APB1_PERIPH_UART4)

#elif (SPLIT_TX_PIN == PB10 && SPLIT_RX_PIN == PB11)
#define SPLIT_UART_DEV UART4
#define SPLIT_UART_TX_PORT CRU_PORT_B
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_10
#define SPLIT_UART_TX_AF CRU_PIN_AF_2
#define SPLIT_UART_RX_PORT CRU_PORT_B
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_11
#define SPLIT_UART_RX_AF CRU_PIN_AF_2
#define SPLIT_UART_CLK_ENABLE() CRU_APB1_EnableClock(CRU_APB1_PERIPH_UART4)

#elif (SPLIT_TX_PIN == PB12 && SPLIT_RX_PIN == PB13)
#define SPLIT_UART_DEV UART5
#define SPLIT_UART_TX_PORT CRU_PORT_B
#define SPLIT_UART_TX_PIN_MASK CRU_PIN_12
#define SPLIT_UART_TX_AF CRU_PIN_AF_2
#define SPLIT_UART_RX_PORT CRU_PORT_B
#define SPLIT_UART_RX_PIN_MASK CRU_PIN_13
#define SPLIT_UART_RX_AF CRU_PIN_AF_2
#define SPLIT_UART_CLK_ENABLE() CRU_APB1_EnableClock(CRU_APB1_PERIPH_UART5)

#else
#error                                                                                                                 \
    "DMK Baikal HW UART Error: Unsupported hardware UART pin pair! Supported pairs: PC6(TX)+PC7(RX), PC8(TX)+PC9(RX), PA6(TX)+PA7(RX), PA2(TX)+PA3(RX), PA4(TX)+PA5(RX), PA10(TX)+PA11(RX), PA12(TX)+PA13(RX), PB6(TX)+PB7(RX), PB2(TX)+PB3(RX), PB4(TX)+PB5(RX), PB10(TX)+PB11(RX), PB12(TX)+PB13(RX). For other pins, use SPLIT_CONNECTION_TYPE SOFT (Bit-Bang)."
#endif

void split_send_event(matrix_event_t *event) {
    split_packet_t pkt;
    split_make_packet(&pkt, event);

    uint8_t *pkt_ptr = (uint8_t *)&pkt;
    for (size_t i = 0; i < sizeof(split_packet_t); i++) {
        // Wait while Transmit FIFO is full
        while (UART_IsActiveFlag(SPLIT_UART_DEV, UART_FLAG_TFNF) == RESET) {
            vTaskDelay(1);
        }
        UART_TransmitData8b(SPLIT_UART_DEV, pkt_ptr[i]);
    }
}

void split_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        if (is_master()) {
            if (UART_IsActiveFlag(SPLIT_UART_DEV, UART_FLAG_RFNE) == SET) {
                uint8_t byte = UART_ReceiveData8b(SPLIT_UART_DEV);
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
    // 1. Enable peripheral clock
    SPLIT_UART_CLK_ENABLE();

    // 2. Configure Pin Multiplexing (AF)
    CRU_SetPinAF(SPLIT_UART_TX_PORT, SPLIT_UART_TX_PIN_MASK, SPLIT_UART_TX_AF);
    CRU_SetPinAF(SPLIT_UART_RX_PORT, SPLIT_UART_RX_PIN_MASK, SPLIT_UART_RX_AF);
    CRU_EnablePinInput(SPLIT_UART_RX_PORT, SPLIT_UART_RX_PIN_MASK);

    // Pull-up for RX line
    CRU_SetPinPull(SPLIT_UART_RX_PORT, SPLIT_UART_RX_PIN_MASK, CRU_PIN_PULL_UP);

    // 3. Initialize UART instance
    UART_InitStruct_TypeDef uartInit;
    UART_StructInit(&uartInit);
    uartInit.BaudRate = SPLIT_UART_BAUD;
    uartInit.DataWidth = UART_DATAWIDTH_8B;
    uartInit.StopBits = UART_STOP_1BIT;
    uartInit.Parity = UART_PARITY_NONE;
    uartInit.CtrlFIFO = ENABLE;
    uartInit.TxFIFOThreshold = UART_TX_FIFO_HALF_FULL;
    uartInit.RxFIFOThreshold = UART_RX_FIFO_CHAR_1;

    UART_Init(SPLIT_UART_DEV, &uartInit);

    // 4. Create FreeRTOS split task
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    static StaticTask_t s_split_task_tcb;
    static StackType_t s_split_task_stack[256];
    xTaskCreateStatic(split_task, "split", 256, NULL, configMAX_PRIORITIES - 1, s_split_task_stack, &s_split_task_tcb);
#else
    xTaskCreate(split_task, "split", 256, NULL, configMAX_PRIORITIES - 1, NULL);
#endif
}

#endif // HW_FULL_DUPLEX

#endif // defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)
