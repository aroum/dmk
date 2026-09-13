#include "macros.h"
#include "FreeRTOS.h"
#include "config.h"
#include "keys.h"
#include "queue.h"
#include "task.h"
#include "usb.h"

#ifdef VIAL
#include "vial.h"
#endif

extern void keyboard_send_key(uint16_t keycode, bool pressed);

#ifndef VIAL
extern const Macro keyboard_macros[];
extern const uint8_t keyboard_macros_count;
#endif

static const Macro *s_active_macro = NULL;
static uint8_t s_active_step = 0;
static TickType_t s_delay_deadline = 0;

/**
 * @brief Start non-blocking playback of a static macro sequence.
 * @param macro Pointer to the Macro descriptor to play back.
 */
static void macros_play(const Macro *macro) {
    if (s_active_macro != NULL) {
        // If a macro is already playing, release any keys that were held down by previous steps to prevent stuck keys
        // on host
        for (uint8_t i = 0; i < s_active_step; i++) {
            if (s_active_macro->steps[i].action == KEY_DOWN) {
                keyboard_send_key((uint16_t)s_active_macro->steps[i].value, false);
            }
        }
    }
    s_active_macro = macro;
    s_active_step = 0;
    s_delay_deadline = 0;
}

/**
 * @brief Periodic service of macro playback steps and delays.
 * @param now Current FreeRTOS tick count.
 * @return Ticks until next required macro execution, or portMAX_DELAY if idle.
 */
TickType_t macros_check_timeouts(TickType_t now) {
    if (!s_active_macro) {
        return portMAX_DELAY;
    }

    if (s_delay_deadline > 0) {
        if (now < s_delay_deadline) {
            return (s_delay_deadline - now);
        }
        s_delay_deadline = 0;
    }

    while (s_active_step < s_active_macro->count) {
        const MacroStep *step = &s_active_macro->steps[s_active_step++];
        if (step->action == KEY_DOWN) {
            keyboard_send_key((uint16_t)step->value, true);
        } else if (step->action == KEY_UP) {
            keyboard_send_key((uint16_t)step->value, false);
        } else if (step->action == DELAY) {
            if (step->value > 0) {
                s_delay_deadline = now + pdMS_TO_TICKS(step->value);
                return pdMS_TO_TICKS(step->value);
            }
        }
    }

    s_active_macro = NULL;
    return portMAX_DELAY;
}

/**
 * @brief Handle execution of dynamic (Vial) or static compile-time keyboard macros (keycodes 0xC0..0xDF / M(0)..M(31)).
 * @param key 32-bit composite keycode.
 * @param pressed True for press (triggers playback), false for release.
 * @return true if key was handled as a macro trigger.
 */
bool macros_process_key(uint32_t key, bool pressed) {
    if (key >= 0xC0 && key <= 0xDF) {
        if (pressed) {
            uint8_t macro_idx = key - 0xC0;
#ifdef VIAL
            // Play dynamic macro stored in EEPROM / flash via Vial
            vial_macro_send(macro_idx);
#else
            // Play static macro sequence defined in C config
            if (macro_idx < keyboard_macros_count) {
                macros_play(&keyboard_macros[macro_idx]);
            }
#endif
        }
        return true;
    }
    return false;
}

/**
 * @brief Fire all macros whose layer_trigger matches the newly activated layer.
 *
 * Called by the layer subsystem after every layer state change. Iterates the
 * static keyboard_macros table and plays back any macro whose layer_trigger
 * field equals new_layer. Macros with layer_trigger == MACRO_NO_LAYER are
 * skipped (they are regular key-triggered macros).
 *
 * @param new_layer The highest active layer index after the state change.
 */
void macros_run_layer_triggers(uint8_t new_layer) {
#ifndef VIAL
    for (uint8_t i = 0; i < keyboard_macros_count; ++i) {
        if (keyboard_macros[i].layer_trigger == new_layer) {
            macros_play(&keyboard_macros[i]);
        }
    }
#else
    (void)new_layer; // Layer triggers are not supported in Vial dynamic mode.
#endif
}
