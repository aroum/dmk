#include "ec_settings.h"
#include "capybara_matrix.h"
#include "settings_storage_guard.h"
#include "usb_descriptors.h"
#include "stm32f0xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

#define TOPRE_TYPICAL_TRAVEL_COUNTS 1500U
#define CAL_BASELINE_SAMPLES 16U
#define HHKB_SETTINGS_SAVE_DELAY_MS 500U

static uint16_t s_cal_baseline[MATRIX_COLS][MATRIX_ROWS];
uint16_t s_cal_peak[MATRIX_COLS][MATRIX_ROWS];
rt_key_t s_rt_keys[MATRIX_COLS][MATRIX_ROWS];

static bool s_custom_calibration = true;
static bool s_calibration_mode = false;
static bool s_capture_baseline = false;
static uint8_t s_cal_baseline_samples = 0;
static uint16_t s_cal_baseline_accum[MATRIX_COLS][MATRIX_ROWS];
static uint32_t s_calibrated_mask[2];
static uint32_t s_cal_down_mask[2];
static uint16_t s_cal_press_start_peak[MATRIX_COLS][MATRIX_ROWS];
static uint16_t s_capture_base[MATRIX_COLS][MATRIX_ROWS];
static uint16_t s_capture_peak[MATRIX_COLS][MATRIX_ROWS];

static rt_cfg_t s_rt_cfg = {
    .rt_enabled = false,
    .act_pm = 600,
    .press_pm = 25,
    .release_pm = 25,
    .reset_pm = 50,
};
static uint16_t s_classic_release_pm = 100;
static uint8_t s_actuation_offset_pct = 60;

static uint8_t s_scan_rate = 4; // 1000 Hz
static bool s_power_saving = true; // Enabled by default
static bool s_oversampling_requested = false;
static uint8_t s_tapping_term_10ms = 22; // 220 ms
static uint8_t s_quick_tap_10ms = 15;    // 150 ms
static uint8_t s_permissive_hold = 1;
static uint8_t s_tap_preferred = 0;
static uint8_t s_led_mode[2] = {0, 0};
static uint8_t s_system = 0;

static volatile bool s_settings_dirty = false;
static volatile TickType_t s_settings_dirty_time = 0;

// Debug console & monitoring
static volatile uint8_t s_debug_flags = 0;
static volatile uint32_t s_matrix_scans_total = 0;
static volatile uint32_t s_matrix_scans_prev = 0;
static volatile uint32_t s_matrix_rate_hz = 0;
static volatile uint8_t s_dbg_col = 0;
static volatile uint8_t s_dbg_row = 0;
static volatile uint16_t s_dbg_raw = 0;
static volatile uint16_t s_dbg_norm = 0;
static volatile bool s_dbg_down = false;
static uint16_t s_last_reported_raw[MATRIX_COLS][MATRIX_ROWS];
static uint32_t s_last_reported_down_mask[2] = {0, 0};

static inline bool is_reported_down(uint8_t col, uint8_t row) {
    uint8_t idx = (col << 2) | (row & 3);
    return (s_last_reported_down_mask[idx / 32] & (1U << (idx % 32))) != 0;
}

static inline void set_reported_down(uint8_t col, uint8_t row, bool down) {
    uint8_t idx = (col << 2) | (row & 3);
    if (down) {
        s_last_reported_down_mask[idx / 32] |= (1U << (idx % 32));
    } else {
        s_last_reported_down_mask[idx / 32] &= ~(1U << (idx % 32));
    }
}

static uint16_t compute_crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}

static uint16_t get_default_baseline(uint8_t col, uint8_t row) {
    (void)col; (void)row;
    return 500;
}

static uint16_t get_default_peak(uint8_t col, uint8_t row) {
    (void)col; (void)row;
    return 2000;
}

bool hhkb_settings_storage_writable(void) {
    return true;
}

void hhkb_settings_mark_dirty(void) {
    s_settings_dirty_time = xTaskGetTickCount();
    s_settings_dirty = true;
}

static void stm32f0_flash_unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = 0x45670123UL;
        FLASH->KEYR = 0xCDEF89ABUL;
    }
}

static void stm32f0_flash_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

void hhkb_calibration_save_to_eeprom(void) {
    hhkb_eeprom_cal_t cal;
    memset(&cal, 0, sizeof(cal));
    cal.magic = USER_EEPROM_MAGIC;
    cal.version = USER_EEPROM_VERSION;
    cal.actuation_pct = (uint8_t)(s_rt_cfg.act_pm / 10);
    cal.rt = s_rt_cfg;
    cal.classic_release_pm = s_classic_release_pm;
    cal.scan_rate = (s_scan_rate & 0x7F) | (s_power_saving ? 0x80 : 0);
    cal.tapping_term_10ms = s_tapping_term_10ms;
    cal.quick_tap_10ms = s_quick_tap_10ms;
    cal.flags = (s_permissive_hold ? 1 : 0) |
                (s_tap_preferred ? 2 : 0) |
                (s_oversampling_requested ? 4 : 0) |
                (s_custom_calibration ? 8 : 0) |
                ((s_debug_flags & DEBUG_FLAG_TOPRE) ? 16 : 0) |
                ((s_system & 1) << 5) |
                ((s_led_mode[0] & 3) << 6);
    cal.led_mode[0] = s_led_mode[0];
    cal.led_mode[1] = s_led_mode[1];

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            cal.baseline[c][r] = s_cal_baseline[c][r];
            cal.peak[c][r] = s_cal_peak[c][r];
        }
    }
    cal.crc16 = compute_crc16((const uint8_t *)&cal.baseline, sizeof(cal) - offsetof(hhkb_eeprom_cal_t, baseline));

    // Flash page erase & program on STM32F072
    taskENTER_CRITICAL();
    stm32f0_flash_unlock();

    while (FLASH->SR & FLASH_SR_BSY);
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = USER_EEPROM_ADDR;
    FLASH->CR |= FLASH_CR_STRT;
    while (FLASH->SR & FLASH_SR_BSY);
    FLASH->CR &= ~FLASH_CR_PER;

    const uint16_t *src_hwords = (const uint16_t *)&cal;
    uint32_t dst_addr = USER_EEPROM_ADDR;
    size_t num_hwords = (sizeof(cal) + 1) / 2;

    FLASH->CR |= FLASH_CR_PG;
    for (size_t i = 0; i < num_hwords; i++) {
        *(__IO uint16_t *)(dst_addr + (i * 2)) = src_hwords[i];
        while (FLASH->SR & FLASH_SR_BSY);
    }
    FLASH->CR &= ~FLASH_CR_PG;

    stm32f0_flash_lock();
    taskEXIT_CRITICAL();
}

void hhkb_calibration_load(void) {
    const volatile hhkb_eeprom_cal_t *cal = (const volatile hhkb_eeprom_cal_t *)USER_EEPROM_ADDR;
    rt_cfg_defaults(&s_rt_cfg);
    s_rt_cfg.act_pm = 600;
    s_actuation_offset_pct = 60;
    s_classic_release_pm = 100;
    s_rt_cfg.press_pm = 25;
    s_rt_cfg.release_pm = 25;
    s_rt_cfg.rt_enabled = false;
    s_debug_flags = 0;

    if (cal->magic == USER_EEPROM_MAGIC) {
        uint16_t expected_crc = compute_crc16((const uint8_t *)&cal->baseline, sizeof(*cal) - offsetof(hhkb_eeprom_cal_t, baseline));
        if (cal->crc16 == expected_crc) {
            uint8_t raw_rate = cal->scan_rate & 0x7F;
            if (raw_rate <= 4) s_scan_rate = raw_rate;
            s_power_saving = (cal->scan_rate & 0x80) != 0;
            if (cal->tapping_term_10ms >= 5 && cal->tapping_term_10ms <= 50) s_tapping_term_10ms = cal->tapping_term_10ms;
            if (cal->quick_tap_10ms <= 50) s_quick_tap_10ms = cal->quick_tap_10ms;
            s_permissive_hold = (cal->flags & 1) != 0;
            s_tap_preferred = (cal->flags & 2) != 0;
            s_oversampling_requested = (cal->flags & 4) != 0;
            s_custom_calibration = (cal->flags & 8) != 0;
            if (cal->flags & 16) {
                s_debug_flags |= DEBUG_FLAG_TOPRE;
            } else {
                s_debug_flags &= ~DEBUG_FLAG_TOPRE;
            }
            s_system = (cal->flags >> 5) & 1;
            s_led_mode[0] = (cal->flags >> 6) & 3;
            if (cal->led_mode[0] <= 4) s_led_mode[0] = cal->led_mode[0];
            if (cal->led_mode[1] <= 4) s_led_mode[1] = cal->led_mode[1];

            if (cal->rt.act_pm >= 25 && cal->rt.act_pm <= 1000) s_rt_cfg.act_pm = cal->rt.act_pm;
            if (cal->rt.press_pm >= 25 && cal->rt.press_pm <= 1000) s_rt_cfg.press_pm = cal->rt.press_pm;
            if (cal->rt.release_pm >= 25 && cal->rt.release_pm <= 1000) s_rt_cfg.release_pm = cal->rt.release_pm;
            if (cal->rt.reset_pm <= 300) s_rt_cfg.reset_pm = cal->rt.reset_pm;
            s_rt_cfg.rt_enabled = cal->rt.rt_enabled;

            s_actuation_offset_pct = cal->actuation_pct;
            if (cal->classic_release_pm >= 25 && cal->classic_release_pm <= 1000)
                s_classic_release_pm = cal->classic_release_pm;

            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                    s_cal_baseline[c][r] = cal->baseline[c][r];
                    s_cal_peak[c][r] = cal->peak[c][r];
                }
            }
        }
    }

    if (cal->magic != USER_EEPROM_MAGIC) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                s_cal_baseline[c][r] = get_default_baseline(c, r);
                s_cal_peak[c][r] = get_default_peak(c, r);
            }
        }
    }

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            rt_key_set_range(&s_rt_keys[c][r], s_cal_baseline[c][r], s_cal_peak[c][r]);
            rt_key_reset(&s_rt_keys[c][r]);
        }
    }
}

void hhkb_settings_init(void) {
    hhkb_calibration_load();
}

void vial_keyboard_eeprom_task(void) {
    if (s_settings_dirty && (xTaskGetTickCount() - s_settings_dirty_time) >= pdMS_TO_TICKS(HHKB_SETTINGS_SAVE_DELAY_MS)) {
        s_settings_dirty = false;
        hhkb_calibration_save_to_eeprom();
    }
}

void hhkb_calibration_start(void) {
    s_calibration_mode = true;
    s_capture_baseline = true;
    s_cal_baseline_samples = 0;
    memset(s_cal_baseline_accum, 0, sizeof(s_cal_baseline_accum));
    memset(s_calibrated_mask, 0, sizeof(s_calibrated_mask));
    memset(s_cal_down_mask, 0, sizeof(s_cal_down_mask));
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            s_capture_base[c][r] = 0;
            s_capture_peak[c][r] = 0;
            s_cal_press_start_peak[c][r] = 0;
        }
    }
}

void hhkb_calibration_finish(void) {
    if (!s_calibration_mode) return;
    s_calibration_mode = false;
    s_capture_baseline = false;

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            uint8_t idx = (c * MATRIX_ROWS) + r;
            if (s_calibrated_mask[idx / 32] & (1U << (idx % 32))) {
                if (s_capture_peak[c][r] > s_capture_base[c][r] + 100) {
                    s_cal_baseline[c][r] = s_capture_base[c][r];
                    s_cal_peak[c][r] = s_capture_peak[c][r];
                }
            }
            rt_key_set_range(&s_rt_keys[c][r], s_cal_baseline[c][r], s_cal_peak[c][r]);
            rt_key_reset(&s_rt_keys[c][r]);
        }
    }
    hhkb_settings_mark_dirty();
}

bool hhkb_calibration_enabled(void) { return s_custom_calibration; }
void hhkb_calibration_set_enabled(bool enabled) {
    s_custom_calibration = enabled;
    hhkb_settings_mark_dirty();
}
bool hhkb_is_calibrating(void) { return s_calibration_mode; }
bool hhkb_cal_is_capturing_baseline(void) { return s_capture_baseline; }

uint8_t hhkb_get_scan_rate(void) { return s_scan_rate; }
void hhkb_set_scan_rate(uint8_t rate) {
    if (rate <= 4) { s_scan_rate = rate; hhkb_settings_mark_dirty(); }
}

bool hhkb_get_power_saving(void) { return s_power_saving; }
void hhkb_set_power_saving(bool enabled) {
    if (s_power_saving == enabled) return;
    s_power_saving = enabled;
    hhkb_settings_mark_dirty();
}
bool hhkb_get_oversampling(void) { return s_oversampling_requested; }
void hhkb_set_oversampling(bool enabled) {
    s_oversampling_requested = enabled;
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
    switch (id) {
        case 0: s_tapping_term_10ms = value; break;
        case 1: s_permissive_hold = value; break;
        case 2: s_tap_preferred = value; break;
        case 3: s_quick_tap_10ms = value; break;
    }
    hhkb_settings_mark_dirty();
}

uint16_t hold_tap_config_value(uint8_t id) {
    return (uint16_t)hhkb_get_hold_tap_setting(id);
}

uint8_t hhkb_get_led_mode(uint8_t index) { return index < 2 ? s_led_mode[index] : 0; }
void hhkb_set_led_mode(uint8_t index, uint8_t mode) {
    if (index < 2) { s_led_mode[index] = mode; hhkb_settings_mark_dirty(); }
}
uint8_t hhkb_get_system(void) { return s_system; }
void hhkb_set_system(uint8_t system) { s_system = system; hhkb_settings_mark_dirty(); }

uint8_t hhkb_rt_get(uint8_t id) {
    switch (id) {
        case 1: return (uint8_t)(s_rt_cfg.act_pm / 25);
        case 3: return s_rt_cfg.rt_enabled ? 1 : 0;
        case 4: return (uint8_t)(s_rt_cfg.press_pm / 25);
        case 5: return (uint8_t)(s_classic_release_pm / 25);
        case 6: return (uint8_t)(s_rt_cfg.reset_pm / 25);
        case 11: return (uint8_t)(s_rt_cfg.release_pm / 25);
        default: return 0;
    }
}

void hhkb_rt_set(uint8_t id, uint8_t value) {
    switch (id) {
        case 1: s_rt_cfg.act_pm = (uint16_t)value * 25; break;
        case 3: s_rt_cfg.rt_enabled = (value != 0); break;
        case 4: s_rt_cfg.press_pm = (uint16_t)value * 25; break;
        case 5: s_classic_release_pm = (uint16_t)value * 25; break;
        case 6: s_rt_cfg.reset_pm = (uint16_t)value * 25; break;
        case 11: s_rt_cfg.release_pm = (uint16_t)value * 25; break;
    }
    hhkb_settings_mark_dirty();
}

void hhkb_set_actuation_point_pct(uint8_t pct) {
    s_actuation_offset_pct = pct;
    s_rt_cfg.act_pm = (uint16_t)pct * 10;
    hhkb_settings_mark_dirty();
}
uint8_t hhkb_get_actuation_point_pct(void) { return s_actuation_offset_pct; }

bool hhkb_get_debug_flag(uint8_t flag) { return (s_debug_flags & flag) != 0; }
void hhkb_set_debug_flag(uint8_t flag, bool enabled) {
    if (enabled) s_debug_flags |= flag;
    else s_debug_flags &= ~flag;
    hhkb_settings_mark_dirty();
}

void hhkb_record_matrix_scan(void) { s_matrix_scans_total++; }
uint32_t hhkb_get_matrix_rate_hz(void) { return s_matrix_rate_hz; }
void hhkb_calc_rates(uint32_t elapsed_ms) {
    if (elapsed_ms == 0) return;
    s_matrix_rate_hz = (s_matrix_scans_total - s_matrix_scans_prev) * 1000 / elapsed_ms;
    s_matrix_scans_prev = s_matrix_scans_total;
}

bool hhkb_get_debug_enabled(void) { return hhkb_get_debug_flag(DEBUG_FLAG_TOPRE); }
void hhkb_set_debug_enabled(bool enabled) { hhkb_set_debug_flag(DEBUG_FLAG_TOPRE, enabled); }

void hhkb_debug_feed_sample(uint8_t col, uint8_t row, uint16_t raw, uint16_t norm, bool is_pressed) {
    s_dbg_col = col; s_dbg_row = row; s_dbg_raw = raw; s_dbg_norm = norm; s_dbg_down = is_pressed;
}

void hhkb_get_debug_key_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "R%d C%d", s_dbg_row, s_dbg_col);
}
void hhkb_get_debug_raw_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "ADC:%u", s_dbg_raw);
}
void hhkb_get_debug_travel_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "%u pm %s", s_dbg_norm, s_dbg_down ? "DOWN" : "UP");
}
void hhkb_get_cal_status_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, s_calibration_mode ? "Calibrating..." : "Calibrated");
}

rt_cfg_t hhkb_get_scan_cfg(void) { return s_rt_cfg; }
uint16_t hhkb_get_cal_baseline(uint8_t col, uint8_t row) { return s_cal_baseline[col][row]; }
uint16_t hhkb_get_cal_peak(uint8_t col, uint8_t row) { return s_cal_peak[col][row]; }

void hhkb_update_dynamic_peak(uint8_t col, uint8_t row, uint16_t val) {
    if (val > s_cal_peak[col][row]) {
        s_cal_peak[col][row] = val;
        rt_key_set_range(&s_rt_keys[col][row], s_cal_baseline[col][row], s_cal_peak[col][row]);
    }
}

void hhkb_cal_accumulate_baseline(uint8_t col, uint8_t row, uint16_t val) {
    s_cal_baseline_accum[col][row] += val;
    if (col == MATRIX_COLS - 1 && row == MATRIX_ROWS - 1) {
        if (++s_cal_baseline_samples >= CAL_BASELINE_SAMPLES) {
            s_capture_baseline = false;
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                    s_capture_base[c][r] = s_cal_baseline_accum[c][r] / CAL_BASELINE_SAMPLES;
                }
            }
        }
    }
}

void hhkb_cal_record_peak(uint8_t col, uint8_t row, uint16_t val) {
    uint8_t idx = (col * MATRIX_ROWS) + row;
    if (val > s_capture_peak[col][row]) {
        s_capture_peak[col][row] = val;
        s_calibrated_mask[idx / 32] |= (1U << (idx % 32));
    }
}

rt_key_t *hhkb_get_rt_key(uint8_t col, uint8_t row) { return &s_rt_keys[col][row]; }

// Override core weak power saving check
bool keyboard_power_saving_enabled(void) {
    return hhkb_get_power_saving();
}

