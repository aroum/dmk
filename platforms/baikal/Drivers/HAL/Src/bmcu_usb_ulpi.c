/**
 * *****************************************************************************
 *  @file       bmcu_usb_ulpi.c
 *  @author     Baikal electronics SDK team
 *  @brief      UTMI+ Low Pin Interface (ULPI) module driver 
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

#include "bmcu_usb_ulpi.h"
#include "bmcu_def.h"

/**
 * @brief Reads a USB ULPI interface register.
 * @param Addr The register address.
 * @returns The register value.
 */
uint8_t USB_ULPI_ReadReg(uint8_t Addr)
{
    uint8_t val;

    USB->CTRL.ULPI.ULPIREGADDR = Addr;
    USB->CTRL.ULPI.ULPIREGCONTROL &= ~USB_ULPIREGCONTROL_ULPIREGCMPLT;
    USB->CTRL.ULPI.ULPIREGCONTROL |= (USB_ULPIREGCONTROL_ULPIRDNWR | USB_ULPIREGCONTROL_ULPIREGREQ);
    while ((USB->CTRL.ULPI.ULPIREGCONTROL & USB_ULPIREGCONTROL_ULPIREGCMPLT) != USB_ULPIREGCONTROL_ULPIREGCMPLT);
    val = USB->CTRL.ULPI.ULPIREGDATA;
    USB->CTRL.ULPI.ULPIREGCONTROL &= ~USB_ULPIREGCONTROL_ULPIREGCMPLT;

    return val;
}

/**
 * @brief Writes a USB ULPI interface register.
 * @param Addr The register address.
 * @param Value The register value.
 */
void USB_ULPI_WriteReg(uint8_t Addr, uint8_t Value)
{
    USB->CTRL.ULPI.ULPIREGADDR = Addr;
    USB->CTRL.ULPI.ULPIREGDATA = Value;
    USB->CTRL.ULPI.ULPIREGCONTROL &= ~USB_ULPIREGCONTROL_ULPIREGCMPLT;
    USB->CTRL.ULPI.ULPIREGCONTROL &= ~USB_ULPIREGCONTROL_ULPIRDNWR;
    USB->CTRL.ULPI.ULPIREGCONTROL |= USB_ULPIREGCONTROL_ULPIREGREQ;
    while ((USB->CTRL.ULPI.ULPIREGCONTROL & USB_ULPIREGCONTROL_ULPIREGCMPLT) != USB_ULPIREGCONTROL_ULPIREGCMPLT);
    USB->CTRL.ULPI.ULPIREGCONTROL &= ~USB_ULPIREGCONTROL_ULPIREGCMPLT;
}
