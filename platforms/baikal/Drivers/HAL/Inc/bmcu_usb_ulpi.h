/**
 * *****************************************************************************
 *  @file       bmcu_usb_ulpi.h
 *  @author     Baikal electronics SDK team
 *  @brief      UTMI+ Low Pin Interface (ULPI) module driver header file
 *  @version    2.2.0
 *  @date       2026.04.02
 * *****************************************************************************
 *  @copyright Copyright (c) 2025 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#ifndef __BMCU_USB_ULPI_H
#define __BMCU_USB_ULPI_H

#include "bmcu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Reads a USB ULPI interface register. */
uint8_t USB_ULPI_ReadReg(uint8_t Addr);
/** @brief Writes a USB ULPI interface register. */
void USB_ULPI_WriteReg(uint8_t Addr, uint8_t Value);

#ifdef __cplusplus
}
#endif

#endif /* __BMCU_USB_ULPI_H */
