"""Unit tests for the minimal dmk_snprintf formatter, refactored VIA controls, and RT clamp."""
import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from .test_hybrid_button import ROOT, function


class DmkFmtAndViaControlsTest(unittest.TestCase):
    def test_dmk_vsnprintf_formats_and_truncation(self):
        usb_desc = (ROOT / 'dmk_core/drivers/usb_descriptors.c').read_text()
        code = r'''
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
''' + function(usb_desc, 'static void fmt_emit(') + '\n' + function(usb_desc, 'static void fmt_uint(') + '\n' + function(usb_desc, 'int dmk_vsnprintf(') + '\n' + function(usb_desc, 'int dmk_snprintf(') + r'''
int main(void) {
    char buf[64];
    int n = dmk_snprintf(buf, sizeof(buf), "%% %c %s %s", 'A', "hi", (const char *)NULL);
    assert(n == 13 && strcmp(buf, "% A hi (null)") == 0);

    n = dmk_snprintf(buf, sizeof(buf), "%u %lu %d %ld", 0u, 1234567890UL, -42, -987654321L);
    assert(strcmp(buf, "0 1234567890 -42 -987654321") == 0);
    assert(n == (int)strlen(buf));

    dmk_snprintf(buf, sizeof(buf), "%d", INT_MIN);
    assert(strcmp(buf, "-2147483648") == 0);

    dmk_snprintf(buf, sizeof(buf), "%u.%03u mm 0x%04x %X", 2u, 45u, 0x2au, 0xfeU);
    assert(strcmp(buf, "2.045 mm 0x002a FE") == 0);

    char small[6];
    n = dmk_snprintf(small, sizeof(small), "123456789");
    assert(n == 9 && strcmp(small, "12345") == 0);

    // Zero-size buffer must not write out of bounds and still return required length.
    n = dmk_snprintf(NULL, 0, "abc");
    assert(n == 3);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'fmt_test.c'
            exe = Path(tmp) / 'fmt_test'
            src.write_text(code)
            subprocess.run(['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_via_controls_dispatch_round_trip(self):
        via_src = (ROOT / 'keyboards/hhkb/modules/via_settings/hhkb_via_controls.c').read_text()
        body = via_src[via_src.index('static void copy_str28('):]
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define DEBUG_FLAG_TOPRE 1
#define DEBUG_FLAG_VIA   2
#define DEBUG_FLAG_RATE  4
#define DEBUG_FLAG_CAL   8
#define DEBUG_FLAG_USB   16
#define EC_CURVE_ENABLE  30
#define EC_CURVE_STATUS  37
#define DMK_FW_VERSION   "26.11.25-test"

uint8_t ec_curve_settings[8];
const char *ec_curve_status = "Curve OK";
static uint8_t s_dbg_flags, s_gesc_ovr, s_gesc_ign, s_power_saving, s_oversampling;
static uint8_t s_scan_rate = 4, s_ht[4], s_led[2], s_rt[12];
static bool s_cal_enabled = true, s_sof = true, s_bt_over, s_bt_wake;
static uint8_t s_bt_hz = 1, s_bt_deep = 3, s_bt_sleep = 30, s_bt_ind = 0x0f;
static uint8_t s_bt_act[2] = {1, 0};
static unsigned s_dirty, s_cal_start, s_cal_finish, s_cal_print, s_show_bat, s_slot_req;

void hhkb_settings_mark_dirty(void) { ++s_dirty; }
uint8_t hhkb_gesc_ignore(void) { return s_gesc_ign; }
void hhkb_gesc_set_ignore(uint8_t v) { s_gesc_ign = v; }
uint8_t hhkb_gesc_overrides(void) { return s_gesc_ovr; }
void hhkb_gesc_set_override(uint8_t i, bool e) { if (e) s_gesc_ovr |= 1u << i; else s_gesc_ovr &= ~(1u << i); }
bool sof_lock_enabled(void) { return s_sof; }
bool sof_lock_set_enabled(bool e) { s_sof = e; return true; }
bool hhkb_get_debug_flag(uint8_t m) { return (s_dbg_flags & m) != 0; }
void hhkb_set_debug_flag(uint8_t m, bool e) { if (e) s_dbg_flags |= m; else s_dbg_flags &= ~m; }
uint8_t hhkb_bt_sleep_hz(void) { return s_bt_hz; }
bool hhkb_bt_set_sleep_hz(uint8_t v) { if (v < 1 || v > 10) return false; s_bt_hz = v; return true; }
uint8_t hhkb_bt_deep_minutes(void) { return s_bt_deep; }
bool hhkb_bt_set_deep_minutes(uint8_t v) { s_bt_deep = v; return true; }
bool hhkb_bt_oversampling(void) { return s_bt_over; }
void hhkb_bt_set_oversampling(bool e) { s_bt_over = e; }
uint8_t hhkb_bt_button_action(bool l) { return s_bt_act[l]; }
void hhkb_bt_set_button_action(bool l, uint8_t v) { s_bt_act[l] = v; }
uint8_t hhkb_hybrid_battery_level(void) { return 75; }
uint8_t hhkb_bt_indicator_flags(void) { return s_bt_ind; }
void hhkb_bt_set_indicator_flag(uint8_t i, bool e) { if (e) s_bt_ind |= 1u << i; else s_bt_ind &= ~(1u << i); }
void hhkb_hybrid_show_battery(void) { ++s_show_bat; }
uint8_t hhkb_bt_sleep_minutes(void) { return s_bt_sleep; }
bool hhkb_bt_matrix_wake(void) { return s_bt_wake; }
void hhkb_bt_set_sleep(uint8_t m, bool w) { s_bt_sleep = m; s_bt_wake = w; }
bool hhkb_hybrid_request_slot(uint8_t s, bool p) { (void)s; (void)p; ++s_slot_req; return true; }
bool ec_curve_set(uint8_t id, uint8_t v) { ec_curve_settings[id - EC_CURVE_ENABLE] = v; return true; }
const char *hhkb_compact_status(void) { return "Saved (compressed)"; }
void hhkb_get_cal_status_str(char *b, size_t n) { strncpy(b, "Cal OK", n); b[n - 1] = '\0'; }
void hhkb_get_debug_key_str(char *b, size_t n) { strncpy(b, "C0 R0", n); b[n - 1] = '\0'; }
void hhkb_get_debug_raw_str(char *b, size_t n) { strncpy(b, "Raw:100", n); b[n - 1] = '\0'; }
void hhkb_get_debug_travel_str(char *b, size_t n) { strncpy(b, "Travel:2.0", n); b[n - 1] = '\0'; }
uint8_t hhkb_get_power_saving(void) { return s_power_saving; }
void hhkb_set_power_saving(bool e) { s_power_saving = e; }
uint8_t hhkb_get_oversampling(void) { return s_oversampling; }
void hhkb_set_oversampling(bool e) { s_oversampling = e; }
uint8_t hhkb_get_hold_tap_setting(uint8_t i) { return s_ht[i]; }
void hhkb_set_hold_tap_setting(uint8_t i, uint8_t v) { s_ht[i] = v; }
uint8_t hhkb_get_scan_rate(void) { return s_scan_rate; }
void hhkb_set_scan_rate(uint8_t v) { s_scan_rate = v; }
bool hhkb_calibration_enabled(void) { return s_cal_enabled; }
void hhkb_calibration_set_enabled(bool e) { s_cal_enabled = e; }
uint8_t hhkb_get_led_mode(uint8_t i) { return s_led[i]; }
void hhkb_set_led_mode(uint8_t i, uint8_t v) { s_led[i] = v; }
void hhkb_calibration_request_start(void) { ++s_cal_start; }
void hhkb_calibration_request_finish(void) { ++s_cal_finish; }
void hhkb_cal_print_keys(void) { ++s_cal_print; }
void debug_console_printf(const char *fmt, ...) { (void)fmt; }
uint8_t hhkb_rt_get(uint8_t id) { return s_rt[id]; }
void hhkb_rt_set(uint8_t id, uint8_t v) { s_rt[id] = v; }

''' + body + r'''
static bool call(uint8_t op, uint8_t id, uint8_t val, uint8_t *resp) {
    uint8_t req[32] = {op, 0, id, val};
    return via_custom_value_command_kb(req, resp);
}

int main(void) {
    uint8_t resp[32] = {0};
    // Save command (0x09)
    assert(call(0x09, 0, 0, resp) && s_dirty == 1);
    // Debug flags 19, 23, 24, 25, 62
    const uint8_t dbg_ids[] = {19, 23, 24, 25, 62};
    for (unsigned i = 0; i < 5; i++) {
        assert(call(0x07, dbg_ids[i], 1, resp));
        assert(call(0x08, dbg_ids[i], 0, resp) && resp[3] == 1);
        assert(!call(0x07, dbg_ids[i], 2, resp));
        assert(call(0x07, dbg_ids[i], 0, resp));
    }
    // Read-only strings 12, 20, 21, 22, 26, 28, EC_CURVE_STATUS
    assert(call(0x08, 12, 0, resp) && strcmp((char *)&resp[3], "Saved (compressed)") == 0);
    assert(call(0x08, 28, 0, resp) && strcmp((char *)&resp[3], DMK_FW_VERSION) == 0);
    assert(call(0x08, 26, 0, resp) && strcmp((char *)&resp[3], "Cal OK") == 0);
    assert(call(0x08, 20, 0, resp) && strcmp((char *)&resp[3], "C0 R0") == 0);
    assert(call(0x08, 21, 0, resp) && strcmp((char *)&resp[3], "Raw:100") == 0);
    assert(call(0x08, 22, 0, resp) && strcmp((char *)&resp[3], "Travel:2.0") == 0);
    assert(call(0x08, EC_CURVE_STATUS, 0, resp) && strcmp((char *)&resp[3], "Curve OK") == 0);
    assert(!call(0x07, 12, 1, resp) && !call(0x07, 28, 1, resp) && !call(0x07, 26, 1, resp));

    // Hybrid controls
    assert(call(0x08, 44, 0, resp) && resp[3] == 1);
    assert(call(0x08, 50, 0, resp) && resp[3] == 75 && !call(0x07, 50, 1, resp));
    assert(call(0x07, 56, 5, resp) && call(0x08, 56, 0, resp) && resp[3] == 5);
    assert(!call(0x07, 56, 11, resp));
    assert(call(0x07, 51, 1, resp) && s_show_bat == 1 && !call(0x07, 51, 0, resp));

    // Calibration triggers, LED, RT, Hold-Tap, print
    assert(call(0x07, 7, 1, resp) && s_cal_start == 1 && call(0x07, 7, 0, resp) && s_cal_start == 1);
    assert(call(0x07, 8, 1, resp) && s_cal_finish == 1);
    assert(call(0x07, 27, 1, resp) && s_cal_print == 1);
    assert(call(0x07, 9, 3, resp) && call(0x08, 9, 0, resp) && resp[3] == 3);
    assert(call(0x07, 14, 22, resp) && call(0x08, 14, 0, resp) && resp[3] == 22);
    assert(call(0x07, 3, 50, resp) && call(0x08, 3, 0, resp) && resp[3] == 50);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'via_test.c'
            exe = Path(tmp) / 'via_test'
            src.write_text(code)
            subprocess.run(['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-DHHKB_HYBRID=1', '-DHHKB_COMPACT_STORAGE=1',
                            '-fsanitize=address,undefined', str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_uncalibrated_rt_clamp_and_via_showif(self):
        ec_src = (ROOT / 'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool rt_enabled;
    uint16_t act_pm;
    uint16_t press_pm;
    uint16_t release_pm;
    uint16_t reset_pm;
    uint8_t bottom_dz;
} rt_cfg_t;

static rt_cfg_t s_rt_cfg = { .rt_enabled = true, .act_pm = 750, .press_pm = 25, .release_pm = 50, .reset_pm = 50, .bottom_dz = 5 };
static uint16_t s_classic_release_pm = 100;
static bool s_custom_calibration = false;
static unsigned crit_depth;
#define taskENTER_CRITICAL() (++crit_depth)
#define taskEXIT_CRITICAL() (--crit_depth)

''' + function(ec_src, 'rt_cfg_t hhkb_get_scan_cfg(') + r'''

int main(void) {
    // Uncalibrated + RT enabled: press_pm/release_pm below 100 are clamped to 100.
    rt_cfg_t cfg = hhkb_get_scan_cfg();
    assert(crit_depth == 0);
    assert(cfg.press_pm == 100 && cfg.release_pm == 100);

    // Values >= 100 stay untouched when uncalibrated.
    s_rt_cfg.press_pm = 150;
    s_rt_cfg.release_pm = 200;
    cfg = hhkb_get_scan_cfg();
    assert(cfg.press_pm == 150 && cfg.release_pm == 200);

    // Classic mode: uses s_classic_release_pm even if below 100.
    s_rt_cfg.rt_enabled = false;
    s_classic_release_pm = 50;
    cfg = hhkb_get_scan_cfg();
    assert(cfg.release_pm == 50);

    // Calibrated + RT enabled: full range down to 25 pm is unlocked.
    s_rt_cfg.rt_enabled = true;
    s_rt_cfg.press_pm = 25;
    s_rt_cfg.release_pm = 25;
    s_custom_calibration = true;
    cfg = hhkb_get_scan_cfg();
    assert(cfg.press_pm == 25 && cfg.release_pm == 25);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'rt_clamp.c'
            exe = Path(tmp) / 'rt_clamp'
            src.write_text(code)
            subprocess.run(['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

        definition = json.loads((ROOT / 'keyboards/hhkb/via.json').read_text())
        act_tab = next(x for x in definition['menus'][0]['content'] if x['label'] == 'Calibration')
        with_rt = next(x for x in act_tab['content'] if x.get('label') == 'With Rapid Trigger')
        rel_hyst = next(x for x in act_tab['content'] if x.get('label') == 'Release hysteresis')
        self.assertEqual(with_rt.get('showIf'), '{id_hhkb_rapid_trigger} == 1')
        self.assertEqual(rel_hyst.get('showIf'), '{id_hhkb_rapid_trigger} == 0')
