#include "topre_settings.h"
#include "classic_matrix.h"
#include "settings_storage_guard.h"
#include "usb_descriptors.h"
#include "stm32l0xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

#define TOPRE_TYPICAL_TRAVEL_COUNTS 2370U
#define CAL_BASELINE_SAMPLES 16U
#define HHKB_SETTINGS_SAVE_DELAY_MS 500U

static uint16_t s_cal_baseline[MATRIX_COLS][MATRIX_ROWS];
static uint16_t s_cal_peak[MATRIX_COLS][MATRIX_ROWS];
static rt_key_t s_rt_keys[MATRIX_COLS][MATRIX_ROWS];

static bool s_custom_calibration = true;
static bool s_calibration_mode = false;
static bool s_capture_baseline = false;
static uint8_t s_cal_baseline_samples = 0;
static uint16_t s_cal_baseline_accum[MATRIX_COLS][MATRIX_ROWS];
static uint32_t s_calibrated_mask[2];
static uint16_t s_capture_base[MATRIX_COLS][MATRIX_ROWS];
static uint16_t s_capture_peak[MATRIX_COLS][MATRIX_ROWS];

static rt_cfg_t s_rt_cfg = {
    .rt_enabled = false,
    .act_pm = 750,
    .press_pm = 25,
    .release_pm = 25,
    .reset_pm = 50,
};
static uint16_t s_classic_release_pm = 100; // Default 10% travel hysteresis (0.4 mm)
static uint8_t s_actuation_offset_pct = 75;

static uint8_t s_scan_rate = 4; // 1000 Hz
static bool s_oversampling_requested = false;
static uint8_t s_tapping_term_10ms = 22; // 220 ms
static uint8_t s_quick_tap_10ms = 15;    // 150 ms
static uint8_t s_permissive_hold = 1;
static uint8_t s_tap_preferred = 0;
static uint8_t s_led_mode[2] = {2, 3};
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

#define HHKB_FACTORY_BASELINE_ADDR 0x08081000UL
static uint16_t get_factory_baseline(uint8_t col, uint8_t row) {
    const uint16_t *base = (const uint16_t *)HHKB_FACTORY_BASELINE_ADDR;
    uint8_t idx = (col << 3) | (row & 7);
    uint16_t val = base[idx];
    return (val >= 100 && val <= 2000) ? val : 0;
}

static uint16_t get_default_baseline(uint8_t col, uint8_t row) {
    uint16_t f_base = get_factory_baseline(col, row);
    return (f_base != 0) ? f_base : 800;
}

static uint16_t get_default_peak(uint8_t col, uint8_t row) {
    uint16_t base = get_default_baseline(col, row);
    uint32_t peak = (uint32_t)base + TOPRE_TYPICAL_TRAVEL_COUNTS;
    return (peak <= 4095) ? (uint16_t)peak : 4000;
}

#define WORD_ALIGNED_SIZE(n) (((n) + 3U) & ~3U)
bool hhkb_settings_storage_writable(void) {
    return hhkb_settings_region_writable((volatile const uint8_t *)USER_EEPROM_ADDR,
        sizeof(hhkb_eeprom_cal_t), offsetof(hhkb_eeprom_cal_t, rt),
        WORD_ALIGNED_SIZE(offsetof(hhkb_eeprom_cal_t, scan_rate)),
        sizeof(hhkb_eeprom_cal_t));
}

void hhkb_settings_mark_dirty(void) {
    s_settings_dirty_time = xTaskGetTickCount();
    s_settings_dirty = true;
}

void hhkb_calibration_save_to_eeprom(void) {
    if (!hhkb_settings_storage_writable()) return;
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        FLASH->PEKEYR = 0x89ABCDEF;
        FLASH->PEKEYR = 0x02030405;
    }
    while (FLASH->PECR & FLASH_PECR_PELOCK);

    hhkb_eeprom_cal_t cal;
    cal.magic = USER_EEPROM_MAGIC;
    cal.version = USER_EEPROM_VERSION;
    cal.actuation_pct = (uint8_t)(s_rt_cfg.act_pm / 10);
    cal.rt = s_rt_cfg;
    cal.classic_release_pm = s_classic_release_pm;
    cal.scan_rate = s_scan_rate;
    cal.tapping_term_10ms = s_tapping_term_10ms;
    cal.quick_tap_10ms = s_quick_tap_10ms;
    cal.flags = (s_permissive_hold ? 1 : 0) |
                (s_tap_preferred ? 2 : 0) |
                (s_oversampling_requested ? 4 : 0) |
                (s_custom_calibration ? 8 : 0) |
                ((s_debug_flags & DEBUG_FLAG_TOPRE) ? 16 : 0) |
                ((s_system & 1) << 5) |
                ((s_led_mode[0] & 3) << 6);

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            cal.baseline[c][r] = s_cal_baseline[c][r];
            cal.peak[c][r] = s_cal_peak[c][r];
        }
    }
    cal.crc16 = compute_crc16((const uint8_t *)&cal.baseline, sizeof(cal) - offsetof(hhkb_eeprom_cal_t, baseline));

    volatile uint32_t *dst = (volatile uint32_t *)USER_EEPROM_ADDR;
    const uint32_t *src = (const uint32_t *)&cal;
    for (size_t i = 0; i < sizeof(hhkb_eeprom_cal_t) / 4; i++) {
        if (dst[i] != src[i]) {
            while (FLASH->SR & FLASH_SR_BSY);
            dst[i] = src[i];
            while (FLASH->SR & FLASH_SR_BSY);
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
    FLASH->PECR |= FLASH_PECR_PELOCK;
}

void hhkb_calibration_load(void) {
    const volatile hhkb_eeprom_cal_t *cal = (const volatile hhkb_eeprom_cal_t *)USER_EEPROM_ADDR;
    rt_cfg_defaults(&s_rt_cfg);
    s_rt_cfg.act_pm = 750;
    s_actuation_offset_pct = 75;
    s_classic_release_pm = 100;
    s_rt_cfg.press_pm = 25;
    s_rt_cfg.release_pm = 25;
    s_rt_cfg.rt_enabled = false;
    s_debug_flags = 0;

    if (cal->magic == USER_EEPROM_MAGIC) {
        // Versioned format with CRC16
        uint16_t expected_crc = compute_crc16((const uint8_t *)&cal->baseline, sizeof(*cal) - offsetof(hhkb_eeprom_cal_t, baseline));
        if (cal->crc16 == expected_crc) {
            if (cal->scan_rate <= 4) s_scan_rate = cal->scan_rate;
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

            if (cal->rt.act_pm >= 25 && cal->rt.act_pm <= 1000) s_rt_cfg.act_pm = cal->rt.act_pm;
            if (cal->rt.press_pm >= 25 && cal->rt.press_pm <= 1000) s_rt_cfg.press_pm = cal->rt.press_pm;
            if (cal->rt.release_pm >= 25 && cal->rt.release_pm <= 1000) s_rt_cfg.release_pm = cal->rt.release_pm;
            if (cal->rt.reset_pm <= 150) s_rt_cfg.reset_pm = cal->rt.reset_pm;
            if (s_rt_cfg.act_pm <= s_rt_cfg.reset_pm) s_rt_cfg.act_pm = s_rt_cfg.reset_pm + 25;
            s_rt_cfg.rt_enabled = cal->rt.rt_enabled;
            s_actuation_offset_pct = (uint8_t)(s_rt_cfg.act_pm / 10);
            if (cal->classic_release_pm >= 25 && cal->classic_release_pm <= 1000) s_classic_release_pm = cal->classic_release_pm;

            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                    uint16_t def_b = get_default_baseline(col, row);
                    uint16_t def_p = get_default_peak(col, row);
                    s_cal_baseline[col][row] = (s_custom_calibration && cal->baseline[col][row] >= 100 && cal->baseline[col][row] <= 2000) ? cal->baseline[col][row] : def_b;
                    s_cal_peak[col][row] = (s_custom_calibration && cal->peak[col][row] > s_cal_baseline[col][row] + RT_MIN_RANGE && cal->peak[col][row] <= 4095) ? cal->peak[col][row] : def_p;
                }
            }
        }
    } else if (cal->magic == USER_EEPROM_LEGACY_MAGIC || cal->magic == USER_EEPROM_C_MAGIC || cal->magic == USER_EEPROM_PREV_MAGIC || cal->magic == USER_EEPROM_OLD_MAGIC) {
        // Legacy fallback
        if (cal->magic != USER_EEPROM_OLD_MAGIC) {
            if (cal->rt.act_pm >= 25 && cal->rt.act_pm <= 1000) s_rt_cfg.act_pm = cal->rt.act_pm;
            if (cal->rt.press_pm >= 25 && cal->rt.press_pm <= 1000) s_rt_cfg.press_pm = cal->rt.press_pm;
            if (cal->rt.release_pm >= 25 && cal->rt.release_pm <= 1000) s_rt_cfg.release_pm = cal->rt.release_pm;
            if (cal->rt.reset_pm <= 150) s_rt_cfg.reset_pm = cal->rt.reset_pm;
            if (s_rt_cfg.act_pm <= s_rt_cfg.reset_pm) s_rt_cfg.act_pm = s_rt_cfg.reset_pm + 25;
            s_rt_cfg.rt_enabled = cal->rt.rt_enabled;
            s_actuation_offset_pct = (uint8_t)(s_rt_cfg.act_pm / 10);
        }
        s_custom_calibration = true;
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                uint16_t def_b = get_default_baseline(col, row);
                uint16_t def_p = get_default_peak(col, row);
                s_cal_baseline[col][row] = (cal->baseline[col][row] >= 100 && cal->baseline[col][row] <= 2000) ? cal->baseline[col][row] : def_b;
                s_cal_peak[col][row] = (cal->peak[col][row] > s_cal_baseline[col][row] + RT_MIN_RANGE && cal->peak[col][row] <= 4095) ? cal->peak[col][row] : def_p;
            }
        }
    } else {
        // Defaults: factory baseline + typical travel
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                s_cal_baseline[col][row] = get_default_baseline(col, row);
                s_cal_peak[col][row] = get_default_peak(col, row);
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

void vial_keyboard_eeprom_task(void) {
    if (!s_settings_dirty) return;
    TickType_t now = xTaskGetTickCount();
    if ((now - s_settings_dirty_time) >= pdMS_TO_TICKS(HHKB_SETTINGS_SAVE_DELAY_MS)) {
        taskENTER_CRITICAL();
        s_settings_dirty = false;
        taskEXIT_CRITICAL();
        hhkb_calibration_save_to_eeprom();
    }
}

void hhkb_calibration_start(void) {
    memset(s_cal_baseline_accum, 0, sizeof(s_cal_baseline_accum));
    s_cal_baseline_samples = CAL_BASELINE_SAMPLES;
    s_calibrated_mask[0] = 0;
    s_calibrated_mask[1] = 0;
    s_capture_baseline = true;
    s_calibration_mode = true;
    if (hhkb_get_debug_flag(DEBUG_FLAG_CAL)) {
        debug_console_printf("\r\n[CAL] Calibration started: capturing baseline (16 samples)...\r\n");
    }
}

void hhkb_calibration_finish(void) {
    if (!s_calibration_mode) return;
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            uint8_t k_idx = c * 4 + r;
            bool was_actuated = (k_idx < 32)
                ? (s_calibrated_mask[0] & (1U << k_idx)) != 0
                : (s_calibrated_mask[1] & (1U << (k_idx - 32))) != 0;
            if (was_actuated) {
                s_cal_baseline[c][r] = s_capture_base[c][r];
                s_cal_peak[c][r] = s_capture_peak[c][r];
            }
            rt_key_set_range(&s_rt_keys[c][r], s_cal_baseline[c][r], s_cal_peak[c][r]);
            rt_key_reset(&s_rt_keys[c][r]);
        }
    }
    if (hhkb_get_debug_flag(DEBUG_FLAG_CAL)) {
        uint8_t count = (uint8_t)(__builtin_popcount(s_calibrated_mask[0]) + __builtin_popcount(s_calibrated_mask[1]));
        debug_console_printf("[CAL] Calibration complete and saved to EEPROM (%u/60 keys updated).\r\n", count);
    }
    s_calibration_mode = false;
    s_custom_calibration = true;
    hhkb_settings_mark_dirty();
}

bool hhkb_calibration_enabled(void) { return s_custom_calibration; }
void hhkb_calibration_set_enabled(bool enabled) {
    if (s_custom_calibration == enabled) return;
    s_custom_calibration = enabled;
    hhkb_settings_mark_dirty();
}
bool hhkb_is_calibrating(void) { return s_calibration_mode; }

uint8_t hhkb_get_scan_rate(void) { return s_scan_rate; }
void hhkb_set_scan_rate(uint8_t rate) {
    if (rate > 4) return;
    taskENTER_CRITICAL();
    s_scan_rate = rate;
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}

bool hhkb_get_oversampling(void) { return s_oversampling_requested; }
void hhkb_set_oversampling(bool enabled) {
    if (s_oversampling_requested == enabled) return;
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

uint8_t hhkb_get_led_mode(uint8_t index) { return index < 2 ? s_led_mode[index] : 0; }
void hhkb_set_led_mode(uint8_t index, uint8_t mode) {
    if (index < 2 && mode <= 4) { s_led_mode[index] = mode; hhkb_settings_mark_dirty(); }
}

uint8_t hhkb_get_system(void) { return s_system; }
void hhkb_set_system(uint8_t system) {
    if (system <= 1 && s_system != system) { s_system = system; hhkb_settings_mark_dirty(); }
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
        s_rt_cfg.rt_enabled = value != 0;
    } else if (id == 6) {
        if (value <= 6) {
            s_rt_cfg.reset_pm = value * 25;
            if (s_rt_cfg.act_pm <= s_rt_cfg.reset_pm) s_rt_cfg.act_pm = s_rt_cfg.reset_pm + 25;
        }
    } else if (value >= 1 && value <= 40) {
        uint16_t pm = value * 25;
        if (id == 1) {
            if (pm <= s_rt_cfg.reset_pm) pm = s_rt_cfg.reset_pm + 25;
            s_rt_cfg.act_pm = pm;
            s_actuation_offset_pct = (uint8_t)(pm / 10);
        }
        if (id == 4) s_rt_cfg.press_pm = pm;
        if (id == 5) s_classic_release_pm = pm;
        if (id == 11) s_rt_cfg.release_pm = pm;
    }
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}

void hhkb_set_actuation_point_pct(uint8_t pct) {
    if (pct < 10) pct = 10;
    if (pct > 90) pct = 90;
    taskENTER_CRITICAL();
    s_actuation_offset_pct = pct;
    uint16_t pm = (uint16_t)pct * 10;
    if (pm <= s_rt_cfg.reset_pm) pm = s_rt_cfg.reset_pm + 25;
    s_rt_cfg.act_pm = pm;
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}

uint8_t hhkb_get_actuation_point_pct(void) {
    return (uint8_t)(s_rt_cfg.act_pm / 10);
}

// Debug API
bool hhkb_get_debug_flag(uint8_t flag) {
    return (s_debug_flags & flag) != 0;
}

void hhkb_set_debug_flag(uint8_t flag, bool enabled) {
    if (enabled) {
        s_debug_flags |= flag;
    } else {
        s_debug_flags &= ~flag;
    }
    if (flag == DEBUG_FLAG_TOPRE) {
        hhkb_settings_mark_dirty();
    }
}

void hhkb_record_matrix_scan(void) {
    s_matrix_scans_total++;
}

uint32_t hhkb_get_matrix_rate_hz(void) {
    return s_matrix_rate_hz;
}

void hhkb_calc_rates(uint32_t elapsed_ms) {
    if (elapsed_ms == 0) return;
    uint32_t total = s_matrix_scans_total;
    uint32_t diff = total - s_matrix_scans_prev;
    s_matrix_scans_prev = total;
    s_matrix_rate_hz = (uint32_t)(((uint64_t)diff * 1000ULL + (elapsed_ms / 2)) / elapsed_ms);
}

bool hhkb_get_debug_enabled(void) { return hhkb_get_debug_flag(DEBUG_FLAG_TOPRE); }
void hhkb_set_debug_enabled(bool enabled) { hhkb_set_debug_flag(DEBUG_FLAG_TOPRE, enabled); }

void hhkb_debug_feed_sample(uint8_t col, uint8_t row, uint16_t raw, uint16_t norm, bool is_pressed) {
    if (col >= MATRIX_COLS || row >= MATRIX_ROWS) return;

    uint16_t base = s_cal_baseline[col][row];
    bool was_pressed = is_reported_down(col, row);

    if (!hhkb_get_debug_flag(DEBUG_FLAG_TOPRE)) return;

    // 1. Immediate notification on state transitions (Press / Release)
    if (is_pressed != was_pressed) {
        set_reported_down(col, row, is_pressed);
        s_last_reported_raw[col][row] = raw;
        s_dbg_col = col;
        s_dbg_row = row;
        s_dbg_raw = raw;
        s_dbg_norm = norm;
        s_dbg_down = is_pressed;
        debug_console_printf("[Topre] C%u R%u: raw=%u base=%u peak=%u travel=%u%% %s\r\n",
            col, row, raw, base, s_cal_peak[col][row], norm / 10, is_pressed ? "DN" : "UP");
        return;
    }

    // 2. Continuous travel updates while key is pressed or significantly actuated (travel >= 5%)
    if (is_pressed || norm >= 50) {
        int16_t diff = (int16_t)(raw - s_last_reported_raw[col][row]);
        if (diff > 40 || diff < -40) {
            s_last_reported_raw[col][row] = raw;
            s_dbg_col = col;
            s_dbg_row = row;
            s_dbg_raw = raw;
            s_dbg_norm = norm;
            s_dbg_down = is_pressed;
            debug_console_printf("[Topre] C%u R%u: raw=%u base=%u peak=%u travel=%u%% %s\r\n",
                col, row, raw, base, s_cal_peak[col][row], norm / 10, is_pressed ? "DN" : "UP");
        }
    } else if (norm < 50 && !is_pressed && s_last_reported_raw[col][row] != base) {
        s_last_reported_raw[col][row] = base;
    }
}

void hhkb_get_debug_key_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "C%u R%u (%s)", s_dbg_col, s_dbg_row, s_dbg_down ? "Pressed" : "Released");
}

void hhkb_get_debug_raw_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "Raw:%u (B:%u P:%u)", s_dbg_raw, s_cal_baseline[s_dbg_col][s_dbg_row], s_cal_peak[s_dbg_col][s_dbg_row]);
}

void hhkb_get_debug_travel_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "Travel: %u.%u%%", s_dbg_norm / 10, s_dbg_norm % 10);
}

rt_cfg_t hhkb_get_scan_cfg(void) {
    taskENTER_CRITICAL();
    rt_cfg_t cfg = s_rt_cfg;
    if (!cfg.rt_enabled) cfg.release_pm = s_classic_release_pm;
    taskEXIT_CRITICAL();
    return cfg;
}

uint16_t hhkb_get_cal_baseline(uint8_t col, uint8_t row) { return s_cal_baseline[col][row]; }
uint16_t hhkb_get_cal_peak(uint8_t col, uint8_t row) { return s_cal_peak[col][row]; }
void hhkb_update_dynamic_peak(uint8_t col, uint8_t row, uint16_t val) {
    s_cal_peak[col][row] = val;
    rt_key_set_range(&s_rt_keys[col][row], s_cal_baseline[col][row], s_cal_peak[col][row]);
}
void hhkb_cal_accumulate_baseline(uint8_t col, uint8_t row, uint16_t val) {
    s_cal_baseline_accum[col][row] += val;
}
void hhkb_cal_record_peak(uint8_t col, uint8_t row, uint16_t val) {
    if (val > s_capture_peak[col][row]) s_capture_peak[col][row] = val;
    if (val >= s_capture_base[col][row] + RT_MIN_RANGE) {
        uint8_t k_idx = col * 4 + row;
        bool newly_calibrated = false;
        if (k_idx < 32) {
            if (!(s_calibrated_mask[0] & (1U << k_idx))) {
                s_calibrated_mask[0] |= (1U << k_idx);
                newly_calibrated = true;
            }
        } else {
            if (!(s_calibrated_mask[1] & (1U << (k_idx - 32)))) {
                s_calibrated_mask[1] |= (1U << (k_idx - 32));
                newly_calibrated = true;
            }
        }
        if (newly_calibrated && hhkb_get_debug_flag(DEBUG_FLAG_CAL)) {
            uint8_t count = (uint8_t)(__builtin_popcount(s_calibrated_mask[0]) + __builtin_popcount(s_calibrated_mask[1]));
            uint8_t pct = (uint8_t)((count * 100U) / 60U);
            char bar[21];
            uint8_t filled = (uint8_t)((count * 20U) / 60U);
            for (uint8_t b = 0; b < 20; b++) bar[b] = (b < filled) ? '=' : ' ';
            bar[20] = '\0';
            debug_console_printf("[CAL] [%s] %u%% (%u/60) | Key C%u R%u peak: %u\r\n",
                                 bar, pct, count, col, row, s_capture_peak[col][row]);
            if (count >= 60) {
                debug_console_printf("[CAL] >>> All 60 keys calibrated! Click 'Save calibration' in VIA. <<<\r\n");
            }
        }
    }
}
bool hhkb_cal_is_capturing_baseline(void) { return s_calibration_mode && s_capture_baseline; }
void hhkb_cal_decrement_samples(void) {
    if (--s_cal_baseline_samples == 0) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                s_capture_base[c][r] = (uint16_t)(s_cal_baseline_accum[c][r] / CAL_BASELINE_SAMPLES);
                s_capture_peak[c][r] = s_capture_base[c][r];
            }
        }
        s_capture_baseline = false;
        if (hhkb_get_debug_flag(DEBUG_FLAG_CAL)) {
            debug_console_printf("[CAL] Baseline captured! Press every key firmly to record peak travel.\r\n");
        }
    }
}
rt_key_t *hhkb_get_rt_key(uint8_t col, uint8_t row) { return &s_rt_keys[col][row]; }
