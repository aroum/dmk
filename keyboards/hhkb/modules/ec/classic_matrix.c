#include "classic_matrix.h"
#include "ec_settings.h"
#include "ec_curve.h"
#include "rapid_trigger.h"
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
    if (!(RCC->APB1ENR & RCC_APB1ENR_TIM2EN)) {
        delay_us_nop(us);
        return;
    }
    uint16_t ticks = us * HHKB_SCAN_TIMER_TICKS_PER_US;
    while ((uint16_t)(TIM2->CNT - start) < ticks) {}
}
#define scan_delay_us(us) ((RCC->APB1ENR & RCC_APB1ENR_TIM2EN) ? scan_wait_since((uint16_t)TIM2->CNT, (us)) : delay_us_nop(us))

static bool s_oversampling_applied = false;

static inline uint16_t adc_read_sample(uint32_t saved_primask) {
    uint32_t timeout = 5000;
    while ((ADC1->CR & ADC_CR_ADSTP) && --timeout);
    if (!timeout) return UINT16_MAX;
    ADC1->ISR = ADC_ISR_EOC; // Discard completion left by a previously timed-out sample.
    ADC1->CR |= ADC_CR_ADSTART;
    // At the 1 MHz battery ADC clock, a 4x conversion outlasts a Nordic
    // SPI byte. Once triggered, acquisition is hardware-timed: let SPI run.
    if (ADC1->CFGR1 & ADC_CFGR1_AUTOFF) __set_PRIMASK(saved_primask);
    timeout = 5000;
    while (!(ADC1->ISR & ADC_ISR_EOC) && --timeout);
    if (!timeout) return UINT16_MAX;
    return (uint16_t)(ADC1->DR & 0x0FFF);
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
    hhkb_record_matrix_scan();
    scan_adc_prepare();

    bool changed = false;
    uint16_t row_settle_us = (ADC1->CFGR1 & ADC_CFGR1_AUTOFF) ? 25 : 3;
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
    bool debug_topre = hhkb_get_debug_flag(DEBUG_FLAG_TOPRE);

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        rt_key_t *col_keys = s_rt_keys[col];

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
            matrix_row_t prev_row = current_matrix[row];

            scan_wait_since(settle_start, 2);

            uint32_t saved_primask = __get_PRIMASK();
            __disable_irq();

            // Discharge capacitor C57
            GPIOC->BSRR = (1U << 8);
            scan_delay_us(2);
            GPIOC->BRR  = (1U << 8);

            // Drive row strobe active (LOW)
            GPIOB->BRR = row_pins[row];
            scan_delay_us(row_settle_us);

            uint16_t val = adc_read_sample(saved_primask);
            GPIOB->BSRR = row_pins[row];

            __set_PRIMASK(saved_primask);

            settle_start = (uint16_t)TIM2->CNT;

            // Never calibrate or update RT state from a stale ADC data register.
            if (val == UINT16_MAX) {
                if (capture) capture_failed = true;
                if (col_keys[row].down) current_matrix[row] |= 1U << col;
                else current_matrix[row] &= ~(1U << col);
                if (current_matrix[row] != prev_row) changed = true;
                continue;
            }

            if (is_calibrating) {
                if (capture) {
                    hhkb_cal_accumulate_baseline(col, row, val);
                } else {
                    hhkb_cal_record_peak(col, row, val);
                }
            }
            // Calibrated endpoints stay fixed outside an explicit calibration.
            // A single runtime spike must not permanently expand the range.

            bool is_pressed;
            rt_key_t *key = &col_keys[row];
            {
                if (val <= key->base) {
                    key->ext = 0;
                    key->down = false;
                    key->latched = false;
                    is_pressed = false;
                } else {
                    is_pressed = rt_update_curve(key, &scan_cfg, val, curve);
                }
            }

            // Feed debug monitoring only when EC debug streaming is active
            if (__builtin_expect(debug_topre, 0)) {
                uint16_t norm = rt_normalize_curve(key, val, curve);
                hhkb_debug_feed_sample(col, row, val, norm, is_pressed);
            }

            if (is_pressed) current_matrix[row] |= (1U << col);
            else current_matrix[row] &= ~(1U << col);

            if (current_matrix[row] != prev_row) {
                changed = true;
            }
        }
    }

    if (capture) {
        // A missing sample must not bias the 16-frame baseline average.
        if (capture_failed) hhkb_calibration_start();
        else hhkb_cal_decrement_samples();
    }

    // Disable OpAmp
    GPIOC->BRR = (1U << 7);
    scan_adc_idle();
    return changed;
}
