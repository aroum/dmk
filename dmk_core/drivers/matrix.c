// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// HAL includes
#include "hal_gpio.h"

// General C includes
#include <stdbool.h>
#include <stdint.h>

// Project includes
#include "config.h"
#include "led.h"
#include "matrix.h"
#include "split.h"
#include "task_internal.h"

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
#include "encoder.h"
#endif

/*
 * Matrix scan types:
 * - ROW2COL: Rows are driven, columns are read.
 * - COL2ROW: Columns are driven, rows are read.
 * - DIRECT:  Direct per-key GPIO connection to GND/VCC.
 * - MH3SS2:  Multi-sensor Hall effect matrix mode.
 */
#define MATRIX_TYPE_ROW2COL_VAL 1
#define MATRIX_TYPE_COL2ROW_VAL 2
#define MATRIX_TYPE_DIRECT_VAL 3
#define MATRIX_TYPE_MH3SS2_VAL 4

#define ROW2COL MATRIX_TYPE_ROW2COL_VAL
#define COL2ROW MATRIX_TYPE_COL2ROW_VAL
#define DIRECT MATRIX_TYPE_DIRECT_VAL
#define MH3SS2 MATRIX_TYPE_MH3SS2_VAL

#ifndef MATRIX_TYPE
#if defined(MATRIX_MH3SS2)
#define MATRIX_TYPE MH3SS2
#elif defined(MATRIX_TYPE_ROW2COL)
#define MATRIX_TYPE ROW2COL
#elif defined(MATRIX_TYPE_COL2ROW)
#define MATRIX_TYPE COL2ROW
#elif defined(MATRIX_TYPE_DIRECT_PIN)
#define MATRIX_TYPE DIRECT
#else
#define MATRIX_TYPE ROW2COL
#endif
#endif

#ifndef MH3SS2_ROW_PINS_PER_KEY
#define MH3SS2_ROW_PINS_PER_KEY 1
#endif

// Settling delay in microseconds (default 1us for matrix capacitance settling without CPU waste)
#ifndef MATRIX_IO_DELAY_US
#define MATRIX_IO_DELAY_US 1
#endif

// Debounce timing configuration (default: 5ms integrator filter)
#ifndef DEBOUNCE
#define DEBOUNCE 5
#endif

#define POLL_INTERVAL_MS 1
#define DEBOUNCE_TICKS (DEBOUNCE / POLL_INTERVAL_MS)

#if (DEBOUNCE_TICKS < 1)
#undef DEBOUNCE_TICKS
#define DEBOUNCE_TICKS 1
#endif

#if defined(MATRIX_ACTIVE_LOW)
#define MATRIX_DRV_ACT 0
#define MATRIX_DRV_IDL 1
#define MATRIX_READ(pin) (!hal_gpio_get(pin))
#define MATRIX_PULL_UP true
#else
#define MATRIX_DRV_ACT 1
#define MATRIX_DRV_IDL 0
#define MATRIX_READ(pin) (hal_gpio_get(pin))
#define MATRIX_PULL_UP false
#endif

#if defined(DIRECT_PINS_ACTIVE_HIGH)
#define DIRECT_PULL_UP false
#define DIRECT_READ(pin) (hal_gpio_get(pin))
#else
#define DIRECT_PULL_UP true
#define DIRECT_READ(pin) (!hal_gpio_get(pin))
#endif

// Physical pin mapping arrays derived from config.h
#if defined(COL_PINS) && defined(ROW_PINS)
static const pin_t columns_gpios[] = COL_PINS;
static const pin_t rows_gpios[] = ROW_PINS;
#define CUR_NUM_COLS (sizeof(columns_gpios) / sizeof(columns_gpios[0]))
#define CUR_NUM_ROWS (sizeof(rows_gpios) / sizeof(rows_gpios[0]))
#elif defined(DIRECT_PINS)
static const pin_t direct_pins[] = DIRECT_PINS;
#define CUR_NUM_KEYS (sizeof(direct_pins) / sizeof(direct_pins[0]))
#else
#error "Pin configuration is missing! Define COL_PINS and ROW_PINS, or DIRECT_PINS in config.h"
#endif

// FreeRTOS queue handle used to send debounced switch events to the keyboard processing task
extern QueueHandle_t matrix_queue;

// Debounce state tracking structures:
// Integrator counters increment on pressed state, decrement on release.
// uint8_t saves 50% RAM compared to uint16_t on RAM-constrained microcontrollers.
#if (MATRIX_TYPE == ROW2COL) || (MATRIX_TYPE == COL2ROW) || (MATRIX_TYPE == MH3SS2)
static uint8_t debounce_counters[NUM_COLS][NUM_ROWS] = {{0}};
static bool debounced_state[NUM_COLS][NUM_ROWS] = {{false}};
#elif (MATRIX_TYPE == DIRECT)
static uint8_t debounce_counters[CUR_NUM_KEYS] = {0};
static bool debounced_state[CUR_NUM_KEYS] = {false};
#endif

static void init_pins(const pin_t *pins, size_t count, bool is_output, bool idle_val, bool pull_up) {
    for (size_t i = 0; i < count; ++i) {
        hal_gpio_init(pins[i]);
        hal_gpio_set_dir(pins[i], is_output);
        if (is_output) {
            hal_gpio_put(pins[i], idle_val);
        } else if (pull_up) {
            hal_gpio_pull_up(pins[i]);
        } else {
            hal_gpio_pull_down(pins[i]);
        }
    }
}

/**
 * @brief Initialize matrix GPIO pins and default electrical states (pull-ups/pull-downs).
 */
void matrix_init(void) {
#if (MATRIX_TYPE == ROW2COL)
    init_pins(rows_gpios, CUR_NUM_ROWS, true, MATRIX_DRV_IDL, false);
    init_pins(columns_gpios, CUR_NUM_COLS, false, 0, MATRIX_PULL_UP);
#elif (MATRIX_TYPE == COL2ROW)
    init_pins(columns_gpios, CUR_NUM_COLS, true, MATRIX_DRV_IDL, false);
    init_pins(rows_gpios, CUR_NUM_ROWS, false, 0, MATRIX_PULL_UP);
#elif (MATRIX_TYPE == MH3SS2)
    init_pins(columns_gpios, CUR_NUM_COLS, true, 0, false);
    init_pins(rows_gpios, CUR_NUM_ROWS, false, 0, false);
#elif (MATRIX_TYPE == DIRECT)
    init_pins(direct_pins, CUR_NUM_KEYS, false, 0, DIRECT_PULL_UP);
#endif
}

/**
 * @brief Dispatch debounced matrix switch event to local queue or split interconnect.
 * @param matrix_event Pointer to the event containing row, col, split, and pressed state.
 */
void matrix_send_event(matrix_event_t *matrix_event) {
#if defined(NUM_ROWS_SPLIT) && defined(NUM_COLS_SPLIT)
#if defined(RIGHT)
    // Right half shifts its column indices to avoid collision with the left half
    matrix_event->col += SPLIT_COL_OFFSET;
#endif

    if (is_master()) {
        // Master half pushes directly to the local processing queue
        xQueueSend(matrix_queue, matrix_event, 0);
    } else {
        // Slave half transmits event across UART/PIO split interconnect
        split_send_event(matrix_event);
    }
#else
    xQueueSend(matrix_queue, matrix_event, 0);
#endif
}

#if (MATRIX_TYPE != DIRECT)
/**
 * @brief Eager debounce filter for 2D matrix switches.
 * Immediately dispatches switch state transitions on contact (0ms latency),
 * then locks out physical chatter for DEBOUNCE_TICKS.
 * @param col Column index
 * @param row Row index
 * @param raw_state Raw electrical reading (true = closed/pressed)
 */
static inline void matrix_update_key(uint8_t col, uint8_t row, bool raw_state) {
    if (debounce_counters[col][row] > 0) {
        debounce_counters[col][row]--;
    } else if (raw_state != debounced_state[col][row]) {
        debounced_state[col][row] = raw_state;
        debounce_counters[col][row] = DEBOUNCE_TICKS;
        matrix_event_t event = {.split = 0, .col = col, .row = row, .pressed = raw_state ? 1 : 0};
        matrix_send_event(&event);
    }
}
#endif

#if (MATRIX_TYPE == DIRECT)
/**
 * @brief Eager debounce filter for direct-pin switches.
 * Immediately dispatches switch state transitions on contact (0ms latency),
 * then locks out physical chatter for DEBOUNCE_TICKS.
 * @param key_idx Key index in direct_pins array
 * @param raw_state Raw electrical reading (true = closed/pressed)
 */
static inline void matrix_update_direct_key(uint8_t key_idx, bool raw_state) {
    if (debounce_counters[key_idx] > 0) {
        debounce_counters[key_idx]--;
    } else if (raw_state != debounced_state[key_idx]) {
        debounced_state[key_idx] = raw_state;
        debounce_counters[key_idx] = DEBOUNCE_TICKS;
        matrix_event_t event = {.split = 0, .col = key_idx, .row = 0, .pressed = raw_state ? 1 : 0};
        matrix_send_event(&event);
    }
}
#endif

/**
 * @brief Performs a full scan of the keyboard matrix or direct pins.
 */
void matrix_scan(void) {
#if (MATRIX_TYPE == ROW2COL)
    for (uint32_t j = 0; j < CUR_NUM_ROWS; ++j) {
        hal_gpio_put(rows_gpios[j], MATRIX_DRV_ACT);
#if (MATRIX_IO_DELAY_US > 0)
        hal_sleep_us(MATRIX_IO_DELAY_US);
#endif
        for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
            matrix_update_key((uint8_t)i, (uint8_t)j, MATRIX_READ(columns_gpios[i]));
        }
        hal_gpio_put(rows_gpios[j], MATRIX_DRV_IDL);
    }
#elif (MATRIX_TYPE == COL2ROW)
    for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
        hal_gpio_put(columns_gpios[i], MATRIX_DRV_ACT);
#if (MATRIX_IO_DELAY_US > 0)
        hal_sleep_us(MATRIX_IO_DELAY_US);
#endif
        for (uint32_t j = 0; j < CUR_NUM_ROWS; ++j) {
            matrix_update_key((uint8_t)i, (uint8_t)j, MATRIX_READ(rows_gpios[j]));
        }
        hal_gpio_put(columns_gpios[i], MATRIX_DRV_IDL);
    }
#elif (MATRIX_TYPE == MH3SS2)
    for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
        hal_gpio_put(columns_gpios[i], 1);
#if (MATRIX_IO_DELAY_US > 0)
        hal_sleep_us(MATRIX_IO_DELAY_US);
#endif

        for (uint32_t j = 0; j < NUM_ROWS; ++j) {
            bool raw_state = false;
            uint32_t base_pin = j * MH3SS2_ROW_PINS_PER_KEY;
            for (uint32_t p = 0; p < MH3SS2_ROW_PINS_PER_KEY; ++p) {
                uint32_t pin_idx = base_pin + p;
                if (pin_idx < CUR_NUM_ROWS && hal_gpio_get(rows_gpios[pin_idx])) {
                    raw_state = true;
                    break;
                }
            }
            matrix_update_key((uint8_t)i, (uint8_t)j, raw_state);
        }

        hal_gpio_put(columns_gpios[i], 0);
    }
#elif (MATRIX_TYPE == DIRECT)
    for (uint32_t k = 0; k < CUR_NUM_KEYS; ++k) {
        matrix_update_direct_key((uint8_t)k, DIRECT_READ(direct_pins[k]));
    }
#endif
}

/**
 * @brief FreeRTOS task handling periodic matrix scanning and rotary encoders.
 */
void matrix_task(void *pvParameters) {
    (void)pvParameters;
    matrix_init();
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
    encoder_init();
#endif

    while (1) {
        matrix_scan();
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
        encoder_scan();
#endif
        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }
}

/**
 * @brief Check if a specific switch is currently in a debounced pressed state.
 * @param row Row index
 * @param col Column index
 * @return true if switch is pressed, false otherwise.
 */
bool matrix_is_pressed(unsigned char row, unsigned char col) {
#if (MATRIX_TYPE == ROW2COL) || (MATRIX_TYPE == COL2ROW) || (MATRIX_TYPE == MH3SS2)
    if (col < NUM_COLS && row < NUM_ROWS) {
        return debounced_state[col][row];
    }
#elif (MATRIX_TYPE == DIRECT)
    if (col < CUR_NUM_KEYS) {
        return debounced_state[col];
    }
#endif
    return false;
}
