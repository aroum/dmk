#ifndef SPLIT_TEST_CLOCKS_H
#define SPLIT_TEST_CLOCKS_H
#define clk_sys 0
static inline unsigned clock_get_hz(int clk) { (void)clk; return 125000000; }
#endif
