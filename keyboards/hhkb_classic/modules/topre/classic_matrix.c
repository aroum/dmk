#include "classic_matrix.h"
#include "rapid_trigger.h"
#include "settings_storage_guard.h"
#include <stddef.h>
#include "pin_defs.h"
#include "stm32l0xx.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#define DELAY_US(us) for (volatile uint32_t i = 0; i < (us * 6); i++) __NOP()

#ifdef HHKB_SCAN_1KHZ
static inline void scan_wait_since(uint16_t start, uint16_t us) {
    // Round up one tick to guarantee the requested minimum delay.
    uint16_t ticks = us * HHKB_SCAN_TIMER_TICKS_PER_US + 1U;
    while ((uint16_t)(TIM2->CNT - start) < ticks) {}
}
#define scan_delay_us(us) scan_wait_since((uint16_t)TIM2->CNT, (us))
#else
#define scan_delay_us(us) DELAY_US(us)
#endif
// Hysteresis gap between press and release thresholds (percentage of full key travel).
// Press threshold is at actuation_pct% of travel; release is at (actuation_pct - HYSTERESIS_PCT)%.
#define HYSTERESIS_PCT 15

// 4x15 capacitive Topre matrix: only press thresholds stored per-key.
// Release threshold is computed inline as press_threshold - hysteresis_gap.
static uint16_t s_press_threshold[MATRIX_COLS][MATRIX_ROWS];
#ifdef HHKB_ADC_DIAGNOSTICS
static volatile uint16_t s_adc_diagnostics[MATRIX_COLS][MATRIX_ROWS];
#endif
#ifdef HHKB_ADC_DMA
static volatile uint16_t s_adc_dma_sample;
#endif
#ifdef HHKB_FACTORY_THRESHOLDS
#define HHKB_FACTORY_HYSTERESIS 50
static uint16_t s_factory_threshold[MATRIX_COLS][MATRIX_ROWS];

static void load_factory_thresholds(void) {
    const uint16_t *press = (const uint16_t *)0x08081100;
    const int16_t *offset = (const int16_t *)0x08081200;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            unsigned index = 8U * col + row;
            int32_t threshold = (int32_t)press[index] + offset[index];
            s_factory_threshold[col][row] =
                press[index] > 92 && press[index] <= 4095 &&
                threshold > HHKB_FACTORY_HYSTERESIS && threshold <= 4095 ? (uint16_t)threshold : 0;
        }
    }
}
#endif

// Calibration state
static uint16_t s_cal_baseline[MATRIX_COLS][MATRIX_ROWS]; // Resting (unpressed) level
static uint16_t s_cal_peak[MATRIX_COLS][MATRIX_ROWS];     // Peak (fully pressed) level
static bool s_calibration_mode = false;
static bool s_custom_calibration;
static uint8_t s_led_mode[2] = {2, 3};
static uint8_t s_system;
static uint8_t s_scan_rate = 4; // 50, 125, 250, 500, 1000 Hz
#ifdef HHKB_ADC_OVERSAMPLING
static volatile bool s_oversampling_requested = true;
static bool s_oversampling_applied = true;
#else
static volatile bool s_oversampling_requested;
static bool s_oversampling_applied;
#endif
static uint8_t s_tapping_term_10ms = 22;
static uint8_t s_quick_tap_10ms = 15;
static uint8_t s_permissive_hold = 1;
static uint8_t s_tap_preferred;
static volatile bool s_settings_dirty;
static bool s_capture_baseline;
static uint16_t s_capture_base[MATRIX_COLS][MATRIX_ROWS];
static uint16_t s_capture_peak[MATRIX_COLS][MATRIX_ROWS];
static rt_cfg_t s_rt_cfg;
static uint16_t s_classic_release_pm = 25;
static rt_key_t s_rt_keys[MATRIX_COLS][MATRIX_ROWS];
static uint8_t s_actuation_offset_pct = 50; // Default actuation point: 50%

// EEPROM storage: 32-bit aligned to avoid M0+ unaligned access faults
#define USER_EEPROM_MAGIC 0x484B4344
#define USER_EEPROM_C_MAGIC 0x484B4343
#define USER_EEPROM_PREVIOUS_MAGIC 0x484B4342
#define USER_EEPROM_OLD_MAGIC 0x484B4341 // "HKCA"
#define USER_EEPROM_ADDR  0x08081500

typedef struct __attribute__((aligned(4))) {
    uint32_t magic;
    uint8_t  actuation_pct;
    uint8_t  reserved[3];
    uint16_t baseline[MATRIX_COLS][MATRIX_ROWS];
    uint16_t peak[MATRIX_COLS][MATRIX_ROWS];
    rt_cfg_t rt;
    uint8_t custom_calibration;
    uint8_t led_mode[2];
    uint8_t system;
    uint16_t classic_release_pm;
} hhkb_eeprom_cal_t;
_Static_assert(sizeof(hhkb_eeprom_cal_t) == 264, "HHKB settings must stay within the existing EEPROM record");

#define WORD_ALIGNED_SIZE(n) (((n) + 3U) & ~3U)
bool hhkb_settings_storage_writable(void) {
    return hhkb_settings_region_writable((volatile const uint8_t *)USER_EEPROM_ADDR,
        sizeof(hhkb_eeprom_cal_t), offsetof(hhkb_eeprom_cal_t, rt),
        WORD_ALIGNED_SIZE(offsetof(hhkb_eeprom_cal_t, custom_calibration)),
        WORD_ALIGNED_SIZE(offsetof(hhkb_eeprom_cal_t, classic_release_pm)));
}

static inline uint16_t adc_read_sample(void) {
    while (ADC1->CR & ADC_CR_ADSTP);
#ifdef HHKB_ADC_DMA
    // One circular DMA transfer per explicitly triggered key measurement.
    // Clear the previous completion before starting the next conversion.
    DMA1->IFCR = DMA_IFCR_CGIF1;
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(DMA1->ISR & DMA_ISR_TCIF1));
    return s_adc_dma_sample & 0x0FFF;
#else
    ADC1->CR |= ADC_CR_ADSTART;
    while (!(ADC1->ISR & ADC_ISR_EOC));
    return (uint16_t)(ADC1->DR & 0x0FFF);
#endif
}

// Compute the hysteresis gap for a single key based on its calibrated travel range
static inline uint16_t hysteresis_gap(uint8_t col, uint8_t row) {
    uint32_t delta = s_cal_peak[col][row] - s_cal_baseline[col][row];
    return (uint16_t)((delta * HYSTERESIS_PCT) / 100);
}

static void update_thresholds_for_key(uint8_t col, uint8_t row) {
    uint32_t base = s_cal_baseline[col][row];
    uint32_t peak = s_cal_peak[col][row];

    if (peak > base + 150) {
        uint32_t delta = peak - base;
        s_press_threshold[col][row] = base + (delta * s_actuation_offset_pct) / 100;
    } else {
        // Fallback default Topre threshold
        s_press_threshold[col][row] = 1375;
    }
}

/**
 * @brief Saves calibration to STM32L0 EEPROM word by word, yielding between writes
 * to prevent locking the USB stack or NVM access for 200 ms.
 * Static buffer avoids large stack allocation (~250 bytes) in FreeRTOS task.
 */
void hhkb_calibration_save_to_eeprom(void) {
    if (!hhkb_settings_storage_writable()) return;
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        FLASH->PEKEYR = 0x89ABCDEF; // FLASH_PEKEY1
        FLASH->PEKEYR = 0x02030405; // FLASH_PEKEY2
    }
    while (FLASH->PECR & FLASH_PECR_PELOCK);

    static hhkb_eeprom_cal_t cal;
    cal.magic = USER_EEPROM_MAGIC;
    cal.actuation_pct = s_actuation_offset_pct;
    cal.rt = s_rt_cfg;
    cal.classic_release_pm = s_classic_release_pm;
    cal.custom_calibration = s_custom_calibration;
    cal.led_mode[0] = s_led_mode[0]; cal.led_mode[1] = s_led_mode[1];
    cal.system = s_system;
    cal.reserved[0] = 0x80 | (s_scan_rate + 1) | (s_permissive_hold << 4) |
                      (s_tap_preferred << 5) | (s_oversampling_requested << 6);
    cal.reserved[1] = s_tapping_term_10ms;
    cal.reserved[2] = 0x80 | s_quick_tap_10ms; // Oversampling preference is valid.

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            cal.baseline[c][r] = s_cal_baseline[c][r];
            cal.peak[c][r] = s_cal_peak[c][r];
        }
    }

    volatile uint32_t *dst = (volatile uint32_t *)USER_EEPROM_ADDR;
    const uint32_t *src = (const uint32_t *)&cal;
    size_t words = (sizeof(hhkb_eeprom_cal_t) + 3) / 4;

    for (size_t i = 0; i < words; i++) {
        while (FLASH->SR & FLASH_SR_BSY);
        if (dst[i] == src[i]) continue;
        dst[i] = src[i];
        while (FLASH->SR & FLASH_SR_BSY);

        // Yield to let USB and other FreeRTOS tasks run smoothly
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    // Re-lock EEPROM
    FLASH->PECR |= FLASH_PECR_PELOCK;
}

void hhkb_calibration_load(void) {
    const hhkb_eeprom_cal_t *cal = (const hhkb_eeprom_cal_t *)USER_EEPROM_ADDR;
#ifdef HHKB_FACTORY_THRESHOLDS
    load_factory_thresholds();
#endif

    rt_cfg_defaults(&s_rt_cfg);
    s_scan_rate = 4;
#ifdef HHKB_ADC_OVERSAMPLING
    s_oversampling_requested = true;
#else
    s_oversampling_requested = false;
#endif
    s_tapping_term_10ms = 22;
    s_quick_tap_10ms = 15;
    s_permissive_hold = 1;
    s_tap_preferred = 0;
    s_rt_cfg.press_pm = 25;
    s_rt_cfg.release_pm = 25;
#ifdef HHKB_RAPID_TRIGGER
    s_rt_cfg.rt_enabled = true;
#endif
    if (cal->magic == USER_EEPROM_MAGIC || cal->magic == USER_EEPROM_C_MAGIC || cal->magic == USER_EEPROM_PREVIOUS_MAGIC || cal->magic == USER_EEPROM_OLD_MAGIC) {
        if (cal->magic == USER_EEPROM_MAGIC && (cal->reserved[0] & 7) >= 1 && (cal->reserved[0] & 7) <= 5)
            s_scan_rate = (cal->reserved[0] & 7) - 1;
        if (cal->magic == USER_EEPROM_MAGIC && cal->reserved[1] >= 5 && cal->reserved[1] <= 50) {
            s_tapping_term_10ms = cal->reserved[1];
            if ((cal->reserved[2] & 0x7f) <= 50)
                s_quick_tap_10ms = cal->reserved[2] & 0x7f;
        }
        if (cal->magic == USER_EEPROM_MAGIC && (cal->reserved[2] & 0x80))
            s_oversampling_requested = (cal->reserved[0] & 0x40) != 0;
        if (cal->magic == USER_EEPROM_MAGIC && (cal->reserved[0] & 0x80)) {
            s_permissive_hold = (cal->reserved[0] & 0x10) != 0;
            s_tap_preferred = (cal->reserved[0] & 0x20) != 0;
        }
        if (cal->magic != USER_EEPROM_OLD_MAGIC && cal->rt.act_pm >= 25 && cal->rt.act_pm <= 1000 && cal->rt.press_pm >= 25 && cal->rt.press_pm <= 1000 && cal->rt.release_pm >= 25 && cal->rt.release_pm <= 1000 && cal->rt.reset_pm <= 150) s_rt_cfg = cal->rt;
        s_classic_release_pm = (cal->magic == USER_EEPROM_MAGIC && cal->classic_release_pm >= 25 && cal->classic_release_pm <= 1000) ? cal->classic_release_pm : s_rt_cfg.release_pm;
#ifndef HHKB_RAPID_TRIGGER
        s_rt_cfg.rt_enabled = false;
#endif
        s_custom_calibration = (cal->magic == USER_EEPROM_MAGIC || cal->magic == USER_EEPROM_C_MAGIC) ? cal->custom_calibration == 1 : true;
        if (cal->magic == USER_EEPROM_MAGIC || cal->magic == USER_EEPROM_C_MAGIC) {
            s_led_mode[0] = cal->led_mode[0] <= 4 ? cal->led_mode[0] : 2;
            s_led_mode[1] = cal->led_mode[1] <= 4 ? cal->led_mode[1] : 3;
            s_system = cal->system <= 1 ? cal->system : 0;
        }
        s_actuation_offset_pct = (cal->actuation_pct >= 10 && cal->actuation_pct <= 90) ? cal->actuation_pct : 50;

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                uint16_t base = s_custom_calibration ? cal->baseline[col][row] : 800;
                uint16_t peak = s_custom_calibration ? cal->peak[col][row] : 1800;
                if (base > 4095) base = 800;
                if (peak > 4095 || peak <= base || peak - base < RT_MIN_RANGE)
                    peak = base + RT_DEFAULT_RANGE;
                s_cal_baseline[col][row] = base;
                s_cal_peak[col][row] = peak;
                update_thresholds_for_key(col, row);
            }
        }
    } else {
        // Defaults: Resting ~800, Peak ~1800, Press 1375 (~50%)
        s_actuation_offset_pct = 50;
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                s_cal_baseline[col][row] = 800;
                s_cal_peak[col][row] = 1800;
                s_press_threshold[col][row] = 1375;
            }
        }
    }
    for (uint8_t c = 0; c < MATRIX_COLS; c++) for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        rt_key_set_range(&s_rt_keys[c][r], s_cal_baseline[c][r], s_cal_peak[c][r]);
        rt_key_reset(&s_rt_keys[c][r]);
    }
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

    // PC6 (U1 Enable), PC7 (OpAmp Enable), PC8 (Discharge), PC14 (Power) as Push-Pull Output
    GPIOC->MODER &= ~(
        (3U << (6 * 2)) | (3U << (7 * 2)) | (3U << (8 * 2)) | (3U << (14 * 2))
    );
    GPIOC->MODER |= (
        (1U << (6 * 2)) | (1U << (7 * 2)) | (1U << (8 * 2)) | (1U << (14 * 2))
    );

    // PA1 as Analog input (ADC1_IN1)
    GPIOA->MODER |= (3U << (1 * 2));

    // Default pin states
    GPIOB->BSRR = (1U << 0) | (1U << 1) | (1U << 2) | (1U << 10); // Deselect rows (active LOW)
    GPIOC->BSRR = (1U << 14) | (1U << 7); // Power switch board and enable OpAmp

    // Setup ADC1
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    if (ADC1->ISR & ADC_ISR_ADRDY) ADC1->ISR |= ADC_ISR_ADRDY;
    if (ADC1->CR & ADC_CR_ADEN) ADC1->CR |= ADC_CR_ADDIS;
    while (ADC1->CR & ADC_CR_ADEN);

    ADC1->CR |= ADC_CR_ADVREGEN;
    DELAY_US(2000);

    // Synchronous ADC clock derived from the 32 MHz HCLK.
    uint32_t adc_cfgr2 = ADC1->CFGR2 &
        ~(ADC_CFGR2_CKMODE | ADC_CFGR2_OVSE | ADC_CFGR2_OVSR |
          ADC_CFGR2_OVSS | ADC_CFGR2_TOVS);
#ifdef HHKB_ADC_16MHZ
    adc_cfgr2 |= LL_ADC_CLOCK_SYNC_PCLK_DIV2;
#else
    adc_cfgr2 |= LL_ADC_CLOCK_SYNC_PCLK_DIV4;
#endif
#ifdef HHKB_ADC_OVERSAMPLING
    // One software trigger accumulates four conversions and returns their 12-bit average.
    adc_cfgr2 |= LL_ADC_OVS_GRP_REGULAR_CONTINUED | LL_ADC_OVS_RATIO_4 | LL_ADC_OVS_SHIFT_RIGHT_2;
#endif
    ADC1->CFGR2 = adc_cfgr2;

    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL);

    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));

    ADC1->CFGR1 &= ~(ADC_CFGR1_EXTEN | ADC_CFGR1_CONT);
    ADC1->SMPR = 0; // Shortest sample time
    ADC1->CHSELR = (1U << 1); // Channel 1 (PA1)
#ifdef HHKB_ADC_DMA
    // ADC1 uses DMA1 channel 1 on STM32L072. Keep a one-sample circular
    // destination armed; conversion timing remains controlled by the scanner.
    RCC->AHBENR |= RCC_AHBENR_DMAEN;
    DMA1_Channel1->CCR = 0;
    DMA1_CSELR->CSELR &= ~DMA_CSELR_C1S;
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR = (uint32_t)&s_adc_dma_sample;
    DMA1_Channel1->CNDTR = 1;
    DMA1->IFCR = DMA_IFCR_CGIF1;
    DMA1_Channel1->CCR = DMA_CCR_CIRC | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 | DMA_CCR_EN;
    ADC1->CFGR1 |= ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG;
#endif

    // Load calibration from EEPROM
    hhkb_calibration_load();
}

void hhkb_calibration_start(void) {
    if (!s_custom_calibration) return;
    taskENTER_CRITICAL();
    s_capture_baseline = true;
    s_calibration_mode = true;
    taskEXIT_CRITICAL();
}

void hhkb_calibration_finish(void) {
    taskENTER_CRITICAL();
    if (s_calibration_mode && !s_capture_baseline) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            if (s_capture_peak[c][r] >= s_capture_base[c][r] + RT_MIN_RANGE) {
                s_cal_baseline[c][r] = s_capture_base[c][r];
                s_cal_peak[c][r] = s_capture_peak[c][r];
                rt_key_set_range(&s_rt_keys[c][r], s_cal_baseline[c][r], s_cal_peak[c][r]);
                rt_key_reset(&s_rt_keys[c][r]);
                update_thresholds_for_key(c, r);
            }
        }
    }
    s_calibration_mode = false;
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}

bool hhkb_calibration_enabled(void) { return s_custom_calibration; }
void hhkb_calibration_set_enabled(bool enabled) {
    taskENTER_CRITICAL();
    s_custom_calibration = enabled;
    if (!enabled) {
        s_calibration_mode = false; s_capture_baseline = false;
        for (uint8_t c = 0; c < MATRIX_COLS; c++) for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            s_cal_baseline[c][r] = 800; s_cal_peak[c][r] = 1800;
            rt_key_set_range(&s_rt_keys[c][r], 800, 1800); rt_key_reset(&s_rt_keys[c][r]);
        }
    }
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}
uint8_t hhkb_get_led_mode(uint8_t index) { return index < 2 ? s_led_mode[index] : 0; }
void hhkb_set_led_mode(uint8_t index, uint8_t mode) { if (index < 2 && mode <= 4) { s_led_mode[index] = mode; hhkb_settings_mark_dirty(); } }
uint8_t hhkb_get_system(void) { return s_system; }
void hhkb_set_system(uint8_t system) { if (system <= 1 && s_system != system) { s_system = system; hhkb_settings_mark_dirty(); } }
void hhkb_settings_mark_dirty(void) { s_settings_dirty = true; }
uint8_t hhkb_get_scan_rate(void) { return s_scan_rate; }
bool hhkb_get_oversampling(void) { return s_oversampling_requested; }
void hhkb_set_oversampling(bool enabled) {
    if (s_oversampling_requested == enabled) return;
    s_oversampling_requested = enabled;
    hhkb_settings_mark_dirty();
}
void hhkb_set_scan_rate(uint8_t rate) {
    if (rate > 4) return;
    taskENTER_CRITICAL();
    s_scan_rate = rate;
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}
uint8_t hhkb_get_hold_tap_setting(uint8_t id) {
    switch (id) {
    case 0: return s_tapping_term_10ms;
    case 1: return s_permissive_hold;
    case 2: return s_tap_preferred;
    case 3: return s_quick_tap_10ms;
    default: return 0;
    }
}
void hhkb_set_hold_tap_setting(uint8_t id, uint8_t value) {
    taskENTER_CRITICAL();
    if (id == 0 && value >= 5 && value <= 50) s_tapping_term_10ms = value;
    else if (id == 1 && value <= 1) s_permissive_hold = value;
    else if (id == 2 && value <= 1) s_tap_preferred = value;
    else if (id == 3 && value <= 50) s_quick_tap_10ms = value;
    else { taskEXIT_CRITICAL(); return; }
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}
uint16_t hold_tap_config_value(uint8_t id) {
    switch (id) {
    case 0: return (uint16_t)s_tapping_term_10ms * 10;
    case 1: return s_permissive_hold;
    case 2: return s_tap_preferred;
    case 3: return (uint16_t)s_quick_tap_10ms * 10;
    default: return 0;
    }
}
void vial_keyboard_eeprom_task(void) {
    taskENTER_CRITICAL();
    bool dirty = s_settings_dirty;
    s_settings_dirty = false;
    taskEXIT_CRITICAL();
    if (dirty) hhkb_calibration_save_to_eeprom();
}

uint8_t hhkb_rt_get(uint8_t id) {
    switch (id) {
    case 1: return s_rt_cfg.act_pm / 25;
    case 3: return s_rt_cfg.rt_enabled;
    case 4: return s_rt_cfg.press_pm / 25;
    case 5: return s_classic_release_pm / 25;
    case 11: return s_rt_cfg.release_pm / 25;
    case 6: return s_rt_cfg.reset_pm / 25;
    default: return 0;
    }
}
void hhkb_rt_set(uint8_t id, uint8_t value) {
    taskENTER_CRITICAL();
    if (id == 3) {
#ifdef HHKB_RAPID_TRIGGER
        s_rt_cfg.rt_enabled = value != 0;
#else
        s_rt_cfg.rt_enabled = false;
#endif
    } else if (id == 6) {
        if (value <= 6) s_rt_cfg.reset_pm = value * 25;
    } else if (value >= 1 && value <= 40) {
        if (id == 1) s_rt_cfg.act_pm = value * 25;
        if (id == 4) s_rt_cfg.press_pm = value * 25;
        if (id == 5) s_classic_release_pm = value * 25;
        if (id == 11) s_rt_cfg.release_pm = value * 25;
    }
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}

void hhkb_set_actuation_point_pct(uint8_t pct) {
    if (pct < 10) pct = 10;
    if (pct > 90) pct = 90;
    s_actuation_offset_pct = pct;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            update_thresholds_for_key(col, row);
        }
    }
}

uint8_t hhkb_get_actuation_point_pct(void) {
    return s_actuation_offset_pct;
}

bool hhkb_is_calibrating(void) {
    return s_calibration_mode;
}

#ifdef HHKB_SCAN_1KHZ
__attribute__((section(".RamFunc"), optimize("O2")))
#endif
bool hhkb_matrix_scan_raw(matrix_row_t current_matrix[]) {
    // Only the matrix task touches ADC configuration. Apply VIA changes between
    // complete scans, never during a key conversion or its DMA transfer.
    bool oversampling = s_oversampling_requested && s_scan_rate <= 3;
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

    GPIOB->BSRR = 0x0407; // Deselect all rows
    GPIOC->BSRR = (1U << 7); // Enable OpAmp

    GPIOC->BSRR = (1U << 6);
    GPIOB->BRR  = (1U << 15);

    bool capture = s_calibration_mode && s_capture_baseline;
    rt_cfg_t scan_cfg = s_rt_cfg;
    if (!scan_cfg.rt_enabled) scan_cfg.release_pm = s_classic_release_pm;
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        if (col == 8) {
            GPIOC->BRR  = (1U << 6);
            GPIOB->BSRR = (1U << 15);
        }

        GPIOB->BSRR = col_mux_bsrr[col];
#ifdef HHKB_SCAN_1KHZ
        uint16_t settle_start = (uint16_t)TIM2->CNT;
#endif

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t prev_row = current_matrix[row];

#ifdef HHKB_SCAN_1KHZ
            scan_wait_since(settle_start, 2);
#else
            scan_delay_us(2);
#endif
            // Discharge capacitor C57
            GPIOC->BSRR = (1U << 8);
            scan_delay_us(2);
            GPIOC->BRR  = (1U << 8);

            // Drive row strobe active (LOW)
            GPIOB->BRR = row_pins[row];
            scan_delay_us(3);

            uint16_t val = adc_read_sample();
#ifdef HHKB_SCAN_1KHZ
            // The conversion is complete. Process it while the next row settles.
            GPIOB->BSRR = row_pins[row];
            settle_start = (uint16_t)TIM2->CNT;
#endif
#ifdef HHKB_ADC_DIAGNOSTICS
            s_adc_diagnostics[col][row] = val;
#endif

            if (s_calibration_mode) {
                if (capture) {
                    s_capture_base[col][row] = val;
                    s_capture_peak[col][row] = val;
                } else if (val > s_capture_peak[col][row]) {
                    s_capture_peak[col][row] = val;
                }
            }
            bool is_pressed;
#ifdef HHKB_FACTORY_THRESHOLDS
            uint16_t threshold = s_factory_threshold[col][row];
            if (!scan_cfg.rt_enabled && !s_custom_calibration && threshold != 0) {
                if (prev_row & (1U << col)) threshold -= HHKB_FACTORY_HYSTERESIS;
                is_pressed = val > threshold;
                rt_key_reset(&s_rt_keys[col][row]);
            } else
#endif
            {
                rt_key_t *key = &s_rt_keys[col][row];
#ifdef HHKB_SCAN_1KHZ
                if (val <= key->base) {
                    // This sample normalizes to zero and always resets the key.
                    key->ext = 0;
                    key->down = false;
                    key->latched = false;
                    is_pressed = false;
                } else
#endif
                {
                    is_pressed = rt_update(key, &scan_cfg, val);
                }
            }
            if (is_pressed) current_matrix[row] |= (1U << col);
            else current_matrix[row] &= ~(1U << col);

            if (current_matrix[row] != prev_row) {
                changed = true;
            }

#ifndef HHKB_SCAN_1KHZ
            GPIOB->BSRR = row_pins[row];
#endif
        }
    }

    if (capture) s_capture_baseline = false;
    // Disable OpAmp
    GPIOC->BRR = (1U << 7);
    return changed;
}
