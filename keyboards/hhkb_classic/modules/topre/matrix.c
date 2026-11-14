#include "matrix.h"
#include "classic_matrix.h"
#include "queue.h"
#include "task.h"
#include "config.h"
#if defined(HHKB_SCAN_DIAGNOSTICS) || defined(HHKB_SCAN_1KHZ)
#include "stm32l072xx.h"
#endif

extern QueueHandle_t matrix_queue;
static matrix_row_t s_matrix_state[MATRIX_ROWS] = {0};
#ifdef HHKB_SCAN_1KHZ
static TaskHandle_t s_matrix_task;
static volatile uint16_t s_scan_period_ticks;
static volatile uint8_t s_scan_divider;
static uint8_t s_scan_phase;
static uint8_t s_scan_rate_applied;

static void update_scan_period(void) {
    static const uint16_t period_us[] = {10000, 8000, 4000, 2000, 950};
    uint8_t rate = hhkb_get_scan_rate();
    if (rate > 4) rate = 4;
    s_scan_rate_applied = rate;
    s_scan_period_ticks = period_us[rate] * HHKB_SCAN_TIMER_TICKS_PER_US;
    s_scan_divider = rate == 0 ? 2 : 1;
    s_scan_phase = 0;
}

void TIM2_IRQHandler(void) {
    if (!(TIM2->SR & TIM_SR_CC1IF)) return;
    TIM2->SR = (uint32_t)~TIM_SR_CC1IF;

    uint16_t next = (uint16_t)(TIM2->CCR1 + s_scan_period_ticks);
    TIM2->CCR1 = next;
    uint16_t remaining = (uint16_t)(next - (uint16_t)TIM2->CNT);
    if (remaining > s_scan_period_ticks) {
        // A delayed interrupt can miss a compare. Restart from the current count.
        TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
    }

    if (++s_scan_phase < s_scan_divider) return;
    s_scan_phase = 0;

    BaseType_t higher_priority_woken = pdFALSE;
    vTaskNotifyGiveFromISR(s_matrix_task, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}
#endif
#ifdef HHKB_SCAN_DIAGNOSTICS
static volatile uint32_t s_matrix_scan_count;
static volatile uint32_t s_matrix_scan_us;
static volatile uint32_t s_matrix_scan_max_us;
static volatile uint32_t s_matrix_period_us;
static volatile uint32_t s_matrix_work_max_us;
static volatile uint32_t s_matrix_queue_full;
static volatile uint32_t s_matrix_queue_peak;
#ifdef HHKB_SCAN_1KHZ
static volatile uint32_t s_matrix_scan_deadline_misses;
#endif
#endif

bool matrix_is_pressed(unsigned char row, unsigned char col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return false;
    return (s_matrix_state[row] & (1U << col)) != 0;
}

void matrix_task(void *pvParameters) {
    (void)pvParameters;

    if (!matrix_queue) {
        matrix_queue = xQueueCreate(QUEUE_DEF_SIZE, sizeof(matrix_event_t));
    }

#if defined(HHKB_SCAN_DIAGNOSTICS) || defined(HHKB_SCAN_1KHZ)
    // The platform sets APB1 to HCLK with no prescaling.
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
    TIM2->PSC = SystemCoreClock / (1000000U * HHKB_SCAN_TIMER_TICKS_PER_US) - 1U;
    TIM2->ARR = UINT16_MAX;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;
#endif
#ifdef HHKB_SCAN_DIAGNOSTICS
    uint16_t previous_scan_start = (uint16_t)TIM2->CNT;
#endif
#ifdef HHKB_SCAN_1KHZ
    s_matrix_task = xTaskGetCurrentTaskHandle();
    update_scan_period();
    TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
    TIM2->SR = 0;
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, (1U << __NVIC_PRIO_BITS) - 1U);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->DIER = TIM_DIER_CC1IE;
#endif
    matrix_row_t published_state[MATRIX_ROWS] = {0};
    for (;;) {
#ifdef HHKB_SCAN_1KHZ
        if (s_scan_rate_applied != hhkb_get_scan_rate()) {
            NVIC_DisableIRQ(TIM2_IRQn);
            update_scan_period();
            TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
            TIM2->SR = 0;
            NVIC_ClearPendingIRQ(TIM2_IRQn);
            NVIC_EnableIRQ(TIM2_IRQn);
        }
        uint32_t notifications = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#ifdef HHKB_SCAN_DIAGNOSTICS
        if (notifications > 1) s_matrix_scan_deadline_misses += notifications - 1;
#else
        (void)notifications;
#endif
#endif
#ifdef HHKB_SCAN_DIAGNOSTICS
        uint16_t scan_start = (uint16_t)TIM2->CNT;
        if (s_matrix_scan_count != 0) {
            s_matrix_period_us = (uint16_t)(scan_start - previous_scan_start) / HHKB_SCAN_TIMER_TICKS_PER_US;
        }
        previous_scan_start = scan_start;
#endif
        hhkb_matrix_scan_raw(s_matrix_state);
#ifdef HHKB_SCAN_DIAGNOSTICS
        uint32_t scan_us = (uint16_t)(TIM2->CNT - scan_start) / HHKB_SCAN_TIMER_TICKS_PER_US;
        s_matrix_scan_us = scan_us;
        if (scan_us > s_matrix_scan_max_us) s_matrix_scan_max_us = scan_us;
        s_matrix_scan_count++;
#endif

        bool first_event = true;
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t state = s_matrix_state[row];
            if (state == published_state[row]) continue;
            matrix_event_t event = {
                .row = MATRIX_EVENT_ROW_SNAPSHOT | (first_event ? MATRIX_EVENT_SCAN_START : 0) | row,
                .col = (uint8_t)state,
                .row_state_hi = (uint8_t)(state >> 8),
            };
            if (xQueueSend(matrix_queue, &event, 0) != pdPASS) {
#ifdef HHKB_SCAN_DIAGNOSTICS
                s_matrix_queue_full++;
#endif
                // Retry this row from its latest scan on the next pass.
                break;
            }
            published_state[row] = state;
            first_event = false;
        }

#ifdef HHKB_SCAN_DIAGNOSTICS
        if (!first_event) {
            UBaseType_t waiting = uxQueueMessagesWaiting(matrix_queue);
            if (waiting > s_matrix_queue_peak) s_matrix_queue_peak = waiting;
        }
        uint32_t work_us = (uint16_t)(TIM2->CNT - scan_start) / HHKB_SCAN_TIMER_TICKS_PER_US;
        if (work_us > s_matrix_work_max_us) s_matrix_work_max_us = work_us;
#endif
#ifdef HHKB_SCAN_1KHZ
        // Discard periods that elapsed during this pass so a late scan does not
        // consume the CPU time needed by the keyboard and USB tasks.
        uint32_t missed = ulTaskNotifyTake(pdTRUE, 0);
#ifdef HHKB_SCAN_DIAGNOSTICS
        s_matrix_scan_deadline_misses += missed;
#else
        (void)missed;
#endif
#else
        vTaskDelay(pdMS_TO_TICKS(1));
#endif
    }
}
