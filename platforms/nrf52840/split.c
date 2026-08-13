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

#if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX) || (SPLIT_CONNECTION_TYPE == HW_FULL_DUPLEX)

#include "nrf_gpio.h"
#include "nrf_uarte.h"

#ifndef SPLIT_UART_INSTANCE
#define SPLIT_UART_INSTANCE NRF_UARTE0
#endif

static uint8_t rx_byte_buf;

void split_send_event(matrix_event_t *event) {
    split_packet_t pkt;
    pkt.header = 0xA5;
    pkt.row = event->row;
    pkt.col = event->col;
    pkt.pressed = event->pressed;

    uint8_t *pkt_ptr = (uint8_t *)&pkt;

    if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX || SPLIT_TX_PIN == SPLIT_RX_PIN) {
        nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STOPRX);
    }

    nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDTX);
    nrf_uarte_tx_buffer_set(SPLIT_UART_INSTANCE, pkt_ptr, sizeof(split_packet_t));
    nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STARTTX);

    uint32_t timeout = 10000;
    while (!nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDTX) && --timeout > 0) {
        vTaskDelay(1);
    }
    nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDTX);

    if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX || SPLIT_TX_PIN == SPLIT_RX_PIN) {
        nrf_uarte_rx_buffer_set(SPLIT_UART_INSTANCE, &rx_byte_buf, 1);
        nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STARTRX);
    }
}

void split_task(void *pvParameters) {
    (void)pvParameters;

    nrf_uarte_rx_buffer_set(SPLIT_UART_INSTANCE, &rx_byte_buf, 1);
    nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STARTRX);

    while (1) {
        if (is_master()) {
            if (nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDRX)) {
                nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDRX);
                uint8_t byte = rx_byte_buf;

                nrf_uarte_rx_buffer_set(SPLIT_UART_INSTANCE, &rx_byte_buf, 1);
                nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STARTRX);

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
    uint32_t tx_pin = SPLIT_TX_PIN;
    uint32_t rx_pin = SPLIT_RX_PIN;

    if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX || tx_pin == rx_pin) {
        // Single-wire Open-Drain (S0D1) with Pull-up
        nrf_gpio_cfg(tx_pin, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0D1,
                     NRF_GPIO_PIN_NOSENSE);
        rx_pin = tx_pin;
    } else {
        nrf_gpio_cfg_output(tx_pin);
        nrf_gpio_cfg_input(rx_pin, NRF_GPIO_PIN_PULLUP);
    }

    nrf_uarte_txrx_pins_set(SPLIT_UART_INSTANCE, tx_pin, rx_pin);

#if SPLIT_UART_BAUD == 115200
    nrf_uarte_baudrate_set(SPLIT_UART_INSTANCE, NRF_UARTE_BAUDRATE_115200);
#else
    nrf_uarte_baudrate_set(SPLIT_UART_INSTANCE, NRF_UARTE_BAUDRATE_19200);
#endif

    nrf_uarte_configure(SPLIT_UART_INSTANCE, NRF_UARTE_PARITY_EXCLUDED, NRF_UARTE_HWFC_DISABLED);
    nrf_uarte_enable(SPLIT_UART_INSTANCE);

    xTaskCreate(split_task, "split", 512, NULL, configMAX_PRIORITIES - 1, NULL);
}

#else // Software Bit-Bang mode (SOFT)

void split_send_event(matrix_event_t *event) {
    split_soft_send_event(event);
}

void split_init(void) {
    split_soft_init();
}

#endif // SOFT

#endif // defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)
