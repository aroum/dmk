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

// Settling delay in microseconds (default 1us: fast inline NOP delay without timer peripheral spinlock)
#ifndef MATRIX_IO_DELAY_US
#define MATRIX_IO_DELAY_US 1
#endif

static inline void matrix_settle_delay(void) {
#if (MATRIX_IO_DELAY_US > 1)
    hal_sleep_us(MATRIX_IO_DELAY_US);
#elif (MATRIX_IO_DELAY_US == 1)
    // Ultra-fast ~150-200ns settling pause (enough for PCB capacitance without peripheral timer spinlock)
    for (volatile int d = 0; d < 20; ++d) {
        __asm__ volatile("nop");
    }
#endif
}

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
#define MATRIX_READ_SNAPSHOT(snap, pin) (!hal_gpio_snapshot_get(snap, pin))
#define MATRIX_PULL_UP true
#else
#define MATRIX_DRV_ACT 1
#define MATRIX_DRV_IDL 0
#define MATRIX_READ(pin) (hal_gpio_get(pin))
#define MATRIX_READ_SNAPSHOT(snap, pin) (hal_gpio_snapshot_get(snap, pin))
#define MATRIX_PULL_UP false
#endif

#if defined(DIRECT_PINS_ACTIVE_HIGH)
#define DIRECT_PULL_UP false
#define DIRECT_READ(pin) (hal_gpio_get(pin))
#define DIRECT_READ_SNAPSHOT(snap, pin) (hal_gpio_snapshot_get(snap, pin))
#else
#define DIRECT_PULL_UP true
#define DIRECT_READ(pin) (!hal_gpio_get(pin))
#define DIRECT_READ_SNAPSHOT(snap, pin) (!hal_gpio_snapshot_get(snap, pin))
#endif

// Physical pin mapping arrays derived from config.h
#if defined(COL_PINS) && defined(ROW_PINS)
static const pin_t columns_gpios[] = COL_PINS;
static const pin_t rows_gpios[] = ROW_PINS;
#define CUR_NUM_COLS (sizeof(columns_gpios) / sizeof(columns_gpios[0]))
#define CUR_NUM_ROWS (sizeof(rows_gpios) / sizeof(rows_gpios[0]))

// Unified drive and sense pin abstractions to deduplicate ROW2COL and COL2ROW logic
#if (MATRIX_TYPE == ROW2COL)
#define MATRIX_DRV_PINS     rows_gpios
#define MATRIX_NUM_DRV      CUR_NUM_ROWS
#define MATRIX_SENSE_PINS   columns_gpios
#define MATRIX_NUM_SENSE    CUR_NUM_COLS
#define MATRIX_DISPATCH_KEY(drv, sense, state) matrix_update_key((uint8_t)(sense), (uint8_t)(drv), state)
#elif (MATRIX_TYPE == COL2ROW)
#define MATRIX_DRV_PINS     columns_gpios
#define MATRIX_NUM_DRV      CUR_NUM_COLS
#define MATRIX_SENSE_PINS   rows_gpios
#define MATRIX_NUM_SENSE    CUR_NUM_ROWS
#define MATRIX_DISPATCH_KEY(drv, sense, state) matrix_update_key((uint8_t)(drv), (uint8_t)(sense), state)
#endif

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
#if (MATRIX_TYPE == ROW2COL) || (MATRIX_TYPE == COL2ROW)
    init_pins(MATRIX_DRV_PINS, MATRIX_NUM_DRV, true, MATRIX_DRV_IDL, false);
    init_pins(MATRIX_SENSE_PINS, MATRIX_NUM_SENSE, false, 0, MATRIX_PULL_UP);
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

/**
 * @brief Unified eager debounce filter for both matrix switches and direct pins.
 * Immediately dispatches switch state transitions on contact (0ms latency),
 * then locks out physical chatter for DEBOUNCE_TICKS.
 */
static inline void debounce_filter(uint8_t *counter, bool *debounced, uint8_t col, uint8_t row, bool raw_state) {
    if (*counter > 0) {
        (*counter)--;
    } else if (raw_state != *debounced) {
        *debounced = raw_state;
        *counter = DEBOUNCE_TICKS;
        matrix_event_t event = {.split = 0, .col = col, .row = row, .pressed = raw_state ? 1 : 0};
        matrix_send_event(&event);
    }
}

#if (MATRIX_TYPE != DIRECT)
static inline void matrix_update_key(uint8_t col, uint8_t row, bool raw_state) {
    debounce_filter(&debounce_counters[col][row], &debounced_state[col][row], col, row, raw_state);
}
#endif

#if (MATRIX_TYPE == DIRECT)
static inline void matrix_update_direct_key(uint8_t key_idx, bool raw_state) {
    debounce_filter(&debounce_counters[key_idx], &debounced_state[key_idx], key_idx, 0, raw_state);
}
#endif

/**
 * @brief Performs a full scan of the keyboard matrix or direct pins.
 * Uses 1-cycle port snapshot to sample all sense pins simultaneously without bus overhead.
 */
void matrix_scan(void) {
#if (MATRIX_TYPE == ROW2COL) || (MATRIX_TYPE == COL2ROW)
    for (uint32_t d = 0; d < MATRIX_NUM_DRV; ++d) {
        hal_gpio_put(MATRIX_DRV_PINS[d], MATRIX_DRV_ACT);
        matrix_settle_delay();
        hal_gpio_snapshot_t snapshot = hal_gpio_snapshot();
        for (uint32_t s = 0; s < MATRIX_NUM_SENSE; ++s) {
            bool raw = MATRIX_READ_SNAPSHOT(snapshot, MATRIX_SENSE_PINS[s]);
            MATRIX_DISPATCH_KEY(d, s, raw);
        }
        hal_gpio_put(MATRIX_DRV_PINS[d], MATRIX_DRV_IDL);
    }
#elif (MATRIX_TYPE == MH3SS2)
    for (uint32_t i = 0; i < CUR_NUM_COLS; ++i) {
        hal_gpio_put(columns_gpios[i], 1);
        matrix_settle_delay();
        hal_gpio_snapshot_t snapshot = hal_gpio_snapshot();

        for (uint32_t j = 0; j < NUM_ROWS; ++j) {
            bool raw_state = false;
            uint32_t base_pin = j * MH3SS2_ROW_PINS_PER_KEY;
            for (uint32_t p = 0; p < MH3SS2_ROW_PINS_PER_KEY; ++p) {
                uint32_t pin_idx = base_pin + p;
                if (pin_idx < CUR_NUM_ROWS && hal_gpio_snapshot_get(snapshot, rows_gpios[pin_idx])) {
                    raw_state = true;
                    break;
                }
            }
            matrix_update_key((uint8_t)i, (uint8_t)j, raw_state);
        }

        hal_gpio_put(columns_gpios[i], 0);
    }
#elif (MATRIX_TYPE == DIRECT)
    hal_gpio_snapshot_t snapshot = hal_gpio_snapshot();
    for (uint32_t k = 0; k < CUR_NUM_KEYS; ++k) {
        matrix_update_direct_key((uint8_t)k, DIRECT_READ_SNAPSHOT(snapshot, direct_pins[k]));
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
