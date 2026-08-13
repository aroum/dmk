#ifndef _SPLIT_H
#define _SPLIT_H

#include "compile.h"
#include "config.h"
#include "matrix.h"
#include <stdint.h> // uint8_t

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

#if SPLIT_HARDWARE_OPTION == 1
#define SPLIT_UART_BAUD 115200
#elif SPLIT_HARDWARE_OPTION == 2
#define SPLIT_UART_BAUD 19200
#else
#define SPLIT_UART_BAUD 19200
#endif

#define BIT_TIME_US (1000000 / SPLIT_UART_BAUD)

#ifndef SPLIT_COL_OFFSET
#ifdef NUM_COLS_SPLIT
#define SPLIT_COL_OFFSET NUM_COLS_SPLIT
#else
#define SPLIT_COL_OFFSET 0
#endif
#endif

typedef struct {
    uint8_t header; // 0xA5
    uint8_t row;
    uint8_t col;
    uint8_t pressed;
} split_packet_t;

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
bool is_master(void);
void split_send_event(matrix_event_t *event);

// Universal software bit-bang engine
void split_soft_init(void);
void split_soft_send_event(matrix_event_t *event);

// Common split packet parser
void split_process_received_byte(uint8_t byte);

#else

static inline void split_init(void) {}
static inline bool is_master(void) { return true; }
static inline void split_send_event(matrix_event_t *event) { (void)event; }
static inline void split_soft_init(void) {}
static inline void split_soft_send_event(matrix_event_t *event) { (void)event; }
static inline void split_process_received_byte(uint8_t byte) { (void)byte; }

#endif

#endif // _SPLIT_H
