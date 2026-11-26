#ifndef EC_SETTINGS_H
#define EC_SETTINGS_H

#include "config.h"
#include "rapid_trigger.h"
#include "ec_curve.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HHKB_JIS
#define USER_EEPROM_MAGIC          0x484B4A45UL // JIS record geometry
#else
#define USER_EEPROM_MAGIC          0x484B4345UL // "HKCE" (versioned with CRC16)
#endif
#define USER_EEPROM_LEGACY_MAGIC   0x484B4344UL // "HKCD"
#define USER_EEPROM_C_MAGIC        0x484B4343UL // "HKCC"
#define USER_EEPROM_PREV_MAGIC     0x484B4342UL // "HKCB"
#define USER_EEPROM_OLD_MAGIC      0x484B4341UL // "HKCA"
#define USER_EEPROM_ADDR           0x08081500UL
#define USER_EEPROM_VERSION        2

typedef struct __attribute__((aligned(4))) {
    uint32_t magic;
    uint8_t  version;
    uint8_t  actuation_pct;
    uint16_t crc16;
    uint16_t baseline[MATRIX_COLS][MATRIX_ROWS]; // 120 bytes
    uint16_t peak[MATRIX_COLS][MATRIX_ROWS];     // 120 bytes
    rt_cfg_t rt;                                 // 10 bytes
    uint8_t  scan_rate;                          // bits 0..2: 0..4, bits 3..4: Grave Escape ignore, bit 7: power saving
    uint8_t  tapping_term_10ms;                  // 5..50
    uint8_t  quick_tap_10ms;                     // 0..50
    uint8_t  flags;                              // bit 0: permissive_hold, bit 1: tap_pref, bit 2: oversampling, bit 3: custom_cal, bit 4: debug, bit 5: system, bit 6..7: led0
    uint16_t classic_release_pm;
    uint8_t curve[EC_CURVE_FIELDS];
    uint8_t led_mode[2]; // led_mode[1] high nibble: Grave Escape overrides; Hybrid: led_mode[0] high nibble stores sleep Hz (0 = legacy 1 Hz)
    uint8_t debug_flags; // bits 0..5: debug; bit 6: bottom-zone byte valid; bit 7: Hybrid deep-timer format
    uint8_t reserved[3];
} hhkb_eeprom_cal_t;
_Static_assert(sizeof(rt_cfg_t)==10, "Preserve RT EEPROM layout");
_Static_assert(offsetof(hhkb_eeprom_cal_t, curve) == 24 + 4 * MATRIX_ROWS * MATRIX_COLS, "Preserve the v1 settings record");

// Calibration & Settings API
void hhkb_settings_init(void);
void hhkb_calibration_load(void);
void hhkb_calibration_save_to_eeprom(void);
// start/finish: matrix task only (call apply_pending at a scan boundary).
// Other tasks must use the request_* functions.
void hhkb_calibration_start(void);
void hhkb_calibration_finish(void);
void hhkb_calibration_request_start(void);
void hhkb_calibration_request_finish(void);
void hhkb_calibration_apply_pending(void);
bool hhkb_calibration_enabled(void);
void hhkb_calibration_set_enabled(bool enabled);
bool hhkb_is_calibrating(void);
bool hhkb_settings_storage_writable(void);
void hhkb_settings_mark_dirty(void);
void vial_keyboard_eeprom_task(void);

#ifdef HHKB_HYBRID
uint8_t hhkb_bt_sleep_minutes(void);
uint8_t hhkb_bt_sleep_hz(void);
bool hhkb_bt_set_sleep_hz(uint8_t hz);
bool hhkb_bt_matrix_wake(void);
void hhkb_bt_set_sleep(uint8_t minutes, bool matrix_wake);
uint8_t hhkb_bt_indicator_flags(void);
void hhkb_bt_set_indicator_flag(uint8_t flag, bool enabled);
uint8_t hhkb_bt_button_action(bool long_press);
void hhkb_bt_set_button_action(bool long_press, uint8_t action);
uint8_t hhkb_bt_last_slot(void);
void hhkb_bt_set_last_slot(uint8_t slot);
#endif

// Grave Escape overrides: Alt, Ctrl, GUI, Shift (both sides).
uint8_t hhkb_gesc_overrides(void);
uint8_t hhkb_gesc_ignore(void);
void hhkb_gesc_set_ignore(uint8_t value);
void hhkb_gesc_set_override(uint8_t index, bool enabled);

// Getters & Setters for VIA
uint8_t hhkb_get_scan_rate(void);
uint8_t hhkb_get_effective_scan_rate(void);
void hhkb_set_scan_rate(uint8_t rate);
bool hhkb_get_power_saving(void);
void hhkb_set_power_saving(bool enabled);
bool hhkb_get_oversampling(void);
void hhkb_set_oversampling(bool enabled);
uint8_t hhkb_get_hold_tap_setting(uint8_t id);
void hhkb_set_hold_tap_setting(uint8_t id, uint8_t value);
uint16_t hold_tap_config_value(uint8_t id);
uint8_t hhkb_get_led_mode(uint8_t index);
void hhkb_set_led_mode(uint8_t index, uint8_t mode);
uint8_t hhkb_get_system(void);
void hhkb_set_system(uint8_t system);
uint8_t hhkb_rt_get(uint8_t id);
void hhkb_rt_set(uint8_t id, uint8_t value);
void hhkb_set_actuation_point_pct(uint8_t pct);
uint8_t hhkb_get_actuation_point_pct(void);

// Debug Console & Monitoring API
enum {
    DEBUG_FLAG_USB   = 1U << 4,
    DEBUG_FLAG_EC    = 1U << 0, // 0x01: EC matrix debug [EC]
    DEBUG_FLAG_TOPRE = 1U << 0, // Backward-compatible alias
    DEBUG_FLAG_VIA   = 1U << 1, // 0x02: VIA telemetry diag [VIA_DIAG]
    DEBUG_FLAG_RATE  = 1U << 2, // 0x04: Polling/scan rate monitor [RATE]
    DEBUG_FLAG_CAL   = 1U << 3, // 0x08: Calibration progress monitor [CAL]
    DEBUG_FLAG_EEPROM = 1U << 5, // 0x20: EEPROM write diagnostics [EEPROM]
};

bool hhkb_get_debug_flag(uint8_t flag);
void hhkb_set_debug_flag(uint8_t flag, bool enabled);
void hhkb_record_matrix_scan(void);
uint32_t hhkb_get_matrix_rate_hz(void);
void hhkb_calc_rates(uint32_t elapsed_ms);

bool hhkb_get_debug_enabled(void);
void hhkb_set_debug_enabled(bool enabled);
void hhkb_debug_feed_sample(uint8_t col, uint8_t row, uint16_t raw, uint16_t norm, bool is_pressed);
void hhkb_get_debug_key_str(char *buf, size_t max_len);
void hhkb_get_debug_raw_str(char *buf, size_t max_len);
void hhkb_get_debug_travel_str(char *buf, size_t max_len);
void hhkb_cal_print_keys(void);
void hhkb_get_cal_status_str(char *buf, size_t max_len);

// Direct matrix arrays for zero-call-overhead inner loop scan
extern uint16_t s_cal_peak[MATRIX_COLS][MATRIX_ROWS];
extern rt_key_t s_rt_keys[MATRIX_COLS][MATRIX_ROWS];

// Matrix accessors for scan module
rt_cfg_t hhkb_get_scan_cfg(void);
uint16_t hhkb_get_cal_baseline(uint8_t col, uint8_t row);
uint16_t hhkb_get_cal_peak(uint8_t col, uint8_t row);
void hhkb_cal_accumulate_baseline(uint8_t col, uint8_t row, uint16_t val);
void hhkb_cal_record_peak(uint8_t col, uint8_t row, uint16_t val);
bool hhkb_cal_is_capturing_baseline(void);
void hhkb_cal_decrement_samples(void);
void hhkb_cal_log_flush(void);
rt_key_t *hhkb_get_rt_key(uint8_t col, uint8_t row);

#ifdef __cplusplus
}
#endif

#endif // EC_SETTINGS_H

bool hhkb_bt_oversampling(void);
void hhkb_bt_set_oversampling(bool enabled);

uint8_t hhkb_bt_deep_minutes(void);
bool hhkb_bt_set_deep_minutes(uint8_t minutes);
