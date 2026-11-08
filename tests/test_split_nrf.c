#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "split.h"
#include "nrf_uarte.h"

TickType_t g_mock_ticks;
split_test_uart_t split_test_uart;
static bool master;
bool is_master(void) { return master; }
void hal_sleep_us(uint32_t us) { (void)us; }
void split_process_received_byte(uint8_t byte) { (void)byte; }
size_t split_test_uart_poll_rx(uint8_t *frame, TickType_t now);

static void inject(const uint8_t *bytes, size_t count, bool complete) {
    assert(count <= split_test_uart.rx_size);
    memcpy(split_test_uart.rx, bytes, count);
    split_test_uart.rx_amount = count;
    split_test_uart.events[NRF_UARTE_EVENT_RXDRDY] = true;
    split_test_uart.events[NRF_UARTE_EVENT_ENDRX] = complete;
}

int main(void) {
    split_test_uart.allow_stop = true;
    split_test_uart.allow_tx = true;
    split_init();
    assert(split_test_uart.rx_size == sizeof(split_packet_t));
    matrix_event_t event = {.row = 1, .col = 2, .pressed = true};
    split_packet_t packet;
    split_make_packet(&packet, &event);
    uint8_t result[sizeof(packet)];
    inject((const uint8_t *)&packet, sizeof(packet), true);
    assert(split_test_uart_poll_rx(result, 100) == sizeof(packet));
    assert(memcmp(result, &packet, sizeof(packet)) == 0);
    assert(split_test_uart.starts == 2);
    inject((const uint8_t *)&packet, 3, false);
    assert(split_test_uart_poll_rx(result, 200) == 0);
    assert(split_test_uart_poll_rx(result, 202) == 0);
    assert(split_test_uart_poll_rx(result, 203) == 3);
    assert(memcmp(result, &packet, 3) == 0);
    assert(split_test_uart.starts == 3);
    inject((const uint8_t *)&packet, 2, false);
    assert(split_test_uart_poll_rx(result, UINT32_MAX - 1) == 0);
    assert(split_test_uart_poll_rx(result, 1) == 2); // tick wrap
    unsigned tx = split_test_uart.transmissions;
    split_transport_send_packet(&packet);
    assert(split_test_uart.transmissions == tx + 1);
    assert(split_test_uart.tx_size == sizeof(packet));
    assert(g_mock_ticks == 12); // slave keeps the reply window open
#if SPLIT_TX_PIN == SPLIT_RX_PIN
    split_test_uart.allow_stop = false;
    unsigned resets = split_test_uart.resets;
    tx = split_test_uart.transmissions;
    split_transport_send_packet(&packet);
    assert(split_test_uart.transmissions == tx && split_test_uart.resets == resets + 1);
    split_test_uart.allow_stop = true;
#endif
    master = true;
    split_test_uart.allow_tx = false;
    unsigned resets_before_timeout = split_test_uart.resets;
    TickType_t before_timeout = g_mock_ticks;
    split_transport_reply_packet(&packet);
    assert(split_test_uart.resets == resets_before_timeout + 1);
    assert(g_mock_ticks - before_timeout == 100); // no extra slave reply wait
    assert(split_test_uart.rx_size == sizeof(packet));
    puts("nRF split DMA receive and turnaround tests passed");
    return 0;
}
