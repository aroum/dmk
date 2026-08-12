/**
 * *****************************************************************************
 *  @file       BSP/Components/led/led.h
 *  @author     Baikal electronics SDK team
 *  @brief      LED driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *  @details
 *  Board Support Package is intended to support the development board and assist
 *  in developing new software projects based on it.
 *  It contains board-specific definitions, constants and functions.
 *
 *  File content:
 *      - User LED initialization and control functions
 *
 *  Example: init the user LED, turn it on, toggle twice and deinit LED
 *          <--- application code
 *      bsp_led_init();
 *      bsp_led_on();
 *          <--- add delay if required
 *      bsp_led_toggle();
 *          <--- add delay if required
 *      bsp_led_toggle();
 *          <--- add delay if required
 *      bsp_led_deinit();
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __LED_H
#define __LED_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief   Initialize user LED
 * @details Configure CRU settings of the LED pin, enable the corresponding
 *          GPIO port clock and set the GPIO pin as input
 * @note    This function should be called before any other LED control
 *          functions
 * @param   None
 * @retval  None
 */
void bsp_led_init(void);

/**
 * @brief   Deinitialize user LED
 * @details Set default CRU and GPIO settings of the LED pin
 * @note    This function does not disable LED GPIO port clock since the
 *          same port may be used for another functionality.
 * @param   None
 * @retval  None
 */
void bsd_led_deinit(void);

/**
 * @brief   Turn the user LED on
 * @param   None
 * @retval  None
 */
void bsp_led_on(void);

/**
 * @brief   Turn the user LED off
 * @param   None
 * @retval  None
 */
void bsp_led_off(void);

/**
 * @brief   Toggle the user LED state
 * @param   None
 * @retval  None
 */
void bsp_led_toggle(void);

#endif /* __LED_H */
