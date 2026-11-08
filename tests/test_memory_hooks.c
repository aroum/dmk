#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"

extern volatile uint32_t dmk_malloc_failure_count;
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t task, char *name);
static unsigned resets;
void platform_system_reset(void) { ++resets; }

int main(void) {
    dmk_malloc_failure_count = 0;
    vApplicationMallocFailedHook();
    vApplicationMallocFailedHook();
    assert(dmk_malloc_failure_count == 2);
    dmk_malloc_failure_count = UINT32_MAX;
    vApplicationMallocFailedHook();
    assert(dmk_malloc_failure_count == UINT32_MAX);
    vApplicationStackOverflowHook(NULL, NULL);
    assert(resets == 1);
    puts("FreeRTOS failure hook tests passed");
    return 0;
}
