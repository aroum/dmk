#include "combos.h"
#include "FreeRTOS.h"
#include "config.h"
#include "keys.h"
#include "layers.h"
#include "queue.h"
#include "task.h"
#include "usb.h"
#include <string.h>

#if defined(VIAL) || defined(CHORDS_COUNT)

#define MAX_BUFFERED_KEYS 8

typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t layer;
    uint16_t via_kc;
    TickType_t press_time;
    bool sent;
    bool consumed;
} combo_key_t;

static combo_key_t s_keys[MAX_BUFFERED_KEYS];
static uint8_t s_keys_count = 0;
static TickType_t s_last_press_time = 0;

#ifdef VIAL
#include "vial.h"

#define COMBO_MAX_ACTIVE 4
#ifndef COMBO_TERM_MS
#define COMBO_TERM_MS 50
#endif

typedef struct {
    uint8_t combo_idx;
    uint16_t output;
    uint8_t count;
    bool output_released;
    struct {
        uint8_t row, col;
        bool released;
    } triggers[4];
} active_combo_t;

static active_combo_t s_active[COMBO_MAX_ACTIVE];
static uint8_t s_active_count = 0;
#endif

#ifdef CHORDS_COUNT
extern const Chord my_chords[];
#ifndef CHORD_TIMEOUT_MS
#define CHORD_TIMEOUT_MS 50
#endif
#endif

extern void process_key_event(uint8_t row, uint8_t col, uint32_t key, bool pressed);
extern void oneshot_on_key_press(uint32_t key);

void combos_init(void) {
    s_keys_count = 0;
    s_last_press_time = 0;
    memset(s_keys, 0, sizeof(s_keys));
#ifdef VIAL
    s_active_count = 0;
    memset(s_active, 0, sizeof(s_active));
#endif
}

void chords_flush(void) {
    for (uint8_t b = 0; b < s_keys_count; ++b) {
        if (!s_keys[b].consumed) {
            uint32_t key = layers_lookup_key(s_keys[b].row, s_keys[b].col);
            oneshot_on_key_press(key);
            process_key_event(s_keys[b].row, s_keys[b].col, key, true);
        }
    }
    s_keys_count = 0;
}

bool combos_has_active(void) {
#if defined(CHORDS_COUNT) || defined(VIAL)
    return s_keys_count > 0;
#else
    return false;
#endif
}

TickType_t combos_check_timeouts(TickType_t now) {
    if (!combos_has_active()) {
        return portMAX_DELAY;
    }
    TickType_t min_remaining = portMAX_DELAY;
#if defined(CHORDS_COUNT) || defined(VIAL)
    if (s_keys_count > 0) {
#ifdef CHORDS_COUNT
        const TickType_t timeout_ticks = pdMS_TO_TICKS(CHORD_TIMEOUT_MS);
#else
        const TickType_t timeout_ticks = pdMS_TO_TICKS(COMBO_TERM_MS);
#endif
        TickType_t elapsed = now - s_last_press_time;
        if (elapsed >= timeout_ticks) {
            chords_flush();
        } else {
            TickType_t remaining = timeout_ticks - elapsed;
            if (remaining < min_remaining)
                min_remaining = remaining;
        }
    }
#else
    (void)now;
#endif
    return min_remaining;
}

#ifdef VIAL
static bool check_vial_combos(uint8_t row, uint8_t col) {
    for (int i = 0; i < VIAL_COMBO_ENTRIES; i++) {
        vial_combo_entry_t *c = &vial_combos[i];
        if (c->output == 0)
            continue;

        int input_count = 0, match_count = 0, match_indices[4];
        TickType_t min_t = 0xFFFFFFFF, max_t = 0;

        for (int k = 0; k < 4; k++) {
            if (c->input[k] == 0)
                continue;
            input_count++;
            for (int p = 0; p < s_keys_count; p++) {
                if (s_keys[p].via_kc == c->input[k] && !s_keys[p].consumed) {
                    match_indices[match_count++] = p;
                    if (s_keys[p].press_time < min_t)
                        min_t = s_keys[p].press_time;
                    if (s_keys[p].press_time > max_t)
                        max_t = s_keys[p].press_time;
                    break;
                }
            }
        }

        if (input_count >= 2 && match_count == input_count && (max_t - min_t <= pdMS_TO_TICKS(COMBO_TERM_MS))) {
            for (int k = 0; k < match_count; k++) {
                int idx = match_indices[k];
                s_keys[idx].consumed = true;
                if (s_keys[idx].sent) {
                    process_key_event(s_keys[idx].row, s_keys[idx].col,
                                      layers_lookup_key(s_keys[idx].row, s_keys[idx].col), false);
                }
            }
            if (s_active_count < COMBO_MAX_ACTIVE) {
                active_combo_t *ac = &s_active[s_active_count++];
                ac->combo_idx = (uint8_t)i;
                ac->output = c->output;
                ac->count = (uint8_t)input_count;
                ac->output_released = false;
                for (int k = 0; k < input_count; k++) {
                    ac->triggers[k].row = s_keys[match_indices[k]].row;
                    ac->triggers[k].col = s_keys[match_indices[k]].col;
                    ac->triggers[k].released = false;
                }
            }
            process_key_event(row, col, from_via_keycode(c->output), true);
            return true;
        }
    }
    return false;
}

static bool process_vial_release(uint8_t row, uint8_t col) {
    for (int i = 0; i < s_active_count; i++) {
        active_combo_t *ac = &s_active[i];
        for (int k = 0; k < ac->count; k++) {
            if (ac->triggers[k].row == row && ac->triggers[k].col == col) {
                ac->triggers[k].released = true;

                // Send key-up for the combo output as soon as ANY constituent key is released
                if (!ac->output_released) {
                    process_key_event(row, col, from_via_keycode(ac->output), false);
                    ac->output_released = true;
                }

                // Check if ALL trigger keys for this combo are now released
                bool all_released = true;
                for (int j = 0; j < ac->count; j++) {
                    if (!ac->triggers[j].released) {
                        all_released = false;
                        break;
                    }
                }

                if (all_released) {
                    memmove(&s_active[i], &s_active[i + 1], (s_active_count - 1 - i) * sizeof(active_combo_t));
                    s_active_count--;
                }

                return true;
            }
        }
    }
    return false;
}
#endif

static bool is_key_in_any_combo(uint8_t row, uint8_t col, uint16_t via_kc) {
#ifdef VIAL
    if (via_kc != 0) {
        for (int i = 0; i < VIAL_COMBO_ENTRIES; i++) {
            vial_combo_entry_t *c = &vial_combos[i];
            if (c->output != 0) {
                for (int k = 0; k < 4; k++) {
                    if (c->input[k] == via_kc) {
                        return true;
                    }
                }
            }
        }
    }
#endif
#ifdef CHORDS_COUNT
    uint8_t cur_layer = layers_get_active();
    for (uint8_t c = 0; c < CHORDS_COUNT; ++c) {
        const Chord *chord = &my_chords[c];
        for (uint8_t k = 0; k < chord->key_count; ++k) {
            if (chord->keys[k].row == row && chord->keys[k].col == col &&
                chord->keys[k].layer == cur_layer) {
                return true;
            }
        }
    }
#endif
    (void)row;
    (void)col;
    (void)via_kc;
    return false;
}

bool combos_process_event(uint8_t row, uint8_t col, bool pressed, TickType_t now) {
    if (!pressed) {
#ifdef VIAL
        // 1. Process active combo releases first
        if (process_vial_release(row, col)) {
            // Also ensure key is removed from buffer if it was still pending
            for (int p = 0; p < s_keys_count; p++) {
                if (s_keys[p].row == row && s_keys[p].col == col) {
                    memmove(&s_keys[p], &s_keys[p + 1], (s_keys_count - 1 - p) * sizeof(combo_key_t));
                    s_keys_count--;
                    break;
                }
            }
            return true;
        }
#endif

        // 2. If a buffered key was released before combo timeout (quick tap), flush press first
        for (int p = 0; p < s_keys_count; p++) {
            if (s_keys[p].row == row && s_keys[p].col == col) {
                if (!s_keys[p].consumed) {
                    uint32_t key = layers_lookup_key(row, col);
                    oneshot_on_key_press(key);
                    process_key_event(row, col, key, true);
                }
                memmove(&s_keys[p], &s_keys[p + 1], (s_keys_count - 1 - p) * sizeof(combo_key_t));
                s_keys_count--;
                break;
            }
        }

#ifdef CHORDS_COUNT
        for (uint8_t b = 0; b < s_keys_count; ++b) {
            if (s_keys[b].row == row && s_keys[b].col == col) {
                chords_flush();
                break;
            }
        }
#endif
        return false;
    }

    if (pressed) {
        uint16_t via_kc = 0;
#ifdef VIAL
        int16_t ki = keyboard_get_flat_key_index(row, col);
        if (ki >= 0) {
            via_kc = to_via_keycode(dynamic_keymap[0][ki]);
        }
#endif

        // Zero-delay fast-path: if this key does not participate in any active combo or chord,
        // bypass combo buffering completely and send immediately to USB (0ms latency).
        if (!is_key_in_any_combo(row, col, via_kc)) {
            if (s_keys_count > 0) {
                chords_flush();
            }
            return false;
        }

        if (s_keys_count < MAX_BUFFERED_KEYS) {
            s_keys[s_keys_count++] = (combo_key_t){.row = row,
                                                   .col = col,
                                                   .layer = layers_get_active(),
                                                   .via_kc = via_kc,
                                                   .press_time = now,
                                                   .sent = false,
                                                   .consumed = false};
            s_last_press_time = now;

#ifdef VIAL
            if (check_vial_combos(row, col)) {
                return true;
            }
#endif

#ifdef CHORDS_COUNT
            for (uint8_t c = 0; c < CHORDS_COUNT; ++c) {
                const Chord *chord = &my_chords[c];
                if (chord->key_count == s_keys_count) {
                    bool match = true;
                    for (uint8_t k = 0; k < chord->key_count && match; ++k) {
                        bool found = false;
                        for (uint8_t b = 0; b < s_keys_count; ++b) {
                            if (s_keys[b].row == chord->keys[k].row && s_keys[b].col == chord->keys[k].col &&
                                s_keys[b].layer == chord->keys[k].layer) {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                            match = false;
                    }
                    if (match) {
                        if (chord->action)
                            chord->action();
                        s_keys_count = 0;
                        return true;
                    }
                }
            }
#endif
            return true;
        }
        chords_flush();
    }
    return false;
}

#else

void combos_init(void) {}
void chords_flush(void) {}
TickType_t combos_check_timeouts(TickType_t now) {
    (void)now;
    return portMAX_DELAY;
}
bool combos_process_event(uint8_t r, uint8_t c, bool p, TickType_t now) {
    (void)r;
    (void)c;
    (void)p;
    (void)now;
    return false;
}

#endif
