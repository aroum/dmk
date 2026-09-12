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
#if defined(__ARM_ARCH) || defined(__arm__)
    __asm__ volatile("wfi");
#endif
}

// Default weak implementations for module hooks
__attribute__((weak)) void hook_early_init(void) {}

__attribute__((weak)) void hook_layer_change(uint8_t active_layer) {
    (void)active_layer;
}

__attribute__((weak)) bool hook_matrix_change(uint8_t row, uint8_t col, bool pressed) {
    (void)row;
    (void)col;
    (void)pressed;
    return false;
}

__attribute__((weak)) bool hook_process_key(uint32_t keycode, bool pressed) {
    (void)keycode;
    (void)pressed;
    return false;
}

__attribute__((weak)) void hook_key_sent(uint16_t keycode, bool pressed) {
    (void)keycode;
    (void)pressed;
}

__attribute__((weak)) void hook_hid_led_change(uint8_t led_mask) {
    (void)led_mask;
}

__attribute__((weak)) bool hook_mouse_move(int8_t *dx, int8_t *dy) {
    (void)dx;
    (void)dy;
    return true;
}

__attribute__((weak)) bool hook_mouse_scroll(int8_t *wheel, int8_t *pan) {
    (void)wheel;
    (void)pan;
    return true;
}

__attribute__((weak)) void hook_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan) {
    (void)buttons;
    (void)dx;
    (void)dy;
    (void)wheel;
    (void)pan;
}

__attribute__((weak)) bool hook_gamepad_report(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry,
                                               uint8_t *hat, uint32_t *buttons) {
    (void)x;
    (void)y;
    (void)z;
    (void)rz;
    (void)rx;
    (void)ry;
    (void)hat;
    (void)buttons;
    return true;
}

__attribute__((weak)) void hook_midi_send(const uint8_t *msg, uint8_t len) {
    (void)msg;
    (void)len;
}