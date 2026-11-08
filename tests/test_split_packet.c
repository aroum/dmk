#include <assert.h>
#include <stdio.h>
#include "split.h"

int main(void) {
    matrix_event_t event = {.row = 1, .col = 2, .pressed = true};
    split_packet_t packet;
    split_make_packet(&packet, &event);
    assert(split_packet_valid(&packet));
    uint8_t sequence = packet.sequence;
    split_make_packet(&packet, &event);
    assert(packet.sequence == (uint8_t)(sequence + 1));
    assert(split_packet_valid(&packet));
    packet.col ^= 1;
    assert(!split_packet_valid(&packet));
    packet.crc = split_packet_crc(&packet);
    packet.row = NUM_ROWS;
    packet.crc = split_packet_crc(&packet);
    assert(!split_packet_valid(&packet));
    packet.row = 0;
    packet.pressed = 2;
    packet.crc = split_packet_crc(&packet);
    assert(!split_packet_valid(&packet));
    split_make_rgb_packet(&packet, 9, 3, 0x123456);
    assert(split_packet_valid(&packet));
    assert(packet.row == SPLIT_CONTROL_ROW && packet.col == 3);
    assert(packet.pressed == 0x12 && packet.green == 0x34 && packet.blue == 0x56);
    packet.blue ^= 1;
    assert(!split_packet_valid(&packet));
    split_make_led_packet(&packet, 10, 0x15);
    assert(split_packet_valid(&packet));
    assert(packet.col == SPLIT_CONTROL_LED_MASK && packet.pressed == 0x15);
    packet.blue = 1;
    packet.crc = split_packet_crc(&packet);
    assert(!split_packet_valid(&packet));
    puts("split packet tests passed");
    return 0;
}
