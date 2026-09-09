#include "hal_gpio.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "board_pins.h"
#include "clk.h"
#include <stddef.h>

static MDR_PORT_TypeDef * const ports[] = { MDR_PORTA, MDR_PORTB, MDR_PORTC, MDR_PORTD, MDR_PORTE, MDR_PORTF };
static const uint32_t pclks[] = { RST_CLK_PCLK_PORTA, RST_CLK_PCLK_PORTB, RST_CLK_PCLK_PORTC, RST_CLK_PCLK_PORTD, RST_CLK_PCLK_PORTE, RST_CLK_PCLK_PORTF };

static inline bool get_gpio_port(uint8_t gpio, MDR_PORT_TypeDef **port, uint16_t *pin, uint32_t *pclk) {
    uint8_t idx = gpio / 16;
    if (idx >= sizeof(ports) / sizeof(ports[0])) return false;
    if (port) *port = ports[idx];
    if (pin) *pin = (1 << (gpio & 0x0F));
    if (pclk) *pclk = pclks[idx];
    return true;
}

void hal_gpio_init(uint8_t gpio) {
    uint32_t pclk;
    if (get_gpio_port(gpio, NULL, NULL, &pclk)) RST_CLK_PCLKcmd(pclk, ENABLE);
}

static void configure_port(uint8_t gpio, uint8_t oe, uint8_t pull_up, uint8_t pull_down) {
    MDR_PORT_TypeDef *port;
    uint16_t pin;
    if (!get_gpio_port(gpio, &port, &pin, NULL)) return;
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
    if (!get_gpio_port(gpio, &port, &pin, NULL)) return;
    if (value) PORT_SetBits(port, pin); else PORT_ResetBits(port, pin);
}

bool hal_gpio_get(uint8_t gpio) {
    MDR_PORT_TypeDef *port;
    uint16_t pin;
    if (!get_gpio_port(gpio, &port, &pin, NULL)) return false;
    return PORT_ReadInputDataBit(port, pin) != RESET;
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
