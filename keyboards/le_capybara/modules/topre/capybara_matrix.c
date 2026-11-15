#include "capybara_matrix.h"
#include "topre_settings.h"
#include "rapid_trigger.h"
#include "stm32f0xx.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_adc.h"
#include "FreeRTOS.h"
#include "task.h"

/* Column channel mapping for 74HC4051 multiplexers */
static const uint8_t s_col_channels[EC_COLS] = {
    0, 1, 2, 3, 4, 10, 12, 14, 15, 13
};

static inline void delay_nop(uint32_t n) {
    for (volatile uint32_t i = 0; i < n; i++) {
        __NOP();
    }
}

static inline void select_col(uint8_t col) {
    uint8_t ch = s_col_channels[col];
    bool mux1 = (ch & 8) != 0; // ch >= 8 uses U6 (EN1=PC14), else U4 (EN0=PA8)

    // Disable both MUXes first (active low)
    GPIOA->BSRR = (1U << 8);  // EN0 = HIGH (disabled)
    GPIOC->BSRR = (1U << 14); // EN1 = HIGH (disabled)

    // Set address lines S0 (PB15), S1 (PB14), S2 (PB13)
    uint32_t bsrr = 0;
    if (ch & 1) bsrr |= (1U << 15); else bsrr |= (1U << (15 + 16));
    if (ch & 2) bsrr |= (1U << 14); else bsrr |= (1U << (14 + 16));
    if (ch & 4) bsrr |= (1U << 13); else bsrr |= (1U << (13 + 16));
    GPIOB->BSRR = bsrr;

    // Enable active MUX (active low: drive LOW)
    if (mux1) {
        GPIOC->BRR = (1U << 14); // EN1 = LOW
    } else {
        GPIOA->BRR = (1U << 8);  // EN0 = LOW
    }
}

static inline void row_low(uint8_t row) {
    switch (row) {
        case 0: GPIOA->BRR = (1U << 3); break;
        case 1: GPIOA->BRR = (1U << 5); break;
        case 2: GPIOA->BRR = (1U << 7); break;
        case 3: GPIOB->BRR = (1U << 1); break;
        default: break;
    }
}

static inline void row_high(uint8_t row) {
    switch (row) {
        case 0: GPIOA->BSRR = (1U << 3); break;
        case 1: GPIOA->BSRR = (1U << 5); break;
        case 2: GPIOA->BSRR = (1U << 7); break;
        case 3: GPIOB->BSRR = (1U << 1); break;
        default: break;
    }
}

static inline uint16_t adc_read_sample(void) {
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC));
    return (uint16_t)(ADC1->DR & 0x0FFF);
}

void capybara_matrix_hardware_init(void) {
    // 1. Enable GPIO Clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

    // 2. Configure Row drive pins as Push-Pull Outputs (PA3, PA5, PA7, PB1), initial LOW
    GPIOA->MODER &= ~((3U << (3 * 2)) | (3U << (5 * 2)) | (3U << (7 * 2)));
    GPIOA->MODER |=  ((1U << (3 * 2)) | (1U << (5 * 2)) | (1U << (7 * 2)));
    GPIOA->OSPEEDR |= ((3U << (3 * 2)) | (3U << (5 * 2)) | (3U << (7 * 2)));
    GPIOA->BRR = (1U << 3) | (1U << 5) | (1U << 7);

    GPIOB->MODER &= ~(3U << (1 * 2));
    GPIOB->MODER |=  (1U << (1 * 2));
    GPIOB->OSPEEDR |= (3U << (1 * 2));
    GPIOB->BRR = (1U << 1);

    // 3. Configure MUX Control Pins:
    GPIOB->MODER &= ~((3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2)));
    GPIOB->MODER |=  ((1U << (13 * 2)) | (1U << (14 * 2)) | (1U << (15 * 2)));
    GPIOB->OSPEEDR |= ((3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2)));

    GPIOA->MODER &= ~(3U << (8 * 2));
    GPIOA->MODER |=  (1U << (8 * 2));
    GPIOA->BSRR = (1U << 8);

    GPIOC->MODER &= ~(3U << (14 * 2));
    GPIOC->MODER |=  (1U << (14 * 2));
    GPIOC->BSRR = (1U << 14);

    // 4. Configure Discharge Pin (PA1) as Push-Pull Output, initial LOW (discharge active)
    GPIOA->MODER &= ~(3U << (1 * 2));
    GPIOA->MODER |=  (1U << (1 * 2));
    GPIOA->OSPEEDR |= (3U << (1 * 2));
    GPIOA->BRR = (1U << 1);

    // 5. Configure Discrete Switch: KNOB_SW (PB3) as Input with Pull-Up
    GPIOB->MODER &= ~(3U << (3 * 2));
    GPIOB->PUPDR &= ~(3U << (3 * 2));
    GPIOB->PUPDR |=  (1U << (3 * 2)); // Pull-Up

    // 6. Configure ADC input: PA2 (IN2) as Analog
    GPIOA->MODER |= (3U << (2 * 2));

    // 7. Enable ADC clock and calibrate ADC
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;

    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL);

    ADC1->CFGR2 = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
    ADC1->CFGR1 = 0;
    ADC1->CHSELR = ADC_CHSELR_CHSEL2;
    ADC1->SMPR = LL_ADC_SAMPLINGTIME_1CYCLE_5;

    ADC1->ISR = ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));

    // Initialize Topre settings, calibration and Rapid Trigger structures
    hhkb_settings_init();
}

bool capybara_matrix_scan_raw(matrix_row_t current_matrix[]) {
    hhkb_record_matrix_scan();
    bool changed = false;

    bool capture = hhkb_cal_is_capturing_baseline();
    bool is_calibrating = hhkb_is_calibrating();
    rt_cfg_t scan_cfg = hhkb_get_scan_cfg();
    bool debug_topre = hhkb_get_debug_flag(DEBUG_FLAG_TOPRE);

    // 1. Scan EC Topre Matrix (Rows 0..3, Cols 0..9)
    for (uint8_t col = 0; col < EC_COLS; col++) {
        select_col(col);
        const uint16_t *col_peak = s_cal_peak[col];
        rt_key_t *col_keys = s_rt_keys[col];

        for (uint8_t row = 0; row < EC_ROWS; row++) {
            row_low(row);
            GPIOA->BRR = (1U << 1); // Discharge active (PA1 LOW)
            delay_nop(12);

            taskENTER_CRITICAL();
            GPIOA->BSRR = (1U << 1); // Block discharge route
            row_high(row);           // Row pulse HIGH
            delay_nop(6);

            uint16_t sample = adc_read_sample();
            taskEXIT_CRITICAL();

            row_low(row);
            GPIOA->BRR = (1U << 1);  // Discharge active (PA1 LOW)

            if (capture) {
                hhkb_cal_accumulate_baseline(col, row, sample);
            }
            if (is_calibrating) {
                hhkb_cal_record_peak(col, row, sample);
            } else if (sample > col_peak[row]) {
                hhkb_update_dynamic_peak(col, row, sample);
            }

            rt_key_t *key = &col_keys[row];
            bool is_pressed = rt_update(key, &scan_cfg, sample);

            if (debug_topre && sample > (key->base + 100)) {
                hhkb_debug_feed_sample(col, row, sample, rt_normalize(key, sample), is_pressed);
            }

            bool was_pressed = (current_matrix[row] & (1U << col)) != 0;
            if (is_pressed != was_pressed) {
                if (is_pressed) {
                    current_matrix[row] |= (1U << col);
                } else {
                    current_matrix[row] &= ~(1U << col);
                }
                changed = true;
            }
        }
    }

    // 2. Scan Discrete Switch: KNOB_SW (PB3) at Row 4, Col 0 (Active LOW with pull-up)
    bool knob_pressed = (GPIOB->IDR & (1U << 3)) == 0;
    bool was_knob_pressed = (current_matrix[4] & (1U << 0)) != 0;

    if (knob_pressed != was_knob_pressed) {
        if (knob_pressed) {
            current_matrix[4] |= (1U << 0);
        } else {
            current_matrix[4] &= ~(1U << 0);
        }
        changed = true;
    }

    return changed;
}
