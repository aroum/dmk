/*
 * Minimal SDK configuration for nRF52 USB HID Keyboard (USBD) support.
 * This file enables the required USB Device stack and HID keyboard class
 * in the nRF5 SDK. It should be placed in the nRF52 platform include directory
 * and is automatically included via the platform include path.
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

/* Enable the core USB device driver */
#define USBD_ENABLED 1

/* Enable the application-level USB device library */
#define APP_USBD_ENABLED 1

/* Enable the HID class library */
#define APP_USBD_CLASS_HID_ENABLED 1

/* Enable the HID keyboard implementation */
#define APP_USBD_HID_KBD_ENABLED 1

/* Power management settings */
#define USBD_POWER_DETECTION 1
#define APP_USBD_CONFIG_POWER_EVENTS_PROCESS 0

/* Enable logging over USB (optional) */
#define NRF_LOG_BACKEND_USB_ENABLED 0

/* Clock configuration required for USB (ensure HFCLK is enabled) */
#define NRF_CLOCK_ENABLED 1
#define NRF_CLOCK_CONFIG_LF_SRC 1

/* Additional required drivers */
#define NRF_DRV_USBD_ENABLED 1

/* Disable unused USB classes to reduce binary size */
#define APP_USBD_CLASS_CDC_ACM_ENABLED 0
#define APP_USBD_CLASS_MSC_ENABLED 0
#define APP_USBD_CLASS_HID_MOUSE_ENABLED 0

#endif // SDK_CONFIG_H
