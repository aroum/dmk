// HAL GPIO for Baikal – implemented using the Baikal SDK
#include "hal_gpio.h"
#include "bmcu_cru.h"
#include "bmcu_gpio.h"

static inline GPIO_TypeDef *get_gpio_port(pin_t pin) {
    if (pin < 16)
        return GPIO0;
    if (pin < 32)
        return GPIO1;
    return GPIO2;
}

static inline uint16_t get_gpio_pin_mask(pin_t pin) {
    return (uint16_t)(1U << (pin % 16));
}

void hal_gpio_init(pin_t gpio) {
    GPIO_TypeDef *port = get_gpio_port(gpio);
    uint16_t mask = get_gpio_pin_mask(gpio);
    GPIO_InitStruct_TypeDef init = {0};
    GPIO_StructInit(&init);
    init.PinMask = mask;
    init.Mode = GPIO_MODE_INPUT;
    GPIO_Init(port, &init);
}

void hal_gpio_set_dir(pin_t gpio, bool is_output) {
    GPIO_TypeDef *port = get_gpio_port(gpio);
    uint16_t mask = get_gpio_pin_mask(gpio);
    GPIO_InitStruct_TypeDef init = {0};
    GPIO_StructInit(&init);
    init.PinMask = mask;
    init.Mode = is_output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
    GPIO_Init(port, &init);
}

void hal_gpio_put(pin_t gpio, bool value) {
    GPIO_TypeDef *port = get_gpio_port(gpio);
    uint16_t mask = get_gpio_pin_mask(gpio);
    if (value) {
        GPIO_SetOutputPin(port, mask);
    } else {
        GPIO_ResetOutputPin(port, mask);
    }
}

bool hal_gpio_get(pin_t gpio) {
    GPIO_TypeDef *port = get_gpio_port(gpio);
    uint16_t mask = get_gpio_pin_mask(gpio);
    return (GPIO_ReadInputPort(port) & mask) != 0U;
}

void hal_gpio_pull_down(pin_t gpio) {
    uint8_t port = gpio / 16;
    uint16_t mask = get_gpio_pin_mask(gpio);
    CRU_SetPinPull(port, mask, CRU_PIN_PULL_DOWN);
}

void hal_gpio_pull_up(pin_t gpio) {
    uint8_t port = gpio / 16;
    uint16_t mask = get_gpio_pin_mask(gpio);
    CRU_SetPinPull(port, mask, CRU_PIN_PULL_UP);
}

void hal_sleep_us(uint32_t us) {
    __delay_us(us);
}

void platform_init(void) {
    // Enable clocks for GPIO0, GPIO1, GPIO2 via CRU
    // Assuming they are enabled, or enable if needed.
}
