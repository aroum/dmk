// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// General C includes
#include <stdbool.h>

// Project includes
#include "config.h"
#include "hal_gpio.h"
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
QueueHandle_t usb_queue;
QueueHandle_t led_queue;

/**
 * @brief Firmware entry point: initializes platform HAL, allocates queues, spawns tasks, and starts RTOS scheduler.
 */
int main(void) {
    // Platform-specific low-level clock and system peripheral setup
    platform_init();

    // Create FreeRTOS queues with optimized memory footprints
    matrix_queue = xQueueCreate(QUEUE_DEF_SIZE, sizeof(matrix_event_t));
    usb_queue = xQueueCreate(QUEUE_DEF_SIZE, sizeof(key_event_t));
    led_queue = xQueueCreate(QUEUE_LED_SIZE, sizeof(bool));

    if (matrix_queue == NULL || usb_queue == NULL || led_queue == NULL) {
        while (1) {
            __NOP();
        }
    }

    BaseType_t status = pdPASS;

#if defined(ROLE_CONTROLLER)
    // LED heartbeat & lock indicator task
    if (xTaskCreate(led_task, "led", TASK_STACK_LED, NULL, TASK_PRIO_DEF, NULL) != pdPASS) {
        status = pdFAIL;
    }

    // Keyboard state machine, layer stack, and tap engine task
    if (xTaskCreate(keyboard_task, "keyboard", TASK_STACK_KEYBOARD, NULL, TASK_PRIO_DEF, NULL) != pdPASS) {
        status = pdFAIL;
    }

#if defined(RGB_NUM) && (defined(MCU_rp2040) || defined(MCU_rp2350) || defined(MCU_nrf52840) || defined(MCU_milandr))
    // RGB animations task
    if (xTaskCreate(rgb_task, "rgb", TASK_STACK_RGB, NULL, TASK_PRIO_DEF, NULL) != pdPASS) {
        status = pdFAIL;
    }
#endif
#endif

    // Matrix switch scanner task
    if (xTaskCreate(matrix_task, "matrix", TASK_STACK_MATRIX, NULL, TASK_PRIO_DEF, NULL) != pdPASS) {
        status = pdFAIL;
    }

#if defined(ROLE_CONTROLLER)
    // USB HID / MIDI report pump task
    if (xTaskCreate(usb_task, "usb", TASK_STACK_USB, NULL, TASK_PRIO_DEF, NULL) != pdPASS) {
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

#if defined(RGB_NUM) && (defined(MCU_rp2040) || defined(MCU_rp2350) || defined(MCU_nrf52840) || defined(MCU_milandr))
    // Initialize WS2812 hardware drivers before scheduler starts
    rgb_init();
#endif

    // Start FreeRTOS preemptive multitasking scheduler
    vTaskStartScheduler();

    // Should never reach here unless heap memory was exhausted
    while (1) {
    }
}
