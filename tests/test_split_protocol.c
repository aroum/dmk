#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "matrix.h"
#include "split.h"

TickType_t g_mock_ticks;
volatile bool usb_mounted;
QueueHandle_t matrix_queue = (void *)1;
static matrix_event_t events[16];
static unsigned event_count, show_count;
static uint8_t led_mask, rgb_index;
static uint32_t rgb_color;
static bool waveform_active;
static uint32_t waveform_us;
static int bad_stop_byte = -1;
static split_packet_t waveform_packet;
typedef struct { uint32_t at; bool high; } waveform_edge_t;
static waveform_edge_t edges[200];
static unsigned edge_count;
static bool capture_waveform, replay_waveform, gpio_output, gpio_latch, gpio_level = true;
static void record_gpio_level(bool high) {
    if (high == gpio_level) return;
    gpio_level = high;
    if (capture_waveform) {
        assert(edge_count < sizeof(edges) / sizeof(edges[0]));
        edges[edge_count++] = (waveform_edge_t){g_mock_ticks * 1000u + waveform_us, high};
    }
}
void hal_gpio_init(uint8_t pin) { (void)pin; }
void hal_gpio_set_dir(uint8_t pin, bool output) {
    (void)pin; gpio_output = output; record_gpio_level(output ? gpio_latch : true);
}
void hal_gpio_put(uint8_t pin, bool value) {
    (void)pin; gpio_latch = value; if (gpio_output) record_gpio_level(value);
}
void hal_gpio_pull_up(uint8_t pin) { (void)pin; }
void hal_sleep_us(uint32_t us) { waveform_us += us; }
bool hal_gpio_get(uint8_t pin) {
    (void)pin;
    if (replay_waveform) {
        bool high = true;
        for (unsigned i = 0; i < edge_count && edges[i].at <= waveform_us; ++i) high = edges[i].high;
        return high;
    }
    if (!waveform_active) return true;
    if (waveform_us < 4000) return false;
    if (waveform_us < 4000 + SPLIT_SOFT_WAKE_GAP_US) return true;
    uint32_t elapsed = waveform_us - (4000 + SPLIT_SOFT_WAKE_GAP_US);
    uint32_t byte_index = elapsed / (11 * BIT_TIME_US);
    uint32_t bit = (elapsed / BIT_TIME_US) % 11;
    if (byte_index >= sizeof(waveform_packet)) return true;
    if (bit == 0) return false;
    if (bit == 9 && (int)byte_index == bad_stop_byte) return false;
    if (bit >= 9) return true;
    return (((const uint8_t *)&waveform_packet)[byte_index] >> (bit - 1)) & 1;
}
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t wait) {
    (void)wait;
    if (queue == matrix_queue) {
        assert(event_count < sizeof(events) / sizeof(events[0]));
        events[event_count++] = *(const matrix_event_t *)item;
    }
    return pdTRUE;
}
static bool mock_pressed;
static uint8_t queried_row, queried_col;
bool matrix_is_pressed(unsigned char row, unsigned char col) {
    queried_row = row; queried_col = col; return mock_pressed;
}
void led_set_output_mask(uint8_t mask) { led_mask = mask; }
void rgb_set_pixel_raw(uint32_t index, uint32_t color) { rgb_index = index; rgb_color = color; }
void rgb_show(void) { ++show_count; }
bool split_test_next_output_packet(split_packet_t *packet);
void split_test_expire_remote(TickType_t now);
bool split_test_soft_receive_frame(uint32_t wait_us);
void split_test_soft_transmit(const split_packet_t *packet);
matrix_event_t split_test_make_snapshot(uint8_t row, uint8_t col);
static split_packet_t last_reply;
static unsigned reply_count;
void split_transport_reply_packet(const split_packet_t *packet) {
    last_reply = *packet;
    ++reply_count;
}

static void feed(const split_packet_t *packet) {
    const uint8_t *bytes = (const uint8_t *)packet;
    for (size_t i = 0; i < sizeof(*packet); ++i) split_process_received_byte(bytes[i]);
}

int main(void) {
    split_common_init();
    mock_pressed = true;
    matrix_event_t snapshot = split_test_make_snapshot(1, 2);
    assert(queried_row == 1 && queried_col == 2 && snapshot.row == 1 && snapshot.pressed);
#if defined(RIGHT) || defined(SPLIT_RIGHT)
    assert(snapshot.col == 5);
#else
    assert(snapshot.col == 2);
#endif
    split_packet_t packet;
    usb_mounted = false;
    split_make_led_packet(&packet, 1, 0x05);
    feed(&packet);
    assert(led_mask == 0x05 && event_count == 0);
    split_make_rgb_packet(&packet, 2, 1, 0x123456);
    feed(&packet);
    assert(rgb_index == 1 && rgb_color == 0x123456 && show_count == 1);
    packet.blue ^= 1;
    feed(&packet);
    assert(show_count == 1);
    matrix_event_t event = {.row = 1, .col = 2, .pressed = 1};
    split_make_packet(&packet, &event);
    g_mock_ticks += 21; // discard any partial candidate from the corrupt frame
    feed(&packet);
    assert(event_count == 0); // slave must not consume its own UART echo
    // Exercise the actual software receiver against a timed UART waveform.
    split_make_led_packet(&waveform_packet, 3, 0x19);
    waveform_us = 0;
    waveform_active = true;
    assert(split_test_soft_receive_frame(1000));
    waveform_active = false;
    assert(led_mask == 0x19);
    // The listener may wake during the preamble rather than at its beginning.
    split_make_led_packet(&waveform_packet, 4, 0x2A);
    waveform_us = 1900;
    waveform_active = true;
    assert(split_test_soft_receive_frame(1000));
    waveform_active = false;
    assert(led_mask == 0x2A);
    // A framing error must discard the entire command.
    split_make_led_packet(&waveform_packet, 5, 0x55);
    waveform_us = 0;
    bad_stop_byte = 3;
    waveform_active = true;
    assert(!split_test_soft_receive_frame(1000));
    waveform_active = false;
    bad_stop_byte = -1;
    assert(led_mask == 0x2A);
    usb_mounted = true;
    feed(&packet);
    assert(event_count == 1 && events[0].pressed && events[0].row == 1 && events[0].col == 2);
    feed(&packet);
    assert(event_count == 1); // repeated snapshot is not a second press
    event.pressed = 0;
    split_make_packet(&packet, &event);
    feed(&packet);
    assert(event_count == 2 && !events[1].pressed);
    // Lost release is repaired by a later snapshot; cable loss releases once.
    event.pressed = 1;
    split_make_packet(&packet, &event);
    feed(&packet);
    assert(event_count == 3 && events[2].pressed);
    g_mock_ticks += (2000u + NUM_ROWS_SPLIT * NUM_COLS_SPLIT * 50u) - 1;
    split_test_expire_remote(g_mock_ticks);
    assert(event_count == 3);
    ++g_mock_ticks;
    split_test_expire_remote(g_mock_ticks);
    assert(event_count == 4 && !events[3].pressed);
    split_test_expire_remote(g_mock_ticks);
    assert(event_count == 4);
    feed(&packet); // reconnect with a still-held key
    assert(event_count == 5 && events[4].pressed);
    event.pressed = 0;
    split_make_packet(&packet, &event);
    feed(&packet);
    assert(event_count == 6 && !events[5].pressed);
    split_set_remote_leds(0x13);
    split_set_remote_rgb(1, 0xAABBCC);
    assert(split_test_next_output_packet(&packet));
    assert(split_packet_valid(&packet) && packet.col == SPLIT_CONTROL_LED_MASK && packet.pressed == 0x13);
    assert(split_test_next_output_packet(&packet));
    assert(split_packet_valid(&packet) && packet.col == 1 && packet.pressed == 0xAA && packet.green == 0xBB && packet.blue == 0xCC);
    // A lost response is retried in a later round, with the same output value.
    assert(split_test_next_output_packet(&packet));
    assert(packet.col == SPLIT_CONTROL_LED_MASK && packet.pressed == 0x13);
    assert(split_test_next_output_packet(&packet));
    assert(packet.col == 1 && packet.pressed == 0xAA && packet.green == 0xBB && packet.blue == 0xCC);
    unsigned replies_before_echo = reply_count;
    uint8_t led_before_echo = led_mask;
    split_make_led_packet(&packet, 11, 0x77);
    feed(&packet); // master must ignore its own echoed output command
    assert(led_mask == led_before_echo && reply_count == replies_before_echo);
    // Each valid matrix frame grants exactly one response, including a repeated snapshot.
    split_make_packet(&packet, &event);
    TickType_t before_reply = g_mock_ticks;
    feed(&packet);
    assert(g_mock_ticks - before_reply == SPLIT_REPLY_TURNAROUND_MS);
    assert(reply_count == 1 && split_packet_valid(&last_reply) && last_reply.row == SPLIT_CONTROL_ROW);
    feed(&packet);
    assert(reply_count == 2 && split_packet_valid(&last_reply));
    assert(last_reply.col == 1); // next cached output is RGB index 1
    usb_mounted = false;
    feed(&last_reply);
    assert(rgb_index == 1 && rgb_color == 0xAABBCC && show_count == 2);
    // Record the production transmitter's GPIO edges, then replay those exact
    // edges into the production receiver on the other half.
    usb_mounted = false;
    split_make_led_packet(&packet, 77, 0x35);
    waveform_us = 0;
    edge_count = 0;
    gpio_output = false;
    gpio_level = true;
    capture_waveform = true;
    split_test_soft_transmit(&packet);
    capture_waveform = false;
    assert(edge_count > 2 && !edges[0].high);
    waveform_us = edges[0].at;
    replay_waveform = true;
    assert(split_test_soft_receive_frame(1000));
    replay_waveform = false;
    assert(led_mask == 0x35);
    // The same captured waveform path carries a matrix snapshot to master,
    // which selects an output reply that the slave can apply.
    usb_mounted = true;
    split_make_packet(&packet, &event);
    waveform_us = 0;
    edge_count = 0;
    gpio_output = false;
    gpio_level = true;
    capture_waveform = true;
    split_test_soft_transmit(&packet);
    capture_waveform = false;
    assert(edge_count > 2 && !edges[0].high);
    waveform_us = edges[0].at;
    replay_waveform = true;
    assert(split_test_soft_receive_frame(1000));
    replay_waveform = false;
    assert(reply_count == 3 && last_reply.col == SPLIT_CONTROL_LED_MASK);
    usb_mounted = false;
    feed(&last_reply);
    assert(led_mask == 0x13);
    usb_mounted = true;
    // Bad CRC must not generate a reply or mutate the matrix.
    packet.crc ^= 1;
    feed(&packet);
    assert(reply_count == 3 && event_count == 6);
    puts("split protocol receive and reply selection tests passed");
    return 0;
}
