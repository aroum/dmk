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

/**
 * @brief Main matrix scanning task executed periodically by FreeRTOS scheduler.
 * @param pvParameters FreeRTOS task parameter pointer (unused)
 */
void matrix_task(void *pvParameters);

/**
 * @brief Query current debounced press state of a matrix switch coordinate.
 * @param row Matrix row index
 * @param col Matrix col index
 * @return true if switch is currently pressed, false otherwise
 */
bool matrix_is_pressed(unsigned char row, unsigned char col);

#endif // _MATRIX_H
