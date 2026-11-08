#ifndef SPLIT_TEST_PICO_STDLIB_H
#define SPLIT_TEST_PICO_STDLIB_H
#include <stdint.h>
extern uint32_t test_pio_delay_us;
static inline void gpio_init(unsigned pin) { (void)pin; }
static inline void gpio_pull_up(unsigned pin) { (void)pin; }
static inline void sleep_us(uint32_t us) { test_pio_delay_us += us; }
#endif
