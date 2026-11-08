#ifndef SPLIT_TEST_SEMPHR_H
#define SPLIT_TEST_SEMPHR_H
#include "FreeRTOS.h"
static inline SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *storage) { return storage; }
static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t wait) {
    (void)wait; assert(mutex); return pdTRUE;
}
static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex) { assert(mutex); return pdTRUE; }
#endif
