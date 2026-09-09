#include "hold_tap.h"
#include "config.h"
#include "keys.h"
#include "layers.h"
#include "task.h"
#include <string.h>

#ifndef TAPPING_TERM_DEFAULT
#define TAPPING_TERM_DEFAULT 200
#endif

static HTTracker ht_trackers[MAX_HT_TRACKERS];

extern void keyboard_send_key(uint16_t keycode, bool pressed);
extern void keyboard_send_modifiers(uint8_t mod_mask, bool pressed);
extern void oneshot_send_lazy_mods(void);
extern void oneshot_on_tap_key(void);

void hold_tap_init(void) {
    memset(ht_trackers, 0, sizeof(ht_trackers));
}

static void activate_hold(HTTracker *tracker) {
    tracker->state = HT_STATE_HOLD;
    uint8_t target = (tracker->keycode >> 8) & 0xFF;
    if (target < 16) layers_on(target);
    else keyboard_send_modifiers(target, true);
}

void hold_tap_permissive_resolve(uint8_t except_row, uint8_t except_col) {
    for (int i = 0; i < MAX_HT_TRACKERS; i++) {
        if (ht_trackers[i].state == HT_STATE_PRESSED &&
            (ht_trackers[i].row != except_row || ht_trackers[i].col != except_col)) {
            activate_hold(&ht_trackers[i]);
        }
    }
}

TickType_t hold_tap_check_timeouts(TickType_t now) {
    TickType_t min_remaining = portMAX_DELAY;
    for (int i = 0; i < MAX_HT_TRACKERS; i++) {
        if (ht_trackers[i].state == HT_STATE_PRESSED) {
            TickType_t elapsed = now - ht_trackers[i].press_time;
            if (elapsed >= ht_trackers[i].timeout_ticks) {
                activate_hold(&ht_trackers[i]);
            } else {
                TickType_t rem = ht_trackers[i].timeout_ticks - elapsed;
                if (rem < min_remaining) min_remaining = rem;
            }
        }
    }
    return min_remaining;
}

bool hold_tap_process_event(uint8_t row, uint8_t col, uint32_t key, bool pressed) {
    if ((key & 0xFF000000) != DMK_HT) return false;

    if (pressed) {
        for (int i = 0; i < MAX_HT_TRACKERS; i++) {
            if (ht_trackers[i].state == HT_STATE_IDLE) {
                uint32_t ms = (key >> 16) & 0xFF;
                ht_trackers[i] = (HTTracker){
                    .row = row, .col = col, .keycode = key,
                    .press_time = xTaskGetTickCount(),
                    .timeout_ticks = pdMS_TO_TICKS(ms ? ms : TAPPING_TERM_DEFAULT),
                    .state = HT_STATE_PRESSED
                };
                break;
            }
        }
    } else {
        for (int i = 0; i < MAX_HT_TRACKERS; i++) {
            if (ht_trackers[i].state != HT_STATE_IDLE && ht_trackers[i].row == row && ht_trackers[i].col == col) {
                uint8_t target = (ht_trackers[i].keycode >> 8) & 0xFF;
                uint8_t kc = ht_trackers[i].keycode & 0xFF;

                if (ht_trackers[i].state == HT_STATE_PRESSED) {
                    oneshot_send_lazy_mods();
                    keyboard_send_key(kc, true);
                    keyboard_send_key(kc, false);
                    oneshot_on_tap_key();
                } else if (ht_trackers[i].state == HT_STATE_HOLD) {
                    if (target < 16) layers_off(target);
                    else keyboard_send_modifiers(target, false);
                }
                ht_trackers[i].state = HT_STATE_IDLE;
                break;
            }
        }
    }
    return true;
}
