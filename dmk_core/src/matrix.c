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

/**
 * @brief Initialize matrix GPIO pins and default electrical states (pull-ups/pull-downs).
 */
void matrix_init(void) {
#if (MATRIX_TYPE == ROW2COL)
    // Configure row pins as active drivers
    for (uint32_t j = 0; j < CUR_NUM_ROWS; ++j) {
        hal_gpio_init(rows_gpios[j]);
        hal_gpio_set_dir(rows_gpios[j], true);
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_put(rows_gpios[j], 1); // idle high
#else
        hal_gpio_put(rows_gpios[j], 0); // idle low
#endif
    }
    // Configure column pins as sensed inputs with appropriate bias
    for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
        hal_gpio_init(columns_gpios[i]);
        hal_gpio_set_dir(columns_gpios[i], false);
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_pull_up(columns_gpios[i]);
#else
        hal_gpio_pull_down(columns_gpios[i]);
#endif
    }
#elif (MATRIX_TYPE == COL2ROW)
    // Configure column pins as active drivers
    for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
        hal_gpio_init(columns_gpios[i]);
        hal_gpio_set_dir(columns_gpios[i], true);
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_put(columns_gpios[i], 1); // idle high
#else
        hal_gpio_put(columns_gpios[i], 0); // idle low
#endif
    }
    // Configure row pins as sensed inputs with appropriate bias
    for (uint32_t j = 0; j < CUR_NUM_ROWS; ++j) {
        hal_gpio_init(rows_gpios[j]);
        hal_gpio_set_dir(rows_gpios[j], false);
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_pull_up(rows_gpios[j]);
#else
        hal_gpio_pull_down(rows_gpios[j]);
#endif
    }
#elif (MATRIX_TYPE == MH3SS2)
    // Hall effect matrix: Columns drive active high, rows pulled down
    for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
        hal_gpio_init(columns_gpios[i]);
        hal_gpio_set_dir(columns_gpios[i], true);
        hal_gpio_put(columns_gpios[i], 0); // idle low
    }
    for (uint32_t j = 0; j < CUR_NUM_ROWS; ++j) {
        hal_gpio_init(rows_gpios[j]);
        hal_gpio_set_dir(rows_gpios[j], false);
        hal_gpio_pull_down(rows_gpios[j]);
    }
#elif (MATRIX_TYPE == DIRECT)
    // Direct pins: Each key has an independent GPIO input pin
    for (uint32_t k = 0; k < CUR_NUM_KEYS; ++k) {
        hal_gpio_init(direct_pins[k]);
        hal_gpio_set_dir(direct_pins[k], false);
#if defined(DIRECT_PINS_ACTIVE_HIGH)
        hal_gpio_pull_down(direct_pins[k]);
#else
        hal_gpio_pull_up(direct_pins[k]);
#endif
    }
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
#if defined(ROLE_CONTROLLER)
    xQueueSend(matrix_queue, matrix_event, 0);
#endif
#endif
}

#if (MATRIX_TYPE != DIRECT)
/**
 * @brief Integrator debounce filter for 2D matrix switches.
 * @param col Column index
 * @param row Row index
 * @param raw_state Raw electrical reading (true = closed/pressed)
 */
static inline void matrix_update_key(uint8_t col, uint8_t row, bool raw_state) {
    if (raw_state) {
        if (debounce_counters[col][row] < DEBOUNCE_TICKS) {
            debounce_counters[col][row]++;
            if (debounce_counters[col][row] == DEBOUNCE_TICKS) {
                debounced_state[col][row] = true;
                matrix_event_t event = {.split = 0, .col = col, .row = row, .pressed = 1};
                matrix_send_event(&event);
            }
        }
    } else {
        if (debounce_counters[col][row] > 0) {
            debounce_counters[col][row]--;
            if (debounce_counters[col][row] == 0) {
                debounced_state[col][row] = false;
                matrix_event_t event = {.split = 0, .col = col, .row = row, .pressed = 0};
                matrix_send_event(&event);
            }
        }
    }
}
#endif

#if (MATRIX_TYPE == DIRECT)
/**
 * @brief Integrator debounce filter for direct-pin switches.
 * @param key_idx Key index in direct_pins array
 * @param raw_state Raw electrical reading (true = closed/pressed)
 */
static inline void matrix_update_direct_key(uint8_t key_idx, bool raw_state) {
    if (raw_state) {
        if (debounce_counters[key_idx] < DEBOUNCE_TICKS) {
            debounce_counters[key_idx]++;
            if (debounce_counters[key_idx] == DEBOUNCE_TICKS) {
                debounced_state[key_idx] = true;
                matrix_event_t event = {.split = 0, .col = key_idx, .row = 0, .pressed = 1};
                matrix_send_event(&event);
            }
        }
    } else {
        if (debounce_counters[key_idx] > 0) {
            debounce_counters[key_idx]--;
            if (debounce_counters[key_idx] == 0) {
                debounced_state[key_idx] = false;
                matrix_event_t event = {.split = 0, .col = key_idx, .row = 0, .pressed = 0};
                matrix_send_event(&event);
            }
        }
    }
}
#endif

/**
 * @brief Performs a full scan of the keyboard matrix or direct pins.
 */
void matrix_scan(void) {
#if (MATRIX_TYPE == ROW2COL)
    for (uint32_t j = 0; j < CUR_NUM_ROWS; ++j) {
        // Drive current row active
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_put(rows_gpios[j], 0); // drive low
#else
        hal_gpio_put(rows_gpios[j], 1); // drive high
#endif
#if (MATRIX_IO_DELAY_US > 0)
        hal_sleep_us(MATRIX_IO_DELAY_US);
#endif

        // Read all columns for this row
        for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
#if defined(MATRIX_ACTIVE_LOW)
            bool raw_state = !hal_gpio_get(columns_gpios[i]);
#else
            bool raw_state = hal_gpio_get(columns_gpios[i]);
#endif
            matrix_update_key((uint8_t)i, (uint8_t)j, raw_state);
        }

        // Return row to idle state
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_put(rows_gpios[j], 1); // restore idle high
#else
        hal_gpio_put(rows_gpios[j], 0); // restore idle low
#endif
    }
#elif (MATRIX_TYPE == COL2ROW)
    for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
        // Drive current column active
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_put(columns_gpios[i], 0);
#else
        hal_gpio_put(columns_gpios[i], 1);
#endif
#if (MATRIX_IO_DELAY_US > 0)
        hal_sleep_us(MATRIX_IO_DELAY_US);
#endif

        // Read all rows for this column
        for (uint32_t j = 0; j < CUR_NUM_ROWS; ++j) {
#if defined(MATRIX_ACTIVE_LOW)
            bool raw_state = !hal_gpio_get(rows_gpios[j]);
#else
            bool raw_state = hal_gpio_get(rows_gpios[j]);
#endif
            matrix_update_key((uint8_t)i, (uint8_t)j, raw_state);
        }

        // Return column to idle state
#if defined(MATRIX_ACTIVE_LOW)
        hal_gpio_put(columns_gpios[i], 1);
#else
        hal_gpio_put(columns_gpios[i], 0);
#endif
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
#if defined(DIRECT_PINS_ACTIVE_HIGH)
        bool raw_state = hal_gpio_get(direct_pins[k]);
#else
        bool raw_state = !hal_gpio_get(direct_pins[k]);
#endif
        matrix_update_direct_key((uint8_t)k, raw_state);
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
