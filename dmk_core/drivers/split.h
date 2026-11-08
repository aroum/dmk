#ifndef _SPLIT_H
#define _SPLIT_H

#include "compile.h"
#include "config.h"
#include "matrix.h"
#include <stdint.h> // uint8_t
#include <stddef.h>

#include <stdbool.h> // bool

#define SPLIT_TYPE_SOFT 1
#define SPLIT_TYPE_HW_HALF_DUPLEX 2
#define SPLIT_TYPE_HW_FULL_DUPLEX 3

#ifndef SOFT
#define SOFT SPLIT_TYPE_SOFT
#endif

#ifndef HW_HALF_DUPLEX
#define HW_HALF_DUPLEX SPLIT_TYPE_HW_HALF_DUPLEX
#endif

#ifndef HW_FULL_DUPLEX
#define HW_FULL_DUPLEX SPLIT_TYPE_HW_FULL_DUPLEX
#endif

#ifndef SPLIT_HARDWARE_OPTION
#define SPLIT_HARDWARE_OPTION 2
#endif

#ifndef SPLIT_UART_BAUD
#define SPLIT_UART_BAUD 115200
#endif

#define BIT_TIME_US (1000000 / SPLIT_UART_BAUD)
#ifndef SPLIT_SOFT_WAKE_GAP_US
#define SPLIT_SOFT_WAKE_GAP_US 250U
#endif
#if BIT_TIME_US < 1 || SPLIT_SOFT_WAKE_GAP_US == 0 || SPLIT_SOFT_WAKE_GAP_US >= 1000
#error "Invalid split soft UART baud rate or wake gap"
#endif

#ifndef SPLIT_COL_OFFSET
#ifdef NUM_COLS_SPLIT
#define SPLIT_COL_OFFSET NUM_COLS_SPLIT
#else
#define SPLIT_COL_OFFSET 0
#endif
#endif

#define SPLIT_PACKET_HEADER 0xA5
#define SPLIT_CONTROL_ROW 0xFF
#define SPLIT_CONTROL_LED_MASK 0xFF
// Allow the transmitting half to finish DMA/PIO turnaround before the reply.
#define SPLIT_REPLY_TURNAROUND_MS 3

typedef struct {
    uint8_t header;
    uint8_t sequence;
    uint8_t row;
    uint8_t col;
    uint8_t pressed;
    uint8_t green;
    uint8_t blue;
    uint8_t crc;
} split_packet_t;

static inline uint8_t split_packet_crc(const split_packet_t *pkt) {
    const uint8_t *bytes = (const uint8_t *)pkt;
    uint8_t crc = 0;
    for (size_t i = 0; i < offsetof(split_packet_t, crc); ++i) {
        crc ^= bytes[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (uint8_t)((crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1);
    }
    return crc;
}

static inline bool split_packet_valid(const split_packet_t *pkt) {
    if (pkt->header != SPLIT_PACKET_HEADER || pkt->crc != split_packet_crc(pkt)) return false;
    if (pkt->row == SPLIT_CONTROL_ROW) {
        return pkt->col != SPLIT_CONTROL_LED_MASK || (pkt->green == 0 && pkt->blue == 0);
    }
    return pkt->pressed <= 1 && pkt->green == 0 && pkt->blue == 0 &&
           pkt->row < NUM_ROWS && pkt->col < NUM_COLS;
}

static inline void split_make_packet(split_packet_t *pkt, const matrix_event_t *event) {
    static uint8_t sequence;
    pkt->header = SPLIT_PACKET_HEADER;
    pkt->sequence = sequence++;
    pkt->row = event->row;
    pkt->col = event->col;
    pkt->pressed = event->pressed;
    pkt->green = 0;
    pkt->blue = 0;
    pkt->crc = split_packet_crc(pkt);
}

// Commands carry resolved output values, not layer or keycode semantics.
// Index 255 is reserved for a bitmask of ordinary indicator LEDs.
static inline void split_make_rgb_packet(split_packet_t *pkt, uint8_t sequence, uint8_t index, uint32_t rgb) {
    pkt->header = SPLIT_PACKET_HEADER;
    pkt->sequence = sequence;
    pkt->row = SPLIT_CONTROL_ROW;
    pkt->col = index;
    pkt->pressed = (uint8_t)(rgb >> 16);
    pkt->green = (uint8_t)(rgb >> 8);
    pkt->blue = (uint8_t)rgb;
    pkt->crc = split_packet_crc(pkt);
}

static inline void split_make_led_packet(split_packet_t *pkt, uint8_t sequence, uint8_t mask) {
    split_make_rgb_packet(pkt, sequence, SPLIT_CONTROL_LED_MASK, (uint32_t)mask << 16);
}

// Standard Pin & Connection Type Fallbacks
#ifndef SPLIT_TX_PIN
#ifdef SERIAL_PIN
#define SPLIT_TX_PIN SERIAL_PIN
#endif
#endif

#ifndef SPLIT_RX_PIN
#ifdef SERIAL_PIN
#define SPLIT_RX_PIN SERIAL_PIN
#elif defined(SPLIT_TX_PIN)
#define SPLIT_RX_PIN SPLIT_TX_PIN
#endif
#endif

#ifndef SERIAL_PIN
#ifdef SPLIT_TX_PIN
#define SERIAL_PIN SPLIT_TX_PIN
#endif
#endif

#if defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

// Split communication initialization, to be executed at startup
void split_init(void);
void split_common_init(void);
void split_transport_send_packet(const split_packet_t *packet);
void split_transport_reply_packet(const split_packet_t *packet);
bool is_master(void);
void split_send_event(matrix_event_t *event);
// Set desired physical outputs on the remote half; these calls do not block on I/O.
void split_set_remote_leds(uint8_t mask);
void split_set_remote_rgb(uint8_t index, uint32_t rgb);

// Universal software bit-bang engine
void split_soft_init(void);
void split_soft_send_packet(const split_packet_t *packet);

// Common split packet parser
void split_process_received_byte(uint8_t byte);

#else

static inline void split_init(void) {}
static inline void split_common_init(void) {}
static inline bool is_master(void) {
    return true;
}
static inline void split_send_event(matrix_event_t *event) {
    (void)event;
}
static inline void split_set_remote_leds(uint8_t mask) { (void)mask; }
static inline void split_set_remote_rgb(uint8_t index, uint32_t rgb) { (void)index; (void)rgb; }
static inline void split_soft_init(void) {}
static inline void split_soft_send_packet(const split_packet_t *packet) {
    (void)packet;
}
static inline void split_process_received_byte(uint8_t byte) {
    (void)byte;
}

#endif

#endif // _SPLIT_H
