#ifndef SPLIT_TEST_PIO_PROGRAM_H
#define SPLIT_TEST_PIO_PROGRAM_H
#include "hardware/pio.h"
static const split_test_pio_program_t split_tx_program = {.offset = 10};
static const split_test_pio_program_t split_rx_program = {.offset = 20};
static inline pio_sm_config split_tx_program_get_default_config(uint offset) { (void)offset; return (pio_sm_config){0}; }
static inline pio_sm_config split_rx_program_get_default_config(uint offset) { (void)offset; return (pio_sm_config){0}; }
#endif
