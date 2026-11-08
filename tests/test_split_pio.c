#include <assert.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "hardware/pio.h"
#include "split.h"

TickType_t g_mock_ticks;
split_test_pio_t split_test_pio;
unsigned test_pio_pin, test_pio_tx_pin, test_pio_offset, test_pio_mode, test_pio_writes;
uint32_t test_pio_tx[16], test_pio_rx, test_pio_delay_us;
bool test_pio_rx_ready;
static bool master;
bool is_master(void) { return master; }
void split_process_received_byte(uint8_t byte) { (void)byte; }
bool split_test_pio_receive_one(uint8_t *byte);

int main(void) {
    split_init();
    assert(test_pio_mode == PIO_FIFO_JOIN_RX && test_pio_offset == 20 && test_pio_pin == SPLIT_RX_PIN);
    uint8_t byte;
    assert(!split_test_pio_receive_one(&byte));
    test_pio_rx = (uint32_t)0xA5 << 24;
    test_pio_rx_ready = true;
    assert(split_test_pio_receive_one(&byte) && byte == 0xA5);
    matrix_event_t event = {.row = 1, .col = 2, .pressed = true};
    split_packet_t packet;
    split_make_packet(&packet, &event);
    split_transport_send_packet(&packet);
    assert(test_pio_writes == sizeof(packet) && test_pio_tx_pin == SPLIT_TX_PIN);
    const uint8_t *bytes = (const uint8_t *)&packet;
    for (unsigned i = 0; i < sizeof(packet); ++i) assert((uint8_t)test_pio_tx[i] == (uint8_t)~bytes[i]);
    assert(test_pio_mode == PIO_FIFO_JOIN_RX && test_pio_offset == 20 && test_pio_pin == SPLIT_RX_PIN);
    assert(test_pio_delay_us == 20 * BIT_TIME_US && g_mock_ticks == 12);
    master = true;
    test_pio_writes = 0;
    split_make_led_packet(&packet, 2, 0x05);
    split_transport_reply_packet(&packet);
    assert(test_pio_writes == sizeof(packet) && g_mock_ticks == 12);
    assert(test_pio_mode == PIO_FIFO_JOIN_RX && test_pio_pin == SPLIT_RX_PIN);
    puts("PIO split transmit, receive and turnaround tests passed");
    return 0;
}
