#ifndef SPLIT_TEST_HAL_GPIO_H
#define SPLIT_TEST_HAL_GPIO_H
#include <stdbool.h>
#include <stdint.h>
typedef uint8_t pin_t;
typedef struct { uint32_t p0, p1, p2; } hal_gpio_snapshot_t;
void hal_gpio_init(pin_t pin);
void hal_gpio_set_dir(pin_t pin, bool output);
void hal_gpio_put(pin_t pin, bool value);
bool hal_gpio_get(pin_t pin);
void hal_gpio_pull_up(pin_t pin);
void hal_gpio_pull_down(pin_t pin);
hal_gpio_snapshot_t hal_gpio_snapshot(void);
bool hal_gpio_snapshot_get(hal_gpio_snapshot_t snapshot, pin_t pin);
void hal_sleep_us(uint32_t us);
#endif
