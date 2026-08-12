/**
 * *****************************************************************************
 *  @file       BSP/Components/serial/serial.h
 *  @author     Baikal electronics SDK team
 *  @brief      Serial interface driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 *  @details
 *  Board Support Package is intended to support the development board and assist
 *  in developing new software projects based on it.
 *  It contains board-specific definitions, constants and functions.
 *
 *  File content:
 *      - Serial interface initialization and control functions
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __SERIAL_H
#define __SERIAL_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief   Initialize serial interface
 * @param   None
 * @retval  None
 */
void bsp_serial_init(void);

/**
 * @brief   Deinitialize serial interface
 * @param   None
 * @retval  None
 */
void bsp_serial_deinit(void);

/**
 * @brief   Writes a character to the serial interface
 * @note    This is a blocking function
 * @param   ch The character to be written
 * @retval  The written character
 */
int bsp_serial_putchar(int ch);

/**
 * @brief   Reads a character from the serial interface
 * @note    This is a blocking function
 * @param   None
 * @retval  The obtained character
 */
int bsp_serial_getchar(void);

#endif /* __SERIAL_H */
