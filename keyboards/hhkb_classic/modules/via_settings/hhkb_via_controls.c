#include "classic_matrix.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Channel 0: calibration, actuation and LEDs. Values 1..6 retain their IDs. */
bool via_custom_value_command_kb(uint8_t const *request, uint8_t *response) {
    if (request[1] != 0) return false;
    uint8_t id = request[2];
    if (request[0] == 0x09) { hhkb_settings_mark_dirty(); return true; }
    if (id < 1 || id > 24) return false;
    if (request[0] == 0x08) {
        if (id == 12) {
            const char *status = hhkb_settings_storage_writable() ? "Writable" : "Occupied: not saved";
            strcpy((char *)&response[3], status);
        } else if (id == 19) {
            response[3] = hhkb_get_debug_flag(DEBUG_FLAG_TOPRE);
        } else if (id == 23) {
            response[3] = hhkb_get_debug_flag(DEBUG_FLAG_VIA);
        } else if (id == 24) {
            response[3] = hhkb_get_debug_flag(DEBUG_FLAG_RATE);
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
        if (id == 12 || (id >= 20 && id <= 22)) return false;
        if (id == 19) {
            if (request[3] > 1) return false;
            hhkb_set_debug_flag(DEBUG_FLAG_TOPRE, request[3] != 0);
        } else if (id == 23) {
            if (request[3] > 1) return false;
            hhkb_set_debug_flag(DEBUG_FLAG_VIA, request[3] != 0);
        } else if (id == 24) {
            if (request[3] > 1) return false;
            hhkb_set_debug_flag(DEBUG_FLAG_RATE, request[3] != 0);
        } else if (id == 18) {
            if (request[3] > 1) return false;
            hhkb_set_oversampling(request[3] != 0);
        } else if (id >= 14) {
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
        } else if (id != 7 && id != 8) {
            hhkb_rt_set(id, request[3]);
        }
        return true;
    }
    return false;
}
