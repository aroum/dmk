#include "classic_matrix.h"
#include "ec_curve.h"
#ifdef HHKB_HYBRID
#include "hybrid.h"
#endif
#include "usb_descriptors.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Channel 0: calibration, actuation and LEDs. Values 1..6 retain their IDs. */
bool via_custom_value_command_kb(uint8_t const *request, uint8_t *response) {
    if (request[1] != 0) return false;
    uint8_t id = request[2];
    if (request[0] == 0x09) { hhkb_settings_mark_dirty(); return true; }
#ifdef HHKB_HYBRID
    if (id == 56) {
        if (request[0] == 0x08) response[3] = hhkb_bt_sleep_hz();
        else if (request[0] != 0x07 || !hhkb_bt_set_sleep_hz(request[3])) return false;
        return true;
    }
    if (id == 55) {
        if (request[0] == 0x08) response[3] = hhkb_bt_deep_minutes();
        else if (request[0] != 0x07 || !hhkb_bt_set_deep_minutes(request[3])) return false;
        return true;
    }
    if (id == 54) {
        if (request[0] == 0x08) response[3] = hhkb_bt_oversampling();
        else if (request[0] == 0x07 && request[3] <= 1)
            hhkb_bt_set_oversampling(request[3] != 0);
        else return false;
        return true;
    }
    if (id == 52 || id == 53) {
        bool long_press = id == 53;
        if (request[0] == 0x08) response[3] = hhkb_bt_button_action(long_press);
        else if (request[0] == 0x07 && request[3] <= (long_press ? 2 : 5))
            hhkb_bt_set_button_action(long_press, request[3]);
        else return false;
        return true;
    }
    if (id >= 46 && id <= 50) {
        if (id == 50) {
            if (request[0] == 0x08) response[3] = hhkb_hybrid_battery_level();
            else return false;
            return true;
        }
        if (request[0] == 0x08) response[3] = (hhkb_bt_indicator_flags() >> (id - 46)) & 1;
        else if (request[0] == 0x07 && request[3] <= 1)
            hhkb_bt_set_indicator_flag(id - 46, request[3] != 0);
        else return false;
        return true;
    }
    if (id == 51) {
        if (request[0] == 0x08) response[3] = 0;
        else if (request[0] == 0x07 && request[3]) hhkb_hybrid_show_battery();
        else return false;
        return true;
    }
#endif
    if (id == 44 && request[0] == 0x08) {
#ifdef HHKB_HYBRID
        response[3] = 1;
#else
        response[3] = 0;
#endif
        return true;
    }
#ifdef HHKB_HYBRID
    if (id == 40 || id == 41) {
        if (request[0] == 0x08) {
            response[3] = id == 40 ? hhkb_bt_sleep_minutes() : hhkb_bt_matrix_wake();
        } else if (request[0] == 0x07 && request[3] <= (id == 40 ? 60 : 1)) {
            hhkb_bt_set_sleep(id == 40 ? request[3] : hhkb_bt_sleep_minutes(),
                              id == 41 ? request[3] != 0 : hhkb_bt_matrix_wake());
        } else {
            return false;
        }
        return true;
    }
    if (id == 42 || id == 43) {
        if (request[0] == 0x08) {
            response[3] = 0;
            return true;
        }
        return request[0] == 0x07 && (request[3] == 0 ||
               hhkb_hybrid_request_slot(request[3] - 1, id == 43));
    }
#endif
    if (id >= EC_CURVE_ENABLE && id <= EC_CURVE_STATUS) {
        if (request[0] == 0x08) {
            if (id == EC_CURVE_STATUS) strcpy((char *)&response[3], ec_curve_status);
            else response[3] = ec_curve_settings[id - EC_CURVE_ENABLE];
            return true;
        }
        if (request[0] != 0x07 || !ec_curve_set(id, request[3])) return false;
        hhkb_settings_mark_dirty();
        return true;
    }
    if (id < 1 || id > 29) return false;
    if (request[0] == 0x08) {
        if (id == 12) {
            const char *status = hhkb_settings_storage_writable() ? "Writable" : "Occupied: not saved";
            strcpy((char *)&response[3], status);
        } else if (id == 28) {
#ifndef DMK_FW_VERSION
#define DMK_FW_VERSION "unknown"
#endif
            strncpy((char *)&response[3], DMK_FW_VERSION, 28);
            response[31] = '\0';
        } else if (id == 29) {
            response[3] = hhkb_get_power_saving();
        } else if (id == 19) {
            response[3] = hhkb_get_debug_flag(DEBUG_FLAG_TOPRE);
        } else if (id == 23) {
            response[3] = hhkb_get_debug_flag(DEBUG_FLAG_VIA);
        } else if (id == 24) {
            response[3] = hhkb_get_debug_flag(DEBUG_FLAG_RATE);
        } else if (id == 25) {
            response[3] = hhkb_get_debug_flag(DEBUG_FLAG_CAL);
        } else if (id == 26) {
            hhkb_get_cal_status_str((char *)&response[3], 28);
        } else if (id == 27) {
            response[3] = 0;
        } else if (id == 20) {
            hhkb_get_debug_key_str((char *)&response[3], 28);
        } else if (id == 21) {
            hhkb_get_debug_raw_str((char *)&response[3], 28);
        } else if (id == 22) {
            hhkb_get_debug_travel_str((char *)&response[3], 28);
        } else if (id == 18) {
            response[3] = hhkb_get_oversampling();
        } else if (id >= 14) {
            response[3] = hhkb_get_hold_tap_setting(id - 14);
        } else if (id == 13) {
            response[3] = hhkb_get_scan_rate();
        } else if (id == 2) {
            response[3] = hhkb_calibration_enabled();
        } else if (id == 9 || id == 10) {
            response[3] = hhkb_get_led_mode(id - 9);
        } else if (id == 7 || id == 8) {
            response[3] = 0;
        } else {
            response[3] = hhkb_rt_get(id);
        }
        return true;
    }
    if (request[0] == 0x07) {
        if (id == 12 || id == 26 || id == 28 || (id >= 20 && id <= 22)) return false;
        if (id == 29) {
            if (request[3] > 1) return false;
            hhkb_set_power_saving(request[3] != 0);
        } else if (id == 19) {
            if (request[3] > 1) return false;
            hhkb_set_debug_flag(DEBUG_FLAG_TOPRE, request[3] != 0);
        } else if (id == 23) {
            if (request[3] > 1) return false;
            hhkb_set_debug_flag(DEBUG_FLAG_VIA, request[3] != 0);
        } else if (id == 24) {
            if (request[3] > 1) return false;
            hhkb_set_debug_flag(DEBUG_FLAG_RATE, request[3] != 0);
        } else if (id == 25) {
            if (request[3] > 1) return false;
            hhkb_set_debug_flag(DEBUG_FLAG_CAL, request[3] != 0);
        } else if (id == 18) {
            if (request[3] > 1) return false;
            hhkb_set_oversampling(request[3] != 0);
        } else if (id >= 14 && id <= 17) {
            hhkb_set_hold_tap_setting(id - 14, request[3]);
        } else if (id == 13) {
            hhkb_set_scan_rate(request[3]);
        } else if (id == 2) {
            hhkb_calibration_set_enabled(request[3] != 0);
        } else if (id == 7 && request[3]) {
            hhkb_calibration_start();
        } else if (id == 8 && request[3]) {
            hhkb_calibration_finish();
        } else if (id == 9 || id == 10) {
            hhkb_set_led_mode(id - 9, request[3]);
        } else if (id == 27) {
            if (request[3]) {
                char status[28];
                hhkb_get_cal_status_str(status, sizeof(status));
                debug_console_printf("[CAL] %s\r\n", status);
            }
        } else if (id != 7 && id != 8) {
            hhkb_rt_set(id, request[3]);
        }
        return true;
    }
    return false;
}
