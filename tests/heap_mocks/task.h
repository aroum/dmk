#ifndef DMK_HEAP_TEST_TASK_H
#define DMK_HEAP_TEST_TASK_H
#include "FreeRTOS.h"
extern bool g_scheduler_running;
static inline BaseType_t xTaskGetSchedulerState(void) { return g_scheduler_running ? 1 : taskSCHEDULER_NOT_STARTED; }
static inline void vTaskSuspendAll(void) {}
static inline BaseType_t xTaskResumeAll(void) { return pdTRUE; }
#endif
