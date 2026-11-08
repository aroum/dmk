#ifndef MAGNETENO_UI_H
#define MAGNETENO_UI_H

#include <stdbool.h>
#include <stdint.h>
#include "../leds/magneteno_leds.h"

#ifdef __cplusplus
extern "C" {
#endif

extern MagnetenoUIMode g_ui_mode;
extern int8_t g_piano_octave;
extern uint8_t g_mixer_bank;

#define KC_SYNTH_TOGGLE 0x7E40
#define KC_SYNTH_TOGG   KC_SYNTH_TOGGLE

/**
 * @brief Initialize Magneteno UI state machine and start periodic UI task.
 */
void magneteno_ui_init(void);

/**
 * @brief Hook called on key state change to drive UI modes, piano notes,
 * sequencer steps, parameter locks, mixer, and menu navigation.
 * @return True if event was consumed by Magneteno UI, false to pass to QMK/DMK matrix.
 */
bool magneteno_ui_process_key(uint8_t col, bool pressed);

/**
 * @brief Check if synthesizer mode is currently enabled.
 */
bool magneteno_ui_is_synth_enabled(void);

/**
 * @brief Set synthesizer mode enabled/disabled.
 */
void magneteno_ui_set_synth_enabled(bool enabled);

/**
 * @brief Toggle synthesizer mode.
 */
void magneteno_ui_toggle_synth_enabled(void);

#ifdef __cplusplus
}
#endif

#endif // MAGNETENO_UI_H
