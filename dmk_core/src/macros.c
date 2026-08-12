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

/**
 * @brief Execute a single static macro step sequence.
 * @param macro Pointer to the Macro descriptor to play back.
 */
static void macros_play(const Macro *macro) {
    for (uint8_t s = 0; s < macro->count; ++s) {
        const MacroStep *step = &macro->steps[s];
        if (step->action == KEY_DOWN) {
            keyboard_send_key((uint16_t)step->value, true);
        } else if (step->action == KEY_UP) {
            keyboard_send_key((uint16_t)step->value, false);
        } else if (step->action == DELAY) {
            vTaskDelay(pdMS_TO_TICKS(step->value));
        }
    }
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
