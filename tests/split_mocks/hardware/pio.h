#ifndef SPLIT_TEST_PIO_H
#define SPLIT_TEST_PIO_H
#include <stdbool.h>
#include <stdint.h>
typedef unsigned int uint;
typedef struct { unsigned unused; } split_test_pio_t;
typedef split_test_pio_t *PIO;
typedef struct { unsigned unused; } pio_sm_config;
typedef struct { unsigned offset; } split_test_pio_program_t;
extern split_test_pio_t split_test_pio;
#define pio0 (&split_test_pio)
#define PIO_FIFO_JOIN_RX 1
#define PIO_FIFO_JOIN_TX 2
extern unsigned test_pio_pin, test_pio_tx_pin, test_pio_offset, test_pio_mode, test_pio_writes;
extern uint32_t test_pio_tx[16], test_pio_rx;
extern bool test_pio_rx_ready;
static inline void pio_sm_set_enabled(PIO p, uint sm, bool enabled) { (void)p; (void)sm; (void)enabled; }
static inline void sm_config_set_in_pins(pio_sm_config *c, uint pin) { (void)c; test_pio_pin = pin; }
static inline void sm_config_set_jmp_pin(pio_sm_config *c, uint pin) { (void)c; (void)pin; }
static inline void sm_config_set_in_shift(pio_sm_config *c, bool shift, bool autopush, uint bits) { (void)c; (void)shift; (void)autopush; (void)bits; }
static inline void sm_config_set_fifo_join(pio_sm_config *c, int mode) { (void)c; test_pio_mode = mode; }
static inline void sm_config_set_clkdiv(pio_sm_config *c, float div) { (void)c; (void)div; }
static inline void pio_sm_init(PIO p, uint sm, uint offset, pio_sm_config *c) { (void)p; (void)sm; (void)c; test_pio_offset = offset; }
static inline void pio_sm_set_consecutive_pindirs(PIO p, uint sm, uint pin, uint count, bool output) { (void)p; (void)sm; (void)pin; (void)count; (void)output; }
static inline void sm_config_set_out_pins(pio_sm_config *c, uint pin, uint count) { (void)c; (void)count; test_pio_pin = pin; test_pio_tx_pin = pin; }
static inline void sm_config_set_sideset_pins(pio_sm_config *c, uint pin) { (void)c; (void)pin; }
static inline void sm_config_set_out_shift(pio_sm_config *c, bool shift, bool autopull, uint bits) { (void)c; (void)shift; (void)autopull; (void)bits; }
static inline void pio_sm_put_blocking(PIO p, uint sm, uint32_t value) { (void)p; (void)sm; test_pio_tx[test_pio_writes++] = value; }
static inline bool pio_sm_is_tx_fifo_empty(PIO p, uint sm) { (void)p; (void)sm; return true; }
static inline bool pio_sm_is_rx_fifo_empty(PIO p, uint sm) { (void)p; (void)sm; return !test_pio_rx_ready; }
static inline uint32_t pio_sm_get(PIO p, uint sm) { (void)p; (void)sm; test_pio_rx_ready = false; return test_pio_rx; }
static inline uint pio_add_program(PIO p, const split_test_pio_program_t *program) { (void)p; return program->offset; }
static inline uint pio_claim_unused_sm(PIO p, bool required) { (void)p; (void)required; return 0; }
static inline void pio_gpio_init(PIO p, uint pin) { (void)p; (void)pin; }
#endif
