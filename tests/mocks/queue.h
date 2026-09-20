#ifndef MOCK_QUEUE_H
#define MOCK_QUEUE_H

#include "FreeRTOS.h"

static inline BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait) {
    (void)xQueue;
    (void)pvItemToQueue;
    (void)xTicksToWait;
    return pdTRUE;
}

static inline BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait) {
    (void)xQueue;
    (void)pvBuffer;
    (void)xTicksToWait;
    return pdFALSE;
}

#endif // MOCK_QUEUE_H
