/**
 * @file    config.h
 * @brief   Board configuration for DMK firmware.
 */

#ifndef DMK_CONFIG_H
#define DMK_CONFIG_H

#include <stdint.h>

/* External HSE crystal frequency [Hz] */
#ifndef HSE_Value
#define HSE_Value ((uint32_t)8000000)
#endif

/*
 * GPIO pins in PxN form (e.g. PB6 = port B, pin 6).
 * See board_pins.h for PORT/Pin/PCLK expansion.
 */
#define BOARD_BUTTON PB6
#define BOARD_LED PB7

#endif /* DMK_CONFIG_H */
