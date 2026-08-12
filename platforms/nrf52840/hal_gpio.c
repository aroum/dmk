// HAL GPIO for nRF52 using nRF5 SDK nrf_gpio driver
#include "hal_gpio.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

void hal_gpio_init(pin_t gpio) {
    // Direction is set separately via hal_gpio_set_dir
    (void)gpio;
}

void hal_gpio_set_dir(pin_t gpio, bool is_output) {
    if (is_output) {
        nrf_gpio_cfg_output(gpio);
    } else {
        nrf_gpio_cfg_input(gpio, NRF_GPIO_PIN_NOPULL);
    }
}

void hal_gpio_put(pin_t gpio, bool value) {
    if (value) {
        nrf_gpio_pin_set(gpio);
    } else {
        nrf_gpio_pin_clear(gpio);
    }
}

bool hal_gpio_get(pin_t gpio) {
    return (bool)nrf_gpio_pin_read(gpio);
}

void hal_gpio_pull_down(pin_t gpio) {
    nrf_gpio_cfg_input(gpio, NRF_GPIO_PIN_PULLDOWN);
}

void hal_gpio_pull_up(pin_t gpio) {
    nrf_gpio_cfg_input(gpio, NRF_GPIO_PIN_PULLUP);
}

void hal_sleep_us(uint32_t us) {
    nrf_delay_us(us);
}

void platform_init(void) {
    // No platform-specific configuration needed for nRF52840
}
