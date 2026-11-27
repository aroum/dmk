#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_TUSB_MCU OPT_MCU_STM32L0
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE
#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_ENDPPOINT_MAX 8

#if defined(DEBUG_CONSOLE_ENABLE) && defined(VIAL)
#define CFG_TUD_HID (3 + CFG_TUD_HID_PFU)
#elif defined(VIAL)
#define CFG_TUD_HID (2 + CFG_TUD_HID_PFU)
#else
#define CFG_TUD_HID (1 + CFG_TUD_HID_PFU)
#endif
#if defined(HHKB_HYBRID) && HHKB_PFU_CHANNEL
/* HHKB Hybrid: one more HID interface, the stock vendor channel (keyboards/hhkb/modules/pfu_channel). */
#define CFG_TUD_HID_PFU 1
#else
#define CFG_TUD_HID_PFU 0
#endif
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

#define CFG_TUD_HID_EP_BUFSIZE 64

#ifdef __cplusplus
}
#endif

#endif // TUSB_CONFIG_H_
