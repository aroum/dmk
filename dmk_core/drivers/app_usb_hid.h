/**
 * @file    app_usb_hid.h
 * @brief   USB HID Keyboard driver definitions and API.
 */

#ifndef __APP_USB_HID_H
#define __APP_USB_HID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#if defined(MCU_milandr)
#include "MDR32FxQI_config.h"
#include "MDR32FxQI_usb_device.h"
#else
typedef enum { USB_SUCCESS = 0, USB_ERROR, USB_ERR_BUSY, USB_ERR_INV_REQ } USB_Result;
#endif

/* HID Descriptor Types */
#define USB_HID_DESCRIPTOR_HID 0x21
#define USB_HID_DESCRIPTOR_REPORT 0x22

/* HID Class Requests */
#define USB_HID_GET_REPORT 0x01
#define USB_HID_GET_IDLE 0x02
#define USB_HID_GET_PROTOCOL 0x03
#define USB_HID_SET_REPORT 0x09
#define USB_HID_SET_IDLE 0x0A
#define USB_HID_SET_PROTOCOL 0x0B

/* Keyboard Modifier Bitmasks */
#define KEYBOARD_MODIFIER_LEFTCTRL (1 << 0)
#define KEYBOARD_MODIFIER_LEFTSHIFT (1 << 1)
#define KEYBOARD_MODIFIER_LEFTALT (1 << 2)
#define KEYBOARD_MODIFIER_LEFTGUI (1 << 3)
#define KEYBOARD_MODIFIER_RIGHTCTRL (1 << 4)
#define KEYBOARD_MODIFIER_RIGHTSHIFT (1 << 5)
#define KEYBOARD_MODIFIER_RIGHTALT (1 << 6)
#define KEYBOARD_MODIFIER_RIGHTGUI (1 << 7)

/* Common Keyboard Usages */
#define KEY_F 0x09

/* USB Endpoint Assignments for HID */
#define USB_HID_EP_SEND USB_EP1

#pragma pack(push, 1)
/**
 * @brief Standard HID keyboard input report.
 */
typedef struct {
#ifdef EXTRAKEY_ENABLE
    uint8_t ReportID;
#endif
    uint8_t Modifier;    /* Modifier keys bitmask */
    uint8_t Reserved;    /* Reserved byte, must be 0 */
    uint8_t Keycodes[6]; /* Up to 6 concurrent keycodes */
} USB_HID_KeyboardReport_TypeDef;
#pragma pack(pop)

/**
 * @brief  Initializes the USB HID Keyboard.
 * @retval USB_Result.
 */
USB_Result USB_HID_Init(void);

/**
 * @brief  Sends a keyboard input report to the host via Interrupt EP1.
 * @param  report: Pointer to the populated report structure.
 * @retval USB_Result.
 */
USB_Result USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *report);

/**
 * @brief  Sends a consumer report to the host.
 * @param  usage: Consumer Usage Page usage ID.
 * @retval USB_Result.
 */
USB_Result USB_HID_SendConsumerReport(uint16_t usage);

/**
 * @brief  USB device reset handler to reconfigure HID endpoints.
 * @retval USB_Result.
 */
USB_Result USB_HID_Reset(void);

/**
 * @brief  Standard request GET_DESCRIPTOR handler for HID specific descriptors.
 * @param  wVALUE: Descriptor type (high byte) and index (low byte).
 * @param  wINDEX: Language ID or Interface number.
 * @param  wLENGTH: Maximum number of bytes requested by host.
 * @retval USB_Result.
 */
USB_Result USB_HID_GetDescriptor(uint16_t wVALUE, uint16_t wINDEX, uint16_t wLENGTH);

/**
 * @brief  Class request handler to process HID requests (GET/SET REPORT/IDLE/PROTOCOL).
 * @retval USB_Result.
 */
USB_Result USB_HID_ClassRequest(void);

#if defined(MCU_milandr)
/**
 * @brief  Set configuration handler to update mounting status.
 * @param  wValue: Configuration value.
 * @retval USB_Result.
 */
USB_Result USB_HID_SetConfiguration(uint16_t wValue);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APP_USB_HID_H */
