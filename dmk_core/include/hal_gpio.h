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

/**
 * @brief Initialize a GPIO pin.
 * @param gpio Hardware pin identifier
 */
void hal_gpio_init(pin_t gpio);

/**
 * @brief Set GPIO pin direction.
 * @param gpio Hardware pin identifier
 * @param is_output True for output, false for input
 */
void hal_gpio_set_dir(pin_t gpio, bool is_output);

/**
 * @brief Set GPIO pin output state.
 * @param gpio Hardware pin identifier
 * @param value True for high, false for low
 */
void hal_gpio_put(pin_t gpio, bool value);

/**
 * @brief Read current state of a GPIO pin.
 * @param gpio Hardware pin identifier
 * @return true if pin is high, false if low
 */
bool hal_gpio_get(pin_t gpio);

/**
 * @brief Enable internal pull-down resistor on GPIO pin.
 * @param gpio Hardware pin identifier
 */
void hal_gpio_pull_down(pin_t gpio);

/**
 * @brief Enable internal pull-up resistor on GPIO pin.
 * @param gpio Hardware pin identifier
 */
void hal_gpio_pull_up(pin_t gpio);

/**
 * @brief Busy-wait delay in microseconds.
 * @param us Duration in microseconds
 */
void hal_sleep_us(uint32_t us);

/**
 * @brief Initialize hardware platform clocks, peripherals, and interrupts.
 */
void platform_init(void);

/**
 * @brief Reset microcontroller and jump to ROM/UF2 bootloader mode.
 */
void platform_bootloader_jump(void);

#endif
