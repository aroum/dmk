#include "ec_settings.h"
#include "capybara_matrix.h"
#include "settings_storage_guard.h"
#include "usb_descriptors.h"
#include "vial.h"
#include "stm32f0xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#define CAL_REQ_ENTER() taskENTER_CRITICAL()
#define CAL_REQ_EXIT() taskEXIT_CRITICAL()
#include "cal_request.h"

#define TOPRE_TYPICAL_TRAVEL_COUNTS 1500U
#define CAL_BASELINE_SAMPLES 16U
#define CAL_REST_TOLERANCE (RT_MIN_RANGE / 3U)
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
static uint16_t s_capture_latest[MATRIX_COLS][MATRIX_ROWS];
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
static uint16_t s_stream_raw;
static bool s_stream_active;
static uint8_t s_stream_col;
static uint8_t s_stream_row;

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

static void apply_calibrated_range(uint8_t col, uint8_t row) {
    uint16_t base = s_custom_calibration ? s_cal_baseline[col][row] : get_default_baseline(col, row);
    uint16_t peak = s_custom_calibration ? s_cal_peak[col][row] : get_default_peak(col, row);
    rt_key_set_range(&s_rt_keys[col][row], base, peak);
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
    // The matrix task owns calibration updates; snapshot them atomically.
    taskENTER_CRITICAL();
    cal.actuation_pct = (uint8_t)(s_rt_cfg.act_pm / 10);
    cal.rt = s_rt_cfg;
    memcpy(cal.curve, ec_curve_settings, sizeof(cal.curve));
    cal.debug_flags = s_debug_flags;
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

    memcpy(cal.baseline, s_cal_baseline, sizeof(cal.baseline));
    memcpy(cal.peak, s_cal_peak, sizeof(cal.peak));
    taskEXIT_CRITICAL();
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
    for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            s_cal_baseline[col][row] = get_default_baseline(col, row);
            s_cal_peak[col][row] = get_default_peak(col, row);
        }
    }
    rt_cfg_defaults(&s_rt_cfg);
    s_rt_cfg.act_pm = 600;
    s_actuation_offset_pct = 60;
    s_classic_release_pm = 100;
    s_rt_cfg.press_pm = 25;
    s_rt_cfg.release_pm = 25;
    s_rt_cfg.rt_enabled = false;
    s_debug_flags = 0;

    if (cal->magic == USER_EEPROM_MAGIC) {
        bool current = cal->version == USER_EEPROM_VERSION;
        size_t size = current ? sizeof(*cal) : offsetof(hhkb_eeprom_cal_t, curve);
        uint16_t expected_crc = compute_crc16((const uint8_t *)&cal->baseline, size - offsetof(hhkb_eeprom_cal_t, baseline));
        if ((current || cal->version == 1) && cal->crc16 == expected_crc) {
            if (current) {
                for (unsigned i = 0; i < EC_CURVE_FIELDS; ++i)
                    ec_curve_set(EC_CURVE_ENABLE + i, cal->curve[i]);
            }
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
            if (current) s_debug_flags = cal->debug_flags;

            if (cal->rt.act_pm >= 25 && cal->rt.act_pm <= (current ? 1125 : 1000)) s_rt_cfg.act_pm = cal->rt.act_pm;
            if (cal->rt.press_pm >= 25 && cal->rt.press_pm <= (current ? 1125 : 1000)) s_rt_cfg.press_pm = cal->rt.press_pm;
            if (cal->rt.release_pm >= 25 && cal->rt.release_pm <= (current ? 1125 : 1000)) s_rt_cfg.release_pm = cal->rt.release_pm;
            if (cal->rt.reset_pm <= 300) s_rt_cfg.reset_pm = cal->rt.reset_pm;
            s_rt_cfg.rt_enabled = cal->rt.rt_enabled;

            s_actuation_offset_pct = cal->actuation_pct;
            if (cal->classic_release_pm >= 25 && cal->classic_release_pm <= (current ? 1125 : 1000))
                s_classic_release_pm = cal->classic_release_pm;

            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                    uint16_t base = cal->baseline[c][r];
                    uint16_t peak = cal->peak[c][r];
                    if (base != 0 && peak <= 4095 && peak >= base + RT_MIN_RANGE) {
                        s_cal_baseline[c][r] = base;
                        s_cal_peak[c][r] = peak;
                    }
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
            apply_calibrated_range(c, r);
            rt_key_reset(&s_rt_keys[c][r]);
        }
    }
}

void hhkb_settings_init(void) {
    hhkb_calibration_load();
    ec_curve_rebuild();
}

void vial_keyboard_eeprom_task(void) {
    if (s_settings_dirty && (xTaskGetTickCount() - s_settings_dirty_time) >= pdMS_TO_TICKS(HHKB_SETTINGS_SAVE_DELAY_MS)) {
        s_settings_dirty = false;
        ec_curve_rebuild();
        hhkb_calibration_save_to_eeprom();
    }
}

bool vial_keyboard_settings_reset(void) {
    s_settings_dirty = false;
    return vial_stm32f072_erase_settings_page(USER_EEPROM_ADDR);
}

static volatile uint8_t s_cal_request;
static volatile uint8_t s_cal_enable_request;

void hhkb_calibration_request_start(void) { cal_request_post(&s_cal_request, CAL_REQ_START); }
void hhkb_calibration_request_finish(void) { cal_request_post(&s_cal_request, CAL_REQ_FINISH); }
void hhkb_calibration_request_set_enabled(bool enabled) {
    cal_request_post(&s_cal_enable_request, enabled ? CAL_REQ_ENABLE : CAL_REQ_DISABLE);
}

// Matrix task only, outside a scan.
void hhkb_calibration_apply_pending(void) {
    cal_request_t req = cal_request_take(&s_cal_request);
    if (req == CAL_REQ_START) hhkb_calibration_start();
    else if (req == CAL_REQ_FINISH) hhkb_calibration_finish();
    req = cal_request_take(&s_cal_enable_request);
    if (req != CAL_REQ_NONE) hhkb_calibration_set_enabled(req == CAL_REQ_ENABLE);
}

// [CAL] lines are produced in the matrix task, whose 1 KB stack cannot carry
// the vsnprintf chain (~1 KB). Latch compact events here and let the USB task
// format them in hhkb_cal_log_flush(). Single producer (matrix task), single
// consumer (USB task); the barriers order the entry stores against the index.
enum { CAL_LOG_START = 1, CAL_LOG_BASE_INCOMPLETE, CAL_LOG_BASE_READY, CAL_LOG_KEY, CAL_LOG_FINISH };
typedef struct { uint8_t kind, row, col, count; uint16_t a, b; } cal_log_evt_t;
static cal_log_evt_t s_cal_log[8];
static volatile uint8_t s_cal_log_head, s_cal_log_tail;

static void cal_log_push(uint8_t kind, uint8_t row, uint8_t col, uint8_t count, uint16_t a, uint16_t b) {
    uint8_t head = s_cal_log_head;
    if ((uint8_t)(head - s_cal_log_tail) >= 8) return; // full: drop the event
    cal_log_evt_t *evt = &s_cal_log[head % 8];
    evt->kind = kind; evt->row = row; evt->col = col; evt->count = count;
    evt->a = a; evt->b = b;
    __asm volatile("" ::: "memory");
    s_cal_log_head = (uint8_t)(head + 1);
}

void hhkb_cal_log_flush(void) {
    while (s_cal_log_tail != s_cal_log_head) {
        __asm volatile("" ::: "memory");
        const cal_log_evt_t *evt = &s_cal_log[s_cal_log_tail % 8];
        switch (evt->kind) {
        case CAL_LOG_START:
            debug_console_printf("[CAL] Started. Release all keys.\r\n");
            break;
        case CAL_LOG_BASE_INCOMPLETE:
            debug_console_printf("[CAL] Baseline incomplete. Start again.\r\n");
            break;
        case CAL_LOG_BASE_READY:
            debug_console_printf("[CAL] Baseline ready. Press keys fully.\r\n");
            break;
        case CAL_LOG_KEY: {
            char bar[21];
            uint8_t pct = (uint8_t)((evt->count * 100U) / 36U);
            uint8_t filled = (uint8_t)((evt->count * 20U) / 36U);
            for (uint8_t b = 0; b < 20; b++) bar[b] = (b < filled) ? '=' : ' ';
            bar[20] = '\0';
            debug_console_printf("[CAL] [%s] %u%% (%u/36) | Key R%u C%u peak: %u (base: %u)\r\n",
                                 bar, pct, evt->count, evt->row, evt->col, evt->a, evt->b);
            if (evt->count >= 36) {
                debug_console_printf("[CAL] >>> All 36 keys calibrated! Click 'Save calibration' in VIA. <<<\r\n");
            }
            break;
        }
        case CAL_LOG_FINISH:
            debug_console_printf("[CAL] Finished. Updated %u keys.\r\n", evt->count);
            break;
        }
        s_cal_log_tail = (uint8_t)(s_cal_log_tail + 1);
    }
}

void hhkb_calibration_start(void) {
    taskENTER_CRITICAL();
    s_cal_baseline_samples = 0;
    memset(s_cal_baseline_accum, 0, sizeof(s_cal_baseline_accum));
    memset(s_calibrated_mask, 0, sizeof(s_calibrated_mask));
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            s_capture_base[c][r] = UINT16_MAX;
            s_capture_peak[c][r] = 0;
            s_capture_latest[c][r] = UINT16_MAX;
        }
    }
    s_capture_baseline = true;
    s_calibration_mode = true;
    taskEXIT_CRITICAL();
    if (hhkb_get_debug_flag(DEBUG_FLAG_CAL))
        cal_log_push(CAL_LOG_START, 0, 0, 0, 0, 0);
}

void hhkb_calibration_finish(void) {
    taskENTER_CRITICAL();
    if (!s_calibration_mode) {
        taskEXIT_CRITICAL();
        return;
    }
    bool baseline_ready = !s_capture_baseline;
    s_capture_baseline = false;
    if (!baseline_ready) {
        s_calibration_mode = false;
        taskEXIT_CRITICAL();
        if (hhkb_get_debug_flag(DEBUG_FLAG_CAL))
            cal_log_push(CAL_LOG_BASE_INCOMPLETE, 0, 0, 0, 0, 0);
        return;
    }

    unsigned updated = 0;
    for (uint8_t c = 0; c < EC_COLS; c++) {
        for (uint8_t r = 0; r < EC_ROWS; r++) {
            uint8_t idx = (c * MATRIX_ROWS) + r;
            int rest_delta = (int)s_capture_latest[c][r] - s_capture_base[c][r];
            if ((s_calibrated_mask[idx / 32] & (1U << (idx % 32))) &&
                rest_delta >= -(int)CAL_REST_TOLERANCE && rest_delta <= (int)CAL_REST_TOLERANCE) {
                s_cal_baseline[c][r] = s_capture_base[c][r];
                s_cal_peak[c][r] = s_capture_peak[c][r];
                apply_calibrated_range(c, r);
                ++updated;
            }
            rt_key_reset(&s_rt_keys[c][r]);
        }
    }
    s_calibration_mode = false;
    taskEXIT_CRITICAL();
    if (updated) hhkb_settings_mark_dirty();
    if (hhkb_get_debug_flag(DEBUG_FLAG_CAL))
        cal_log_push(CAL_LOG_FINISH, 0, 0, (uint8_t)updated, 0, 0);
}

// Applied state; a pending set_enabled request is not reflected until applied.
bool hhkb_calibration_enabled(void) { return s_custom_calibration; }
// Matrix task only (rewrites rt key ranges); other tasks use request_set_enabled.
void hhkb_calibration_set_enabled(bool enabled) {
    taskENTER_CRITICAL();
    s_custom_calibration = enabled;
    for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            apply_calibrated_range(col, row);
            rt_key_reset(&s_rt_keys[col][row]);
        }
    }
    taskEXIT_CRITICAL();
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
    if (id == 6 && value > 12) return;
    if ((id == 1 || id == 4 || id == 5 || id == 11) && (value < 1 || value > 45)) return;
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

void hhkb_debug_feed_sample(uint8_t col, uint8_t row, uint16_t raw, uint16_t norm, bool is_pressed, bool transitioned) {
    if (transitioned && is_pressed) {
        s_stream_active = true;
        s_stream_col = col;
        s_stream_row = row;
    }

    bool selected = s_stream_active && col == s_stream_col && row == s_stream_row;
    int32_t delta = (int32_t)raw - s_stream_raw;
    if (!transitioned && (!selected || norm < 50 || (delta > -40 && delta < 40))) {
        if (selected && norm < 50 && !is_pressed) {
            s_stream_active = false;
        }
        return;
    }

    if (selected) s_stream_raw = raw;
    s_dbg_col = col; s_dbg_row = row; s_dbg_raw = raw; s_dbg_norm = norm; s_dbg_down = is_pressed;
    debug_console_printf("[EC] R%u C%u : raw=%u base=%u peak=%u travel=%u%% %s\r\n",
                         row, col, raw, s_cal_baseline[col][row], s_cal_peak[col][row],
                         norm / 10, is_pressed ? "DN" : "UP");
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
    unsigned count = __builtin_popcount(s_calibrated_mask[0]) + __builtin_popcount(s_calibrated_mask[1]);
    if (s_cal_request == CAL_REQ_START || s_capture_baseline) snprintf(buf, max_len, "Capturing baseline");
    else if (s_calibration_mode) snprintf(buf, max_len, "Measured: %u keys", count);
    else snprintf(buf, max_len, "Idle");
}

rt_cfg_t hhkb_get_scan_cfg(void) {
    rt_cfg_t cfg = s_rt_cfg;
    if (!cfg.rt_enabled) cfg.release_pm = s_classic_release_pm;
    return cfg;
}
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
    if (val < s_capture_base[col][row]) s_capture_base[col][row] = val;
    if (val > s_capture_peak[col][row]) s_capture_peak[col][row] = val;
    if (col == EC_COLS - 1 && row == EC_ROWS - 1) {
        if (++s_cal_baseline_samples >= CAL_BASELINE_SAMPLES) {
            s_capture_baseline = false;
            for (uint8_t c = 0; c < EC_COLS; c++) {
                for (uint8_t r = 0; r < EC_ROWS; r++) {
                    uint16_t min = s_capture_base[c][r];
                    uint16_t max = s_capture_peak[c][r];
                    s_capture_base[c][r] = (min != 0 && max - min <= RT_MIN_RANGE)
                        ? s_cal_baseline_accum[c][r] / CAL_BASELINE_SAMPLES : UINT16_MAX;
                    s_capture_peak[c][r] = 0;
                }
            }
            if (hhkb_get_debug_flag(DEBUG_FLAG_CAL))
                cal_log_push(CAL_LOG_BASE_READY, 0, 0, 0, 0, 0);
        }
    }
}

void hhkb_cal_record_peak(uint8_t col, uint8_t row, uint16_t val) {
    if (s_capture_baseline || s_capture_base[col][row] == UINT16_MAX) return;
    s_capture_latest[col][row] = val;
    uint8_t idx = (col * MATRIX_ROWS) + row;
    if (val > s_capture_peak[col][row]) {
        s_capture_peak[col][row] = val;
        uint32_t bit = 1U << (idx % 32);
        if (val >= s_capture_base[col][row] + RT_MIN_RANGE &&
            !(s_calibrated_mask[idx / 32] & bit)) {
            s_calibrated_mask[idx / 32] |= bit;
            if (hhkb_get_debug_flag(DEBUG_FLAG_CAL)) {
                uint8_t count = (uint8_t)(__builtin_popcount(s_calibrated_mask[0]) + __builtin_popcount(s_calibrated_mask[1]));
                cal_log_push(CAL_LOG_KEY, row, col, count, val, s_capture_base[col][row]);
            }
        }
    }
}

rt_key_t *hhkb_get_rt_key(uint8_t col, uint8_t row) { return &s_rt_keys[col][row]; }

// Override core weak power saving check
bool keyboard_power_saving_enabled(void) {
    return hhkb_get_power_saving();
}

