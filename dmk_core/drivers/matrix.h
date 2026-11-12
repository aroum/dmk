#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>

#ifndef _MATRIX_H
#define _MATRIX_H

typedef struct matrix_event_s {
    uint8_t row;
    uint8_t col;
    union {
        struct {
            uint8_t pressed : 1;
            uint8_t split : 1;
            uint8_t batched : 1;
            uint8_t batch_start : 1;
            uint8_t reserved : 4;
        };
        uint8_t row_state_hi;
    };
} matrix_event_t;

#define MATRIX_EVENT_ROW_SNAPSHOT 0x80U
#define MATRIX_EVENT_SCAN_START   0x40U
#define MATRIX_EVENT_ROW_MASK     0x3fU

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
