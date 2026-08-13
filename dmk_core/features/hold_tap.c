#include "hold_tap.h"
#include "config.h"
#include "keys.h"
#include "layers.h"
#include "task.h"
#include <string.h>

#ifndef TAPPING_TERM_DEFAULT
#define TAPPING_TERM_DEFAULT 200
#endif

// Array of active hold-tap tracking slots
static HTTracker ht_trackers[MAX_HT_TRACKERS];

// External dispatcher functions
extern void keyboard_send_key(uint16_t keycode, bool pressed);
extern void keyboard_send_modifiers(uint8_t mod_mask, bool pressed);
extern void oneshot_send_lazy_mods(void);
extern void oneshot_on_tap_key(void);

/**
 * @brief Initialize the hold-tap tracker table.
 */
void hold_tap_init(void) {
    memset(ht_trackers, 0, sizeof(ht_trackers));
}

/**
 * @brief Activate the 'hold' role (either a momentary layer switch or modifier press).
 * @param tracker Pointer to the active HTTracker entry.
 */
static void activate_hold(HTTracker *tracker) {
    tracker->state = HT_STATE_HOLD;
    uint32_t key = tracker->keycode;
    uint8_t layer_or_mod = (key >> 8) & 0xFF;
    if (layer_or_mod < 16) {
        // Hold action is a layer switch (LT)
        layers_on(layer_or_mod);
    } else {
        // Hold action is a modifier (MT)
        keyboard_send_modifiers(layer_or_mod, true);
    }
}

/**
 * @brief Permissive hold resolver: if another key is pressed while an HT key is down,
 * immediately resolve the HT key as held rather than waiting for the timeout to elapse.
 * @param except_row Row of the triggering key (excluded from conversion).
 * @param except_col Column of the triggering key (excluded from conversion).
 */
void hold_tap_permissive_resolve(uint8_t except_row, uint8_t except_col) {
    for (int i = 0; i < MAX_HT_TRACKERS; i++) {
        if (ht_trackers[i].state == HT_STATE_PRESSED &&
            (ht_trackers[i].row != except_row || ht_trackers[i].col != except_col)) {
            activate_hold(&ht_trackers[i]);
        }
    }
}

/**
 * @brief Check all active hold-tap timers against current OS tick time.
 * If tapping term has elapsed, promotes key to HT_STATE_HOLD.
 * @param now Current FreeRTOS tick count.
 * @return Remaining ticks until next earliest deadline, or portMAX_DELAY.
 */
TickType_t hold_tap_check_timeouts(TickType_t now) {
    TickType_t min_remaining = portMAX_DELAY;

    for (int i = 0; i < MAX_HT_TRACKERS; i++) {
        if (ht_trackers[i].state == HT_STATE_PRESSED) {
            TickType_t elapsed = now - ht_trackers[i].press_time;
            if (elapsed >= ht_trackers[i].timeout_ticks) {
                activate_hold(&ht_trackers[i]);
            } else {
                TickType_t remaining = ht_trackers[i].timeout_ticks - elapsed;
                if (remaining < min_remaining) {
                    min_remaining = remaining;
                }
            }
        }
    }
    return min_remaining;
}

/**
 * @brief Handle press/release for DMK_HT dual-role keys.
 * On short press (< tapping term), emits tap keycode.
 * On held press (>= tapping term or permissive trigger), activates layer or modifier.
 * @param row Matrix row.
 * @param col Matrix col.
 * @param key 32-bit DMK_HT encoded keycode.
 * @param pressed True for press, false for release.
 * @return true if key was handled as a hold-tap key, false otherwise.
 */
bool hold_tap_process_event(uint8_t row, uint8_t col, uint32_t key, bool pressed) {
    if ((key & 0xFF000000) != DMK_HT) {
        return false;
    }

    if (pressed) {
        int idx = -1;
        for (int i = 0; i < MAX_HT_TRACKERS; i++) {
            if (ht_trackers[i].state == HT_STATE_IDLE) {
                idx = i;
                break;
            }
        }
        if (idx != -1) {
            uint32_t ms = (key >> 16) & 0xFF;
            ht_trackers[idx].row = row;
            ht_trackers[idx].col = col;
            ht_trackers[idx].keycode = key;
            ht_trackers[idx].press_time = xTaskGetTickCount();
            ht_trackers[idx].timeout_ticks = pdMS_TO_TICKS(ms ? ms : TAPPING_TERM_DEFAULT);
            ht_trackers[idx].state = HT_STATE_PRESSED;
        }
    } else {
        int idx = -1;
        for (int i = 0; i < MAX_HT_TRACKERS; i++) {
            if (ht_trackers[i].state != HT_STATE_IDLE && ht_trackers[i].row == row && ht_trackers[i].col == col) {
                idx = i;
                break;
            }
        }
        if (idx != -1) {
            uint32_t stored_key = ht_trackers[idx].keycode;
            uint8_t layer_or_mod = (stored_key >> 8) & 0xFF;
            uint8_t kc = stored_key & 0xFF;

            if (ht_trackers[idx].state == HT_STATE_PRESSED) {
                // Short tap: send press and release of the tap keycode
                oneshot_send_lazy_mods();
                keyboard_send_key(kc, true);
                keyboard_send_key(kc, false);
                oneshot_on_tap_key();
            } else if (ht_trackers[idx].state == HT_STATE_HOLD) {
                // Hold release: deactivate layer or modifier
                if (layer_or_mod < 16) {
                    layers_off(layer_or_mod);
                } else {
                    keyboard_send_modifiers(layer_or_mod, false);
                }
            }
            ht_trackers[idx].state = HT_STATE_IDLE;
        }
    }
    return true;
}
