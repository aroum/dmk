/**
 * @file    app_usb_hid.h
 * @brief   USB HID Keyboard driver definitions and API.
 */

#ifndef __APP_USB_HID_H
#define __APP_USB_HID_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
/**
 * @brief Standard HID keyboard input report.
 */
typedef struct {
    uint8_t ReportID;    /* Report ID: always 1 */
    uint8_t Modifier;    /* Modifier keys bitmask */
    uint8_t Reserved;    /* Reserved byte, must be 0 */
    uint8_t Keycodes[6]; /* Up to 6 concurrent keycodes */
} USB_HID_KeyboardReport_TypeDef;
#pragma pack(pop)

/**
 * @brief  Initializes the USB HID stack.
 * @retval true on success, false otherwise.
 */
bool USB_HID_Init(void);

/**
 * @brief  Sends a keyboard input report to the host.
 * @param  report: Pointer to the populated report structure.
 * @retval true on success, false otherwise.
 */
bool USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report);

/**
 * @brief  Sends a consumer report to the host.
 * @param  usage: Consumer Usage Page usage ID.
 * @retval true on success, false otherwise.
 */
bool USB_HID_SendConsumerReport(uint16_t usage);

/**
 * @brief  Sends a mouse report to the host.
 * @param  buttons: Bitmask of pressed buttons (bit 0 = Left, 1 = Right, 2 = Middle, 3 = Back, 4 = Forward).
 * @param  x: Relative X movement (-127 to 127).
 * @param  y: Relative Y movement (-127 to 127).
 * @param  wheel: Vertical scroll wheel (-127 to 127).
 * @param  pan: Horizontal pan scroll (-127 to 127).
 * @retval true on success, false otherwise.
 */
bool USB_HID_SendMouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan);

/**
 * @brief  Sends a standard DirectInput gamepad report to the host.
 * @param  x: Left stick X axis (-127 to 127).
 * @param  y: Left stick Y axis (-127 to 127).
 * @param  z: Right stick X / Z axis (-127 to 127).
 * @param  rz: Right stick Y / Rz axis (-127 to 127).
 * @param  rx: Left trigger / Rx axis (-127 to 127).
 * @param  ry: Right trigger / Ry axis (-127 to 127).
 * @param  hat: 8-way hat switch / D-Pad (0 = center, 1 = Up, ... 8 = Up-Left).
 * @param  buttons: 32-bit bitmask of buttons.
 * @retval true on success, false otherwise.
 */
bool USB_HID_SendGamepadReport(int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat,
                               uint32_t buttons);

#ifdef __cplusplus
}
#endif

#endif /* __APP_USB_HID_H */
