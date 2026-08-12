#include "hal_gpio.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"
#include "board_pins.h"
#include "clk.h"

typedef struct {
    MDR_PORT_TypeDef *port;
    uint16_t pin;
    uint32_t pclk;
} gpio_map_t;

static gpio_map_t gpio_table[] = {
    // Port A (0-15)
    {MDR_PORTA, PORT_Pin_0, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_1, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_2, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_3, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_4, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_5, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_6, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_7, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_8, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_9, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_10, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_11, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_12, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_13, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_14, RST_CLK_PCLK_PORTA},
    {MDR_PORTA, PORT_Pin_15, RST_CLK_PCLK_PORTA},
    // Port B (16-31)
    {MDR_PORTB, PORT_Pin_0, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_1, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_2, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_3, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_4, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_5, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_6, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_7, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_8, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_9, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_10, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_11, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_12, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_13, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_14, RST_CLK_PCLK_PORTB},
    {MDR_PORTB, PORT_Pin_15, RST_CLK_PCLK_PORTB},
    // Port C (32-47)
    {MDR_PORTC, PORT_Pin_0, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_1, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_2, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_3, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_4, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_5, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_6, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_7, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_8, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_9, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_10, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_11, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_12, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_13, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_14, RST_CLK_PCLK_PORTC},
    {MDR_PORTC, PORT_Pin_15, RST_CLK_PCLK_PORTC},
    // Port D (48-63)
    {MDR_PORTD, PORT_Pin_0, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_1, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_2, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_3, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_4, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_5, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_6, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_7, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_8, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_9, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_10, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_11, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_12, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_13, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_14, RST_CLK_PCLK_PORTD},
    {MDR_PORTD, PORT_Pin_15, RST_CLK_PCLK_PORTD},
    // Port E (64-79)
    {MDR_PORTE, PORT_Pin_0, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_1, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_2, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_3, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_4, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_5, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_6, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_7, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_8, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_9, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_10, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_11, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_12, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_13, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_14, RST_CLK_PCLK_PORTE},
    {MDR_PORTE, PORT_Pin_15, RST_CLK_PCLK_PORTE},
    // Port F (80-95)
    {MDR_PORTF, PORT_Pin_0, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_1, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_2, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_3, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_4, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_5, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_6, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_7, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_8, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_9, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_10, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_11, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_12, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_13, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_14, RST_CLK_PCLK_PORTF},
    {MDR_PORTF, PORT_Pin_15, RST_CLK_PCLK_PORTF},
};

void hal_gpio_init(uint8_t gpio) {
    if (gpio >= sizeof(gpio_table) / sizeof(gpio_table[0]))
        return;

    gpio_map_t *g = &gpio_table[gpio];
    RST_CLK_PCLKcmd(g->pclk, ENABLE);
}

void hal_gpio_set_dir(uint8_t gpio, bool is_output) {
    if (gpio >= sizeof(gpio_table) / sizeof(gpio_table[0]))
        return;

    gpio_map_t *g = &gpio_table[gpio];
    PORT_InitTypeDef PORT_InitStructure;
    PORT_StructInit(&PORT_InitStructure);
    PORT_InitStructure.PORT_Pin = g->pin;
    PORT_InitStructure.PORT_OE = is_output ? PORT_OE_OUT : PORT_OE_IN;
    PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
    PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
    PORT_InitStructure.PORT_SPEED = PORT_SPEED_SLOW;
    PORT_Init(g->port, &PORT_InitStructure);
}

void hal_gpio_put(uint8_t gpio, bool value) {
    if (gpio >= sizeof(gpio_table) / sizeof(gpio_table[0]))
        return;

    gpio_map_t *g = &gpio_table[gpio];
    if (value) {
        PORT_SetBits(g->port, g->pin);
    } else {
        PORT_ResetBits(g->port, g->pin);
    }
}

bool hal_gpio_get(uint8_t gpio) {
    if (gpio >= sizeof(gpio_table) / sizeof(gpio_table[0]))
        return false;

    gpio_map_t *g = &gpio_table[gpio];
    return PORT_ReadInputDataBit(g->port, g->pin) != RESET;
}

void hal_gpio_pull_down(uint8_t gpio) {
    if (gpio >= sizeof(gpio_table) / sizeof(gpio_table[0]))
        return;

    gpio_map_t *g = &gpio_table[gpio];
    PORT_InitTypeDef PORT_InitStructure;
    PORT_StructInit(&PORT_InitStructure);
    PORT_InitStructure.PORT_Pin = g->pin;
    PORT_InitStructure.PORT_OE = PORT_OE_IN;
    PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
    PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
    PORT_InitStructure.PORT_SPEED = PORT_SPEED_SLOW;
    PORT_InitStructure.PORT_PULL_DOWN = PORT_PULL_DOWN_ON;
    PORT_Init(g->port, &PORT_InitStructure);
}

void hal_gpio_pull_up(uint8_t gpio) {
    if (gpio >= sizeof(gpio_table) / sizeof(gpio_table[0]))
        return;

    gpio_map_t *g = &gpio_table[gpio];
    PORT_InitTypeDef PORT_InitStructure;
    PORT_StructInit(&PORT_InitStructure);
    PORT_InitStructure.PORT_Pin = g->pin;
    PORT_InitStructure.PORT_OE = PORT_OE_IN;
    PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
    PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
    PORT_InitStructure.PORT_SPEED = PORT_SPEED_SLOW;
    PORT_InitStructure.PORT_PULL_UP = PORT_PULL_UP_ON;
    PORT_Init(g->port, &PORT_InitStructure);
}

void hal_sleep_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 8; i++) {
        __NOP();
    }
}

void platform_init(void) {
    CLK_Init_80_mhz();
}
