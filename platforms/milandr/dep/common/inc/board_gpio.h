#pragma once

#include <stdbool.h>

void Board_GPIO_Init(void);

bool Board_ButtonPressed(void);
void Board_LED_On(void);
void Board_LED_Off(void);
