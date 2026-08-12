// HAL GPIO for Raspberry Pi (RP2040/RP2350)
#include "hal_gpio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"

void hal_gpio_init(pin_t gpio) {
    gpio_init(gpio);
}

void hal_gpio_set_dir(pin_t gpio, bool is_output) {
    gpio_set_dir(gpio, is_output);
}

void hal_gpio_put(pin_t gpio, bool value) {
    gpio_put(gpio, value);
}

bool hal_gpio_get(pin_t gpio) {
    return gpio_get(gpio);
}

void hal_gpio_pull_down(pin_t gpio) {
    gpio_pull_down(gpio);
}

void hal_gpio_pull_up(pin_t gpio) {
    gpio_pull_up(gpio);
}

void hal_sleep_us(uint32_t us) {
    sleep_us(us);
}

void platform_init(void) {
    // No platform-specific configuration needed for Raspberry Pi
}
