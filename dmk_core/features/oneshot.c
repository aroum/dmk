#include "oneshot.h"
#include "config.h"
#include "keys.h"
#include "layers.h"
#include "task.h"
#include <string.h>

#ifndef ONESHOT_TIMEOUT
#define ONESHOT_TIMEOUT 1500
#endif
#ifndef ONESHOT_QUICK_RELEASE
#define ONESHOT_QUICK_RELEASE 1
#endif
#ifndef ONESHOT_LAZY
#define ONESHOT_LAZY 1
#endif
#ifndef TAPPING_TERM_DEFAULT
#define TAPPING_TERM_DEFAULT 200
#endif

// Array of active one-shot state tracking slots
static OneShotState os_trackers[MAX_OS_TRACKERS];

extern void keyboard_send_modifiers(uint8_t mod_mask, bool pressed);

/**
 * @brief Initialize one-shot tracker structures.
 */
void oneshot_init(void) {
    memset(os_trackers, 0, sizeof(os_trackers));
}

/**
 * @brief Determine if a keycode should consume/consume an active One-Shot modifier/layer.
 * Layer switches, other one-shot keys, and modifiers are ignored.
 * @param key 32-bit composite keycode.
 * @return true if key should consume one-shot state.
 */
bool oneshot_should_consume(uint32_t key) {
    if (key == 0)
        return false;
    // Layer momentary/toggle keys
    if (key >= L_0 && key <= L_15)
        return false;
    if ((key & 0xFF000000) == DMK_MO)
        return false;
    if ((key & 0xFF000000) == DMK_TG)
        return false;
    // One shot keys
    if ((key & 0xFF000000) == DMK_OS)
        return false;
    // Standard modifiers (0xE0 - 0xE7)
    if (key >= 0xE0 && key <= 0xE7)
        return false;
    // Layer navigation keys
    if (key == K_LYRUP || key == K_LYRDWN || key == K_NULL)
        return false;
    // Hold-tap keys (handled separately when their tap role is resolved)
    if ((key & 0xFF000000) == DMK_HT)
        return false;

    return true;
}

/**
 * @brief Send active lazy modifiers right before an actual regular key is pressed.
 */
void oneshot_send_lazy_mods(void) {
    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active && os_trackers[i].is_mod && ONESHOT_LAZY) {
            keyboard_send_modifiers(os_trackers[i].mod_mask, true);
        }
    }
}

/**
 * @brief Called when a tap key is emitted (e.g. from hold-tap resolution) to consume one-shot state.
 */
void oneshot_on_tap_key(void) {
    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active) {
            os_trackers[i].key_pressed = true;
            if (os_trackers[i].pending_release) {
                if (os_trackers[i].is_mod) {
                    keyboard_send_modifiers(os_trackers[i].mod_mask, false);
                } else {
                    layers_off(os_trackers[i].layer);
                }
                memset(&os_trackers[i], 0, sizeof(OneShotState));
            }
        }
    }
}

/**
 * @brief Hook invoked when a regular key is pressed to consume active one-shot modifiers/layers.
 * @param key Keycode that was pressed.
 */
void oneshot_on_key_press(uint32_t key) {
    if (!oneshot_should_consume(key))
        return;

    oneshot_send_lazy_mods();

    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active) {
            os_trackers[i].key_pressed = true;
            if (os_trackers[i].pending_release) {
                if (os_trackers[i].is_mod && ONESHOT_QUICK_RELEASE) {
                    keyboard_send_modifiers(os_trackers[i].mod_mask, false);
                    memset(&os_trackers[i], 0, sizeof(OneShotState));
                } else {
                    os_trackers[i].active = false;
                }
            }
        }
    }
}

/**
 * @brief Hook invoked when a key is released to clean up consumed one-shot states.
 */
void oneshot_on_key_release(void) {
    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].pending_release && !os_trackers[i].active) {
            if (!os_trackers[i].is_mod) {
                layers_off(os_trackers[i].layer);
                memset(&os_trackers[i], 0, sizeof(OneShotState));
            } else if (!ONESHOT_QUICK_RELEASE) {
                keyboard_send_modifiers(os_trackers[i].mod_mask, false);
                memset(&os_trackers[i], 0, sizeof(OneShotState));
            }
        }
    }
}

/**
 * @brief Check all active one-shot timers and cancel expired sticky keys.
 * @param now Current FreeRTOS tick count.
 * @return Remaining ticks until next earliest deadline, or portMAX_DELAY.
 */
TickType_t oneshot_check_timeouts(TickType_t now) {
    TickType_t min_remaining = portMAX_DELAY;
    const TickType_t timeout_ticks = pdMS_TO_TICKS(ONESHOT_TIMEOUT);

    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active && os_trackers[i].pending_release) {
            TickType_t elapsed = now - os_trackers[i].activate_time;
            if (elapsed >= timeout_ticks) {
                if (os_trackers[i].is_mod) {
                    keyboard_send_modifiers(os_trackers[i].mod_mask, false);
                } else {
                    layers_off(os_trackers[i].layer);
                }
                memset(&os_trackers[i], 0, sizeof(OneShotState));
            } else {
                TickType_t remaining = timeout_ticks - elapsed;
                if (remaining < min_remaining) {
                    min_remaining = remaining;
                }
            }
        }
    }
    return min_remaining;
}

/**
 * @brief Handle press/release events for One-Shot Modifiers (OSM) and One-Shot Layers (OSL).
 * If tapped, the modifier/layer stays active until next key press or timeout.
 * If held, behaves like a standard momentary modifier/layer.
 * @param key 32-bit DMK_OS encoded keycode.
 * @param pressed True for press, false for release.
 * @return true if handled as a one-shot key, false otherwise.
 */
bool oneshot_process_event(uint32_t key, bool pressed) {
    if ((key & 0xFF000000) != DMK_OS) {
        return false;
    }

    bool is_mod = (key & 0x10000) != 0;
    uint8_t target = is_mod ? ((key >> 8) & 0xFF) : (key & 0xFF);

    if (pressed) {
        int idx = -1;
        // First check if there's already an active tracker for this target to refresh
        for (int i = 0; i < MAX_OS_TRACKERS; i++) {
            if (os_trackers[i].active) {
                if (is_mod && os_trackers[i].is_mod && os_trackers[i].mod_mask == target) {
                    idx = i;
                    break;
                }
                if (!is_mod && !os_trackers[i].is_mod && os_trackers[i].layer == target) {
                    idx = i;
                    break;
                }
            }
        }
        // Find free slot
        if (idx == -1) {
            for (int i = 0; i < MAX_OS_TRACKERS; i++) {
                if (!os_trackers[i].active && !os_trackers[i].pending_release) {
                    idx = i;
                    break;
                }
            }
        }
        if (idx != -1) {
            os_trackers[idx].activate_time = xTaskGetTickCount();
            os_trackers[idx].active = true;
            os_trackers[idx].pending_release = false;
            os_trackers[idx].is_mod = is_mod;
            os_trackers[idx].key_pressed = false;
            if (is_mod) {
                os_trackers[idx].mod_mask = target;
                if (!ONESHOT_LAZY) {
                    keyboard_send_modifiers(target, true);
                }
            } else {
                os_trackers[idx].layer = target;
                layers_on(target);
            }
        }
    } else {
        int idx = -1;
        for (int i = 0; i < MAX_OS_TRACKERS; i++) {
            if (os_trackers[i].active) {
                if (is_mod && os_trackers[i].is_mod && os_trackers[i].mod_mask == target) {
                    idx = i;
                    break;
                }
                if (!is_mod && !os_trackers[i].is_mod && os_trackers[i].layer == target) {
                    idx = i;
                    break;
                }
            }
        }
        if (idx != -1) {
            TickType_t duration = xTaskGetTickCount() - os_trackers[idx].activate_time;
            if (duration < pdMS_TO_TICKS(TAPPING_TERM_DEFAULT) && !os_trackers[idx].key_pressed) {
                // Short tap: pending release
                os_trackers[idx].pending_release = true;
                os_trackers[idx].activate_time = xTaskGetTickCount();
            } else {
                // Long hold or another key was pressed during hold: deactivate immediately
                if (is_mod) {
                    keyboard_send_modifiers(os_trackers[idx].mod_mask, false);
                } else {
                    layers_off(os_trackers[idx].layer);
                }
                memset(&os_trackers[idx], 0, sizeof(OneShotState));
            }
        }
    }
    return true;
}
