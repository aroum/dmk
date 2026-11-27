#include "matrix.h"
#include "classic_matrix.h"
#include "keyboard.h"
#include "queue.h"
#include "task.h"
#include "task_internal.h"
#include "config.h"
#include "tusb.h"
#include "usb.h"
#include "sof_lock.h"
#ifdef HHKB_HYBRID
#include "hybrid.h"
#endif
#include "stm32l072xx.h"

extern QueueHandle_t matrix_queue;

// Acquisition runs one level above the shared task priority when a level is
// available; the task drops back between scans (see the loop in matrix_task).
#if TASK_PRIO_MATRIX + 1 < configMAX_PRIORITIES
#define MATRIX_SCAN_PRIO (TASK_PRIO_MATRIX + 1)
#else
#define MATRIX_SCAN_PRIO TASK_PRIO_MATRIX
#endif

static matrix_row_t s_matrix_state[MATRIX_ROWS] = {0};
static TaskHandle_t s_matrix_task;

void keyboard_usb_resume_cb(void) {
    if (s_matrix_task != NULL) xTaskNotifyGive(s_matrix_task);
}
static volatile uint16_t s_scan_period_ticks;
static volatile uint8_t s_scan_divider;
static uint8_t s_scan_phase;
static uint8_t s_scan_rate_applied;
static bool s_scan_calibrating;

static void update_scan_period(void) {
    static const uint16_t period_us[] = {10000, 8000, 4000, 2000, 1000, 5000, 11000};
    uint8_t rate = hhkb_get_effective_scan_rate();
    if (rate >= sizeof(period_us) / sizeof(period_us[0])) rate = 4;
    s_scan_rate_applied = rate;
    s_scan_period_ticks = period_us[rate] * HHKB_SCAN_TIMER_TICKS_PER_US;
    s_scan_calibrating = hhkb_is_calibrating();
    // Leave a complete USB service window during calibration at 1 kHz.
    s_scan_divider = rate == 0 || (rate == 4 && s_scan_calibrating) ? 2 : 1;
    s_scan_phase = 0;
}

void TIM2_IRQHandler(void) {
    if (!(TIM2->SR & TIM_SR_CC1IF)) return;
    TIM2->SR = (uint32_t)~TIM_SR_CC1IF;

    uint16_t fired = (uint16_t)TIM2->CCR1;
    uint16_t step = s_scan_period_ticks;
#if HHKB_SOF_LOCK
    step = sof_lock_step(fired, step);
#elif defined(HHKB_SCAN_DIAGNOSTICS)
    sof_lock_note_fire(fired);
#endif
    uint16_t next = (uint16_t)(fired + step);
    TIM2->CCR1 = next;
    uint16_t remaining = (uint16_t)(next - (uint16_t)TIM2->CNT);
    if (remaining > step) {
        // A delayed interrupt can miss a compare. Restart from the current count.
        TIM2->CCR1 = (uint16_t)(TIM2->CNT + step);
    }

    if (++s_scan_phase < s_scan_divider) return;
    s_scan_phase = 0;

    BaseType_t higher_priority_woken = pdFALSE;
    vTaskNotifyGiveFromISR(s_matrix_task, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}
#ifdef HHKB_SCAN_DIAGNOSTICS
static volatile uint32_t s_rt_rebuild_ms;
static volatile uint32_t s_rt_rebuild_count;

void hhkb_note_rt_rebuild(uint32_t ms) {
    s_rt_rebuild_ms = ms;
    s_rt_rebuild_count++;
}

void hhkb_get_rt_rebuild(uint32_t *ms, uint32_t *count) {
    if (ms) *ms = s_rt_rebuild_ms;
    if (count) *count = s_rt_rebuild_count;
}

static volatile uint32_t s_matrix_scan_count;
static volatile uint32_t s_matrix_scan_us;
static volatile uint32_t s_matrix_scan_max_us;
static volatile uint32_t s_matrix_period_us;
static volatile uint32_t s_matrix_work_max_us;
static volatile uint32_t s_matrix_queue_full;
static volatile uint32_t s_matrix_queue_peak;
static volatile uint32_t s_matrix_scan_deadline_misses;
#endif

bool matrix_is_pressed(unsigned char row, unsigned char col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return false;
    return (s_matrix_state[row] & (1U << col)) != 0;
}

#ifdef HHKB_HYBRID
// Called between scans. Battery uses stock-style 4 MHz CPU / 1 MHz ADC;
// TIM2 retains its 4 MHz counter for analog delays and scan scheduling.
static void update_battery_clock(bool battery) {
    bool low = battery;
    uint32_t hz = low ? 4000000U : 32000000U;
    if (SystemCoreClock == hz) return;
    taskENTER_CRITICAL();
    uint16_t count = (uint16_t)TIM2->CNT;
    uint32_t timer_control = TIM2->CR1;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    // ADC clock selection may only change while disabled.
    ADC1->CR |= ADC_CR_ADDIS;
    while (ADC1->CR & ADC_CR_ADEN) {}
    // Change PLL configuration only while running from HSI and PLL is off.
    // Battery matches the analyzed stock tree: HSI16/4 *4 /4 = 4 MHz.
    // USB restores the original DMK HSI16 *4 /2 = 32 MHz tree.
    FLASH->ACR |= FLASH_ACR_LATENCY;
    while (!(FLASH->ACR & FLASH_ACR_LATENCY)) {}
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) {}
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {}
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY) {}
    if (low) {
        RCC->CR |= RCC_CR_HSIDIVEN;
        while (!(RCC->CR & RCC_CR_HSIDIVF)) {}
    } else {
        RCC->CR &= ~RCC_CR_HSIDIVEN;
        while (RCC->CR & RCC_CR_HSIDIVF) {}
    }
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PLLSRC |
                               RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV)) |
                RCC_CFGR_PLLMUL4 | (low ? RCC_CFGR_PLLDIV4 : RCC_CFGR_PLLDIV2);
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {}
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}
    RCC->CR &= ~RCC_CR_MSION;
    if (low) FLASH->ACR &= ~FLASH_ACR_LATENCY;
    SystemCoreClock = hz;
    TIM2->PSC = hz / (1000000U * HHKB_SCAN_TIMER_TICKS_PER_US) - 1U;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CNT = count;
    TIM2->SR &= ~TIM_SR_UIF;
    TIM2->CR1 = timer_control;
    ADC1->CFGR2 = (ADC1->CFGR2 & ~ADC_CFGR2_CKMODE) |
                 (low ? ADC_CFGR2_CKMODE_1 : ADC_CFGR2_CKMODE_0);
    if (low) ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;
    else ADC1->CFGR1 &= ~ADC_CFGR1_AUTOFF;
    if (!low) {
        ADC1->ISR = ADC_ISR_ADRDY;
        ADC1->CR |= ADC_CR_ADEN;
        while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}
    }
    vPortUpdateSysTickClock(hz);
#if HHKB_SOF_LOCK
    sof_lock_reset();
#endif
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

    // main creates and checks this queue before starting the scheduler.
    configASSERT(matrix_queue != NULL);

    // The platform sets APB1 to HCLK with no prescaling.
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
    TIM2->PSC = SystemCoreClock / (1000000U * HHKB_SCAN_TIMER_TICKS_PER_US) - 1U;
    TIM2->ARR = UINT16_MAX;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;
#ifdef HHKB_SCAN_DIAGNOSTICS
    uint16_t previous_scan_start = (uint16_t)TIM2->CNT;
#endif
    s_matrix_task = xTaskGetCurrentTaskHandle();
    update_scan_period();
    TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
    TIM2->SR = 0;
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, (1U << __NVIC_PRIO_BITS) - 1U);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->DIER = TIM_DIER_CC1IE;
#if HHKB_SOF_LOCK
    sof_lock_init();
#endif
    matrix_row_t published_state[MATRIX_ROWS] = {0};
    bool sensor_power_down = false;
#ifdef HHKB_HYBRID
    hhkb_hybrid_init();
    hhkb_hybrid_set_wake_task(xTaskGetCurrentTaskHandle());
    TickType_t last_activity = xTaskGetTickCount();
    TickType_t last_wake_scan = last_activity;
    bool sleeping = false;
    bool manual_sleep = false;
    bool critical_sleep = false;
    bool off_suspended = false;
    bool sleep_battery = false;
    TickType_t sleep_started = last_activity;
#endif
    for (;;) {
#if HHKB_SOF_LOCK
        if (!tud_mounted() || tud_suspended()) sof_lock_reset();
#endif
        // Also reached while sleeping/powered off/power saving.
        hhkb_calibration_apply_pending();
        bool battery = false;
#ifdef HHKB_HYBRID
        hhkb_hybrid_critical_battery_poll(xTaskGetTickCount());
        if (hhkb_hybrid_powered()) hhkb_hybrid_button_poll(xTaskGetTickCount());
        if (!hhkb_hybrid_powered()) {
            if (!off_suspended) {
                TIM2->DIER = 0;
                TIM2->CR1 &= ~TIM_CR1_CEN;
                ulTaskNotifyTake(pdTRUE, 0);
                publish_released_matrix(published_state);
                ADC1->CR |= ADC_CR_ADDIS;
                GPIOC->BRR = (1U << 14) | (1U << 7) | (1U << 8) | (1U << 6);
                GPIOB->BRR = 0xFC07U;
                hhkb_hybrid_sleep_exti(true);
                off_suspended = true;
            }
            update_battery_clock(hhkb_hybrid_on_battery());
            hhkb_hybrid_off_service(xTaskGetTickCount());
            if (!hhkb_hybrid_powered() && hhkb_hybrid_on_battery())
                hhkb_hybrid_enter_stop();
            if (!hhkb_hybrid_powered()) {
                // USB soft-OFF retains TinyUSB and services PC5/SPI at 20 Hz.
                ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));
                continue;
            }
            hhkb_hybrid_sleep_exti(false);
            hhkb_hybrid_set_sleeping(false);
            hhkb_hybrid_wake_radio();
            sleeping = manual_sleep = critical_sleep = off_suspended = false;
            last_activity = xTaskGetTickCount();
            GPIOC->BSRR = 1U << 14;
            if (!(ADC1->CFGR1 & ADC_CFGR1_AUTOFF)) {
                ADC1->ISR = ADC_ISR_ADRDY;
                ADC1->CR |= ADC_CR_ADEN;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            update_scan_period();
            TIM2->CR1 |= TIM_CR1_CEN;
            TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
            TIM2->SR = 0;
            TIM2->DIER = TIM_DIER_CC1IE;
            hhkb_hybrid_refresh_indicators();
            continue;
        }
        battery = hhkb_hybrid_on_battery();
        update_battery_clock(battery);
        bool button = hhkb_hybrid_button();
        TickType_t now = xTaskGetTickCount();
        uint8_t minutes = hhkb_bt_sleep_minutes();
        if (hhkb_hybrid_pairing() && minutes > 10) minutes = 10;
        if (hhkb_hybrid_sleep_requested()) {
            hhkb_hybrid_clear_sleep_request();
            manual_sleep = true;
            if (sleeping) sleep_started = now;
        }
        if (button || minutes == 0 || hhkb_is_calibrating()) {
            last_activity = now;
        }
        bool critical = hhkb_hybrid_critical_battery();
        bool disconnected_due = minutes != 0 && !hhkb_is_calibrating() &&
                                hhkb_hybrid_disconnected_sleep_due(now);
        if (!sleeping && (critical || manual_sleep || disconnected_due || (minutes != 0 &&
            now - last_activity >= pdMS_TO_TICKS((uint32_t)minutes * 60000U)))) {
            // Without key wake there is no intermediate sleep state.
            // Critical-battery warning keeps its separate, USB-only safety path.
            if (!critical && !hhkb_bt_matrix_wake()) {
                hhkb_hybrid_toggle_power();
                continue;
            }
            sleeping = true;
            sleep_started = now;
            sleep_battery = battery;
            hhkb_hybrid_set_sleeping(true);
            TIM2->CR1 &= ~TIM_CR1_CEN;
            TIM2->DIER = 0;
            ulTaskNotifyTake(pdTRUE, 0);
            hhkb_hybrid_sleep_exti(true);
            ADC1->CR |= ADC_CR_ADDIS;
            GPIOC->BRR = (1U << 14) | (1U << 7);
            last_wake_scan = now;
            // Nordic idle is independent of the optional matrix wake probes.
            publish_released_matrix(published_state);
            if (!critical) hhkb_hybrid_sleep_radio_idle();
        }
        if (sleeping) {
            bool button_wake = hhkb_hybrid_take_button_wake();
            bool critical_recovered = critical_sleep && !critical;
            critical_sleep = critical;
            bool radio_wake = hhkb_hybrid_take_link_wake();
            bool wake = battery != sleep_battery ||
                        (!critical && (button_wake || radio_wake)) || critical_recovered;
            uint8_t deep_minutes = hhkb_bt_deep_minutes();
            bool deep_due = battery && !critical && deep_minutes && now - sleep_started >= pdMS_TO_TICKS((uint32_t)deep_minutes * 60000U);
            hhkb_hybrid_service();
            if (!critical && !wake && !hhkb_bt_matrix_wake()) {
                hhkb_hybrid_toggle_power();
                continue;
            }
            TickType_t probe_period = pdMS_TO_TICKS(1000U / hhkb_bt_sleep_hz());
            bool scan = !critical && hhkb_bt_matrix_wake() && now - last_wake_scan >= probe_period;
            if (!wake && !button && deep_due && !scan) { hhkb_hybrid_toggle_power(); continue; }
            if (!wake && !scan) {
                TickType_t elapsed = now - last_wake_scan;
                TickType_t remaining = (critical || !hhkb_bt_matrix_wake()) ? pdMS_TO_TICKS(1000) :
                    (elapsed < probe_period ? probe_period - elapsed : 1);
                uint32_t button_wait = hhkb_hybrid_button_wait_ms(now);
                if (button_wait != UINT32_MAX && pdMS_TO_TICKS(button_wait) < remaining)
                    remaining = pdMS_TO_TICKS(button_wait);
                ulTaskNotifyTake(pdTRUE, remaining);
                continue;
            }
            last_wake_scan = now;
            TIM2->CR1 |= TIM_CR1_CEN;
            GPIOC->BSRR = 1U << 14;
            if (!(ADC1->CFGR1 & ADC_CFGR1_AUTOFF)) {
                ADC1->ISR = ADC_ISR_ADRDY;
                ADC1->CR |= ADC_CR_ADEN;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            if (!wake) {
                matrix_row_t probe[MATRIX_ROWS] = {0};
                hhkb_matrix_scan_raw(probe);
                for (unsigned row = 0; row < MATRIX_ROWS; ++row) {
                    if (probe[row]) wake = true;
                }
            }
            if (!wake) {
                if (!button && deep_due) { hhkb_hybrid_toggle_power(); continue; }
                TIM2->CR1 &= ~TIM_CR1_CEN;
                ADC1->CR |= ADC_CR_ADDIS;
                GPIOC->BRR = (1U << 14) | (1U << 7);
                continue;
            }
            sleeping = false;
            critical_sleep = false;
            hhkb_hybrid_sleep_exti(false);
            hhkb_hybrid_set_sleeping(false);
            hhkb_hybrid_wake_radio();
            manual_sleep = false;
            last_activity = xTaskGetTickCount();
            update_scan_period();
            TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
            TIM2->SR = 0;
            TIM2->DIER = TIM_DIER_CC1IE;
        }
        hhkb_hybrid_service();
#endif
        // Check power saving condition: host disconnected (not mounted) or USB suspended without remote wakeup
        bool remote_wake_suspend = tud_mounted() && tud_suspended() && usb_remote_wakeup_enabled();
        bool usb_power_saving = !battery && keyboard_power_saving_enabled() &&
                                (!tud_mounted() || (tud_suspended() && !remote_wake_suspend));
#ifdef HHKB_HYBRID
        usb_power_saving = usb_power_saving && !hhkb_hybrid_output_bluetooth();
#endif
        if (usb_power_saving) {
            if (!sensor_power_down) {
                // Stop scan interrupts as well as the analog hardware.
                TIM2->DIER &= ~TIM_DIER_CC1IE;
                TIM2->SR = 0;
                NVIC_ClearPendingIRQ(TIM2_IRQn);
                (void)ulTaskNotifyTake(pdTRUE, 0);
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
#if HHKB_SOF_LOCK
            sof_lock_reset();
#endif
            update_scan_period();
            TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
            TIM2->SR = 0;
            NVIC_ClearPendingIRQ(TIM2_IRQn);
            NVIC_EnableIRQ(TIM2_IRQn);
            TIM2->DIER |= TIM_DIER_CC1IE;
            TIM2->CR1 |= TIM_CR1_CEN;
        }

        if (s_scan_rate_applied != hhkb_get_effective_scan_rate() ||
            s_scan_calibrating != hhkb_is_calibrating()) {
            NVIC_DisableIRQ(TIM2_IRQn);
            update_scan_period();
            TIM2->CCR1 = (uint16_t)(TIM2->CNT + s_scan_period_ticks);
            TIM2->SR = 0;
            NVIC_ClearPendingIRQ(TIM2_IRQn);
            NVIC_EnableIRQ(TIM2_IRQn);
        }
        // Wait at elevated priority so the scan fire preempts same-priority
        // USB/key work immediately: at equal priority the wake waits for the
        // running peer to block, which measured up to ~0.9 ms of added
        // fire-to-wake latency and a missed deadline whenever the remainder
        // of the frame could not absorb it. The priority drops again after
        // the scan publishes, so consumers run every loop - including during
        // calibration overruns, where the task never blocks.
#if MATRIX_SCAN_PRIO != TASK_PRIO_MATRIX
        vTaskPrioritySet(NULL, MATRIX_SCAN_PRIO);
#endif
        uint32_t notifications = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#if HHKB_SOF_LOCK
        // Latch the fire that started this scan before the next one can
        // rewrite it mid-scan; includes the fire-to-wake latency in the work.
        uint16_t sof_fire = sof_lock_last_fire();
#endif
#ifdef HHKB_SCAN_DIAGNOSTICS
        if (notifications > 1) s_matrix_scan_deadline_misses += notifications - 1;
#else
        (void)notifications;
#endif
#ifdef HHKB_SCAN_DIAGNOSTICS
        uint16_t scan_start = (uint16_t)TIM2->CNT;
        if (s_matrix_scan_count != 0) {
            s_matrix_period_us = (uint16_t)(scan_start - previous_scan_start) / HHKB_SCAN_TIMER_TICKS_PER_US;
        }
        previous_scan_start = scan_start;
#endif
        hhkb_calibration_apply_pending();
        hhkb_matrix_scan_raw(s_matrix_state);
#ifdef HHKB_SCAN_DIAGNOSTICS
        uint32_t scan_us = (uint16_t)(TIM2->CNT - scan_start) / HHKB_SCAN_TIMER_TICKS_PER_US;
        s_matrix_scan_us = scan_us;
        if (scan_us > s_matrix_scan_max_us) s_matrix_scan_max_us = scan_us;
        s_matrix_scan_count++;
#endif

        // VIA reads s_matrix_state directly. Keep that physical state live,
        // but publish releases to the key-processing queue during calibration.
        bool calibrating = hhkb_is_calibrating();
        bool first_event = true;
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t state = calibrating ? 0 : s_matrix_state[row];
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
#ifdef HHKB_SCAN_DIAGNOSTICS
        if (!first_event) sof_lock_note_publish();
#endif
#if HHKB_SOF_LOCK
        sof_lock_note_work(sof_fire);
#endif
#if MATRIX_SCAN_PRIO != TASK_PRIO_MATRIX
        vTaskPrioritySet(NULL, TASK_PRIO_MATRIX);
#endif
        // Keep the next timer notification. Discarding it here forces a
        // nearly-1-ms scan to wait for the following tick, halving throughput.
        // HHKB shares priority with USB/key processing, so this yield gives
        // ready consumers time even when acquisition overruns its period.
        taskYIELD();
    }
}

#ifdef HHKB_SCAN_DIAGNOSTICS
void hhkb_get_scan_diagnostics(uint32_t *scan_us, uint32_t *scan_max_us, uint32_t *work_max_us, uint32_t *missed, uint32_t *q_full) {
    if (scan_us) *scan_us = s_matrix_scan_us;
    if (scan_max_us) *scan_max_us = s_matrix_scan_max_us;
    if (work_max_us) *work_max_us = s_matrix_work_max_us;
    if (missed) *missed = s_matrix_scan_deadline_misses;
    if (q_full) *q_full = s_matrix_queue_full;
}

void hhkb_reset_scan_diagnostics_peaks(void) {
    s_matrix_scan_max_us = 0;
    s_matrix_work_max_us = 0;
}
#endif
