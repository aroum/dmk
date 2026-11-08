#include <assert.h>
#include <stdio.h>
#include "FreeRTOS.h"

bool g_scheduler_running;
extern volatile uint32_t dmk_malloc_failure_count;
void *pvPortMalloc(size_t size);
void vPortFree(void *ptr);
size_t xPortGetFreeHeapSize(void);
void platform_system_reset(void) { assert(false); }

int main(void) {
    // A 512-word stack and a task control block must fit the configured pool.
    void *stack = pvPortMalloc(512 * sizeof(uint32_t));
    void *control = pvPortMalloc(256);
    assert(stack && control);
    // An oversized optional allocation must fail and return, before scheduler start.
    assert(pvPortMalloc(configTOTAL_HEAP_SIZE) == NULL);
    assert(dmk_malloc_failure_count == 1);
    // The allocator remains usable after failure, including when tasks run.
    g_scheduler_running = true;
    void *small = pvPortMalloc(64);
    assert(small);
    assert(pvPortMalloc(configTOTAL_HEAP_SIZE) == NULL);
    assert(dmk_malloc_failure_count == 2);
    vPortFree(small);
    vPortFree(control);
    vPortFree(stack);
    assert(xPortGetFreeHeapSize() > configTOTAL_HEAP_SIZE - 200);
    puts("heap_4 exhaustion and recovery tests passed");
    return 0;
}
