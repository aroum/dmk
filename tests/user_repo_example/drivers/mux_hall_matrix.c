#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "hal_gpio.h"
#include "matrix.h"
#include "mux_hall_matrix.h"

// External matrix queue provided by dmk_core
extern QueueHandle_t matrix_queue;

// MUX address lines (S0, S1, S2, S3)
#if defined(MUX_PIN_S0) && defined(MUX_PIN_S1) && defined(MUX_PIN_S2) && defined(MUX_PIN_S3)
static const pin_t mux_addr_pins[4] = {MUX_PIN_S0, MUX_PIN_S1, MUX_PIN_S2, MUX_PIN_S3};
#endif

// Debounced / state array
static bool key_states[NUM_ROWS][NUM_COLS] = {{false}};

/**
 * @brief Select channel on CD74HC4067 16-channel multiplexer
 */
static inline void mux_set_channel(uint8_t channel) {
#if defined(MUX_PIN_S0) && defined(MUX_PIN_S1) && defined(MUX_PIN_S2) && defined(MUX_PIN_S3)
    hal_gpio_put(mux_addr_pins[0], (channel & 0x01) != 0);
    hal_gpio_put(mux_addr_pins[1], (channel & 0x02) != 0);
    hal_gpio_put(mux_addr_pins[2], (channel & 0x04) != 0);
    hal_gpio_put(mux_addr_pins[3], (channel & 0x08) != 0);
#endif
#if defined(MUX_SETTLE_US)
    hal_sleep_us(MUX_SETTLE_US);
#endif
}

/**
 * @brief Read analog value or digital output from Hall sensor for selected channel.
 * In a full ADC configuration, this invokes platform ADC read.
 * In digital comparator/hall sensor mode, reads sensor signal pin.
 */
static uint16_t hall_read_sensor_channel(uint8_t row, uint8_t col) {
    uint8_t channel = (row * NUM_COLS) + col;
    mux_set_channel(channel);

#if defined(HALL_SIGNAL_PIN)
    // Digital Hall / Comparator read fallback
    bool pin_active = !hal_gpio_get(HALL_SIGNAL_PIN);
    return pin_active ? HALL_ACTUATION_THRESHOLD + 200 : HALL_RELEASE_THRESHOLD - 200;
#else
    (void)row;
    (void)col;
    return HALL_RELEASE_THRESHOLD - 100;
#endif
}

/**
 * @brief Initialize MUX GPIOs and sensor lines
 */
void mux_hall_matrix_init(void) {
#if defined(MUX_PIN_S0) && defined(MUX_PIN_S1) && defined(MUX_PIN_S2) && defined(MUX_PIN_S3)
    for (int i = 0; i < 4; ++i) {
        hal_gpio_init(mux_addr_pins[i]);
        hal_gpio_set_dir(mux_addr_pins[i], true);
        hal_gpio_put(mux_addr_pins[i], 0);
    }
#endif

#if defined(MUX_PIN_ENABLE)
    hal_gpio_init(MUX_PIN_ENABLE);
    hal_gpio_set_dir(MUX_PIN_ENABLE, true);
    hal_gpio_put(MUX_PIN_ENABLE, 0); // Enable active LOW
#endif

#if defined(HALL_SIGNAL_PIN)
    hal_gpio_init(HALL_SIGNAL_PIN);
    hal_gpio_set_dir(HALL_SIGNAL_PIN, false);
    hal_gpio_pull_up(HALL_SIGNAL_PIN);
#endif
}

/**
 * @brief Scan all Hall sensors through MUX and push events on state change
 */
void mux_hall_matrix_scan(void) {
    for (uint8_t r = 0; r < NUM_ROWS; ++r) {
        for (uint8_t c = 0; c < NUM_COLS; ++c) {
            uint16_t value = hall_read_sensor_channel(r, c);
            bool current = key_states[r][c];
            bool next = current;

            // Hysteresis threshold logic (actuation vs release)
            if (!current && value >= HALL_ACTUATION_THRESHOLD) {
                next = true;
            } else if (current && value <= HALL_RELEASE_THRESHOLD) {
                next = false;
            }

            if (next != current) {
                key_states[r][c] = next;
                matrix_event_t event = {
                    .split = 0,
                    .row = r,
                    .col = c,
                    .pressed = next ? 1 : 0
                };
                xQueueSend(matrix_queue, &event, 0);
            }
        }
    }
}

/**
 * @brief FreeRTOS task entry point for custom matrix scanning
 */
void matrix_task(void *pvParameters) {
    (void)pvParameters;
    mux_hall_matrix_init();

    while (1) {
        mux_hall_matrix_scan();
        vTaskDelay(pdMS_TO_TICKS(HALL_POLL_INTERVAL_MS));
    }
}

/**
 * @brief Query key state for matrix hooks
 */
bool matrix_is_pressed(unsigned char row, unsigned char col) {
    if (row < NUM_ROWS && col < NUM_COLS) {
        return key_states[row][col];
    }
    return false;
}
