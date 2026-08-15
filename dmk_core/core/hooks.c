// FreeRTOS includes
#include "FreeRTOS.h" /* Must come first. */
#include "task.h"     /* RTOS task related API prototypes. */
#include <stdbool.h>
#include <stdint.h>

void vApplicationMallocFailedHook(void) {
    /* The malloc failed hook is enabled by setting
    configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

    Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)pcTaskName;
    (void)xTask;

    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  pxCurrentTCB can be
    inspected in the debugger if the task name passed into this function is
    corrupt. */
    for (;;)
        ;
}

void vApplicationIdleHook(void) {
    for (;;)
        ;
}

// Default weak implementations for module hooks
__attribute__((weak)) void hook_early_init(void) {}

__attribute__((weak)) void hook_layer_change(uint8_t active_layer) {
    (void)active_layer;
}

__attribute__((weak)) void hook_matrix_change(uint8_t row, uint8_t col, bool pressed) {
    (void)row;
    (void)col;
    (void)pressed;
}

__attribute__((weak)) void hook_key_sent(uint16_t keycode, bool pressed) {
    (void)keycode;
    (void)pressed;
}

__attribute__((weak)) void hook_hid_led_change(uint8_t led_mask) {
    (void)led_mask;
}