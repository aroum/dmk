#include "ec_settings.h"
#ifdef HHKB_COMPACT_STORAGE
#include "../via_settings/hhkb_compact_storage.h"
#endif
#ifdef HHKB_HYBRID
#include "hybrid.h"
#endif
#include "classic_matrix.h"
#include "settings_storage_guard.h"
#include "usb_descriptors.h"
#include "dip_switch.h"
#include "layers.h"
#include "stm32l0xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#define CAL_REQ_ENTER() taskENTER_CRITICAL()
#define CAL_REQ_EXIT() taskEXIT_CRITICAL()
#include "cal_request.h"

#define TOPRE_TYPICAL_TRAVEL_COUNTS 2370U
#define CAL_BASELINE_SAMPLES 16U
#define HHKB_SETTINGS_SAVE_DELAY_MS 500U

static uint16_t s_cal_baseline[MATRIX_COLS][MATRIX_ROWS];
uint16_t s_cal_peak[MATRIX_COLS][MATRIX_ROWS];
rt_key_t s_rt_keys[MATRIX_COLS][MATRIX_ROWS];

static bool s_custom_calibration = true;
static bool s_calibration_mode = false;
static bool s_capture_baseline = false;
static uint8_t s_cal_baseline_samples = 0;

#define MATRIX_MASK_WORDS ((MATRIX_COLS * MATRIX_ROWS + 31) / 32)
static uint32_t s_calibrated_mask[MATRIX_MASK_WORDS];
static uint8_t calibrated_key_count(void) {
    uint8_t count = 0;
    for (unsigned i = 0; i < MATRIX_MASK_WORDS; ++i)
        count += __builtin_popcount(s_calibrated_mask[i]);
    return count;
}
static uint32_t s_cal_down_mask[MATRIX_MASK_WORDS];
static uint16_t s_cal_press_start_peak[MATRIX_COLS][MATRIX_ROWS];
static uint16_t s_capture_base[MATRIX_COLS][MATRIX_ROWS];
static uint16_t s_capture_peak[MATRIX_COLS][MATRIX_ROWS];

#define HHKB_CAL_BASE_MIN 20u

static uint8_t s_gesc_overrides, s_gesc_ignore;

static rt_cfg_t s_rt_cfg = {
    .rt_enabled = false,
    .bottom_dz = 5,
    .act_pm = 750,
    .press_pm = 25,
    .release_pm = 25,
    .reset_pm = 50,
};
static uint16_t s_classic_release_pm = 100; // Default 10% travel hysteresis (0.4 mm)
static uint8_t s_actuation_offset_pct = 75;

static uint8_t s_scan_rate = 4; // 1000 Hz
#ifdef HHKB_HYBRID
static bool s_bt_oversampling;
static uint8_t s_bt_sleep_minutes = 30;
static const uint8_t deep_choices[] = {0,1,3,5,10,20,30,40,50,60};
static uint8_t s_bt_deep_index = 2; // 3 minutes; separate from inactivity timeout.
static uint8_t bt_power_encode(uint8_t deep_index, uint8_t long_action) {
    return deep_index * 3 + long_action;
}
static bool bt_power_decode(uint8_t value, bool packed, uint8_t *deep_index, uint8_t *long_action) {
    uint8_t bits = value & 31;
    uint8_t index = packed ? bits / 3 : 2;
    uint8_t action = packed ? bits % 3 : (value >> 3) & 3;
    if (index >= 10 || action >= 3) return false;
    *deep_index = index;
    *long_action = action;
    return true;
}
static bool s_bt_matrix_wake;
static uint8_t s_bt_sleep_hz = 1;
static uint8_t s_bt_indicator_flags = 0x0f;
static uint8_t s_bt_short_action = 1; // Stock: show the connected profile.
static uint8_t s_bt_long_action = 0;  // Stock: power off after 2 seconds.
static uint8_t s_bt_last_slot;
#endif
static bool s_power_saving = true; // Enabled by default
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
static volatile bool s_dbg_pending;
static volatile bool s_dbg_down = false;
static uint16_t s_last_reported_raw[MATRIX_COLS][MATRIX_ROWS];
static uint32_t s_last_reported_down_mask[MATRIX_MASK_WORDS];

static inline bool is_reported_down(uint8_t col, uint8_t row) {
    uint8_t idx = col * MATRIX_ROWS + row;
    return (s_last_reported_down_mask[idx / 32] & (1U << (idx % 32))) != 0;
}

static inline void set_reported_down(uint8_t col, uint8_t row, bool down) {
    uint8_t idx = col * MATRIX_ROWS + row;
    if (down) {
        s_last_reported_down_mask[idx / 32] |= (1U << (idx % 32));
    } else {
        s_last_reported_down_mask[idx / 32] &= ~(1U << (idx % 32));
    }
}

static uint16_t compute_crc16(const uint8_t *data, size_t len) {
#ifdef HHKB_COMPACT_STORAGE
    return hhkb_storage_crc16(data, len);
#else
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
#endif
}

#define HHKB_FACTORY_BASELINE_ADDR 0x08081000UL
static uint16_t get_factory_baseline(uint8_t col, uint8_t row) {
#ifdef HHKB_HYBRID
    (void)col;
    (void)row;
    return 0; // Require DMK calibration until Hybrid factory EEPROM is mapped.
#else
    const uint16_t *base = (const uint16_t *)HHKB_FACTORY_BASELINE_ADDR;
    uint8_t idx = (col << 3) | (row & 7);
    uint16_t val = base[idx];
    return (val >= HHKB_CAL_BASE_MIN && val <= 2000) ? val : 0;
#endif
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
extern bool hhkb_eeprom_clear_words(volatile uint32_t *dst, size_t words);

bool vial_keyboard_reset_allowed(void) {
#if defined(HHKB_SW6_REBOOT_TO_BANK1) && HHKB_SW6_REBOOT_TO_BANK1
    return !dip_switch_get(DIP_SW6);
#else
    return true;
#endif
}

bool vial_keyboard_settings_reset(void) {
    s_settings_dirty = false;
#ifdef HHKB_COMPACT_STORAGE
    return hhkb_compact_reset();
#else
    volatile uint32_t *words = (volatile uint32_t *)USER_EEPROM_ADDR;
    const volatile hhkb_eeprom_cal_t *cal = (const volatile hhkb_eeprom_cal_t *)words;
    size_t current_size = cal->magic == USER_EEPROM_MAGIC && cal->version == USER_EEPROM_VERSION
        ? sizeof(*cal) : offsetof(hhkb_eeprom_cal_t, curve);
    size_t size = hhkb_settings_owned_size(cal->magic, USER_EEPROM_MAGIC, current_size,
        offsetof(hhkb_eeprom_cal_t, rt), WORD_ALIGNED_SIZE(offsetof(hhkb_eeprom_cal_t, scan_rate)),
        offsetof(hhkb_eeprom_cal_t, curve));
    if (size == SIZE_MAX) return true; // Foreign data has no DMK record to reset.
    // The version is cleared near the end of the erase. The appended bytes are cleared first.
    return hhkb_eeprom_clear_words(words, size / sizeof(*words));
#endif
}

bool hhkb_settings_storage_writable(void) {
#ifdef HHKB_COMPACT_STORAGE
    return hhkb_compact_writable();
#else
    const volatile uint8_t *bytes = (const volatile uint8_t *)USER_EEPROM_ADDR;
    if (bytes[4] != USER_EEPROM_VERSION || *(const volatile uint32_t *)bytes != USER_EEPROM_MAGIC) {
        for (size_t i = offsetof(hhkb_eeprom_cal_t, curve); i < sizeof(hhkb_eeprom_cal_t); ++i)
            if (bytes[i] != 0) return false;
    }
    return hhkb_settings_region_writable(bytes, USER_EEPROM_MAGIC,
        sizeof(hhkb_eeprom_cal_t), offsetof(hhkb_eeprom_cal_t, rt),
        WORD_ALIGNED_SIZE(offsetof(hhkb_eeprom_cal_t, scan_rate)),
        offsetof(hhkb_eeprom_cal_t, curve));
#endif
}

void hhkb_settings_mark_dirty(void) {
    s_settings_dirty_time = xTaskGetTickCount();
    s_settings_dirty = true;
}

// Called by vial_eeprom_task in the USB task, before the keymap writer.
// Do not call this EEPROM transaction concurrently from another task.
void hhkb_calibration_save_to_eeprom(void) {
    if (!hhkb_settings_storage_writable()) return;
#ifndef HHKB_COMPACT_STORAGE
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        FLASH->PEKEYR = 0x89ABCDEF;
        FLASH->PEKEYR = 0x02030405;
    }
    if (FLASH->PECR & FLASH_PECR_PELOCK) {
        hhkb_settings_mark_dirty();
        return;
    }

#endif
    hhkb_eeprom_cal_t cal = {0};
    cal.magic = USER_EEPROM_MAGIC;
    cal.version = USER_EEPROM_VERSION;
    // The matrix task owns calibration updates; snapshot them atomically.
    taskENTER_CRITICAL();
    cal.actuation_pct = (uint8_t)(s_rt_cfg.act_pm / 10);
    cal.rt = s_rt_cfg;
    memcpy(cal.curve, ec_curve_settings, sizeof(cal.curve));
    cal.debug_flags = s_debug_flags;
#ifdef HHKB_HYBRID
    cal.reserved[0] = 0x80 | (s_bt_oversampling ? 0x40 : 0) | (s_bt_sleep_minutes + 1);
    cal.reserved[1] = (s_bt_matrix_wake ? 1 : 0) |
                      (s_bt_indicator_flags << 1) | (s_bt_short_action << 5);
    // New packed layout: 10 deep choices x 3 long actions fits in 5 bits.
    // debug_flags bit 7 marks it; record size and CRC coverage are unchanged.
    cal.debug_flags |= 0x80;
    cal.reserved[2] = 0x80 | bt_power_encode(s_bt_deep_index, s_bt_long_action) |
                      (s_bt_last_slot << 5);
#endif
    cal.debug_flags |= (s_gesc_ignore & 2) << 4;
    cal.debug_flags |= 0x40; // rt.bottom_dz occupies the former RT padding byte.
    memcpy(cal.led_mode, s_led_mode, sizeof(cal.led_mode));
    cal.led_mode[1] |= (s_gesc_overrides << 4) | ((s_gesc_ignore & 1) << 3);
#ifdef HHKB_HYBRID
    // Spare high nibble; zero in older records means the default 1 Hz.
    cal.led_mode[0] |= s_bt_sleep_hz << 4;
#endif
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

    memcpy(cal.baseline, s_cal_baseline, sizeof(cal.baseline));
    memcpy(cal.peak, s_cal_peak, sizeof(cal.peak));
    taskEXIT_CRITICAL();
    cal.crc16 = compute_crc16((const uint8_t *)&cal.baseline, sizeof(cal) - offsetof(hhkb_eeprom_cal_t, baseline));

#ifdef HHKB_COMPACT_STORAGE
    hhkb_compact_save_cal(&cal);
#else
    volatile uint32_t *dst = (volatile uint32_t *)USER_EEPROM_ADDR;
    const uint32_t *src = (const uint32_t *)&cal;
    for (size_t i = 0; i < sizeof(hhkb_eeprom_cal_t) / 4; i++) {
        if (dst[i] != src[i]) {
            while (FLASH->SR & FLASH_SR_BSY);
            const uint32_t errors = FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR |
                FLASH_SR_NOTZEROERR | FLASH_SR_FWWERR;
            FLASH->SR = errors | FLASH_SR_EOP;
            dst[i] = src[i];
            while (FLASH->SR & FLASH_SR_BSY);
            if ((FLASH->SR & errors) || dst[i] != src[i]) {
                FLASH->PECR |= FLASH_PECR_PELOCK;
                hhkb_settings_mark_dirty();
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
    FLASH->PECR |= FLASH_PECR_PELOCK;
#endif
}

void hhkb_calibration_load(void) {
    const volatile hhkb_eeprom_cal_t *cal = (const volatile hhkb_eeprom_cal_t *)USER_EEPROM_ADDR;
#ifdef HHKB_COMPACT_STORAGE
    hhkb_eeprom_cal_t restored = {0};
    if (hhkb_compact_load_cal(&restored) || !hhkb_compact_legacy_allowed()) cal = &restored;
#endif
    for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            s_cal_baseline[col][row] = get_default_baseline(col, row);
            s_cal_peak[col][row] = get_default_peak(col, row);
        }
    }
    rt_cfg_defaults(&s_rt_cfg);
    s_rt_cfg.act_pm = 750;
    s_actuation_offset_pct = 75;
    s_classic_release_pm = 100;
    s_rt_cfg.press_pm = 25;
    s_rt_cfg.release_pm = 25;
    s_rt_cfg.rt_enabled = false;
    s_debug_flags = 0;
    s_gesc_overrides = 0;
    s_gesc_ignore = 0;

    if (cal->magic == USER_EEPROM_MAGIC) {
        // Versioned format with CRC16
        bool current = cal->version == USER_EEPROM_VERSION;
        size_t size = current ? sizeof(*cal) : offsetof(hhkb_eeprom_cal_t, curve);
        uint16_t expected_crc = compute_crc16((const uint8_t *)&cal->baseline, size - offsetof(hhkb_eeprom_cal_t, baseline));
        if ((current || cal->version == 1) && cal->crc16 == expected_crc) {
            if (current) {
#ifdef HHKB_HYBRID
                uint8_t sleep_encoded = cal->reserved[0] & 0x3f;
                s_bt_oversampling = (cal->reserved[0] & 0xc0) == 0xc0;
                if (sleep_encoded >= 1 && sleep_encoded <= 61) {
                    s_bt_sleep_minutes = sleep_encoded - 1;
                    s_bt_matrix_wake = (cal->reserved[1] & 1) != 0;
                    // Older records used only bit 0; preserve the new defaults.
                    if (cal->reserved[2] & 0x80) {
                        s_bt_indicator_flags = (cal->reserved[1] >> 1) & 0x0f;
                        s_bt_short_action = (cal->reserved[1] >> 5) & 7;
                        bt_power_decode(cal->reserved[2], (cal->debug_flags & 0x80) != 0,
                                        &s_bt_deep_index, &s_bt_long_action);
                        s_bt_last_slot = (cal->reserved[2] >> 5) & 3;
                    } else if (cal->reserved[1] & 0x80) {
                        // Earlier local Hybrid layout kept its marker in byte 1.
                        s_bt_indicator_flags = (cal->reserved[1] >> 1) & 0x0f;
                        s_bt_short_action = (cal->reserved[1] >> 5) & 3;
                        s_bt_long_action = (cal->reserved[2] >> 3) & 3;
                    }
                }
#endif
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
            if (current) {
                for (unsigned i = 0; i < 2; ++i) {
                    uint8_t led = cal->led_mode[i];
                    if (i == 1) led &= 7;
#ifdef HHKB_HYBRID
                    if (i == 0) led &= 0x0f; // High nibble stores sleep scan rate.
#endif
                    if (led <= 4) s_led_mode[i] = led;
                }
#ifdef HHKB_HYBRID
                uint8_t sleep_hz = cal->led_mode[0] >> 4;
                if (sleep_hz >= 1 && sleep_hz <= 10) s_bt_sleep_hz = sleep_hz;
#endif
                s_debug_flags = cal->debug_flags & 0x1f;
                s_gesc_overrides = cal->led_mode[1] >> 4;
                s_gesc_ignore = ((cal->led_mode[1] >> 3) & 1) | ((cal->debug_flags >> 4) & 2);
            }

            if (cal->rt.act_pm >= 25 && cal->rt.act_pm <= (current ? 1125 : 1000)) s_rt_cfg.act_pm = cal->rt.act_pm;
            if (cal->rt.press_pm >= 25 && cal->rt.press_pm <= (current ? 1125 : 1000)) s_rt_cfg.press_pm = cal->rt.press_pm;
            if (cal->rt.release_pm >= 25 && cal->rt.release_pm <= (current ? 1125 : 1000)) s_rt_cfg.release_pm = cal->rt.release_pm;
            if (cal->rt.reset_pm <= 150) s_rt_cfg.reset_pm = cal->rt.reset_pm;
            if (s_rt_cfg.act_pm <= s_rt_cfg.reset_pm) s_rt_cfg.act_pm = s_rt_cfg.reset_pm + 25;
            s_rt_cfg.rt_enabled = cal->rt.rt_enabled;
            if(current && (cal->debug_flags & 0x40) && cal->rt.bottom_dz <= 10) s_rt_cfg.bottom_dz = cal->rt.bottom_dz;
            s_actuation_offset_pct = (uint8_t)(s_rt_cfg.act_pm / 10);
            if (cal->classic_release_pm >= 25 && cal->classic_release_pm <= (current ? 1125 : 1000)) s_classic_release_pm = cal->classic_release_pm;

            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                    uint16_t def_b = get_default_baseline(col, row);
                    uint16_t def_p = get_default_peak(col, row);
                    s_cal_baseline[col][row] = (s_custom_calibration && cal->baseline[col][row] >= HHKB_CAL_BASE_MIN && cal->baseline[col][row] <= 2000) ? cal->baseline[col][row] : def_b;
                    s_cal_peak[col][row] = (s_custom_calibration && cal->peak[col][row] > s_cal_baseline[col][row] + RT_MIN_RANGE && cal->peak[col][row] <= 4095) ? cal->peak[col][row] : def_p;
                }
            }
        }
    }
#ifndef HHKB_JIS
    else if (cal->magic == USER_EEPROM_LEGACY_MAGIC || cal->magic == USER_EEPROM_C_MAGIC || cal->magic == USER_EEPROM_PREV_MAGIC || cal->magic == USER_EEPROM_OLD_MAGIC) {
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
                s_cal_baseline[col][row] = (cal->baseline[col][row] >= HHKB_CAL_BASE_MIN && cal->baseline[col][row] <= 2000) ? cal->baseline[col][row] : def_b;
                s_cal_peak[col][row] = (cal->peak[col][row] > s_cal_baseline[col][row] + RT_MIN_RANGE && cal->peak[col][row] <= 4095) ? cal->peak[col][row] : def_p;
            }
        }
    }
#endif
    else {
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
    // The hardware initializer rebuilds the LUT after this loader returns.
#ifdef HHKB_COMPACT_STORAGE
    hhkb_calibration_save_to_eeprom();
#endif
}

extern bool hhkb_compact_busy(void);
bool vial_keyboard_eeprom_busy(void) {
    return s_settings_dirty || hhkb_compact_busy();
}

void vial_keyboard_eeprom_task(void) {
    if (!s_settings_dirty) return;
    TickType_t now = xTaskGetTickCount();
    if ((now - s_settings_dirty_time) >= pdMS_TO_TICKS(HHKB_SETTINGS_SAVE_DELAY_MS)) {
        taskENTER_CRITICAL();
        s_settings_dirty = false;
        taskEXIT_CRITICAL();
        ec_curve_rebuild();
        hhkb_calibration_save_to_eeprom();
    }
}

static volatile uint8_t s_cal_request;

void hhkb_calibration_request_start(void) { cal_request_post(&s_cal_request, CAL_REQ_START); }
void hhkb_calibration_request_finish(void) { cal_request_post(&s_cal_request, CAL_REQ_FINISH); }

// Matrix task only, outside a scan.
void hhkb_calibration_apply_pending(void) {
    cal_request_t req = cal_request_take(&s_cal_request);
    if (req == CAL_REQ_START) hhkb_calibration_start();
    else if (req == CAL_REQ_FINISH) hhkb_calibration_finish();
}

/* Matrix task publishes small events; only USB formats monitor messages.
 * Coalesce updates if the host cannot drain the console. */
enum { CAL_LOG_START=1, CAL_LOG_BASE, CAL_LOG_KEY, CAL_LOG_FINISH };
static volatile uint8_t s_cal_log_event;
static uint8_t s_cal_log_key;
static void cal_log(uint8_t event, uint8_t key) {
    if(!hhkb_get_debug_flag(DEBUG_FLAG_CAL)) return;
    taskENTER_CRITICAL();
    s_cal_log_key=key; s_cal_log_event=event;
    taskEXIT_CRITICAL();
}
static void cal_flush_log(void) {
    if(!s_cal_log_event) return;
    taskENTER_CRITICAL();
    uint8_t event=s_cal_log_event, key=s_cal_log_key;
    s_cal_log_event=0;
    uint8_t count=event ? calibrated_key_count() : 0;
    uint8_t col=key/MATRIX_ROWS, row=key%MATRIX_ROWS;
    uint16_t base=s_capture_base[col][row], peak=s_capture_peak[col][row];
    taskEXIT_CRITICAL();
    if(!hhkb_get_debug_flag(DEBUG_FLAG_CAL)) return;
    switch(event) {
    case CAL_LOG_START: debug_console_printf("[CAL] Capturing baseline (16 samples).\r\n"); break;
    case CAL_LOG_BASE: debug_console_printf("[CAL] Baseline captured. Press each key firmly.\r\n"); break;
    case CAL_LOG_KEY: debug_console_printf("[CAL] %u/%u R%u C%u peak=%u base=%u\r\n", count,NUM_KEYS,row,col,peak,base); break;
    case CAL_LOG_FINISH: debug_console_printf("[CAL] Finished. Updated %u/%u keys.\r\n",count,NUM_KEYS); break;
    }
}

void hhkb_calibration_start(void) {
    memset(s_capture_base, 0, sizeof(s_capture_base));
    s_cal_baseline_samples = CAL_BASELINE_SAMPLES;
    memset(s_calibrated_mask, 0, sizeof(s_calibrated_mask));
    memset(s_cal_down_mask, 0, sizeof(s_cal_down_mask));
    s_capture_baseline = true;
    s_calibration_mode = true;
    cal_log(CAL_LOG_START,0);
}

void hhkb_calibration_finish(void) {
    if (!s_calibration_mode) return;
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            uint8_t k_idx = c * MATRIX_ROWS + r;
            bool was_actuated = (s_calibrated_mask[k_idx / 32] & (1U << (k_idx % 32))) != 0;
            // Do not save a range that the loader would reject after reboot.
            if (was_actuated && s_capture_base[c][r] >= HHKB_CAL_BASE_MIN &&
                s_capture_base[c][r] <= 2000 &&
                s_capture_peak[c][r] > s_capture_base[c][r] + RT_MIN_RANGE &&
                s_capture_peak[c][r] <= 4095) {
                s_cal_baseline[c][r] = s_capture_base[c][r];
                s_cal_peak[c][r] = s_capture_peak[c][r];
            } else {
                // Unusable captures remain unchanged, and must not count as saved.
                s_calibrated_mask[k_idx / 32] &= ~(1U << (k_idx % 32));
            }
            rt_key_set_range(&s_rt_keys[c][r], s_cal_baseline[c][r], s_cal_peak[c][r]);
            rt_key_reset(&s_rt_keys[c][r]);
        }
    }
    cal_log(CAL_LOG_FINISH,0);
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

uint8_t hhkb_gesc_ignore(void) { return s_gesc_ignore; }
void hhkb_gesc_set_ignore(uint8_t value) {
    if(value<=3 && value!=s_gesc_ignore) { s_gesc_ignore=value; hhkb_settings_mark_dirty(); }
}
uint8_t hhkb_gesc_overrides(void) { return s_gesc_overrides; }
void hhkb_gesc_set_override(uint8_t index, bool enabled) {
    if (index >= 4) return;
    uint8_t mask = 1U << index;
    uint8_t value = enabled ? s_gesc_overrides | mask : s_gesc_overrides & ~mask;
    if (value == s_gesc_overrides) return;
    s_gesc_overrides = value;
    hhkb_settings_mark_dirty();
}

uint8_t hhkb_get_scan_rate(void) { return s_scan_rate; }
void hhkb_set_scan_rate(uint8_t rate) {
    if (rate > 4) return;
    taskENTER_CRITICAL();
    s_scan_rate = rate;
    taskEXIT_CRITICAL();
    hhkb_settings_mark_dirty();
}

bool hhkb_get_power_saving(void) { return s_power_saving; }
void hhkb_set_power_saving(bool enabled) {
    if (s_power_saving == enabled) return;
    s_power_saving = enabled;
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
    case 65: return s_rt_cfg.bottom_dz;
    case 4: return s_rt_cfg.press_pm / 25;
    case 5: return s_classic_release_pm / 25;
    case 11: return s_rt_cfg.release_pm / 25;
    case 6: return s_rt_cfg.reset_pm / 25;
    default: return 0;
    }
}

void hhkb_rt_set(uint8_t id, uint8_t value) {
    taskENTER_CRITICAL();
    if (id == 65 && value <= 10) {
        s_rt_cfg.bottom_dz = value;
    } else if (id == 3) {
        s_rt_cfg.rt_enabled = value != 0;
    } else if (id == 6) {
        if (value <= 6) {
            s_rt_cfg.reset_pm = value * 25;
            if (s_rt_cfg.act_pm <= s_rt_cfg.reset_pm) s_rt_cfg.act_pm = s_rt_cfg.reset_pm + 25;
        }
    } else if (value >= 1 && value <= 45) {
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
    uint8_t previous = s_debug_flags;
    if (enabled) s_debug_flags |= flag;
    else s_debug_flags &= ~flag;
    if (previous != s_debug_flags) hhkb_settings_mark_dirty();
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

    if (is_pressed == was_pressed) {
        if (!is_pressed && norm < 50) {
            if (s_last_reported_raw[col][row] != base) s_last_reported_raw[col][row] = base;
            return;
        }
        int diff = (int)raw - s_last_reported_raw[col][row];
        if (diff >= -40 && diff <= 40) return;
    } else {
        set_reported_down(col, row, is_pressed);
    }
    s_last_reported_raw[col][row] = raw;
    taskENTER_CRITICAL();
    s_dbg_col = col;
    s_dbg_row = row;
    s_dbg_raw = raw;
    s_dbg_norm = norm;
    s_dbg_down = is_pressed;
    s_dbg_pending = true;
    taskEXIT_CRITICAL();
}

// USB consumes the latest travel update when the console endpoint is available.
void hhkb_debug_flush_sample(void) {
    cal_flush_log();
    if (!s_dbg_pending) return;
    taskENTER_CRITICAL();
    uint8_t row = s_dbg_row, col = s_dbg_col;
    uint16_t raw = s_dbg_raw, norm = s_dbg_norm;
    bool down = s_dbg_down;
    s_dbg_pending = false;
    taskEXIT_CRITICAL();
    if (hhkb_get_debug_enabled())
        debug_console_printf("[EC] R%u C%u : raw=%u base=%u peak=%u travel=%u%% %s\r\n",
            row, col, raw, s_cal_baseline[col][row], s_cal_peak[col][row], norm / 10, down ? "DN" : "UP");
}

void hhkb_get_debug_key_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "C%u R%u (%s)", s_dbg_col, s_dbg_row, s_dbg_down ? "Pressed" : "Released");
}

void hhkb_get_debug_raw_str(char *buf, size_t max_len) {
    snprintf(buf, max_len, "Raw:%u (B:%u P:%u)", s_dbg_raw, s_cal_baseline[s_dbg_col][s_dbg_row], s_cal_peak[s_dbg_col][s_dbg_row]);
}

void hhkb_get_debug_travel_str(char *buf, size_t max_len) {
    if (ec_curve_lut) {
        unsigned depth_um = s_dbg_norm * 4u;
        snprintf(buf, max_len, "Travel: %u.%03u mm", depth_um / 1000, depth_um % 1000);
    } else {
        snprintf(buf, max_len, "Travel: %u.%u%%", s_dbg_norm / 10, s_dbg_norm % 10);
    }
}

void hhkb_cal_print_keys(void) {
    debug_console_printf(s_calibration_mode ? "[CAL] missing:" : "[CAL] unchanged:");
    unsigned missing = 0;
    for (uint8_t c = 0; c < MATRIX_COLS; ++c) {
        for (uint8_t r = 0; r < MATRIX_ROWS; ++r) {
            if (keyboard_get_flat_key_index(r, c) < 0) continue;
            unsigned index = c * MATRIX_ROWS + r;
            if (!(s_calibrated_mask[index / 32] & (1U << (index % 32)))) {
                debug_console_printf(" R%u C%u", r, c);
                ++missing;
            }
        }
    }
    debug_console_printf(missing ? "\r\n" : " none\r\n");
}

void hhkb_get_cal_status_str(char *buf, size_t max_len) {
    if (s_cal_request == CAL_REQ_START) {
        snprintf(buf, max_len, "In progress: 0/%u keys", NUM_KEYS);
        return;
    }
    if (s_calibration_mode) {
        uint8_t count = calibrated_key_count();
        snprintf(buf, max_len, "In progress: %u/%u keys", count, NUM_KEYS);
        return;
    }
    if (!s_custom_calibration) {
        snprintf(buf, max_len, "Using defaults");
        return;
    }

#ifdef HHKB_COMPACT_STORAGE
    const char *status = hhkb_compact_status();
    if (strcmp(status, "Saved (compressed)") != 0 || s_settings_dirty) {
        snprintf(buf, max_len, "%s", s_settings_dirty ? "Pending" : status);
        return;
    }
    const volatile uint16_t (*saved_base)[MATRIX_ROWS] = s_cal_baseline;
    const volatile uint16_t (*saved_peak)[MATRIX_ROWS] = s_cal_peak;
#else
    const volatile hhkb_eeprom_cal_t *cal = (const volatile hhkb_eeprom_cal_t *)USER_EEPROM_ADDR;
    size_t size = cal->version == USER_EEPROM_VERSION ? sizeof(*cal) : offsetof(hhkb_eeprom_cal_t, curve);
    if (cal->magic != USER_EEPROM_MAGIC ||
        (cal->version != USER_EEPROM_VERSION && cal->version != 1) ||
        cal->crc16 != compute_crc16((const uint8_t *)&cal->baseline,
                                    size - offsetof(hhkb_eeprom_cal_t, baseline))) {
        snprintf(buf, max_len, "No verified EEPROM record");
        return;
    }
    const volatile uint16_t (*saved_base)[MATRIX_ROWS] = cal->baseline;
    const volatile uint16_t (*saved_peak)[MATRIX_ROWS] = cal->peak;
#endif
    unsigned custom_count = 0;
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            if (keyboard_get_flat_key_index(r, c) < 0) continue;
            uint16_t base = saved_base[c][r];
            uint16_t peak = saved_peak[c][r];
            if (base >= HHKB_CAL_BASE_MIN && base <= 2000 && peak > base + RT_MIN_RANGE && peak <= 4095 &&
                (base != get_default_baseline(c, r) || peak != get_default_peak(c, r))) {
                custom_count++;
            }
        }
    }
    snprintf(buf, max_len, "%u/%u saved custom ranges", custom_count, NUM_KEYS);
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
void hhkb_cal_accumulate_baseline(uint8_t col, uint8_t row, uint16_t val) {
    s_capture_base[col][row] += val;
}
void hhkb_cal_record_peak(uint8_t col, uint8_t row, uint16_t val) {
    if (val > s_capture_peak[col][row]) s_capture_peak[col][row] = val;

    uint16_t press_thresh = s_capture_base[col][row] + RT_MIN_RANGE;
    uint16_t release_thresh = s_capture_base[col][row] + (RT_MIN_RANGE / 2);
    uint8_t k_idx = col * MATRIX_ROWS + row;
    bool is_down = (s_cal_down_mask[k_idx / 32] & (1U << (k_idx % 32))) != 0;

    if (!is_down && val >= press_thresh) {
        // Transition: key pressed down
        s_cal_down_mask[k_idx / 32] |= (1U << (k_idx % 32));

        s_cal_press_start_peak[col][row] = s_capture_peak[col][row];

        bool newly_calibrated = !(s_calibrated_mask[k_idx / 32] & (1U << (k_idx % 32)));
        s_calibrated_mask[k_idx / 32] |= 1U << (k_idx % 32);
        if (newly_calibrated) cal_log(CAL_LOG_KEY,k_idx);
    } else if (is_down && val < release_thresh) {
        // Transition: key released
        s_cal_down_mask[k_idx / 32] &= ~(1U << (k_idx % 32));

        uint16_t old_peak = s_cal_press_start_peak[col][row];
        uint16_t new_peak = s_capture_peak[col][row];
        if (old_peak > press_thresh && new_peak > old_peak + 20 && hhkb_get_debug_flag(DEBUG_FLAG_CAL)) {
            cal_log(CAL_LOG_KEY,k_idx);
        }
    }
}
bool hhkb_cal_is_capturing_baseline(void) { return s_calibration_mode && s_capture_baseline; }
void hhkb_cal_decrement_samples(void) {
    if (--s_cal_baseline_samples == 0) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                s_capture_base[c][r] /= CAL_BASELINE_SAMPLES;
                s_capture_peak[c][r] = s_capture_base[c][r];
                s_cal_press_start_peak[c][r] = s_capture_base[c][r];
            }
        }
        s_capture_baseline = false;
        cal_log(CAL_LOG_BASE,0);
    }
}
rt_key_t *hhkb_get_rt_key(uint8_t col, uint8_t row) { return &s_rt_keys[col][row]; }

#ifdef HHKB_HYBRID
uint8_t hhkb_bt_deep_minutes(void) { return deep_choices[s_bt_deep_index]; }
bool hhkb_bt_set_deep_minutes(uint8_t minutes) {
    for (uint8_t i = 0; i < sizeof(deep_choices); ++i) {
        if (deep_choices[i] != minutes) continue;
        if (s_bt_deep_index != i) { s_bt_deep_index = i; hhkb_settings_mark_dirty(); }
        return true;
    }
    return false;
}
uint8_t hhkb_bt_sleep_minutes(void) { return s_bt_sleep_minutes; }
uint8_t hhkb_bt_sleep_hz(void) { return s_bt_sleep_hz; }
bool hhkb_bt_set_sleep_hz(uint8_t hz) {
    if (hz < 1 || hz > 10) return false;
    if (s_bt_sleep_hz != hz) { s_bt_sleep_hz = hz; hhkb_settings_mark_dirty(); }
    return true;
}

bool hhkb_bt_matrix_wake(void) { return s_bt_matrix_wake; }

void hhkb_bt_set_sleep(uint8_t minutes, bool matrix_wake) {
    s_bt_sleep_minutes = minutes;
    s_bt_matrix_wake = matrix_wake;
    hhkb_settings_mark_dirty();
}
#endif

uint8_t hhkb_get_effective_scan_rate(void) {
#ifdef HHKB_HYBRID
    if (hhkb_hybrid_on_battery()) return 6; // Fixed 11 ms battery scan, independent of legacy EEPROM rate.
#endif
    return hhkb_get_scan_rate();
}

#ifdef HHKB_HYBRID
bool hhkb_bt_oversampling(void) { return s_bt_oversampling; }
void hhkb_bt_set_oversampling(bool enabled) {
    if (s_bt_oversampling == enabled) return;
    s_bt_oversampling = enabled;
    hhkb_settings_mark_dirty();
}
uint8_t hhkb_bt_indicator_flags(void) { return s_bt_indicator_flags; }
void hhkb_bt_set_indicator_flag(uint8_t flag, bool enabled) {
    if (flag >= 4) return;
    uint8_t mask = 1U << flag;
    uint8_t updated = enabled ? (s_bt_indicator_flags | mask) : (s_bt_indicator_flags & ~mask);
    if (updated == s_bt_indicator_flags) return;
    s_bt_indicator_flags = updated;
    hhkb_settings_mark_dirty();
}
uint8_t hhkb_bt_button_action(bool long_press) {
    return long_press ? s_bt_long_action : s_bt_short_action;
}
void hhkb_bt_set_button_action(bool long_press, uint8_t action) {
    if (action > (long_press ? 2 : 5)) return;
    uint8_t *target = long_press ? &s_bt_long_action : &s_bt_short_action;
    if (*target == action) return;
    *target = action;
    hhkb_settings_mark_dirty();
}
uint8_t hhkb_bt_last_slot(void) { return s_bt_last_slot; }
void hhkb_bt_set_last_slot(uint8_t slot) {
    if (slot > 3 || slot == s_bt_last_slot) return;
    s_bt_last_slot = slot;
    hhkb_settings_mark_dirty();
}
#endif
