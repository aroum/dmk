#include "classic_matrix.h"
#include "topre_settings.h"
#include "rapid_trigger.h"
#include "stm32l0xx.h"
#include "stm32l0xx_ll_adc.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef HHKB_FACTORY_THRESHOLDS
#define HHKB_FACTORY_HYSTERESIS 50
static uint16_t get_factory_threshold(uint8_t col, uint8_t row) {
    const uint16_t *press = (const uint16_t *)0x08081100;
    const int16_t *offset = (const int16_t *)0x08081200;
    uint8_t idx = (col << 3) | (row & 7);
    int32_t threshold = (int32_t)press[idx] + (int32_t)offset[idx];
    return (threshold > HHKB_FACTORY_HYSTERESIS && threshold <= 4095) ? (uint16_t)threshold : 0;
}
#endif

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

static inline uint16_t adc_read_sample(void) {
    uint32_t timeout = 5000;
    while ((ADC1->CR & ADC_CR_ADSTP) && --timeout);
    ADC1->CR |= ADC_CR_ADSTART;
    timeout = 5000;
    while (!(ADC1->ISR & ADC_ISR_EOC) && --timeout);
    return (uint16_t)(ADC1->DR & 0x0FFF);
}

void hhkb_matrix_hardware_init(void) {
    // Enable GPIO clocks
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIODEN;

    // Row drive pins (B0, B1, B2, B10) & Demux pins (B12, B13, B14, B15) as Push-Pull Output
    GPIOB->MODER &= ~(
        (3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (2 * 2)) | (3U << (10 * 2)) |
        (3U << (12 * 2)) | (3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2))
    );
    GPIOB->MODER |= (
        (1U << (0 * 2)) | (1U << (1 * 2)) | (1U << (2 * 2)) | (1U << (10 * 2)) |
        (1U << (12 * 2)) | (1U << (13 * 2)) | (1U << (14 * 2)) | (1U << (15 * 2))
    );
    GPIOB->OSPEEDR |= (
        (3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (2 * 2)) | (3U << (10 * 2)) |
        (3U << (12 * 2)) | (3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2))
    );

    // Row drive pins idle HIGH (unselected)
    GPIOB->BSRR = 0x0407;

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

__attribute__((section(".RamFunc"), optimize("O2")))
bool hhkb_matrix_scan_raw(matrix_row_t current_matrix[]) {
    bool oversampling = hhkb_get_oversampling() && hhkb_get_scan_rate() <= 3;
    if (oversampling != s_oversampling_applied) {
        while (ADC1->CR & ADC_CR_ADSTART);
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN);
        if (oversampling)
            ADC1->CFGR2 |= ADC_CFGR2_OVSE | LL_ADC_OVS_RATIO_4 | LL_ADC_OVS_SHIFT_RIGHT_2;
        else
            ADC1->CFGR2 &= ~ADC_CFGR2_OVSE;
        ADC1->ISR = ADC_ISR_ADRDY;
        ADC1->CR |= ADC_CR_ADEN;
        while (!(ADC1->ISR & ADC_ISR_ADRDY));
        s_oversampling_applied = oversampling;
    }

    bool changed = false;
    static const uint16_t row_pins[MATRIX_ROWS] = {0x1, 0x2, 0x4, 0x400};
    static const uint32_t col_mux_bsrr[MATRIX_COLS] = {
        0x70000000, 0x60001000, 0x50002000, 0x40003000,
        0x30004000, 0x20005000, 0x10006000, 0x7000,
        0x70000000, 0x60001000, 0x50002000, 0x40003000,
        0x30004000, 0x20005000, 0x10006000
    };

    GPIOB->BSRR = 0x0407;    // Deselect all rows
    GPIOC->BSRR = (1U << 7); // Enable OpAmp

    GPIOC->BSRR = (1U << 6);
    GPIOB->BRR  = (1U << 15);

    bool capture = hhkb_cal_is_capturing_baseline();
    bool is_calibrating = hhkb_is_calibrating();
    rt_cfg_t scan_cfg = hhkb_get_scan_cfg();

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        if (col == 8) {
            GPIOC->BRR  = (1U << 6);
            GPIOB->BSRR = (1U << 15);
        }

        GPIOB->BSRR = col_mux_bsrr[col];
        uint16_t settle_start = (uint16_t)TIM2->CNT;

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t prev_row = current_matrix[row];

            scan_wait_since(settle_start, 2);

            taskENTER_CRITICAL();
            // Discharge capacitor C57
            GPIOC->BSRR = (1U << 8);
            scan_delay_us(2);
            GPIOC->BRR  = (1U << 8);

            // Drive row strobe active (LOW)
            GPIOB->BRR = row_pins[row];
            scan_delay_us(3);

            uint16_t val = adc_read_sample();
            GPIOB->BSRR = row_pins[row];
            taskEXIT_CRITICAL();

            settle_start = (uint16_t)TIM2->CNT;

            if (is_calibrating) {
                if (capture) {
                    hhkb_cal_accumulate_baseline(col, row, val);
                } else {
                    hhkb_cal_record_peak(col, row, val);
                }
            } else if (val > hhkb_get_cal_peak(col, row) && val < 4000) {
                // Dynamic peak expansion
                hhkb_update_dynamic_peak(col, row, val);
            }

            bool is_pressed;
#ifdef HHKB_FACTORY_THRESHOLDS
            uint16_t threshold = get_factory_threshold(col, row);
            if (!scan_cfg.rt_enabled && !hhkb_calibration_enabled() && threshold != 0) {
                if (prev_row & (1U << col)) threshold -= HHKB_FACTORY_HYSTERESIS;
                is_pressed = val > threshold;
                rt_key_reset(hhkb_get_rt_key(col, row));
            } else
#endif
            {
                rt_key_t *key = hhkb_get_rt_key(col, row);
                if (val <= key->base) {
                    key->ext = 0;
                    key->down = false;
                    key->latched = false;
                    is_pressed = false;
                } else {
                    is_pressed = rt_update(key, &scan_cfg, val);
                }
            }

            // Feed debug monitoring
            uint16_t norm = rt_normalize(hhkb_get_rt_key(col, row), val);
            hhkb_debug_feed_sample(col, row, val, norm, is_pressed);

            if (is_pressed) current_matrix[row] |= (1U << col);
            else current_matrix[row] &= ~(1U << col);

            if (current_matrix[row] != prev_row) {
                changed = true;
            }
        }
    }

    if (capture) {
        hhkb_cal_decrement_samples();
    }

    // Disable OpAmp
    GPIOC->BRR = (1U << 7);
    return changed;
}
