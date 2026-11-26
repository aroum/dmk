#include "classic_matrix.h"
#include "sof_lock.h"
#ifdef HHKB_COMPACT_STORAGE
#include "hhkb_compact_storage.h"
#endif
#include "ec_curve.h"
#ifdef HHKB_HYBRID
#include "hybrid.h"
#endif
#include "usb_descriptors.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static void copy_str28(char *dst, const char *src) {
    strncpy(dst, src, 28);
    dst[28] = '\0';
}

static uint8_t debug_flag_for_id(uint8_t id) {
    switch (id) {
        case 19: return DEBUG_FLAG_TOPRE;
        case 23: return DEBUG_FLAG_VIA;
        case 24: return DEBUG_FLAG_RATE;
        case 25: return DEBUG_FLAG_CAL;
        case 62: return DEBUG_FLAG_USB;
        case 67: return DEBUG_FLAG_EEPROM;
        default: return 0;
    }
}

/* Channel 0: calibration, actuation and LEDs. Values 1..6 retain their IDs. */
bool via_custom_value_command_kb(uint8_t const *request, uint8_t *response) {
    if (request[1] != 0) return false;
    uint8_t op = request[0];
    if (op == 0x09) {
        hhkb_settings_mark_dirty();
        return true;
    }
    if (op != 0x08 && op != 0x07) return false;
    bool is_get = (op == 0x08);
    uint8_t id = request[2];
    uint8_t val = request[3];

    if (id == 66) {
        if(request[0] == 0x08) response[3]=hhkb_gesc_ignore();
        else if(request[0] == 0x07 && request[3]<=3) hhkb_gesc_set_ignore(request[3]);
        else return false;
        return true;
    }
    if (id == 61) {
        if (request[0] == 0x08) response[3] = sof_lock_enabled();
        else if (request[0] != 0x07 || request[3] > 1 || !sof_lock_set_enabled(request[3] != 0)) return false;
        return true;
    }
    if (id >= 57 && id <= 60) {
        if (request[0] == 0x08) response[3] = (hhkb_gesc_overrides() >> (id - 57)) & 1;
        else if (request[0] == 0x07 && request[3] <= 1)
            hhkb_gesc_set_override(id - 57, request[3] != 0);
        else return false;
        return true;
    }

    uint8_t dbg_mask = debug_flag_for_id(id);
    if (dbg_mask) {
        if (is_get) response[3] = hhkb_get_debug_flag(dbg_mask);
        else if (val <= 1) hhkb_set_debug_flag(dbg_mask, val != 0);
        else return false;
        return true;
    }

#ifdef HHKB_HYBRID
    if (id == 56) {
        if (is_get) response[3] = hhkb_bt_sleep_hz();
        else return hhkb_bt_set_sleep_hz(val);
        return true;
    }
    if (id == 55) {
        if (is_get) response[3] = hhkb_bt_deep_minutes();
        else return hhkb_bt_set_deep_minutes(val);
        return true;
    }
    if (id == 54) {
        if (is_get) response[3] = hhkb_bt_oversampling();
        else if (val <= 1) hhkb_bt_set_oversampling(val != 0);
        else return false;
        return true;
    }
    if (id == 52 || id == 53) {
        bool long_press = id == 53;
        if (is_get) response[3] = hhkb_bt_button_action(long_press);
        else if (val <= (long_press ? 2 : 5)) hhkb_bt_set_button_action(long_press, val);
        else return false;
        return true;
    }
    if (id == 50) {
        if (!is_get) return false;
        response[3] = hhkb_hybrid_battery_level();
        return true;
    }
    if (id >= 46 && id <= 49) {
        if (is_get) response[3] = (hhkb_bt_indicator_flags() >> (id - 46)) & 1;
        else if (val <= 1) hhkb_bt_set_indicator_flag(id - 46, val != 0);
        else return false;
        return true;
    }
    if (id == 51) {
        if (is_get) response[3] = 0;
        else if (val) hhkb_hybrid_show_battery();
        else return false;
        return true;
    }
    if (id == 40 || id == 41) {
        if (is_get) {
            response[3] = id == 40 ? hhkb_bt_sleep_minutes() : hhkb_bt_matrix_wake();
        } else if (val <= (id == 40 ? 60 : 1)) {
            hhkb_bt_set_sleep(id == 40 ? val : hhkb_bt_sleep_minutes(),
                              id == 41 ? val != 0 : hhkb_bt_matrix_wake());
        } else {
            return false;
        }
        return true;
    }
    if (id == 42 || id == 43) {
        if (is_get) {
            response[3] = 0;
            return true;
        }
        return val == 0 || hhkb_hybrid_request_slot(val - 1, id == 43);
    }
#endif
    if (id == 44 && is_get) {
#ifdef HHKB_HYBRID
        response[3] = 1;
#else
        response[3] = 0;
#endif
        return true;
    }
    if (id >= EC_CURVE_ENABLE && id <= EC_CURVE_STATUS) {
        if (is_get) {
            if (id == EC_CURVE_STATUS) copy_str28((char *)&response[3], ec_curve_status);
            else response[3] = ec_curve_settings[id - EC_CURVE_ENABLE];
            return true;
        }
        if (!ec_curve_set(id, val)) return false;
        hhkb_settings_mark_dirty();
        return true;
    }
    if (id < 1 || id > 29) return false;

    switch (id) {
        case 12:
            if (!is_get) return false;
#ifdef HHKB_COMPACT_STORAGE
            copy_str28((char *)&response[3], hhkb_compact_status());
#else
            copy_str28((char *)&response[3], hhkb_settings_storage_writable() ? "Writable" : "Occupied: not saved");
#endif
            return true;
        case 28:
            if (!is_get) return false;
#ifndef DMK_FW_VERSION
#define DMK_FW_VERSION "unknown"
#endif
            copy_str28((char *)&response[3], DMK_FW_VERSION);
            return true;
        case 26:
            if (!is_get) return false;
            hhkb_get_cal_status_str((char *)&response[3], 28);
            return true;
        case 20:
            if (!is_get) return false;
            hhkb_get_debug_key_str((char *)&response[3], 28);
            return true;
        case 21:
            if (!is_get) return false;
            hhkb_get_debug_raw_str((char *)&response[3], 28);
            return true;
        case 22:
            if (!is_get) return false;
            hhkb_get_debug_travel_str((char *)&response[3], 28);
            return true;
        case 29:
            if (is_get) response[3] = hhkb_get_power_saving();
            else if (val <= 1) hhkb_set_power_saving(val != 0);
            else return false;
            return true;
        case 18:
            if (is_get) response[3] = hhkb_get_oversampling();
            else if (val <= 1) hhkb_set_oversampling(val != 0);
            else return false;
            return true;
        case 14: case 15: case 16: case 17:
            if (is_get) response[3] = hhkb_get_hold_tap_setting(id - 14);
            else hhkb_set_hold_tap_setting(id - 14, val);
            return true;
        case 13:
            if (is_get) response[3] = hhkb_get_scan_rate();
            else hhkb_set_scan_rate(val);
            return true;
        case 2:
            if (is_get) response[3] = hhkb_calibration_enabled();
            else hhkb_calibration_set_enabled(val != 0);
            return true;
        case 9: case 10:
            if (is_get) response[3] = hhkb_get_led_mode(id - 9);
            else hhkb_set_led_mode(id - 9, val);
            return true;
        case 7: case 8:
            if (is_get) response[3] = 0;
            else if (val) {
                if (id == 7) hhkb_calibration_request_start();
                else hhkb_calibration_request_finish();
            }
            return true;
        case 27:
            if (is_get) response[3] = 0;
            else if (val) {
                char status[28];
                hhkb_get_cal_status_str(status, sizeof(status));
                debug_console_printf("[CAL] %s\r\n", status);
                hhkb_cal_print_keys();
            }
            return true;
        default:
            if (is_get) response[3] = hhkb_rt_get(id);
            else hhkb_rt_set(id, val);
            return true;
    }
}
