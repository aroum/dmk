#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "pin_defs.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Port snapshot structure supporting up to 64 GPIO lines.
 * Enables single-cycle batch reads of entire GPIO port registers.
 */
typedef struct {
    uint32_t p0;
    uint32_t p1;
    uint32_t p2;
} hal_gpio_snapshot_t;

/**
 * @brief Captures the raw hardware state of all GPIO pins simultaneously.
 * @return hal_gpio_snapshot_t Containing 32-bit port register states.
 */
hal_gpio_snapshot_t hal_gpio_snapshot(void);

/**
 * @brief Fast inline test of a pin's state from a pre-captured port snapshot.
 * Executes in a single CPU instruction (register shift + mask) with zero bus traffic.
 * @param snapshot Port snapshot captured via hal_gpio_snapshot()
 * @param pin Target GPIO pin number
 * @return true if pin is high, false if low
 */
static inline bool hal_gpio_snapshot_get(hal_gpio_snapshot_t snapshot, pin_t pin) {
    if (pin < 32) {
        return (snapshot.p0 & (1UL << pin)) != 0;
    } else if (pin < 64) {
        return (snapshot.p1 & (1UL << (pin - 32))) != 0;
    } else {
        return (snapshot.p2 & (1UL << (pin - 64))) != 0;
    }
}

void hal_gpio_init(pin_t gpio);
void hal_gpio_set_dir(pin_t gpio, bool is_output);
void hal_gpio_put(pin_t gpio, bool value);
bool hal_gpio_get(pin_t gpio);
void hal_gpio_pull_down(pin_t gpio);
void hal_gpio_pull_up(pin_t gpio);
void hal_sleep_us(uint32_t us);
void platform_init(void);

#endif
