#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdbool.h>

// A failed optional allocation must not trap the keyboard before USB starts.
// FreeRTOS returns NULL/pdFAIL to the caller after invoking this hook.
volatile uint32_t dmk_malloc_failure_count;

void vApplicationMallocFailedHook(void) {
#if INCLUDE_xTaskGetSchedulerState == 1
    bool running = xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
    if (running) taskENTER_CRITICAL();
#endif
    if (dmk_malloc_failure_count != UINT32_MAX) ++dmk_malloc_failure_count;
#if INCLUDE_xTaskGetSchedulerState == 1
    if (running) taskEXIT_CRITICAL();
#endif
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name) {
    (void)task;
    (void)task_name;
    // A corrupted task stack cannot safely continue executing.
    extern void platform_system_reset(void);
    platform_system_reset();
}

