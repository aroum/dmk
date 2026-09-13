#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>

#ifndef _MATRIX_H
#define _MATRIX_H

typedef struct matrix_event_s {
    uint8_t row;
    uint8_t col;
    uint8_t pressed : 1;
    uint8_t split : 1;
    uint8_t reserved : 6;
} matrix_event_t;

extern QueueHandle_t matrix_queue;

// Matrix scanning main task, to be executed by FreeRTOS scheduler periodically
void matrix_task(void *pvParameters);

bool matrix_is_pressed(unsigned char row, unsigned char col);

#endif // _MATRIX_H
