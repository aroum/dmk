#ifndef SPLIT_TEST_NRF_UARTE_H
#define SPLIT_TEST_NRF_UARTE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum { NRF_UARTE_EVENT_ENDRX, NRF_UARTE_EVENT_RXDRDY, NRF_UARTE_EVENT_RXTO,
               NRF_UARTE_EVENT_ENDTX, SPLIT_TEST_EVENT_COUNT } nrf_uarte_event_t;
typedef enum { NRF_UARTE_TASK_STARTRX, NRF_UARTE_TASK_STOPRX, NRF_UARTE_TASK_STARTTX } nrf_uarte_task_t;
typedef struct {
    bool events[SPLIT_TEST_EVENT_COUNT], allow_stop, allow_tx;
    uint8_t *rx;
    const uint8_t *tx;
    size_t rx_size, rx_amount, tx_size;
    unsigned starts, stops, transmissions, resets;
} split_test_uart_t;
extern split_test_uart_t split_test_uart;
#define NRF_UARTE0 (&split_test_uart)
#define NRF_UARTE_BAUDRATE_115200 115200
#define NRF_UARTE_BAUDRATE_19200 19200
#define NRF_UARTE_PARITY_EXCLUDED 0
#define NRF_UARTE_HWFC_DISABLED 0
static inline void nrf_uarte_event_clear(split_test_uart_t *uart, nrf_uarte_event_t event) { uart->events[event] = false; }
static inline bool nrf_uarte_event_check(split_test_uart_t *uart, nrf_uarte_event_t event) { return uart->events[event]; }
static inline void nrf_uarte_rx_buffer_set(split_test_uart_t *uart, uint8_t *buffer, size_t size) { uart->rx = buffer; uart->rx_size = size; }
static inline uint32_t nrf_uarte_rx_amount_get(split_test_uart_t *uart) { return (uint32_t)uart->rx_amount; }
static inline void nrf_uarte_tx_buffer_set(split_test_uart_t *uart, const uint8_t *buffer, size_t size) { uart->tx = buffer; uart->tx_size = size; }
static inline void nrf_uarte_task_trigger(split_test_uart_t *uart, nrf_uarte_task_t task) {
    if (task == NRF_UARTE_TASK_STARTRX) { ++uart->starts; uart->rx_amount = 0; }
    else if (task == NRF_UARTE_TASK_STOPRX) { ++uart->stops; if (uart->allow_stop) uart->events[NRF_UARTE_EVENT_RXTO] = true; }
    else if (task == NRF_UARTE_TASK_STARTTX) { ++uart->transmissions; if (uart->allow_tx) uart->events[NRF_UARTE_EVENT_ENDTX] = true; }
}
static inline void nrf_uarte_disable(split_test_uart_t *uart) { ++uart->resets; }
static inline void nrf_uarte_enable(split_test_uart_t *uart) { (void)uart; }
static inline void nrf_uarte_txrx_pins_set(split_test_uart_t *uart, uint32_t tx, uint32_t rx) { (void)uart; (void)tx; (void)rx; }
static inline void nrf_uarte_baudrate_set(split_test_uart_t *uart, uint32_t baud) { (void)uart; (void)baud; }
static inline void nrf_uarte_configure(split_test_uart_t *uart, int parity, int flow) { (void)uart; (void)parity; (void)flow; }
#endif
