#ifndef SYNTH_H
#define SYNTH_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I2S hardware, allocate PIO state machine, and launch Core 1 synth engine.
 */
void synth_init(void);

/**
 * @brief Trigger a MIDI note on with velocity scaling.
 * @param note MIDI note number (0..127).
 * @param velocity Key strike velocity (1..127).
 */
void synth_note_on(uint8_t note, uint8_t velocity);

/**
 * @brief Trigger note release.
 * @param note MIDI note number (0..127).
 */
void synth_note_off(uint8_t note);

/**
 * @brief Immediately release or silence all active voices.
 */
void synth_all_notes_off(void);

/**
 * @brief Set master volume (0..100%).
 */
void synth_set_master_volume(uint8_t volume_percent);

#ifdef __cplusplus
}
#endif

#endif // SYNTH_H
