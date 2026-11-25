#include "classic_matrix.h"
#include "layers.h"
#include "ec_settings.h"
#include "ec_curve.h"
#include "rapid_trigger.h"
#include "scan_cache.h"
#ifdef HHKB_HYBRID
#include "hybrid.h"
#endif
#include "stm32l0xx.h"
#include "stm32l0xx_ll_adc.h"
#include "FreeRTOS.h"
#include "task.h"


static inline void delay_us_nop(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 6; i++) {
        __NOP();
    }
}

static inline void scan_wait_since(uint16_t start, uint16_t us) {
    uint16_t ticks = us * HHKB_SCAN_TIMER_TICKS_PER_US;
    while ((uint16_t)(TIM2->CNT - start) < ticks) {}
}
static inline void scan_delay_us(uint16_t us) {
    scan_wait_since((uint16_t)TIM2->CNT, us);
}

static bool s_oversampling_applied = false;

static inline bool adc_start_sample(void) {
    uint32_t timeout = 5000;
    while ((ADC1->CR & ADC_CR_ADSTP) && --timeout);
    if (!timeout) return false;
    ADC1->ISR = ADC_ISR_EOC; // Discard completion left by a previously timed-out sample.
    ADC1->CR |= ADC_CR_ADSTART;
    return true;
}

static inline uint16_t adc_finish_sample(void) {
    uint32_t timeout = 5000;
    while (!(ADC1->ISR & ADC_ISR_EOC) && --timeout);
    if (!timeout) return UINT16_MAX;
    return (uint16_t)(ADC1->DR & 0x0FFF);
}

static hhkb_scan_cache_t s_scan_cache;
// Reused by the single matrix task; no extra stack pressure.
static uint16_t s_cal_samples[MATRIX_COLS][MATRIX_ROWS];
typedef struct {
    uint16_t raw, travel;
    uint8_t col, row;
    bool valid, pressed;
} scan_pending_t;

// Normalize once while C57 discharges, with IRQs enabled.
static inline __attribute__((always_inline))
void scan_prepare_pending(scan_pending_t *pending, const rt_scan_cfg_t *scan, bool debug) {
    if (pending->valid && pending->raw != UINT16_MAX && debug)
        pending->travel = rt_scan_normalize(&s_rt_keys[pending->col][pending->row], pending->raw, scan);
}

// Pure RAM work: process the preceding key while C57 discharges.
// Logging and calibration callbacks must stay outside the acquisition IRQ mask.
static inline __attribute__((always_inline))
bool scan_process_pending(scan_pending_t *pending, const rt_scan_cfg_t *scan,
                          matrix_row_t matrix[]) {
    if (!pending->valid) return false;
    rt_key_t *key = &s_rt_keys[pending->col][pending->row];
    matrix_row_t previous = matrix[pending->row];
    if (pending->raw == UINT16_MAX) {
        pending->pressed = key->down;
    } else if (!hhkb_scan_cache_entry_valid(&s_scan_cache, pending->col, pending->row)) {
        // This cell's thresholds are not rebuilt for the current cfg yet: take
        // the uncached reference path, which the cache reproduces bit-for-bit.
        // Classic mode must keep last_raw current for the ext restore on a
        // later RT enable; the cached RT path never writes the cache, so the
        // RT fallback does not either.
        uint16_t travel;
        pending->pressed = rt_update_scan_sample(key, scan, pending->raw, &travel);
        if (!scan->cfg.rt_enabled) s_scan_cache.key[pending->col][pending->row].last_raw = pending->raw;
    } else if (scan->cfg.rt_enabled) {
        pending->pressed = rt_update_scan_cached(key, &s_scan_cache.key[pending->col][pending->row], scan, pending->raw, NULL);
    } else {
        pending->pressed = rt_update_raw_cached(key, &s_scan_cache.key[pending->col][pending->row], pending->raw);
    }
    if (pending->pressed) matrix[pending->row] |= 1U << pending->col;
    else matrix[pending->row] &= ~(1U << pending->col);
    return previous != matrix[pending->row];
}

static inline __attribute__((always_inline))
void scan_report_pending(const scan_pending_t *pending, bool capture, bool calibrating,
                         bool debug, bool *capture_failed) {
    if (!pending->valid) return;
    if (pending->raw == UINT16_MAX) {
        if (capture) *capture_failed = true;
        return;
    }
    if (calibrating) {
        if (capture) hhkb_cal_accumulate_baseline(pending->col, pending->row, pending->raw);
        else hhkb_cal_record_peak(pending->col, pending->row, pending->raw);
    }
    if (__builtin_expect(debug, 0))
        hhkb_debug_feed_sample(pending->col, pending->row, pending->raw, pending->travel, pending->pressed);
}

void hhkb_matrix_hardware_init(void) {
    // Enable GPIO clocks
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIODEN;

    // Row drive pins (B0, B1, B2, B10) & Demux pins (B12, B13, B14, B15) as Push-Pull Output
    GPIOB->MODER &= ~(
        (3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (2 * 2)) | (3U << (10 * 2)) | ((HHKB_ROW_MASK & 0x800U) ? (3U << 22) : 0U) |
        (3U << (12 * 2)) | (3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2))
    );
    GPIOB->MODER |= (
        (1U << (0 * 2)) | (1U << (1 * 2)) | (1U << (2 * 2)) | (1U << (10 * 2)) | ((HHKB_ROW_MASK & 0x800U) ? (1U << 22) : 0U) |
        (1U << (12 * 2)) | (1U << (13 * 2)) | (1U << (14 * 2)) | (1U << (15 * 2))
    );
    GPIOB->OSPEEDR |= (
        (3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (2 * 2)) | (3U << (10 * 2)) | ((HHKB_ROW_MASK & 0x800U) ? (3U << 22) : 0U) |
        (3U << (12 * 2)) | (3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2))
    );

    // Row drive pins idle HIGH (unselected)
    GPIOB->BSRR = HHKB_ROW_MASK;

    // Analog control: PC6, PC7, PC8 and sensor board power PC14 as Push-Pull Output
    GPIOC->MODER &= ~((3U << (6 * 2)) | (3U << (7 * 2)) | (3U << (8 * 2)) | (3U << (14 * 2)));
    GPIOC->MODER |=  ((1U << (6 * 2)) | (1U << (7 * 2)) | (1U << (8 * 2)) | (1U << (14 * 2)));
    GPIOC->OSPEEDR |= ((3U << (6 * 2)) | (3U << (7 * 2)) | (3U << (8 * 2)) | (3U << (14 * 2)));
    GPIOC->BRR = (1U << 6) | (1U << 7) | (1U << 8);
    GPIOC->BSRR = (1U << 14); // Enable power for sensor board

    // ADC input: PA1 (IN1) as Analog
    GPIOA->MODER |= (3U << (1 * 2));

    // Enable ADC clock
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;

    ADC1->CR |= ADC_CR_ADVREGEN;
    delay_us_nop(2000); // Restore conservative ADC regulator startup delay

    // Calibrate ADC
    ADC1->CR |= ADC_CR_ADCAL;
    uint32_t cal_timeout = 100000;
    while ((ADC1->CR & ADC_CR_ADCAL) && --cal_timeout);

    ADC1->CFGR1 = 0;
    ADC1->CFGR2 = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
    ADC1->CHSELR = ADC_CHSELR_CHSEL1;
    ADC1->SMPR = 0; // 1.5 ADC clock cycles

    // Enable ADC
    ADC1->ISR = ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    uint32_t rdy_timeout = 100000;
    while (!(ADC1->ISR & ADC_ISR_ADRDY) && --rdy_timeout);

    // Initialize calibration, thresholds and rapid-trigger state before scanning.
    hhkb_calibration_load();
    ec_curve_rebuild();
}

// Keep peripheral power sequencing out of the scarce scanner SRAM section.
__attribute__((noinline)) static void scan_adc_prepare(void) {
    bool requested = hhkb_get_oversampling();
#ifdef HHKB_HYBRID
    if (hhkb_hybrid_on_battery()) requested = hhkb_bt_oversampling();
#endif
    bool oversampling = requested && hhkb_get_effective_scan_rate() != 4;
    if (oversampling != s_oversampling_applied) {
        while (ADC1->CR & ADC_CR_ADSTART);
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN);
        if (oversampling)
            ADC1->CFGR2 |= ADC_CFGR2_OVSE | LL_ADC_OVS_RATIO_4 | LL_ADC_OVS_SHIFT_RIGHT_2;
        else
            ADC1->CFGR2 &= ~ADC_CFGR2_OVSE;
        if (!(ADC1->CFGR1 & ADC_CFGR1_AUTOFF)) {
            ADC1->ISR = ADC_ISR_ADRDY;
            ADC1->CR |= ADC_CR_ADEN;
            while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}
        }
        s_oversampling_applied = oversampling;
    }

    if (!(ADC1->CFGR1 & ADC_CFGR1_AUTOFF) && !(ADC1->CR & ADC_CR_ADEN)) {
        while (ADC1->CR & ADC_CR_ADDIS) {}
        ADC1->ISR = ADC_ISR_ADRDY;
        ADC1->CR |= ADC_CR_ADEN;
        while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}
    }
}

__attribute__((noinline)) static void scan_adc_idle(void) {
#ifdef HHKB_HYBRID
    if (hhkb_hybrid_on_battery()) {
        while (ADC1->CR & ADC_CR_ADSTART) {}
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN) {}
    }
#endif
}

__attribute__((section(".RamFunc"), optimize("O2")))
bool hhkb_matrix_scan_raw(matrix_row_t current_matrix[]) {
#ifdef HHKB_HYBRID
    // Stock scan wrapper reasserts sensor power and keeps PD2 LOW.
    GPIOC->BSRR = 1U << 14;
    GPIOD->BRR = 1U << 2;
#endif
    // The periodic scanner and Hybrid wake probes both start TIM2 first.
    configASSERT((RCC->APB1ENR & RCC_APB1ENR_TIM2EN) && (TIM2->CR1 & TIM_CR1_CEN));
    hhkb_record_matrix_scan();
    scan_adc_prepare();

    bool changed = false;
    bool adc_autoff = (ADC1->CFGR1 & ADC_CFGR1_AUTOFF) != 0;
    uint16_t row_settle_us = adc_autoff ? 25 : 3;
    static const uint16_t row_pins[MATRIX_ROWS] = HHKB_ROW_BITS;
    static const uint32_t col_mux_bsrr[MATRIX_COLS] = {
        0x70000000, 0x60001000, 0x50002000, 0x40003000,
        0x30004000, 0x20005000, 0x10006000, 0x7000,
        0x70000000, 0x60001000, 0x50002000, 0x40003000,
        0x30004000, 0x20005000, 0x10006000
    };

    GPIOB->BSRR = HHKB_ROW_MASK;    // Deselect all rows
    GPIOC->BSRR = (1U << 7); // Enable OpAmp

    GPIOC->BSRR = (1U << 6);
    GPIOB->BRR  = (1U << 15);

    bool capture = hhkb_cal_is_capturing_baseline();
    bool capture_failed = false;
    bool is_calibrating = hhkb_is_calibrating();
    rt_cfg_t scan_cfg = hhkb_get_scan_cfg();
    const uint16_t *curve = ec_curve_lut;
    static uint32_t curve_revision;
    if (curve_revision != ec_curve_revision) {
        curve_revision = ec_curve_revision;
        for (uint8_t col = 0; col < MATRIX_COLS; ++col)
            for (uint8_t row = 0; row < MATRIX_ROWS; ++row)
                rt_key_reset(hhkb_get_rt_key(col, row));
    }
    uint16_t max_travel = curve ? curve[125] : 1000;
    if (scan_cfg.act_pm > max_travel) scan_cfg.act_pm = max_travel;
    if (scan_cfg.press_pm > max_travel) scan_cfg.press_pm = max_travel;
    if (scan_cfg.release_pm > max_travel) scan_cfg.release_pm = max_travel;
    static TickType_t last_debug_sample;
    bool debug_topre = hhkb_get_debug_flag(DEBUG_FLAG_TOPRE);
    if (debug_topre) {
        TickType_t now = xTaskGetTickCount();
        debug_topre = (TickType_t)(now - last_debug_sample) >= pdMS_TO_TICKS(20);
        if (debug_topre) last_debug_sample = now;
    }
    rt_scan_cfg_t prepared;
    rt_prepare_scan(&prepared, &scan_cfg, curve);
    hhkb_scan_cache_prepare(&s_scan_cache, s_rt_keys, &prepared, rt_range_revision, ec_curve_revision);
    scan_pending_t pending = {0};

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        if (col == 8) {
            GPIOC->BRR  = (1U << 6);
            GPIOB->BSRR = (1U << 15);
        }

        GPIOB->BSRR = col_mux_bsrr[col];
        uint16_t settle_start = (uint16_t)TIM2->CNT;

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
#ifdef HHKB_JIS
            static const uint8_t populated[MATRIX_COLS] = {29, 27, 29, 31, 29, 29, 31, 29, 31, 31, 31, 31, 31, 31, 31};
            if (!(populated[col] & (1U << row))) continue;
#endif
            // Overlap all previous-key work with discharge, not the row strobe.
            // Keep >=2 us of discharge and >=4 us since release/mux selection.
            GPIOC->BSRR = (1U << 8);
            uint16_t discharge_start = (uint16_t)TIM2->CNT;
            scan_prepare_pending(&pending, &prepared, debug_topre);
            changed |= scan_process_pending(&pending, &prepared, current_matrix);
            scan_report_pending(&pending, false, false, debug_topre, &capture_failed);
            scan_wait_since(discharge_start, 2);
            scan_wait_since(settle_start, 4);

            uint32_t saved_primask = __get_PRIMASK();
            __disable_irq();
            GPIOC->BRR = (1U << 8);

            // Drive row strobe active (LOW)
            GPIOB->BRR = row_pins[row];
            scan_delay_us(row_settle_us);

            bool started = adc_start_sample();
            // Preserve the existing analog IRQ policy. Battery AUTOFF already
            // allows Nordic SPI after triggering; USB stays masked until EOC.
            if (adc_autoff) __set_PRIMASK(saved_primask);
            uint16_t val = started ? adc_finish_sample() : UINT16_MAX;
            GPIOB->BSRR = row_pins[row];
            __set_PRIMASK(saved_primask);
            settle_start = (uint16_t)TIM2->CNT;

            if (is_calibrating) s_cal_samples[col][row] = val;
            // travel and pressed are assigned when this sample is processed.
            // Avoid aggregate zeroing here: GCC can emit a memset per key.
            pending.raw = val;
            pending.col = col;
            pending.row = row;
            pending.valid = true;
        }
    }

    // Flush the last key in this pass; no sample is deferred to the next scan.
    scan_prepare_pending(&pending, &prepared, debug_topre);
    changed |= scan_process_pending(&pending, &prepared, current_matrix);
    scan_report_pending(&pending, false, false, debug_topre, &capture_failed);

    // Disable OpAmp
    GPIOC->BRR = (1U << 7);
    scan_adc_idle();
    // Calibration bookkeeping must not change the interval between key samples.
    if (is_calibrating) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
                if (keyboard_get_flat_key_index(row, col) < 0) continue;
                scan_pending_t sample = {.raw = s_cal_samples[col][row], .col = col,
                                         .row = row, .valid = true};
                scan_report_pending(&sample, capture, true, false, &capture_failed);
            }
        }
    }
    if (capture) {
        // A missing sample must not bias the 16-frame baseline average.
        if (capture_failed) hhkb_calibration_start();
        else hhkb_cal_decrement_samples();
    }


    return changed;
}
