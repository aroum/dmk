#include "split.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hal_gpio.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
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

#if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX) || (SPLIT_CONNECTION_TYPE == HW_FULL_DUPLEX)

#include "nrf_gpio.h"
#include "nrf_uarte.h"

#ifndef SPLIT_UART_INSTANCE
#define SPLIT_UART_INSTANCE NRF_UARTE0
#endif

// EasyDMA must receive the whole frame: polling a one-byte buffer at the RTOS
// tick rate loses bytes at 115200 baud.
static uint8_t s_rx_buffer[sizeof(split_packet_t)];
static StaticSemaphore_t s_uart_lock_storage;
static SemaphoreHandle_t s_uart_lock;
static bool s_rx_has_data;
static TickType_t s_rx_last_activity;

static void split_uart_start_rx(void) {
    s_rx_has_data = false;
    nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDRX);
    nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXDRDY);
    nrf_uarte_rx_buffer_set(SPLIT_UART_INSTANCE, s_rx_buffer, sizeof(s_rx_buffer));
    nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STARTRX);
}

void split_transport_send_packet(const split_packet_t *packet) {
    xSemaphoreTake(s_uart_lock, portMAX_DELAY);
    bool shared_wire = SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX || SPLIT_TX_PIN == SPLIT_RX_PIN;
    if (shared_wire) {
        nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXTO);
        nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STOPRX);
        unsigned timeout = 1000;
        while (!nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXTO) && timeout--) hal_sleep_us(1);
        if (!nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXTO)) {
            nrf_uarte_disable(SPLIT_UART_INSTANCE);
            nrf_uarte_enable(SPLIT_UART_INSTANCE);
            split_uart_start_rx();
            xSemaphoreGive(s_uart_lock);
            return;
        }
    }

    nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDTX);
    nrf_uarte_tx_buffer_set(SPLIT_UART_INSTANCE, (const uint8_t *)packet, sizeof(*packet));
    nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STARTTX);
    unsigned timeout = 100;
    while (!nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDTX) && timeout--) vTaskDelay(1);
    if (!nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDTX)) {
        // Do not return while EasyDMA may still read the caller's stack.
        nrf_uarte_disable(SPLIT_UART_INSTANCE);
        nrf_uarte_enable(SPLIT_UART_INSTANCE);
        shared_wire = true; // re-arm receive after resetting the peripheral
    }
    nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDTX);
    if (shared_wire) split_uart_start_rx();
    xSemaphoreGive(s_uart_lock);
    if (!is_master()) vTaskDelay(pdMS_TO_TICKS(12));
}

void split_transport_reply_packet(const split_packet_t *packet) {
    split_transport_send_packet(packet);
}

static size_t split_uart_poll_rx(uint8_t *frame, TickType_t now) {
    size_t count = 0;
    xSemaphoreTake(s_uart_lock, portMAX_DELAY);
    if (nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_ENDRX)) {
        count = nrf_uarte_rx_amount_get(SPLIT_UART_INSTANCE);
        if (count > sizeof(s_rx_buffer)) count = sizeof(s_rx_buffer);
        memcpy(frame, s_rx_buffer, count);
        split_uart_start_rx();
    } else {
        if (nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXDRDY)) {
            nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXDRDY);
            s_rx_has_data = true;
            s_rx_last_activity = now;
        }
        if (s_rx_has_data && (TickType_t)(now - s_rx_last_activity) >= pdMS_TO_TICKS(3)) {
            // A truncated frame must not consume the first byte of the
            // next transmission and make the reply collide with it.
            nrf_uarte_event_clear(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXTO);
            nrf_uarte_task_trigger(SPLIT_UART_INSTANCE, NRF_UARTE_TASK_STOPRX);
            unsigned timeout = 1000;
            while (!nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXTO) && timeout--)
                hal_sleep_us(1);
            if (nrf_uarte_event_check(SPLIT_UART_INSTANCE, NRF_UARTE_EVENT_RXTO)) {
                count = nrf_uarte_rx_amount_get(SPLIT_UART_INSTANCE);
                if (count > sizeof(s_rx_buffer)) count = sizeof(s_rx_buffer);
                memcpy(frame, s_rx_buffer, count);
            } else {
                nrf_uarte_disable(SPLIT_UART_INSTANCE);
                nrf_uarte_enable(SPLIT_UART_INSTANCE);
            }
            split_uart_start_rx();
        }
    }
    xSemaphoreGive(s_uart_lock);
    return count;
}

#ifdef DMK_SPLIT_TEST
size_t split_test_uart_poll_rx(uint8_t *frame, TickType_t now) { return split_uart_poll_rx(frame, now); }
#endif

void split_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        uint8_t frame[sizeof(s_rx_buffer)];
        size_t count = split_uart_poll_rx(frame, xTaskGetTickCount());
        for (size_t i = 0; i < count; ++i) split_process_received_byte(frame[i]);
        if (!count) vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void split_init(void) {
    s_uart_lock = xSemaphoreCreateMutexStatic(&s_uart_lock_storage);
    configASSERT(s_uart_lock != NULL);
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
    split_uart_start_rx();

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    static StaticTask_t s_split_task_tcb;
    static StackType_t s_split_task_stack[256];
    xTaskCreateStatic(split_task, "split", 256, NULL, configMAX_PRIORITIES - 1, s_split_task_stack, &s_split_task_tcb);
#else
    xTaskCreate(split_task, "split", 256, NULL, configMAX_PRIORITIES - 1, NULL);
#endif
}

#endif // HW_HALF_DUPLEX / HW_FULL_DUPLEX

#endif // defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)
