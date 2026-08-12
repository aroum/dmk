/**
 * *****************************************************************************
 *  @file       BSP/Components/button/button.h
 *  @author     Baikal electronics SDK team
 *  @brief      User button driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *  @details
 *  Board Support Package is intended to support the development board and assist
 *  in developing new software projects based on it.
 *  It contains board-specific definitions, constants and functions.
 *
 *  File content:
 *      - User button related functions (init, deinit, get the state)
 *
 *  Example: init the user button, save it's state to the boolean variable,
 *             deinit user button
 *          <--- application code
 *      bsp_btn_init();
 *          <--- application code
 *      bool var = bsp_btn_ispressed();
 *          <--- application code
 *      bsp_btn_deinit();
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BUTTON_H
#define __BUTTON_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief   Initialize the user button
 * @details Configure CRU settings of the user button pin, enable the
 *          corresponding GPIO port clock and set the GPIO pin as input
 * @param   None
 * @retval  None
 */
void bsp_btn_init(void);

/**
 * @brief   Deinitialize the user button
 * @details Set default CRU and GPIO settings of the user button pin
 * @param   None
 * @retval  None
 */
void bsp_btn_deinit(void);

/**
 * @brief   Get user button state
 * @param   None
 * @retval  1 if the button is pressed, otherwise 0
 */
uint32_t bsp_btn_ispressed(void);

#endif /* __BUTTON_H */
