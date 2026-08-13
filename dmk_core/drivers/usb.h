#ifndef _USB_H
#define _USB_H

#include "FreeRTOS.h"
#include "queue.h"

#include <stdint.h>

#define KEY_CONSUMER_FLAG 0x8000

typedef struct {
    uint16_t keycode;
    uint8_t pressed;
} key_event_t;

extern QueueHandle_t usb_queue;

// USB HID output main task, to be executed by FreeRTOS scheduler
void usb_task(void *pvParameters);

#endif // _USB_H
