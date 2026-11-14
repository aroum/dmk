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
#define CFG_TUD_HID 3
#elif defined(VIAL)
#define CFG_TUD_HID 2
#else
#define CFG_TUD_HID 1
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
