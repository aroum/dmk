// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Project includes
#include "config.h"
#include "hal_gpio.h"
#include "led.h"
#include "rgb.h"
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

// Repeated snapshots repair lost events. Timeout releases only keys received
// from the remote half, never keys belonging to the local matrix.
#ifndef SPLIT_LINK_TIMEOUT_MS
#define SPLIT_LINK_TIMEOUT_MS (2000U + NUM_ROWS_SPLIT * NUM_COLS_SPLIT * 50U)
#endif
#ifndef SPLIT_SNAPSHOT_INTERVAL_MS
#define SPLIT_SNAPSHOT_INTERVAL_MS 25U
#endif
// A snapshot may be skipped under load; fresh snapshots will retry it. Keep
// queue space for real key transitions, which must never be dropped.
#define SPLIT_SOFT_TX_QUEUE_LENGTH 16
#define SPLIT_SOFT_KEY_RESERVE 4
static QueueHandle_t s_soft_tx_queue;
static bool s_remote_pressed[NUM_ROWS][NUM_COLS];
static TickType_t s_remote_last_seen;
static bool s_remote_connected;
static StaticSemaphore_t s_remote_mutex_storage;
static SemaphoreHandle_t s_remote_mutex;
static StaticSemaphore_t s_transport_mutex_storage;
static SemaphoreHandle_t s_transport_mutex;
#ifndef SPLIT_REMOTE_RGB_COUNT
#ifdef RGB_NUM
#define SPLIT_REMOTE_RGB_COUNT RGB_NUM
#else
#define SPLIT_REMOTE_RGB_COUNT 0
#endif
#endif
#if SPLIT_REMOTE_RGB_COUNT > 255
#error "At most 255 remote RGB outputs are supported"
#endif
#define SPLIT_RGB_STORAGE_COUNT ((SPLIT_REMOTE_RGB_COUNT) ? (SPLIT_REMOTE_RGB_COUNT) : 1)
static uint32_t s_remote_rgb[SPLIT_RGB_STORAGE_COUNT];
static bool s_remote_rgb_valid[SPLIT_RGB_STORAGE_COUNT];
static uint8_t s_remote_led_mask;
static bool s_remote_led_valid;

void split_set_remote_leds(uint8_t mask) {
    bool running = xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
    if (running && !is_master()) return;
    if (running) taskENTER_CRITICAL();
    s_remote_led_mask = mask;
    s_remote_led_valid = true;
    if (running) taskEXIT_CRITICAL();
}

void split_set_remote_rgb(uint8_t index, uint32_t rgb) {
    bool running = xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
    if (index >= SPLIT_REMOTE_RGB_COUNT || (running && !is_master())) return;
    if (running) taskENTER_CRITICAL();
    s_remote_rgb[index] = rgb & 0xFFFFFFu;
    s_remote_rgb_valid[index] = true;
    if (running) taskEXIT_CRITICAL();
}

// A matrix packet grants the master one reply slot. Sending only in this
// slot keeps the slave's matrix transmission and master output commands from
// competing for the shared half-duplex wire.
static bool split_next_output_packet(split_packet_t *packet) {
    static unsigned next_slot;
    static uint8_t sequence;
    const unsigned slots = 1u + SPLIT_REMOTE_RGB_COUNT;
    for (unsigned attempt = 0; attempt < slots; ++attempt) {
        unsigned slot = next_slot++ % slots;
        bool valid;
        uint32_t value;
        taskENTER_CRITICAL();
        if (slot == 0) {
            valid = s_remote_led_valid;
            value = s_remote_led_mask;
        } else {
            valid = s_remote_rgb_valid[slot - 1];
            value = s_remote_rgb[slot - 1];
        }
        taskEXIT_CRITICAL();
        if (valid) {
            if (slot == 0) split_make_led_packet(packet, sequence++, (uint8_t)value);
            else split_make_rgb_packet(packet, sequence++, (uint8_t)(slot - 1), value);
            return true;
        }
    }
    return false;
}

static StaticTask_t s_split_sync_task_tcb;
static StackType_t s_split_sync_task_stack[256];

void split_send_event(matrix_event_t *event) {
    if (!event || !s_transport_mutex) return;
    if (xSemaphoreTake(s_transport_mutex, portMAX_DELAY) == pdTRUE) {
        split_packet_t packet;
        split_make_packet(&packet, event);
        split_transport_send_packet(&packet);
        xSemaphoreGive(s_transport_mutex);
    }
}

static void split_expire_remote(TickType_t now) {
    xSemaphoreTake(s_remote_mutex, portMAX_DELAY);
    if (s_remote_connected && (TickType_t)(now - s_remote_last_seen) >= pdMS_TO_TICKS(SPLIT_LINK_TIMEOUT_MS)) {
        for (unsigned r = 0; r < NUM_ROWS; ++r) {
            for (unsigned c = 0; c < NUM_COLS; ++c) {
                if (s_remote_pressed[r][c]) {
                    s_remote_pressed[r][c] = false;
                    matrix_event_t event = {.row = r, .col = c, .pressed = 0, .split = 1};
                    xQueueSend(matrix_queue, &event, portMAX_DELAY);
                }
            }
        }
        s_remote_connected = false;
    }
    xSemaphoreGive(s_remote_mutex);
}

static matrix_event_t split_make_snapshot(uint8_t row, uint8_t col) {
    matrix_event_t event = {.row = row, .col = col, .split = 1, .pressed = matrix_is_pressed(row, col)};
#if defined(RIGHT) || defined(SPLIT_RIGHT)
    event.col += SPLIT_COL_OFFSET;
#endif
    return event;
}

#ifdef DMK_SPLIT_TEST
bool split_test_next_output_packet(split_packet_t *packet) { return split_next_output_packet(packet); }
void split_test_expire_remote(TickType_t now) { split_expire_remote(now); }
matrix_event_t split_test_make_snapshot(uint8_t row, uint8_t col) { return split_make_snapshot(row, col); }
#endif

static void split_sync_task(void *unused) {
    (void)unused;
    unsigned row = 0, col = 0;
    for (;;) {
        if (!is_master()) {
            matrix_event_t event = split_make_snapshot(row, col);
            if (!s_soft_tx_queue || uxQueueSpacesAvailable(s_soft_tx_queue) > SPLIT_SOFT_KEY_RESERVE) {
                split_send_event(&event);
                if (++col >= NUM_COLS_SPLIT) { col = 0; if (++row >= NUM_ROWS_SPLIT) row = 0; }
            }
        } else {
            split_expire_remote(xTaskGetTickCount());
        }
        vTaskDelay(pdMS_TO_TICKS(SPLIT_SNAPSHOT_INTERVAL_MS));
    }
}

void split_common_init(void) {
    s_transport_mutex = xSemaphoreCreateMutexStatic(&s_transport_mutex_storage);
    s_remote_mutex = xSemaphoreCreateMutexStatic(&s_remote_mutex_storage);
    configASSERT(s_transport_mutex != NULL && s_remote_mutex != NULL);
    TaskHandle_t task = xTaskCreateStatic(split_sync_task, "split_sync", 256, NULL, TASK_PRIO_DEF,
                                         s_split_sync_task_stack, &s_split_sync_task_tcb);
    configASSERT(task != NULL);
    (void)task;
}

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
 * Generates a wake-up pulse, releases line, and transmits split_packet_t.
 * @param event Matrix event structure (row, col, pressed).
 */
static void split_soft_transmit(const split_packet_t *packet) {

    // Emulate open-drain high/idle
    hal_gpio_set_dir(SPLIT_TX_PIN, false);
    hal_gpio_pull_up(SPLIT_TX_PIN);
    vTaskDelay(pdMS_TO_TICKS(1));

    // A four-tick pulse leaves time for the peer's one-tick polling sleep.
    hal_gpio_put(SPLIT_TX_PIN, false);
    hal_gpio_set_dir(SPLIT_TX_PIN, true);
    vTaskDelay(pdMS_TO_TICKS(4));

    // Allow the peer to detect the pulse end before the first start bit. A
    // scheduler delay here can exceed the receiver's inter-byte timeout.
    hal_gpio_set_dir(SPLIT_TX_PIN, false);
    hal_gpio_pull_up(SPLIT_TX_PIN);
    hal_sleep_us(SPLIT_SOFT_WAKE_GAP_US);

    // Keep each byte timed accurately while allowing interrupts between bytes.
    const uint8_t *pkt_ptr = (const uint8_t *)packet;
    for (size_t i = 0; i < sizeof(split_packet_t); i++) {
        taskENTER_CRITICAL();
        soft_uart_send_byte(pkt_ptr[i]);
        taskEXIT_CRITICAL();
    }
}

#ifdef DMK_SPLIT_TEST
void split_test_soft_transmit(const split_packet_t *packet) { split_soft_transmit(packet); }
#endif

static StaticQueue_t s_soft_tx_queue_control;
static uint8_t s_soft_tx_queue_storage[SPLIT_SOFT_TX_QUEUE_LENGTH * sizeof(split_packet_t)];
static StaticTask_t s_soft_tx_task_tcb;
static StackType_t s_soft_tx_task_stack[256];

static bool split_soft_receive_frame(uint32_t wait_us);

static void split_soft_tx_task(void *unused) {
    (void)unused;
    split_packet_t packet;
    for (;;) {
        if (xQueueReceive(s_soft_tx_queue, &packet, portMAX_DELAY) == pdTRUE && !is_master()) {
            split_soft_transmit(&packet);
            // The master may use the reserved reply slot after each packet.
            split_soft_receive_frame(15000);
        }
    }
}

void split_soft_send_packet(const split_packet_t *packet) {
    if (s_soft_tx_queue && packet) xQueueSend(s_soft_tx_queue, packet, portMAX_DELAY);
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

    taskENTER_CRITICAL();
    // Delay to middle of the first data bit (1.5 bit times)
    hal_sleep_us(BIT_TIME_US + (BIT_TIME_US / 2));

    uint8_t val = 0;
    for (int i = 0; i < 8; i++) {
        if (soft_uart_read_bit()) {
            val |= (1 << i);
        }
    }

    // Reject a missing stop bit rather than treating a broken frame as data.
    bool valid_stop = hal_gpio_get(SPLIT_TX_PIN);
    taskEXIT_CRITICAL();
    if (!valid_stop) return false;
    *byte = val;
    hal_sleep_us(BIT_TIME_US / 2);
    return true;
}

/**
 * @brief FreeRTOS task running on master side to listen for slave wake-up pulses and receive packets.
 */
static bool split_soft_receive_frame(uint32_t wait_us) {
    while (hal_gpio_get(SPLIT_TX_PIN) && wait_us) {
        hal_sleep_us(10);
        wait_us = wait_us > 10 ? wait_us - 10 : 0;
    }
    if (!wait_us) return false;
    uint32_t low_duration = 0;
    while (!hal_gpio_get(SPLIT_TX_PIN) && low_duration < 5000) {
        hal_sleep_us(10);
        low_duration += 10;
    }
    // The receiver can wake part-way through the four-tick pulse. UART data
    // cannot hold the line low for this long at the configured baud rate.
    if (low_duration < 1200 || !hal_gpio_get(SPLIT_TX_PIN)) return false;
    split_packet_t pkt;
    uint8_t *bytes = (uint8_t *)&pkt;
    for (size_t i = 0; i < sizeof(pkt); ++i)
        if (!soft_uart_receive_byte(&bytes[i])) return false;
    for (size_t i = 0; i < sizeof(pkt); ++i)
        split_process_received_byte(bytes[i]);
    return true;
}

#ifdef DMK_SPLIT_TEST
bool split_test_soft_receive_frame(uint32_t wait_us) { return split_soft_receive_frame(wait_us); }
#endif

static void split_soft_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        if (is_master()) split_soft_receive_frame(1000);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

#if (configSUPPORT_STATIC_ALLOCATION == 1)
static StaticTask_t s_split_soft_task_tcb;
static StackType_t s_split_soft_task_stack[512];
#endif

/**
 * @brief Initialize universal software bit-bang split communication.
 */
void split_soft_init(void) {
    s_soft_tx_queue = xQueueCreateStatic(SPLIT_SOFT_TX_QUEUE_LENGTH, sizeof(split_packet_t),
                                         s_soft_tx_queue_storage, &s_soft_tx_queue_control);
    configASSERT(s_soft_tx_queue != NULL);
    TaskHandle_t tx_task = xTaskCreateStatic(split_soft_tx_task, "split_tx", 256, NULL, TASK_PRIO_DEF,
                                            s_soft_tx_task_stack, &s_soft_tx_task_tcb);
    configASSERT(tx_task != NULL);
    (void)tx_task;
    hal_gpio_init(SPLIT_TX_PIN);
    hal_gpio_set_dir(SPLIT_TX_PIN, false);
    hal_gpio_pull_up(SPLIT_TX_PIN);

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    xTaskCreateStatic(split_soft_task, "split_soft", 512, NULL, configMAX_PRIORITIES - 1, s_split_soft_task_stack,
                      &s_split_soft_task_tcb);
#else
    xTaskCreate(split_soft_task, "split_soft", 512, NULL, configMAX_PRIORITIES - 1, NULL);
#endif
}

/**
 * @brief Universal packet parser for split UART / PIO / soft receivers.
 * Collects split_packet_t (header 0xA5, row, col, pressed) and enqueues matrix event.
 */
void split_process_received_byte(uint8_t byte) {
    static split_packet_t pkt;
    static uint8_t count;
    static TickType_t last_byte_time;
    TickType_t now = xTaskGetTickCount();
    if (count && (TickType_t)(now - last_byte_time) > pdMS_TO_TICKS(20)) count = 0;
    last_byte_time = now;

    if (!count && byte != SPLIT_PACKET_HEADER) return;
    ((uint8_t *)&pkt)[count++] = byte;
    if (count == sizeof(pkt)) {
        if (split_packet_valid(&pkt) && pkt.row == SPLIT_CONTROL_ROW) {
            // Only the slave applies master output commands. A matrix event can
            // never alias this type because its coordinates are bounds-checked.
            if (!is_master()) {
                if (pkt.col == SPLIT_CONTROL_LED_MASK) {
                    led_set_output_mask(pkt.pressed);
                } else {
                    uint32_t color = ((uint32_t)pkt.pressed << 16) | ((uint32_t)pkt.green << 8) | pkt.blue;
                    rgb_set_pixel_raw(pkt.col, color);
                    rgb_show();
                }
            }
            count = 0;
        } else if (split_packet_valid(&pkt) && pkt.row != SPLIT_CONTROL_ROW) {
            // Only the master interprets matrix packets. This also prevents a
            // UART echo from injecting the slave's own events into its queue.
            if (!is_master()) { count = 0; return; }
            // Respond before a potentially blocking matrix enqueue, while the
            // sender still holds its reply window open.
            if (is_master()) {
                split_packet_t reply;
                if (split_next_output_packet(&reply)) {
                    // The peer may still be draining its TX FIFO or re-arming
                    // RX after half-duplex transmission.
                    vTaskDelay(pdMS_TO_TICKS(SPLIT_REPLY_TURNAROUND_MS));
                    if (is_master()) split_transport_reply_packet(&reply);
                }
            }
            xSemaphoreTake(s_remote_mutex, portMAX_DELAY);
            s_remote_last_seen = now;
            s_remote_connected = true;
            bool changed = s_remote_pressed[pkt.row][pkt.col] != (pkt.pressed != 0);
            s_remote_pressed[pkt.row][pkt.col] = pkt.pressed != 0;
            if (changed) {
                matrix_event_t event = {.split = 1, .row = pkt.row, .col = pkt.col, .pressed = pkt.pressed};
                xQueueSend(matrix_queue, &event, portMAX_DELAY);
            }
            xSemaphoreGive(s_remote_mutex);
            count = 0;
        } else {
            // A dropped byte can place the next header inside this candidate.
            // Retain the suffix and continue looking for a valid CRC.
            const uint8_t *bytes = (const uint8_t *)&pkt;
            uint8_t next = 1;
            while (next < sizeof(pkt) && bytes[next] != SPLIT_PACKET_HEADER) ++next;
            count = (uint8_t)(sizeof(pkt) - next);
            if (count) memmove(&pkt, bytes + next, count);
        }
    }
}

__attribute__((weak)) bool is_master(void) {
#if defined(MCU_rp2040) || defined(MCU_rp2350) || defined(MCU_baikal)
    extern bool tud_mounted(void);
    return tud_mounted();
#else
    extern volatile bool usb_mounted;
    return usb_mounted;
#endif
}

__attribute__((weak)) void split_transport_send_packet(const split_packet_t *packet) {
    split_soft_send_packet(packet);
}

__attribute__((weak)) void split_transport_reply_packet(const split_packet_t *packet) {
    // Called by the master receiver in the slave's reserved reply slot.
    split_soft_transmit(packet);
}

__attribute__((weak)) void split_init(void) {
    split_soft_init();
}

#endif
