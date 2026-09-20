#ifndef MOCK_HAL_GPIO_H
#define MOCK_HAL_GPIO_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t pin_t;

typedef struct {
    uint32_t p0;
    uint32_t p1;
    uint32_t p2;
} hal_gpio_snapshot_t;

static inline void hal_gpio_init(pin_t pin) { (void)pin; }
static inline void hal_gpio_set_dir(pin_t pin, bool is_output) { (void)pin; (void)is_output; }
static inline void hal_gpio_put(pin_t pin, bool val) { (void)pin; (void)val; }
static inline bool hal_gpio_get(pin_t pin) { (void)pin; return false; }
static inline void hal_gpio_pull_up(pin_t pin) { (void)pin; }
static inline void hal_gpio_pull_down(pin_t pin) { (void)pin; }
static inline hal_gpio_snapshot_t hal_gpio_snapshot(void) {
    hal_gpio_snapshot_t s = {0, 0, 0};
    return s;
}
static inline bool hal_gpio_snapshot_get(hal_gpio_snapshot_t s, pin_t pin) {
    (void)s; (void)pin; return false;
}
static inline void hal_sleep_us(uint32_t us) { (void)us; }

#endif // MOCK_HAL_GPIO_H
