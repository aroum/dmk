#ifndef SPLIT_TEST_QUEUE_H
#define SPLIT_TEST_QUEUE_H
#include "FreeRTOS.h"
static inline unsigned uxQueueSpacesAvailable(QueueHandle_t queue) { (void)queue; return 16; }
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t wait);
static inline BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t wait) {
    (void)queue; (void)item; (void)wait; return pdFALSE;
}
static inline QueueHandle_t xQueueCreateStatic(unsigned length, unsigned size, uint8_t *storage, StaticQueue_t *control) {
    (void)length; (void)size; (void)storage; return control;
}
#endif
