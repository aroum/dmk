#ifndef _USB_H
#define _USB_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_CONSUMER_FLAG 0x8000

typedef struct {
    uint16_t keycode;
    uint8_t pressed;
} key_event_t;

// Initialize USB hardware and endpoints
void usb_init(void);

// Zero-Queue Fast Path: updates HID report and transmits to host directly
void usb_process_key(uint16_t keycode, bool pressed);

#ifdef __cplusplus
}
#endif

#endif // _USB_H
