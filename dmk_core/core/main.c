// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// General C includes
#include <stdbool.h>

// Project includes
#include "config.h"
#include "hal_gpio.h"
#include "hooks.h"
#include "keyboard.h"
#include "led.h"
#include "matrix.h"
#include "rgb.h"
#include "split.h"
#include "task_internal.h"
#include "usb.h"

#ifndef __NOP
#define __NOP() __asm__ volatile("nop")
#endif

// Global FreeRTOS inter-task communication queues
QueueHandle_t matrix_queue;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
static uint8_t s_matrix_queue_storage[QUEUE_DEF_SIZE * sizeof(matrix_event_t)];
static StaticQueue_t s_matrix_queue_struct;

static StaticTask_t s_keyboard_task_tcb;
static StackType_t s_keyboard_task_stack[TASK_STACK_KEYBOARD];

#if defined(RGB_NUM)
static StaticTask_t s_rgb_task_tcb;
static StackType_t s_rgb_task_stack[TASK_STACK_RGB];
#endif

static StaticTask_t s_matrix_task_tcb;
static StackType_t s_matrix_task_stack[TASK_STACK_MATRIX];
#endif

/**
 * @brief Firmware entry point: initializes platform HAL, allocates queues, spawns tasks, and starts RTOS scheduler.
 */
int main(void) {
    // Platform-specific low-level clock and system peripheral setup
    platform_init();

    // Initialize board indicator and lock LEDs
    led_init();

    // Initialize USB HID / MIDI hardware and endpoints
    usb_init();

    // Create FreeRTOS matrix event queue
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    matrix_queue =
        xQueueCreateStatic(QUEUE_DEF_SIZE, sizeof(matrix_event_t), s_matrix_queue_storage, &s_matrix_queue_struct);
#else
    matrix_queue = xQueueCreate(QUEUE_DEF_SIZE, sizeof(matrix_event_t));
#endif

    if (matrix_queue == NULL) {
        while (1) {
            __NOP();
        }
    }

    BaseType_t status = pdPASS;

    // Keyboard state machine, layer stack, and tap engine task
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (xTaskCreateStatic(keyboard_task, "keyboard", TASK_STACK_KEYBOARD, NULL, TASK_PRIO_KEYBOARD,
                          s_keyboard_task_stack, &s_keyboard_task_tcb) == NULL) {
        status = pdFAIL;
    }
#else
    if (xTaskCreate(keyboard_task, "keyboard", TASK_STACK_KEYBOARD, NULL, TASK_PRIO_KEYBOARD, NULL) != pdPASS) {
        status = pdFAIL;
    }
#endif

#if defined(RGB_NUM)
    // RGB animations task
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (xTaskCreateStatic(rgb_task, "rgb", TASK_STACK_RGB, NULL, TASK_PRIO_RGB, s_rgb_task_stack, &s_rgb_task_tcb) ==
        NULL) {
        status = pdFAIL;
    }
#else
    if (xTaskCreate(rgb_task, "rgb", TASK_STACK_RGB, NULL, TASK_PRIO_RGB, NULL) != pdPASS) {
        status = pdFAIL;
    }
#endif
#endif

    // Matrix switch scanner task
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (xTaskCreateStatic(matrix_task, "matrix", TASK_STACK_MATRIX, NULL, TASK_PRIO_MATRIX, s_matrix_task_stack,
                          &s_matrix_task_tcb) == NULL) {
        status = pdFAIL;
    }
#else
    if (xTaskCreate(matrix_task, "matrix", TASK_STACK_MATRIX, NULL, TASK_PRIO_MATRIX, NULL) != pdPASS) {
        status = pdFAIL;
    }
#endif

    if (status != pdPASS) {
        while (1) {
            __NOP();
        }
    }

    // Initialize split interconnect (UART / PIO / Software bit-bang)
    split_init();

#if defined(RGB_NUM)
    // Initialize WS2812 hardware drivers before scheduler starts
    rgb_init();
#endif

    // Invoke user modules / extension early initialization
    hook_early_init();

    // Start FreeRTOS preemptive multitasking scheduler
    vTaskStartScheduler();

    // Should never reach here unless heap memory was exhausted
    while (1) {
    }
}
