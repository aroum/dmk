/**
 * *****************************************************************************
 *  @file       BSP/EVU_LI_2_1/bsp.h
 *  @author     Baikal electronics SDK team
 *  @brief      EVU_LI_2_1 BSP header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *  @details
 *  Board Support Package is intended to support the EVU-LI-2.1 board and assist
 *  in developing new software projects based on it.
 *  It contains board-specific definitions, constants and functions.
 *
 *  File content:
 *      - EVU-LI-2.1 board related constants and definitions
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BSP_H
#define __BSP_H

#include "bmcu_cru.h"
#include "bmcu_gpio.h"

/******************************************************************************/
/*                                    LED                                     */
/******************************************************************************/
#define BSP_LED_GPIO_PORT (GPIO0)
#define BSP_LED_GPIO_PIN (GPIO_PIN_15)
#define BSP_LED_CRU_PORT (CRU_PORT_A)
#define BSP_LED_CRU_PIN (CRU_PIN_15)

#include "../Components/led/led.h"

/******************************************************************************/
/*                                   Serial                                   */
/******************************************************************************/
#define BSP_SERIAL_INSTANCE (UART0)
#define BSP_SERIAL_CLK_EN_FN (CRU_APB0_EnableClock)
#define BSP_SERIAL_CLK_PERIPH (CRU_APB0_PERIPH_UART0)
#define BSP_SERIAL_GPIO_CLK_EN_FN (CRU_APB0_EnableClock)
#define BSP_SERIAL_GPIO_CLK_PERIPH (CRU_APB0_PERIPH_GPIO0)
#define BSP_SERIAL_CRU_PIN_AF (CRU_PIN_AF_1)
#define BSP_SERIAL_TX_CRU_PORT (CRU_PORT_A)
#define BSP_SERIAL_TX_CRU_PIN (CRU_PIN_6)
#define BSP_SERIAL_RX_CRU_PORT (CRU_PORT_A)
#define BSP_SERIAL_RX_CRU_PIN (CRU_PIN_7)
#define BSP_SERIAL_CLIC_IRQN (CLIC_UART0_IRQn)
#define BSP_SERIAL_ISR (UART0_IRQHandler)

#include "../Components/serial/serial.h"

/******************************************************************************/
/*                                    UART                                    */
/******************************************************************************/
#define BSP_UART_INSTANCE (UART0)
#define BSP_UART_CLK_EN_FN (CRU_APB0_EnableClock)
#define BSP_UART_CLK_PERIPH (CRU_APB0_PERIPH_UART0)
#define BSP_UART_GPIO_CLK_EN_FN (CRU_APB0_EnableClock)
#define BSP_UART_GPIO_CLK_PERIPH (CRU_APB0_PERIPH_GPIO0)
#define BSP_UART_CRU_PIN_AF (CRU_PIN_AF_1)
#define BSP_UART_TX_CRU_PORT (CRU_PORT_A)
#define BSP_UART_TX_CRU_PIN (CRU_PIN_6)
#define BSP_UART_RX_CRU_PORT (CRU_PORT_A)
#define BSP_UART_RX_CRU_PIN (CRU_PIN_7)
#define BSP_UART_CLIC_IRQN (CLIC_UART0_IRQn)
#define BSP_UART_ISR (UART0_IRQHandler)

#endif /* __BSP_H */
