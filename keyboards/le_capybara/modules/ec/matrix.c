#include "matrix.h"
#include "capybara_matrix.h"
#include "ec_settings.h"
#include "keyboard.h"
#include "queue.h"
#include "task.h"
#include "config.h"
#include "encoder.h"
#include "tusb.h"
#include "stm32f072xb.h"

extern QueueHandle_t matrix_queue;
static matrix_row_t s_matrix_state[MATRIX_ROWS] = {0};

bool matrix_is_pressed(unsigned char row, unsigned char col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return false;
    return (s_matrix_state[row] & (1U << col)) != 0;
}

void matrix_task(void *pvParameters) {
    (void)pvParameters;

    if (!matrix_queue) {
        matrix_queue = xQueueCreate(QUEUE_DEF_SIZE, sizeof(matrix_event_t));
    }

    capybara_matrix_hardware_init();
    encoder_init();

    matrix_row_t published_state[MATRIX_ROWS] = {0};
    static const TickType_t scan_periods_ms[] = {20, 8, 4, 2, 1};

    for (;;) {
        // Also reached while power saving.
        hhkb_calibration_apply_pending();
        // Check power saving condition: host disconnected (not mounted) or USB suspended
        if (keyboard_power_saving_enabled() && (!tud_mounted() || tud_suspended())) {
            // Disable both MUXes to minimize current draw
            GPIOA->BSRR = (1U << 8);  // EN0 = HIGH (disabled)
            GPIOC->BSRR = (1U << 14); // EN1 = HIGH (disabled)
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // 2. Scan EC capacitive and discrete key matrix
        hhkb_calibration_apply_pending();
        capybara_matrix_scan_raw(s_matrix_state);

        // Keep VIA's matrix tester live, but suppress EC key reports during calibration.
        bool calibrating = hhkb_is_calibrating();

        // 3. Scan rotary encoder
        encoder_scan();

        // 4. Dispatch changed row snapshots to DMK event queue
        bool first_event = true;
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t state = calibrating && row < EC_ROWS ? 0 : s_matrix_state[row];
            if (state == published_state[row]) continue;

            matrix_event_t event = {
                .row = MATRIX_EVENT_ROW_SNAPSHOT | (first_event ? MATRIX_EVENT_SCAN_START : 0) | row,
                .col = (uint8_t)state,
                .row_state_hi = (uint8_t)(state >> 8),
            };

            if (xQueueSend(matrix_queue, &event, 0) != pdPASS) {
                break;
            }

            published_state[row] = state;
            first_event = false;
        }

        vTaskDelay(pdMS_TO_TICKS(scan_periods_ms[hhkb_get_scan_rate()]));
    }
}
