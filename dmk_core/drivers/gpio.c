#include "hal_gpio.h"

#if defined(MCU_rp2040) || defined(MCU_rp2350)
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

hal_gpio_snapshot_t hal_gpio_snapshot(void) {
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

void hal_gpio_pull_down(pin_t gpio) {
    gpio_pull_down(gpio);
}

void hal_gpio_pull_up(pin_t gpio) {
    gpio_pull_up(gpio);
}

void hal_sleep_us(uint32_t us) {
    sleep_us(us);
}

void platform_init(void) {}

#elif defined(MCU_nrf52840)
#include "nrf_delay.h"
#include "nrf_gpio.h"

void hal_gpio_init(pin_t gpio) {
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

hal_gpio_snapshot_t hal_gpio_snapshot(void) {
    hal_gpio_snapshot_t s = {0};
    s.p0 = NRF_P0->IN;
#if defined(NRF_P1)
    s.p1 = NRF_P1->IN;
#endif
    return s;
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

void platform_init(void) {}

#elif defined(MCU_baikal)
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

hal_gpio_snapshot_t hal_gpio_snapshot(void) {
    hal_gpio_snapshot_t s = {0};
    s.p0 = (uint32_t)GPIO_ReadInputPort(GPIO0) | ((uint32_t)GPIO_ReadInputPort(GPIO1) << 16);
    s.p1 = (uint32_t)GPIO_ReadInputPort(GPIO2);
    return s;
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

void platform_init(void) {}

#elif defined(MCU_milandr)
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "board_pins.h"
#include "clk.h"
#include <stddef.h>

static MDR_PORT_TypeDef *const ports[] = {MDR_PORTA, MDR_PORTB, MDR_PORTC, MDR_PORTD, MDR_PORTE, MDR_PORTF};
static const uint32_t pclks[] = {RST_CLK_PCLK_PORTA, RST_CLK_PCLK_PORTB, RST_CLK_PCLK_PORTC,
                                 RST_CLK_PCLK_PORTD, RST_CLK_PCLK_PORTE, RST_CLK_PCLK_PORTF};

static inline bool get_gpio_port(uint8_t gpio, MDR_PORT_TypeDef **port, uint16_t *pin, uint32_t *pclk) {
    uint8_t idx = gpio / 16;
    if (idx >= sizeof(ports) / sizeof(ports[0]))
        return false;
    if (port)
        *port = ports[idx];
    if (pin)
        *pin = (1 << (gpio & 0x0F));
    if (pclk)
        *pclk = pclks[idx];
    return true;
}

void hal_gpio_init(uint8_t gpio) {
    uint32_t pclk;
    if (get_gpio_port(gpio, NULL, NULL, &pclk))
        RST_CLK_PCLKcmd(pclk, ENABLE);
}

static void configure_port(uint8_t gpio, uint8_t oe, uint8_t pull_up, uint8_t pull_down) {
    MDR_PORT_TypeDef *port;
    uint16_t pin;
    if (!get_gpio_port(gpio, &port, &pin, NULL))
        return;
    PORT_InitTypeDef init;
    PORT_StructInit(&init);
    init.PORT_Pin = pin;
    init.PORT_OE = oe;
    init.PORT_FUNC = PORT_FUNC_PORT;
    init.PORT_MODE = PORT_MODE_DIGITAL;
    init.PORT_SPEED = PORT_SPEED_SLOW;
    init.PORT_PULL_UP = pull_up;
    init.PORT_PULL_DOWN = pull_down;
    PORT_Init(port, &init);
}

void hal_gpio_set_dir(uint8_t gpio, bool is_output) {
    configure_port(gpio, is_output ? PORT_OE_OUT : PORT_OE_IN, PORT_PULL_UP_OFF, PORT_PULL_DOWN_OFF);
}

void hal_gpio_put(uint8_t gpio, bool value) {
    MDR_PORT_TypeDef *port;
    uint16_t pin;
    if (!get_gpio_port(gpio, &port, &pin, NULL))
        return;
    if (value)
        PORT_SetBits(port, pin);
    else
        PORT_ResetBits(port, pin);
}

bool hal_gpio_get(uint8_t gpio) {
    MDR_PORT_TypeDef *port;
    uint16_t pin;
    if (!get_gpio_port(gpio, &port, &pin, NULL))
        return false;
    return PORT_ReadInputDataBit(port, pin) != RESET;
}

hal_gpio_snapshot_t hal_gpio_snapshot(void) {
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

void hal_gpio_pull_down(uint8_t gpio) {
    configure_port(gpio, PORT_OE_IN, PORT_PULL_UP_OFF, PORT_PULL_DOWN_ON);
}

void hal_gpio_pull_up(uint8_t gpio) {
    configure_port(gpio, PORT_OE_IN, PORT_PULL_UP_ON, PORT_PULL_DOWN_OFF);
}

void hal_sleep_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 8; i++) {
        __NOP();
    }
}

void platform_init(void) {
    CLK_Init_80_mhz();
}

#endif
