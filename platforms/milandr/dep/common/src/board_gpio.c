#include "board_gpio.h"
#include "board_pins.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"

void Board_GPIO_Init(void)
{
  PORT_InitTypeDef PORT_InitStructure;

  RST_CLK_PCLKcmd(BOARD_BUTTON_PCLK | BOARD_LED_PCLK, ENABLE);

  PORT_StructInit(&PORT_InitStructure);
  PORT_InitStructure.PORT_Pin = BOARD_LED_BIT;
  PORT_InitStructure.PORT_OE = PORT_OE_OUT;
  PORT_InitStructure.PORT_FUNC = PORT_FUNC_PORT;
  PORT_InitStructure.PORT_MODE = PORT_MODE_DIGITAL;
  PORT_InitStructure.PORT_SPEED = PORT_SPEED_SLOW;
  PORT_Init(BOARD_LED_PORT, &PORT_InitStructure);

  PORT_InitStructure.PORT_Pin = BOARD_BUTTON_BIT;
  PORT_InitStructure.PORT_OE = PORT_OE_IN;
  PORT_Init(BOARD_BUTTON_PORT, &PORT_InitStructure);
}

bool Board_ButtonPressed(void)
{
  return PORT_ReadInputDataBit(BOARD_BUTTON_PORT, BOARD_BUTTON_BIT) == RESET;
}

void Board_LED_On(void)
{
  PORT_SetBits(BOARD_LED_PORT, BOARD_LED_BIT);
}

void Board_LED_Off(void)
{
  PORT_ResetBits(BOARD_LED_PORT, BOARD_LED_BIT);
}
