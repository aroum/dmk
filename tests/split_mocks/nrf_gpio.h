#ifndef SPLIT_TEST_NRF_GPIO_H
#define SPLIT_TEST_NRF_GPIO_H
#include <stdint.h>
#define NRF_GPIO_PIN_DIR_INPUT 0
#define NRF_GPIO_PIN_INPUT_CONNECT 0
#define NRF_GPIO_PIN_PULLUP 0
#define NRF_GPIO_PIN_S0D1 0
#define NRF_GPIO_PIN_NOSENSE 0
static inline void nrf_gpio_cfg(uint32_t pin, int dir, int input, int pull, int drive, int sense) {
    (void)pin; (void)dir; (void)input; (void)pull; (void)drive; (void)sense;
}
static inline void nrf_gpio_cfg_output(uint32_t pin) { (void)pin; }
static inline void nrf_gpio_cfg_input(uint32_t pin, int pull) { (void)pin; (void)pull; }
#endif
