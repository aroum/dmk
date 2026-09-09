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

static OneShotState os_trackers[MAX_OS_TRACKERS];
extern void keyboard_send_modifiers(uint8_t mod_mask, bool pressed);

void oneshot_init(void) {
    memset(os_trackers, 0, sizeof(os_trackers));
}

bool oneshot_should_consume(uint32_t key) {
    if (key == 0 || (key >= L_0 && key <= L_15) || (key >= 0xE0 && key <= 0xE7)) return false;
    uint32_t pfx = key & 0xFF000000;
    if (pfx == DMK_MO || pfx == DMK_TG || pfx == DMK_OS || pfx == DMK_HT) return false;
    return !(key == K_LYRUP || key == K_LYRDWN || key == K_NULL);
}

void oneshot_send_lazy_mods(void) {
    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active && os_trackers[i].is_mod && ONESHOT_LAZY) {
            keyboard_send_modifiers(os_trackers[i].mod_mask, true);
        }
    }
}

static void deactivate_os(OneShotState *os) {
    if (os->is_mod) keyboard_send_modifiers(os->mod_mask, false);
    else layers_off(os->layer);
    memset(os, 0, sizeof(OneShotState));
}

void oneshot_on_tap_key(void) {
    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active) {
            os_trackers[i].key_pressed = true;
            if (os_trackers[i].pending_release) deactivate_os(&os_trackers[i]);
        }
    }
}

void oneshot_on_key_press(uint32_t key) {
    if (!oneshot_should_consume(key)) return;
    oneshot_send_lazy_mods();

    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active) {
            os_trackers[i].key_pressed = true;
            if (os_trackers[i].pending_release) {
                if (os_trackers[i].is_mod && ONESHOT_QUICK_RELEASE) deactivate_os(&os_trackers[i]);
                else os_trackers[i].active = false;
            }
        }
    }
}

void oneshot_on_key_release(void) {
    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].pending_release && !os_trackers[i].active) {
            if (!os_trackers[i].is_mod || !ONESHOT_QUICK_RELEASE) deactivate_os(&os_trackers[i]);
        }
    }
}

TickType_t oneshot_check_timeouts(TickType_t now) {
    TickType_t min_remaining = portMAX_DELAY;
    const TickType_t timeout_ticks = pdMS_TO_TICKS(ONESHOT_TIMEOUT);

    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active && os_trackers[i].pending_release) {
            TickType_t elapsed = now - os_trackers[i].activate_time;
            if (elapsed >= timeout_ticks) {
                deactivate_os(&os_trackers[i]);
            } else {
                TickType_t rem = timeout_ticks - elapsed;
                if (rem < min_remaining) min_remaining = rem;
            }
        }
    }
    return min_remaining;
}

static int find_os_slot(bool is_mod, uint8_t target) {
    for (int i = 0; i < MAX_OS_TRACKERS; i++) {
        if (os_trackers[i].active && os_trackers[i].is_mod == is_mod &&
            (is_mod ? os_trackers[i].mod_mask : os_trackers[i].layer) == target) {
            return i;
        }
    }
    return -1;
}

bool oneshot_process_event(uint32_t key, bool pressed) {
    if ((key & 0xFF000000) != DMK_OS) return false;

    bool is_mod = (key & 0x10000) != 0;
    uint8_t target = is_mod ? ((key >> 8) & 0xFF) : (key & 0xFF);
    int idx = find_os_slot(is_mod, target);

    if (pressed) {
        if (idx == -1) {
            for (int i = 0; i < MAX_OS_TRACKERS; i++) {
                if (!os_trackers[i].active && !os_trackers[i].pending_release) { idx = i; break; }
            }
        }
        if (idx != -1) {
            os_trackers[idx] = (OneShotState){
                .activate_time = xTaskGetTickCount(), .active = true, .pending_release = false,
                .is_mod = is_mod, .key_pressed = false, .mod_mask = target, .layer = target
            };
            if (is_mod) {
                if (!ONESHOT_LAZY) keyboard_send_modifiers(target, true);
            } else {
                layers_on(target);
            }
        }
    } else if (idx != -1) {
        TickType_t dur = xTaskGetTickCount() - os_trackers[idx].activate_time;
        if (dur < pdMS_TO_TICKS(TAPPING_TERM_DEFAULT) && !os_trackers[idx].key_pressed) {
            os_trackers[idx].pending_release = true;
            os_trackers[idx].activate_time = xTaskGetTickCount();
        } else {
            deactivate_os(&os_trackers[idx]);
        }
    }
    return true;
}
