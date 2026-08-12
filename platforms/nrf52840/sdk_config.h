/*
 * Minimal SDK configuration for nRF52 USB HID Keyboard (USBD) support.
 * This file enables the required USB Device stack and HID keyboard class
 * in the nRF5 SDK. It should be placed in the nRF52 platform directory and
 * is automatically included via the platform include path.
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

#include "config.h"

/* Enable the core USB device driver */
#define USBD_ENABLED 1

/* Enable the application-level USB device library */
#define APP_USBD_ENABLED 1

/* Enable the HID class library */
#define APP_USBD_CLASS_HID_ENABLED 1
#define APP_USBD_HID_ENABLED 1

/* Enable the HID keyboard implementation */
#define APP_USBD_HID_KBD_ENABLED 1

/* Enable the HID generic implementation for Vial */
#ifdef VIAL
#define APP_USBD_HID_GENERIC_ENABLED 1
#endif

/* Power management settings */
#define USBD_POWER_DETECTION 1
#define APP_USBD_CONFIG_POWER_EVENTS_PROCESS 1

/* Enable logging over USB (optional) */
#define NRF_LOG_BACKEND_USB_ENABLED 0

/* Clock configuration required for USB (ensure HFCLK is enabled) */
#define NRF_CLOCK_ENABLED 1
#define NRF_CLOCK_CONFIG_LF_SRC 1
#define CLOCK_CONFIG_LF_SRC 1
#define NRFX_CLOCK_CONFIG_LF_SRC 1

/* Additional required drivers */
#define NRF_DRV_USBD_ENABLED 1

/* nrfx USBD driver must be explicitly enabled */
#define NRFX_USBD_ENABLED 1

/* nRF power driver (legacy wrapper) */
#define NRF_DRV_POWER_ENABLED 1
#define POWER_ENABLED 1
/* DC/DC converter: 0=disabled, 1=enabled */
#define NRFX_POWER_CONFIG_DEFAULT_DCDCEN 0
/* DC/DC converter HV stage (nRF52840 USB requires this) */
#define NRFX_POWER_CONFIG_DEFAULT_DCDCENHV 0

/* Disable unused USB classes to reduce binary size */
#define APP_USBD_CLASS_CDC_ACM_ENABLED 0
#define APP_USBD_CLASS_MSC_ENABLED 0
#define APP_USBD_CLASS_HID_MOUSE_ENABLED 0

/* nrfx driver IRQ priorities (must be numerically >= configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8-__NVIC_PRIO_BITS)) */
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY 6
#define NRFX_POWER_CONFIG_IRQ_PRIORITY 6
#define NRFX_USBD_CONFIG_IRQ_PRIORITY 6
#define NRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST 1

/* Enable nrfx clock and power drivers needed by USB */
#define NRFX_CLOCK_ENABLED 1
#define NRFX_POWER_ENABLED 1
#define NRFX_POWER_CLOCK_CONFIG_IRQ_PRIORITY 6

/* Enable the nRF queue library */
#define NRF_QUEUE_ENABLED 1

/* app_usbd event queue – required for app_usbd_event_queue_process() */
#define APP_USBD_CONFIG_EVENT_QUEUE_ENABLE 1
#define APP_USBD_CONFIG_EVENT_QUEUE_SIZE 32
#define APP_USBD_CONFIG_SOF_HANDLING_MODE 1

/* HID idle report table size */
#define APP_USBD_HID_REPORT_IDLE_TABLE_SIZE 1
#define APP_USBD_HID_DEFAULT_IDLE_RATE 0

/* Enable app_scheduler needed by USB stack event processing */
#define APP_SCHEDULER_ENABLED 0

/* USB Device descriptor: VID, PID, device version */
#ifdef VIAL
#ifndef VIAL_VENDOR_ID
#define VIAL_VENDOR_ID 0xCafe
#endif
#ifndef VIAL_PRODUCT_ID
#define VIAL_PRODUCT_ID 0x4010
#endif
#define APP_USBD_VID VIAL_VENDOR_ID
#define APP_USBD_PID VIAL_PRODUCT_ID
#else
#define APP_USBD_VID 0x1915
#define APP_USBD_PID 0xEEEE
#endif
#define APP_USBD_DEVICE_VER_MAJOR 1
#define APP_USBD_DEVICE_VER_MINOR 0
#define APP_USBD_DEVICE_VER_SUB 0

/* USB Device string IDs */
#define APP_USBD_STRING_ID_MANUFACTURER 1
#define APP_USBD_STRING_ID_PRODUCT 2
#define APP_USBD_STRING_ID_SERIAL 3
#define APP_USBD_STRING_ID_CONFIGURATION 0

/* USB Device power configuration */
#define APP_USBD_CONFIG_SELF_POWERED 0
#define APP_USBD_CONFIG_MAX_POWER 100

/* USB Device SOF timestamp (requires NRF_LOG) */
#define APP_USBD_CONFIG_SOF_TIMESTAMP_PROVIDE 0

/* USB logging */
#define APP_USBD_CONFIG_LOG_ENABLED 0
#define APP_USBD_CONFIG_LOG_LEVEL 3

/* Logging – disabled */
#define NRF_LOG_ENABLED 0
#define NRF_LOG_DEFAULT_LEVEL 0
#define NRF_LOG_DEFERRED 0

/* nrfx USBD and POWER IRQ priorities */
#define USBD_CONFIG_IRQ_PRIORITY 6
#define POWER_CONFIG_IRQ_PRIORITY 6

/* nrfx clock LFCLK source: 0=RC, 1=XTAL, 2=SYNTH */
#define NRFX_CLOCK_CONFIG_LF_SRC 1

/* USB string descriptors – use the exact macro names the SDK expects */
#define APP_USBD_STRINGS_LANGIDS APP_USBD_LANG_AND_SUBLANG(APP_USBD_LANG_ENGLISH, APP_USBD_SUBLANG_ENGLISH_US)
#define APP_USBD_STRINGS_MANUFACTURER APP_USBD_STRING_DESC("DMK")
#define APP_USBD_STRINGS_PRODUCT APP_USBD_STRING_DESC("DMK Keyboard")
#ifdef VIAL
#define APP_USBD_STRING_SERIAL APP_USBD_STRING_DESC("vial:f64c2b3c")
#else
#define APP_USBD_STRING_SERIAL APP_USBD_STRING_DESC("000000000000")
#endif
#define APP_USBD_STRING_SERIAL_EXTERN 0
#define APP_USBD_STRINGS_CONFIGURATION APP_USBD_STRING_DESC("Default")
/* No user-defined USB strings */
#define APP_USBD_STRINGS_USER

/* Maximum USB string descriptor size */
#define APP_USBD_CONFIG_DESC_STRING_SIZE 31

/* Enable SPIM driver and SPIM1 instance */
#define NRFX_SPIM_ENABLED 1
#define NRFX_SPIM1_ENABLED 1
#define NRFX_SPIM_MISO_PULL_CFG 1
#define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY 7

#endif // SDK_CONFIG_H
