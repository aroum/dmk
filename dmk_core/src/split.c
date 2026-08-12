// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Project includes
#include "config.h"
#include "hal_gpio.h"
#include "split.h"
#include "task_internal.h"

#if defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)

#ifndef SPLIT_TX_PIN
#ifdef SERIAL_PIN
#define SPLIT_TX_PIN SERIAL_PIN
#else
#define SPLIT_TX_PIN 0
#endif
#endif

/**
 * @brief Universal software bit-bang: write a single bit using open-drain emulation.
 * Emulates open-drain: logic 1 = input with pull-up, logic 0 = drive low output.
 * @param bit Value (true = 1, false = 0).
 */
static void soft_uart_write_bit(bool bit) {
    if (bit) {
        hal_gpio_set_dir(SPLIT_TX_PIN, false);
        hal_gpio_pull_up(SPLIT_TX_PIN);
    } else {
        hal_gpio_put(SPLIT_TX_PIN, false);
        hal_gpio_set_dir(SPLIT_TX_PIN, true);
    }
    hal_sleep_us(BIT_TIME_US);
}

/**
 * @brief Transmit 8 data bits with 1 start bit and 1 stop bit via soft bit-bang.
 * @param byte Data byte to transmit (LSB first).
 */
static void soft_uart_send_byte(uint8_t byte) {
    soft_uart_write_bit(false); // Start bit (0)
    for (int i = 0; i < 8; i++) {
        soft_uart_write_bit((byte >> i) & 1);
    }
    soft_uart_write_bit(true); // Stop bit (1)
    hal_sleep_us(BIT_TIME_US);
}

/**
 * @brief Send a matrix event from slave half to master half using single-wire software bit-bang.
 * Generates a 2ms low wake-up pulse, releases line, and transmits 4-byte split_packet_t.
 * @param event Matrix event structure (row, col, pressed).
 */
void split_soft_send_event(matrix_event_t *event) {
    split_packet_t pkt;
    pkt.header = 0xA5;
    pkt.row = event->row;
    pkt.col = event->col;
    pkt.pressed = event->pressed;

    // Emulate open-drain high/idle
    hal_gpio_set_dir(SPLIT_TX_PIN, false);
    hal_gpio_pull_up(SPLIT_TX_PIN);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Send wake-up pulse (drive low for 2ms)
    hal_gpio_put(SPLIT_TX_PIN, false);
    hal_gpio_set_dir(SPLIT_TX_PIN, true);
    vTaskDelay(pdMS_TO_TICKS(2));

    // Release line for 1ms
    hal_gpio_set_dir(SPLIT_TX_PIN, false);
    hal_gpio_pull_up(SPLIT_TX_PIN);
    vTaskDelay(pdMS_TO_TICKS(1));

    // Transmit serialized packet in critical section for precise timing
    taskENTER_CRITICAL();
    uint8_t *pkt_ptr = (uint8_t *)&pkt;
    for (size_t i = 0; i < sizeof(split_packet_t); i++) {
        soft_uart_send_byte(pkt_ptr[i]);
    }
    taskEXIT_CRITICAL();
}

/**
 * @brief Read a single bit from the software UART line.
 */
static bool soft_uart_read_bit(void) {
    bool bit = hal_gpio_get(SPLIT_TX_PIN);
    hal_sleep_us(BIT_TIME_US);
    return bit;
}

/**
 * @brief Receive a byte with start bit detection and middle-of-bit sampling.
 * @param byte Output buffer for received byte.
 * @return true if valid byte received, false on timeout.
 */
static bool soft_uart_receive_byte(uint8_t *byte) {
    uint32_t timeout = 1000;
    while (hal_gpio_get(SPLIT_TX_PIN)) {
        if (--timeout == 0)
            return false;
        hal_sleep_us(1);
    }

    // Delay to middle of the first data bit (1.5 bit times)
    hal_sleep_us(BIT_TIME_US + (BIT_TIME_US / 2));

    uint8_t val = 0;
    for (int i = 0; i < 8; i++) {
        if (soft_uart_read_bit()) {
            val |= (1 << i);
        }
    }

    *byte = val;

    timeout = 1000;
    while (!hal_gpio_get(SPLIT_TX_PIN)) {
        if (--timeout == 0)
            break;
        hal_sleep_us(1);
    }
    hal_sleep_us(BIT_TIME_US / 2);

    return true;
}

/**
 * @brief FreeRTOS task running on master side to listen for slave wake-up pulses and receive packets.
 */
static void split_soft_task(void *pvParameters) {
    (void)pvParameters;
    split_packet_t pkt;
    uint8_t *pkt_ptr = (uint8_t *)&pkt;

    while (1) {
        if (is_master()) {
            if (!hal_gpio_get(SPLIT_TX_PIN)) {
                uint32_t low_duration = 0;
                while (!hal_gpio_get(SPLIT_TX_PIN) && low_duration < 2500) {
                    hal_sleep_us(10);
                    low_duration += 10;
                }

                // Check if low duration matches slave wake-up pulse (>= 1.5ms)
                if (low_duration >= 1500) {
                    taskENTER_CRITICAL();

                    uint32_t timeout = 5000;
                    while (!hal_gpio_get(SPLIT_TX_PIN) && timeout > 0) {
                        hal_sleep_us(1);
                        timeout--;
                    }

                    bool success = true;
                    for (size_t i = 0; i < sizeof(split_packet_t); i++) {
                        if (!soft_uart_receive_byte(&pkt_ptr[i])) {
                            success = false;
                            break;
                        }
                    }

                    taskEXIT_CRITICAL();

                    if (success && pkt.header == 0xA5) {
                        matrix_event_t event;
                        event.split = 1;
                        event.row = pkt.row;
                        event.col = pkt.col;
                        event.pressed = pkt.pressed;
                        xQueueSend(matrix_queue, &event, 0);
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/**
 * @brief Initialize universal software bit-bang split communication.
 */
void split_soft_init(void) {
    hal_gpio_init(SPLIT_TX_PIN);
    hal_gpio_set_dir(SPLIT_TX_PIN, false);
    hal_gpio_pull_up(SPLIT_TX_PIN);

    xTaskCreate(split_soft_task, "split_soft", 512, NULL, configMAX_PRIORITIES - 1, NULL);
}

#endif
