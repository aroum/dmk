#ifndef SPLIT_TEST_FREERTOS_H
#define SPLIT_TEST_FREERTOS_H
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef void *QueueHandle_t;
typedef void *TaskHandle_t;
typedef void *SemaphoreHandle_t;
typedef uint32_t StackType_t;
typedef int StaticQueue_t;
typedef int StaticTask_t;
typedef int StaticSemaphore_t;
#define pdTRUE 1
#define pdFALSE 0
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
#define portMAX_DELAY UINT32_MAX
#define configSUPPORT_STATIC_ALLOCATION 1
#define configMAX_PRIORITIES 4
#define configASSERT(x) assert(x)
#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)
#define taskSCHEDULER_NOT_STARTED 0
extern TickType_t g_mock_ticks;
static inline TickType_t xTaskGetTickCount(void) { return g_mock_ticks; }
static inline void vTaskDelay(TickType_t ticks) { g_mock_ticks += ticks; }
static inline BaseType_t xTaskGetSchedulerState(void) { return 1; }
static inline TaskHandle_t xTaskCreateStatic(void (*fn)(void *), const char *name, uint32_t size,
                                             void *arg, unsigned priority, StackType_t *stack, StaticTask_t *tcb) {
    (void)fn; (void)name; (void)size; (void)arg; (void)priority; (void)stack; return tcb;
}
#endif
