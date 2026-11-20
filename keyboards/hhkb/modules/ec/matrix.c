#include "matrix.h"
#include "classic_matrix.h"
#include "keyboard.h"
#include "queue.h"
#include "task.h"
#include "config.h"
#include "tusb.h"
#ifdef HHKB_HYBRID
#include "hybrid.h"
#endif
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
    static const uint16_t period_us[] = {10000, 8000, 4000, 2000, 1000, 5000};
    uint8_t rate = hhkb_get_effective_scan_rate();
    if (rate >= sizeof(period_us) / sizeof(period_us[0])) rate = 4;
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

#ifdef HHKB_HYBRID
// Called between scans, with the ADC idle. Keep TIM2 at 4 MHz and ADC
// at 16 MHz while reducing the CPU/bus clock on low-rate battery scans.
static void update_battery_clock(bool battery) {
    bool low = battery;
    uint32_t hz = low ? 16000000U : 32000000U;
    if (SystemCoreClock == hz) return;
    taskENTER_CRITICAL();
    uint16_t count = (uint16_t)TIM2->CNT;
    uint32_t timer_control = TIM2->CR1;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    // ADC clock selection may only change while disabled.
    ADC1->CR |= ADC_CR_ADDIS;
    while (ADC1->CR & ADC_CR_ADEN) {}
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE) |
                (low ? RCC_CFGR_HPRE_DIV2 : RCC_CFGR_HPRE_DIV1);
    SystemCoreClock = hz;
    TIM2->PSC = hz / (1000000U * HHKB_SCAN_TIMER_TICKS_PER_US) - 1U;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CNT = count;
    TIM2->SR &= ~TIM_SR_UIF;
    TIM2->CR1 = timer_control;
    ADC1->CFGR2 = (ADC1->CFGR2 & ~ADC_CFGR2_CKMODE) |
                 (low ? ADC_CFGR2_CKMODE : ADC_CFGR2_CKMODE_0);
    ADC1->ISR = ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}
    SysTick->LOAD = hz / configTICK_RATE_HZ - 1U;
    SysTick->VAL = 0;
    taskEXIT_CRITICAL();
}

static void publish_released_matrix(matrix_row_t published_state[MATRIX_ROWS]) {
    xQueueReset(matrix_queue);
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        s_matrix_state[row] = 0;
        matrix_event_t event = {
            .row = MATRIX_EVENT_ROW_SNAPSHOT |
                   (row == 0 ? MATRIX_EVENT_SCAN_START : 0) | row,
            .col = 0,
            .row_state_hi = 0,
        };
        xQueueSend(matrix_queue, &event, portMAX_DELAY);
        published_state[row] = 0;
    }
}
#endif

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
    bool sensor_power_down = false;
#ifdef HHKB_HYBRID
    hhkb_hybrid_init();
    TickType_t last_activity = xTaskGetTickCount();
    TickType_t last_wake_scan = last_activity;
    bool sleeping = false;
    bool manual_sleep = false;
    bool sleep_button_armed = true;
#endif
    for (;;) {
        bool battery = false;
#ifdef HHKB_HYBRID
        hhkb_hybrid_button_poll(xTaskGetTickCount());
        if (!hhkb_hybrid_powered()) {
            TIM2->DIER = 0;
            ulTaskNotifyTake(pdTRUE, 0);
            publish_released_matrix(published_state);
            ADC1->CR |= ADC_CR_ADDIS;
            GPIOC->BRR = (1U << 14) | (1U << 7);
            vTaskDelay(pdMS_TO_TICKS(300)); // Keep the orange power-off cue visible.
            hhkb_hybrid_enter_stop();
        }
        battery = hhkb_hybrid_on_battery();
        update_battery_clock(battery);
        bool button = hhkb_hybrid_button();
        TickType_t now = xTaskGetTickCount();
        uint8_t minutes = hhkb_bt_sleep_minutes();
        if (hhkb_hybrid_pairing() && minutes > 10) minutes = 10;
        if (hhkb_hybrid_sleep_requested()) {
            hhkb_hybrid_clear_sleep_request();
            if (battery) manual_sleep = true;
        }
        if (!battery || button || minutes == 0 || hhkb_is_calibrating()) {
            last_activity = now;
        }
        if (!sleeping && battery && (manual_sleep || (minutes != 0 &&
            now - last_activity >= pdMS_TO_TICKS((uint32_t)minutes * 60000U)))) {
            sleeping = true;
            hhkb_hybrid_set_sleeping(true);
            sleep_button_armed = !button;
            TIM2->DIER = 0;
            ulTaskNotifyTake(pdTRUE, 0);
            ADC1->CR |= ADC_CR_ADDIS;
            GPIOC->BRR = (1U << 14) | (1U << 7);
            last_wake_scan = now;
            if (!hhkb_bt_matrix_wake()) {
                publish_released_matrix(published_state);
                for (uint8_t i = 0; i < 40; ++i) {
                    hhkb_hybrid_service();
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
                hhkb_hybrid_enter_stop();
            }
        }
        if (sleeping) {
            if (!button) sleep_button_armed = true;
            bool wake = !battery || (sleep_button_armed && button) ||
                        (!manual_sleep && minutes == 0);
            bool scan = hhkb_bt_matrix_wake() && now - last_wake_scan >= pdMS_TO_TICKS(1000);
            if (!wake && !scan) {
                vTaskDelay(pdMS_TO_TICKS(20));
                continue;
            }
            last_wake_scan = now;
            GPIOC->BSRR = 1U << 14;
            ADC1->ISR = ADC_ISR_ADRDY;
            ADC1->CR |= ADC_CR_ADEN;
            vTaskDelay(pdMS_TO_TICKS(10));
            if (!wake) {
                matrix_row_t probe[MATRIX_ROWS] = {0};
                hhkb_matrix_scan_raw(probe);
                for (unsigned row = 0; row < MATRIX_ROWS; ++row) {
                    if (probe[row]) wake = true;
                }
            }
            if (!wake) {
                ADC1->CR |= ADC_CR_ADDIS;
                GPIOC->BRR = (1U << 14) | (1U << 7);
                continue;
            }
            sleeping = false;
            hhkb_hybrid_set_sleeping(false);
            manual_sleep = false;
            if (button) hhkb_hybrid_suppress_button_release();
            last_activity = xTaskGetTickCount();
            update_scan_period();
            TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
            TIM2->SR = 0;
            TIM2->DIER = TIM_DIER_CC1IE;
        }
        hhkb_hybrid_service();
#endif
        // Check power saving condition: host disconnected (not mounted) or USB suspended
        bool usb_power_saving = !battery && keyboard_power_saving_enabled() &&
                                (!tud_mounted() || tud_suspended());
#ifdef HHKB_HYBRID
        usb_power_saving = usb_power_saving && !hhkb_hybrid_output_bluetooth();
#endif
        if (usb_power_saving) {
            if (!sensor_power_down) {
                // Disable power for capacitive sensor board and OpAmp
                GPIOC->BRR = (1U << 14) | (1U << 7);
                sensor_power_down = true;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        } else if (sensor_power_down) {
            // Restore sensor board and OpAmp power when USB host is active
            GPIOC->BSRR = (1U << 14) | (1U << 7);
            vTaskDelay(pdMS_TO_TICKS(10)); // Settle analog power
            sensor_power_down = false;
        }

#ifdef HHKB_SCAN_1KHZ
        if (s_scan_rate_applied != hhkb_get_effective_scan_rate()) {
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
#ifdef HHKB_HYBRID
            if (state || state != published_state[row]) last_activity = xTaskGetTickCount();
#endif
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
        // Drop overdue timer notifications to prevent
        // starvation of USB and keyboard tasks.
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

#ifdef HHKB_SCAN_DIAGNOSTICS
void hhkb_get_scan_diagnostics(uint32_t *scan_us, uint32_t *scan_max_us, uint32_t *work_max_us, uint32_t *missed, uint32_t *q_full) {
    if (scan_us) *scan_us = s_matrix_scan_us;
    if (scan_max_us) *scan_max_us = s_matrix_scan_max_us;
    if (work_max_us) *work_max_us = s_matrix_work_max_us;
#ifdef HHKB_SCAN_1KHZ
    if (missed) *missed = s_matrix_scan_deadline_misses;
#else
    if (missed) *missed = 0;
#endif
    if (q_full) *q_full = s_matrix_queue_full;
}

void hhkb_reset_scan_diagnostics_peaks(void) {
    s_matrix_scan_max_us = 0;
    s_matrix_work_max_us = 0;
}
#endif
