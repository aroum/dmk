/**
 * @file    board_pins.h
 * @brief   Maps BOARD_* pin identifiers from config.h to SPL symbols.
 */

#ifndef USB_EXAMPLES_BOARD_PINS_H
#define USB_EXAMPLES_BOARD_PINS_H

#include "config.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"

/* Pin registry — add a triplet for each PxN used in config.h */
#define PB6_PORT MDR_PORTB
#define PB6_PIN  PORT_Pin_6
#define PB6_PCLK RST_CLK_PCLK_PORTB

#define PB7_PORT MDR_PORTB
#define PB7_PIN  PORT_Pin_7
#define PB7_PCLK RST_CLK_PCLK_PORTB

#define PE0_PORT         MDR_PORTE
#define PE0_PIN          PORT_Pin_0
#define PE0_PCLK         RST_CLK_PCLK_PORTE
#define PE0_DAC_CHANNEL  2

#define BOARD_GPIO_JOIN2(a, b) a##b
#define BOARD_GPIO_JOIN(a, b)  BOARD_GPIO_JOIN2(a, b)

#define BOARD_BUTTON_PORT BOARD_GPIO_JOIN(BOARD_BUTTON, _PORT)
#define BOARD_BUTTON_BIT  BOARD_GPIO_JOIN(BOARD_BUTTON, _PIN)
#define BOARD_BUTTON_PCLK BOARD_GPIO_JOIN(BOARD_BUTTON, _PCLK)

#define BOARD_LED_PORT BOARD_GPIO_JOIN(BOARD_LED, _PORT)
#define BOARD_LED_BIT  BOARD_GPIO_JOIN(BOARD_LED, _PIN)
#define BOARD_LED_PCLK BOARD_GPIO_JOIN(BOARD_LED, _PCLK)

#define BOARD_DAC_PORT     BOARD_GPIO_JOIN(BOARD_DAC, _PORT)
#define BOARD_DAC_BIT      BOARD_GPIO_JOIN(BOARD_DAC, _PIN)
#define BOARD_DAC_PCLK     BOARD_GPIO_JOIN(BOARD_DAC, _PCLK)
#define BOARD_DAC_CHANNEL  BOARD_GPIO_JOIN(BOARD_DAC, _DAC_CHANNEL)

#endif /* USB_EXAMPLES_BOARD_PINS_H */
