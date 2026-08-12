#include "split.h"
#include "FreeRTOS.h"
#include "config.h"
#include "hal_gpio.h"
#include "queue.h"
#include "task.h"
#include "task_internal.h"
#include "tusb.h"

#if defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

// Standard Pin & Connection Type Fallbacks
#ifndef SPLIT_TX_PIN
#ifdef SERIAL_PIN
#define SPLIT_TX_PIN SERIAL_PIN
#else
#define SPLIT_TX_PIN GPIO0
#endif
#endif

#ifndef SPLIT_RX_PIN
#ifdef SERIAL_PIN
#define SPLIT_RX_PIN SERIAL_PIN
#else
#define SPLIT_RX_PIN SPLIT_TX_PIN
#endif
#endif

#ifndef SERIAL_PIN
#define SERIAL_PIN SPLIT_TX_PIN
#endif

#if (SPLIT_CONNECTION_TYPE == HW_HALF_DUPLEX)
#if (SPLIT_TX_PIN != 38 && SPLIT_TX_PIN != 40)
#error                                                                                                                 \
    "DMK Baikal HW UART Error: Hardware HW_HALF_DUPLEX (RS-485 mode) is only supported on UART6 (PC6) and UART7 (PC8)! For other pins, use SPLIT_CONNECTION_TYPE SOFT for 1-wire."
#endif
#endif

#if (SPLIT_CONNECTION_TYPE == HW_FULL_DUPLEX)
#if (SPLIT_TX_PIN / 16) != (SPLIT_RX_PIN / 16)
#error                                                                                                                 \
    "DMK Baikal HW UART Error: SPLIT_TX_PIN and SPLIT_RX_PIN must be on the SAME GPIO port (e.g. both on Port A, Port B, or Port C)!"
#endif

#if ((SPLIT_TX_PIN / 16) == 0) // Port A
#if !((SPLIT_TX_PIN == 2 && SPLIT_RX_PIN == 3) || (SPLIT_TX_PIN == 6 && SPLIT_RX_PIN == 7) ||                          \
      (SPLIT_TX_PIN == 8 && SPLIT_RX_PIN == 9) || (SPLIT_TX_PIN == 4 && SPLIT_RX_PIN == 5) ||                          \
      (SPLIT_TX_PIN == 10 && SPLIT_RX_PIN == 11) || (SPLIT_TX_PIN == 14 && SPLIT_RX_PIN == 15) ||                      \
      (SPLIT_TX_PIN == 12 && SPLIT_RX_PIN == 13))
#error                                                                                                                 \
    "DMK Baikal HW UART Error: Invalid HW UART pin pair on Port A! Valid pairs: UART0 (PA2+PA3, PA6+PA7, PA8+PA9), UART1 (PA4+PA5, PA10+PA11, PA14+PA15), UART2 (PA12+PA13)."
#endif
#elif ((SPLIT_TX_PIN / 16) == 1) // Port B
#if !((SPLIT_TX_PIN == 18 && SPLIT_RX_PIN == 19) || (SPLIT_TX_PIN == 22 && SPLIT_RX_PIN == 23) ||                      \
      (SPLIT_TX_PIN == 24 && SPLIT_RX_PIN == 25) || (SPLIT_TX_PIN == 20 && SPLIT_RX_PIN == 21) ||                      \
      (SPLIT_TX_PIN == 26 && SPLIT_RX_PIN == 27) || (SPLIT_TX_PIN == 30 && SPLIT_RX_PIN == 31) ||                      \
      (SPLIT_TX_PIN == 28 && SPLIT_RX_PIN == 29))
#error                                                                                                                 \
    "DMK Baikal HW UART Error: Invalid HW UART pin pair on Port B! Valid pairs: UART3 (PB2+PB3, PB6+PB7, PB8+PB9), UART4 (PB4+PB5, PB10+PB11, PB14+PB15), UART5 (PB12+PB13)."
#endif
#elif ((SPLIT_TX_PIN / 16) == 2) // Port C
#if !((SPLIT_TX_PIN == 38 && SPLIT_RX_PIN == 39) || (SPLIT_TX_PIN == 40 && SPLIT_RX_PIN == 41))
#error "DMK Baikal HW UART Error: Invalid HW UART pin pair on Port C! Valid pairs: UART6 (PC6+PC7), UART7 (PC8+PC9)."
#endif
#else
#error                                                                                                                 \
    "DMK Baikal HW UART Error: Selected port does not support Hardware UART! Use Port A, Port B, or Port C, or switch to SPLIT_CONNECTION_TYPE SOFT."
#endif
#endif

bool is_master(void) {
    return tud_mounted();
}

void split_send_event(matrix_event_t *event) {
    split_soft_send_event(event);
}

void split_init(void) {
    split_soft_init();
}

#else

void split_send_event(matrix_event_t *event) {
    (void)event;
}

void split_init(void) {}

#endif
