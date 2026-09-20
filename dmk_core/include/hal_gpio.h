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

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/gpio.h"

static inline void hal_gpio_put(pin_t gpio, bool value) {
    gpio_put(gpio, value);
}

static inline bool hal_gpio_get(pin_t gpio) {
    return gpio_get(gpio);
}

static inline hal_gpio_snapshot_t hal_gpio_snapshot(void) {
    hal_gpio_snapshot_t s = {0};
#if defined(MCU_rp2350)
    uint64_t all = gpio_get_all64();
    s.p0 = (uint32_t)all;
    s.p1 = (uint32_t)(all >> 32);
#else
    s.p0 = gpio_get_all();
#endif
    return s;
}

#elif defined(MCU_nrf52840)
#include "nrf_gpio.h"

static inline void hal_gpio_put(pin_t gpio, bool value) {
    if (value) {
        nrf_gpio_pin_set(gpio);
    } else {
        nrf_gpio_pin_clear(gpio);
    }
}

static inline bool hal_gpio_get(pin_t gpio) {
    return (bool)nrf_gpio_pin_read(gpio);
}

static inline hal_gpio_snapshot_t hal_gpio_snapshot(void) {
    hal_gpio_snapshot_t s = {0};
    s.p0 = NRF_P0->IN;
#if defined(NRF_P1)
    s.p1 = NRF_P1->IN;
#endif
    return s;
}

#elif defined(MCU_baikal)
#include "bmcu_gpio.h"

static inline GPIO_TypeDef *hal_internal_get_gpio_port(pin_t pin) {
    if (pin < 16)
        return GPIO0;
    if (pin < 32)
        return GPIO1;
    return GPIO2;
}

static inline uint16_t hal_internal_get_gpio_pin_mask(pin_t pin) {
    return (uint16_t)(1U << (pin % 16));
}

static inline void hal_gpio_put(pin_t gpio, bool value) {
    GPIO_TypeDef *port = hal_internal_get_gpio_port(gpio);
    uint16_t mask = hal_internal_get_gpio_pin_mask(gpio);
    if (value) {
        GPIO_SetOutputPin(port, mask);
    } else {
        GPIO_ResetOutputPin(port, mask);
    }
}

static inline bool hal_gpio_get(pin_t gpio) {
    GPIO_TypeDef *port = hal_internal_get_gpio_port(gpio);
    uint16_t mask = hal_internal_get_gpio_pin_mask(gpio);
    return (GPIO_ReadInputPort(port) & mask) != 0U;
}

static inline hal_gpio_snapshot_t hal_gpio_snapshot(void) {
    hal_gpio_snapshot_t s = {0};
    s.p0 = (uint32_t)GPIO_ReadInputPort(GPIO0) | ((uint32_t)GPIO_ReadInputPort(GPIO1) << 16);
    s.p1 = (uint32_t)GPIO_ReadInputPort(GPIO2);
    return s;
}

#elif defined(MCU_milandr)
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"

static inline void hal_gpio_put(pin_t gpio, bool value) {
    uint8_t idx = gpio / 16;
    MDR_PORT_TypeDef *port = (idx == 0) ? MDR_PORTA :
                             (idx == 1) ? MDR_PORTB :
                             (idx == 2) ? MDR_PORTC :
                             (idx == 3) ? MDR_PORTD :
                             (idx == 4) ? MDR_PORTE : MDR_PORTF;
    uint16_t pin = (1 << (gpio & 0x0F));
    if (value)
        PORT_SetBits(port, pin);
    else
        PORT_ResetBits(port, pin);
}

static inline bool hal_gpio_get(pin_t gpio) {
    uint8_t idx = gpio / 16;
    MDR_PORT_TypeDef *port = (idx == 0) ? MDR_PORTA :
                             (idx == 1) ? MDR_PORTB :
                             (idx == 2) ? MDR_PORTC :
                             (idx == 3) ? MDR_PORTD :
                             (idx == 4) ? MDR_PORTE : MDR_PORTF;
    uint16_t pin = (1 << (gpio & 0x0F));
    return PORT_ReadInputDataBit(port, pin) != RESET;
}

static inline hal_gpio_snapshot_t hal_gpio_snapshot(void) {
    hal_gpio_snapshot_t s = {0};
    if (MDR_RST_CLK->PER_CLOCK & RST_CLK_PCLK_PORTA)
        s.p0 |= (uint32_t)PORT_ReadInputData(MDR_PORTA);
    if (MDR_RST_CLK->PER_CLOCK & RST_CLK_PCLK_PORTB)
        s.p0 |= ((uint32_t)PORT_ReadInputData(MDR_PORTB) << 16);
    if (MDR_RST_CLK->PER_CLOCK & RST_CLK_PCLK_PORTC)
        s.p1 |= (uint32_t)PORT_ReadInputData(MDR_PORTC);
    if (MDR_RST_CLK->PER_CLOCK & RST_CLK_PCLK_PORTD)
        s.p1 |= ((uint32_t)PORT_ReadInputData(MDR_PORTD) << 16);
    if (MDR_RST_CLK->PER_CLOCK & RST_CLK_PCLK_PORTE)
        s.p2 |= (uint32_t)PORT_ReadInputData(MDR_PORTE);
    if (MDR_RST_CLK->PER_CLOCK & RST_CLK_PCLK_PORTF)
        s.p2 |= ((uint32_t)PORT_ReadInputData(MDR_PORTF) << 16);
    return s;
}

#else
hal_gpio_snapshot_t hal_gpio_snapshot(void);
void hal_gpio_put(pin_t gpio, bool value);
bool hal_gpio_get(pin_t gpio);
#endif

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
