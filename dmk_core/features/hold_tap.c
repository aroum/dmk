#include "hold_tap.h"
#include "hold_tap_keycode.h"
#include "config.h"
#include "keys.h"
#include "layers.h"
#include "task.h"
#include <string.h>

#ifndef TAPPING_TERM_DEFAULT
#define TAPPING_TERM_DEFAULT 200
#endif

#ifndef QUICK_TAP_TERM
#define QUICK_TAP_TERM 0
#endif

static HTTracker ht_trackers[MAX_HT_TRACKERS];
static HTLastRelease s_last_release;
static uint8_t s_active_ht_count = 0;

__attribute__((weak)) uint16_t hold_tap_config_value(uint8_t id) {
    switch (id) {
    case 0: return TAPPING_TERM_DEFAULT;
    case 1: return DMK_PERMISSIVE_HOLD_ENABLED;
    case 2: return 0;
    case 3: return QUICK_TAP_TERM;
    default: return 0;
    }
}

extern void keyboard_send_key(uint16_t keycode, bool pressed);
extern void keyboard_send_modifier_mask(uint8_t mod_mask, bool pressed);
extern void oneshot_send_lazy_mods(void);
extern void oneshot_on_tap_key(void);

void hold_tap_init(void) {
    memset(ht_trackers, 0, sizeof(ht_trackers));
    memset(&s_last_release, 0, sizeof(s_last_release));
    s_active_ht_count = 0;
}

bool hold_tap_has_active(void) {
    return s_active_ht_count > 0;
}

static void send_action(uint32_t action, bool pressed) {
    if (action == K_NULL || action == K_TRNS) return;
    uint32_t type = action & 0xFF000000u;
    if (type == DMK_MO) {
        if (pressed) layers_on(action & 15); else layers_off(action & 15);
    } else if (type == DMK_TG) {
        if (pressed) layers_toggle(action & 15);
    } else if (type == DMK_MK) {
        keyboard_send_modifier_mask((action >> 8) & 0xFF, pressed);
    } else {
        keyboard_send_key((uint16_t)action, pressed);
    }
}
static void send_hold_action(uint32_t key, bool pressed) {
    uint32_t hold, tap; uint8_t ms;
    if (ht_decode(key, &hold, &tap, &ms)) send_action(hold, pressed);
}
static void send_tap_action(uint32_t key, bool pressed) {
    uint32_t hold, tap; uint8_t ms;
    if (ht_decode(key, &hold, &tap, &ms)) {
        if (pressed && tap <= 0xFF) oneshot_send_lazy_mods();
        send_action(tap, pressed);
        if (!pressed && tap <= 0xFF) oneshot_on_tap_key();
    }
}

static void activate_hold(HTTracker *tracker) {
    if (tracker->state == HT_STATE_PRESSED && s_active_ht_count > 0) {
        s_active_ht_count--;
    }
    tracker->state = HT_STATE_HOLD;
    send_hold_action(tracker->keycode, true);
}

void hold_tap_permissive_resolve(uint8_t except_row, uint8_t except_col) {
    if (!hold_tap_config_value(1) || hold_tap_config_value(2)) return;
    for (int i = 0; i < MAX_HT_TRACKERS; i++) {
        if (ht_trackers[i].state == HT_STATE_PRESSED &&
            (ht_trackers[i].row != except_row || ht_trackers[i].col != except_col)) {
            activate_hold(&ht_trackers[i]);
        }
    }
}

TickType_t hold_tap_check_timeouts(TickType_t now) {
    if (s_active_ht_count == 0) {
        return portMAX_DELAY;
    }
    TickType_t min_remaining = portMAX_DELAY;
    for (int i = 0; i < MAX_HT_TRACKERS; i++) {
        if (ht_trackers[i].state == HT_STATE_PRESSED) {
            TickType_t elapsed = now - ht_trackers[i].press_time;
            if (elapsed >= ht_trackers[i].timeout_ticks) {
                activate_hold(&ht_trackers[i]);
            } else {
                TickType_t rem = ht_trackers[i].timeout_ticks - elapsed;
                if (rem < min_remaining)
                    min_remaining = rem;
            }
        }
    }
    return min_remaining;
}

bool hold_tap_process_event(uint8_t row, uint8_t col, uint32_t key, bool pressed) {
    if (!DMK_IS_HT(key))
        return false;

    uint32_t hold, tap; uint8_t ms;
    if (!ht_decode(key, &hold, &tap, &ms)) return true;
    TickType_t now = xTaskGetTickCount();

    if (pressed) {
        // Quick-Tap / Auto-repeat tap detection:
        // If the exact same Hold-Tap key was recently tapped and released within QUICK_TAP_TERM,
        // treat this press as a continuous tap hold (repeating the tap keycode).
        uint16_t quick_tap_ms = hold_tap_config_value(3);
        if (s_last_release.valid &&
            quick_tap_ms &&
            s_last_release.row == row &&
            s_last_release.col == col &&
            s_last_release.keycode == key &&
            (now - s_last_release.release_time) <= pdMS_TO_TICKS(quick_tap_ms)) {

            s_last_release.valid = false;
            for (int i = 0; i < MAX_HT_TRACKERS; i++) {
                if (ht_trackers[i].state == HT_STATE_IDLE) {
                    ht_trackers[i] = (HTTracker){.row = row,
                                                 .col = col,
                                                 .keycode = key,
                                                 .press_time = now,
                                                 .timeout_ticks = 0,
                                                 .state = HT_STATE_REPEAT};
                    send_tap_action(key, true);
                    return true;
                }
            }
        }
        s_last_release.valid = false;

        for (int i = 0; i < MAX_HT_TRACKERS; i++) {
            if (ht_trackers[i].state == HT_STATE_IDLE) {
                ht_trackers[i] = (HTTracker){.row = row,
                                             .col = col,
                                             .keycode = key,
                                             .press_time = now,
                                             .timeout_ticks = pdMS_TO_TICKS(ms ? ms : hold_tap_config_value(0)),
                                             .state = HT_STATE_PRESSED};
                s_active_ht_count++;
                break;
            }
        }
    } else {
        for (int i = 0; i < MAX_HT_TRACKERS; i++) {
            if (ht_trackers[i].state != HT_STATE_IDLE && ht_trackers[i].row == row && ht_trackers[i].col == col) {

                if (ht_trackers[i].state == HT_STATE_REPEAT) {
                    send_tap_action(ht_trackers[i].keycode, false);
                    s_last_release.valid = false;
                } else if (ht_trackers[i].state == HT_STATE_PRESSED) {
                    if (s_active_ht_count > 0) {
                        s_active_ht_count--;
                    }
                    send_tap_action(ht_trackers[i].keycode, true);
                    send_tap_action(ht_trackers[i].keycode, false);

                    // Record tap release for potential quick-tap repeat
                    s_last_release.row = row;
                    s_last_release.col = col;
                    s_last_release.keycode = ht_trackers[i].keycode;
                    s_last_release.release_time = now;
                    s_last_release.valid = true;
                } else if (ht_trackers[i].state == HT_STATE_HOLD) {
                    send_hold_action(ht_trackers[i].keycode, false);
                    s_last_release.valid = false;
                }
                ht_trackers[i].state = HT_STATE_IDLE;
                break;
            }
        }
    }
    return true;
}
