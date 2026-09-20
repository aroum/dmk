#ifndef MOCK_FREERTOS_H
#define MOCK_FREERTOS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint32_t TickType_t;
typedef long BaseType_t;
typedef void *QueueHandle_t;
typedef void *TaskHandle_t;

#define pdTRUE 1
#define pdFALSE 0
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
#define portMAX_DELAY 0xFFFFFFFFUL

#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)

extern TickType_t g_mock_ticks;
static inline TickType_t xTaskGetTickCount(void) {
    return g_mock_ticks;
}

static inline void vTaskDelay(TickType_t ticks) {
    g_mock_ticks += ticks;
}

#endif // MOCK_FREERTOS_H
