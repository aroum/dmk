#include "FreeRTOS.h"
#include "config.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hall_calibration.h"
#include "matrix.h"
#include "queue.h"
#include "task.h"

#include <stdbool.h>
#include <stdint.h>

extern QueueHandle_t matrix_queue;

// Multiplexer channel mapping table for keys 1..8
static const uint8_t mux_channel_lut[8] = MUX_CHANNEL_MAP;

// Debounced switch states for the 10 keys
static bool key_states[NUM_KEYS] = {false};

/**
 * @brief Switch SN74LV4052A multiplexer to a given channel
 * @param channel 0-indexed channel (0..7)
 */
static inline void sn74lv4052a_select_channel(uint8_t channel) {
    // S0, S1 address select
    hal_gpio_put(MUX_PIN_S0, (channel & 0x01) != 0);
    hal_gpio_put(MUX_PIN_S1, (channel & 0x02) != 0);
    
    // Enable active low INH
    hal_gpio_put(MUX_PIN_INH, 0);

#if defined(MUX_SETTLE_US)
    hal_sleep_us(MUX_SETTLE_US);
#endif
}

/**
 * @brief Initialize ADC and GPIO peripherals for Hall effect scanning
 */
static void magneteno_matrix_init(void) {
    // 1. Initialize MUX control GPIOs
    hal_gpio_init(MUX_PIN_S0);
    hal_gpio_set_dir(MUX_PIN_S0, true);
    hal_gpio_put(MUX_PIN_S0, 0);

    hal_gpio_init(MUX_PIN_S1);
    hal_gpio_set_dir(MUX_PIN_S1, true);
    hal_gpio_put(MUX_PIN_S1, 0);

    hal_gpio_init(MUX_PIN_INH);
    hal_gpio_set_dir(MUX_PIN_INH, true);
    hal_gpio_put(MUX_PIN_INH, 0); // Active low

    // 2. Initialize ADC inputs via universal HAL ADC
    hal_adc_init(MUX_ADC_PIN);
    hal_adc_init(DIRECT_KEY9_PIN);
    hal_adc_init(DIRECT_KEY10_PIN);

    // 3. Initialize Hall Calibration and Rapid Trigger engine
    hall_calibration_init(NUM_KEYS);
}

/**
 * @brief Process single key press/release event using Rapid Trigger calibration engine
 */
static inline void update_key_state(uint8_t key_idx, uint16_t adc_val) {
    bool current = key_states[key_idx];
    bool next = hall_process_sample(key_idx, adc_val);

    if (next != current) {
        key_states[key_idx] = next;
        matrix_event_t event = {
            .split = 0,
            .row = 0,
            .col = key_idx,
            .pressed = next ? 1 : 0
        };
        xQueueSend(matrix_queue, &event, 0);
    }
}

/**
 * @brief Scan all 10 Hall sensors (8 via MUX + 2 direct)
 */
static void magneteno_matrix_scan(void) {
    // 1. Scan 8 keys connected via SN74LV4052A Multiplexer -> MUX_ADC_PIN
    for (uint8_t i = 0; i < 8; ++i) {
        sn74lv4052a_select_channel(mux_channel_lut[i]);
        uint16_t val = hal_adc_read(MUX_ADC_PIN);
        update_key_state(i, val);
    }

    // 2. Scan Key 9 directly on DIRECT_KEY9_PIN
    uint16_t val_key9 = hal_adc_read(DIRECT_KEY9_PIN);
    update_key_state(8, val_key9);

    // 3. Scan Key 10 directly on DIRECT_KEY10_PIN
    uint16_t val_key10 = hal_adc_read(DIRECT_KEY10_PIN);
    update_key_state(9, val_key10);
}

/**
 * @brief FreeRTOS task handling periodic matrix scanning
 */
void matrix_task(void *pvParameters) {
    (void)pvParameters;
    magneteno_matrix_init();

    while (1) {
        magneteno_matrix_scan();
        vTaskDelay(pdMS_TO_TICKS(1)); // 1000Hz polling rate
    }
}

/**
 * @brief Matrix state lookup
 */
bool matrix_is_pressed(unsigned char row, unsigned char col) {
    (void)row;
    if (col < NUM_KEYS) {
        return key_states[col];
    }
    return false;
}
