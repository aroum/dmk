#include "combos.h"
#include "FreeRTOS.h"
#include "config.h"
#include "keys.h"
#include "layers.h"
#include "queue.h"
#include "task.h"
#include "usb.h"
#include <string.h>

#ifdef VIAL
#include "vial.h"

// Ring buffer of recently pressed keys evaluated for Vial dynamic combo matching
typedef struct {
    uint8_t row;
    uint8_t col;
    uint16_t via_kc;
    TickType_t press_time;
    bool sent;
    bool consumed;
} combo_key_t;

#define COMBO_MAX_KEYS 8
static combo_key_t combo_keys[COMBO_MAX_KEYS];
static uint8_t combo_keys_count = 0;

// Tracker for currently active (triggered) combos to properly handle chorded releases
typedef struct {
    uint8_t combo_idx;
    uint16_t output;
    uint8_t trigger_count;
    struct {
        uint8_t row;
        uint8_t col;
    } triggers[4];
} active_combo_t;

#define COMBO_MAX_ACTIVE 4
static active_combo_t active_combos[COMBO_MAX_ACTIVE];
static uint8_t active_combos_count = 0;

#ifndef COMBO_TERM_MS
#define COMBO_TERM_MS 50
#endif

#endif // VIAL

#ifdef CHORDS_COUNT
extern const Chord my_chords[];

// Buffer storing simultaneously pressed keys for static compile-time chords
typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t layer;
    TickType_t press_time;
} BufferedKey;

#define MAX_BUFFERED_KEYS 8
static BufferedKey chord_buffer[MAX_BUFFERED_KEYS];
static uint8_t chord_buffer_count = 0;
static TickType_t last_buffer_press_time = 0;

#ifndef CHORD_TIMEOUT_MS
#define CHORD_TIMEOUT_MS 50
#endif
#endif // CHORDS_COUNT

extern void process_key_event(uint8_t row, uint8_t col, uint32_t key, bool pressed);
extern void oneshot_on_key_press(uint32_t key);

/**
 * @brief Initialize combo and chord tracking data structures.
 */
void combos_init(void) {
#ifdef VIAL
    combo_keys_count = 0;
    active_combos_count = 0;
    memset(combo_keys, 0, sizeof(combo_keys));
    memset(active_combos, 0, sizeof(active_combos));
#endif
#ifdef CHORDS_COUNT
    chord_buffer_count = 0;
    memset(chord_buffer, 0, sizeof(chord_buffer));
#endif
}

#ifdef CHORDS_COUNT
/**
 * @brief Flush chord buffer and dispatch all buffered keys as standard individual key presses.
 */
void chords_flush(void) {
    for (uint8_t b = 0; b < chord_buffer_count; ++b) {
        uint8_t r = chord_buffer[b].row;
        uint8_t c = chord_buffer[b].col;
        uint32_t key = layers_lookup_key(r, c);

        oneshot_on_key_press(key);
        process_key_event(r, c, key, true);
    }
    chord_buffer_count = 0;
}
#else
void chords_flush(void) {
    // No-op when static chords are not configured
}
#endif

/**
 * @brief Check timeouts for chord buffer. If tapping term expires, flushes buffered keys.
 * @param now Current FreeRTOS tick count.
 * @return Remaining ticks until next timeout or portMAX_DELAY.
 */
TickType_t combos_check_timeouts(TickType_t now) {
    TickType_t min_remaining = portMAX_DELAY;

#ifdef CHORDS_COUNT
    if (chord_buffer_count > 0) {
        const TickType_t timeout_ticks = pdMS_TO_TICKS(CHORD_TIMEOUT_MS);
        TickType_t elapsed = now - last_buffer_press_time;
        if (elapsed >= timeout_ticks) {
            chords_flush();
        } else {
            TickType_t remaining = timeout_ticks - elapsed;
            if (remaining < min_remaining) {
                min_remaining = remaining;
            }
        }
    }
#else
    (void)now;
#endif

    return min_remaining;
}

#ifdef VIAL
/**
 * @brief Process dynamic Vial combos configured at runtime through the Vial GUI.
 * @param row Matrix row.
 * @param col Matrix col.
 * @param pressed True for press, false for release.
 * @param now Current FreeRTOS tick count.
 * @return true if event matched or was intercepted by combo engine.
 */
static bool process_vial_combos(uint8_t row, uint8_t col, bool pressed, TickType_t now) {
    if (pressed) {
        int16_t ki = keyboard_get_flat_key_index(row, col);
        if (ki < 0)
            return false;

        uint32_t dmk_key = dynamic_keymap[0][ki];
        uint16_t via_kc = to_via_keycode(dmk_key);

        if (combo_keys_count < COMBO_MAX_KEYS) {
            combo_keys[combo_keys_count].row = row;
            combo_keys[combo_keys_count].col = col;
            combo_keys[combo_keys_count].via_kc = via_kc;
            combo_keys[combo_keys_count].press_time = now;
            combo_keys[combo_keys_count].sent = false;
            combo_keys[combo_keys_count].consumed = false;
            combo_keys_count++;
        }

        for (int i = 0; i < VIAL_COMBO_ENTRIES; i++) {
            vial_combo_entry_t *c = &vial_combos[i];
            if (c->output == 0)
                continue;

            int input_count = 0;
            for (int k = 0; k < 4; k++) {
                if (c->input[k] != 0) {
                    input_count++;
                }
            }
            if (input_count < 2)
                continue;

            int match_count = 0;
            int match_indices[4];
            TickType_t min_time = 0xFFFFFFFF;
            TickType_t max_time = 0;

            for (int k = 0; k < 4; k++) {
                uint16_t req_kc = c->input[k];
                if (req_kc == 0)
                    continue;

                bool found = false;
                for (int p = 0; p < combo_keys_count; p++) {
                    if (combo_keys[p].via_kc == req_kc && !combo_keys[p].consumed) {
                        match_indices[match_count] = p;
                        found = true;
                        if (combo_keys[p].press_time < min_time)
                            min_time = combo_keys[p].press_time;
                        if (combo_keys[p].press_time > max_time)
                            max_time = combo_keys[p].press_time;
                        break;
                    }
                }
                if (found) {
                    match_count++;
                }
            }

            if (match_count == input_count && (max_time - min_time <= pdMS_TO_TICKS(COMBO_TERM_MS))) {
                for (int k = 0; k < match_count; k++) {
                    int idx = match_indices[k];
                    combo_keys[idx].consumed = true;
                    if (combo_keys[idx].sent) {
                        uint32_t key = layers_lookup_key(combo_keys[idx].row, combo_keys[idx].col);
                        process_key_event(combo_keys[idx].row, combo_keys[idx].col, key, false);
                    }
                }

                if (active_combos_count < COMBO_MAX_ACTIVE) {
                    active_combos[active_combos_count].combo_idx = (uint8_t)i;
                    active_combos[active_combos_count].output = c->output;
                    active_combos[active_combos_count].trigger_count = (uint8_t)input_count;
                    for (int k = 0; k < input_count; k++) {
                        active_combos[active_combos_count].triggers[k].row = combo_keys[match_indices[k]].row;
                        active_combos[active_combos_count].triggers[k].col = combo_keys[match_indices[k]].col;
                    }
                    active_combos_count++;
                }

                uint32_t combo_output_dmk = from_via_keycode(c->output);
                process_key_event(row, col, combo_output_dmk, true);
                return true;
            }
        }
        return false;
    } else {
        bool was_combo_trigger = false;
        int found_active_idx = -1;
        for (int i = 0; i < active_combos_count; i++) {
            for (int k = 0; k < active_combos[i].trigger_count; k++) {
                if (active_combos[i].triggers[k].row == row && active_combos[i].triggers[k].col == col) {
                    found_active_idx = i;
                    was_combo_trigger = true;
                    break;
                }
            }
            if (was_combo_trigger)
                break;
        }

        if (was_combo_trigger) {
            for (int p = 0; p < combo_keys_count; p++) {
                if (combo_keys[p].row == row && combo_keys[p].col == col) {
                    for (int j = p; j < combo_keys_count - 1; j++) {
                        combo_keys[j] = combo_keys[j + 1];
                    }
                    combo_keys_count--;
                    break;
                }
            }

            bool trigger_still_pressed = false;
            active_combo_t *ac = &active_combos[found_active_idx];
            for (int k = 0; k < ac->trigger_count; k++) {
                for (int p = 0; p < combo_keys_count; p++) {
                    if (combo_keys[p].row == ac->triggers[k].row && combo_keys[p].col == ac->triggers[k].col) {
                        trigger_still_pressed = true;
                        break;
                    }
                }
                if (trigger_still_pressed)
                    break;
            }

            if (!trigger_still_pressed) {
                uint32_t combo_output_dmk = from_via_keycode(ac->output);
                process_key_event(row, col, combo_output_dmk, false);

                for (int j = found_active_idx; j < active_combos_count - 1; j++) {
                    active_combos[j] = active_combos[j + 1];
                }
                active_combos_count--;
            }
            return true;
        }

        for (int p = 0; p < combo_keys_count; p++) {
            if (combo_keys[p].row == row && combo_keys[p].col == col) {
                for (int j = p; j < combo_keys_count - 1; j++) {
                    combo_keys[j] = combo_keys[j + 1];
                }
                combo_keys_count--;
                break;
            }
        }
        return false;
    }
}
#endif // VIAL

/**
 * @brief Dispatch switch event to combo / chord engine.
 * @param row Matrix row.
 * @param col Matrix col.
 * @param pressed True for press, false for release.
 * @param now Current FreeRTOS tick count.
 * @return true if intercepted as part of a combo or chord.
 */
bool combos_process_event(uint8_t row, uint8_t col, bool pressed, TickType_t now) {
#ifdef VIAL
    if (process_vial_combos(row, col, pressed, now)) {
        return true;
    }
#endif

#ifdef CHORDS_COUNT
    if (pressed) {
        if (chord_buffer_count < MAX_BUFFERED_KEYS) {
            chord_buffer[chord_buffer_count].row = row;
            chord_buffer[chord_buffer_count].col = col;
            chord_buffer[chord_buffer_count].layer = layers_get_active();
            chord_buffer[chord_buffer_count].press_time = now;
            chord_buffer_count++;
            last_buffer_press_time = now;

            for (uint8_t c = 0; c < CHORDS_COUNT; ++c) {
                const Chord *chord = &my_chords[c];
                if (chord->key_count == chord_buffer_count) {
                    bool match = true;
                    for (uint8_t k = 0; k < chord->key_count; ++k) {
                        bool key_found = false;
                        for (uint8_t b = 0; b < chord_buffer_count; ++b) {
                            if (chord_buffer[b].row == chord->keys[k].row &&
                                chord_buffer[b].col == chord->keys[k].col &&
                                chord_buffer[b].layer == chord->keys[k].layer) {
                                key_found = true;
                                break;
                            }
                        }
                        if (!key_found) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        if (chord->action) {
                            chord->action();
                        }
                        chord_buffer_count = 0;
                        return true;
                    }
                }
            }
            return true; // Wait for chord timeout or additional keys
        } else {
            chords_flush();
        }
    } else {
        // If released key was in chord buffer, flush buffer
        for (uint8_t b = 0; b < chord_buffer_count; ++b) {
            if (chord_buffer[b].row == row && chord_buffer[b].col == col) {
                chords_flush();
                break;
            }
        }
    }
#else
    (void)row;
    (void)col;
    (void)pressed;
    (void)now;
#endif

    return false;
}
