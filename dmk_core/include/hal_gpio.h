#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "pin_defs.h"
#include <stdbool.h>
#include <stdint.h>

void hal_gpio_init(pin_t gpio);
void hal_gpio_set_dir(pin_t gpio, bool is_output);
void hal_gpio_put(pin_t gpio, bool value);
bool hal_gpio_get(pin_t gpio);
void hal_gpio_pull_down(pin_t gpio);
void hal_gpio_pull_up(pin_t gpio);
void hal_sleep_us(uint32_t us);
void platform_init(void);

#endif
