#ifndef SEQUENCER_MAGNETENO_H
#define SEQUENCER_MAGNETENO_H

#include <stdbool.h>
#include <stdint.h>

#define SEQ_MAX_TRACKS 8
#define SEQ_MAX_STEPS 32
#define SEQ_STEPS_PER_PAGE 8
#define SEQ_PAGES 4

// Parameter Lock flags (bitmask)
#define PLOCK_FLAG_NONE        0x00
#define PLOCK_FLAG_CUTOFF      0x01
#define PLOCK_FLAG_RESONANCE   0x02
#define PLOCK_FLAG_DECAY       0x04
#define PLOCK_FLAG_WAVETABLE   0x08
#define PLOCK_FLAG_DRIVE       0x10
#define PLOCK_FLAG_FX_SEND     0x20

typedef struct {
    uint8_t flags;         // Bitmask of active parameter locks
    uint8_t cutoff;        // 0..127 (mapped to 20Hz - 20kHz)
    uint8_t resonance;     // 0..127 (Q factor)
    uint8_t decay;         // 0..127 (Amp/Filter decay)
    uint8_t wavetable_pos; // 0..127 (Wavetable frame index)
    uint8_t drive;         // 0..127 (Saturation / overdrive)
    uint8_t fx_send;       // 0..127 (Delay / Reverb send level)
} SeqPLock;

typedef struct {
    uint8_t note;          // MIDI Note (0..127, 0 = rest if not enabled)
    uint8_t velocity;      // Strike velocity (1..127)
    uint8_t probability;   // 0..100% trigger chance
    bool enabled;          // Step active flag (false = muted / rest)
} SeqNote;

typedef struct {
    SeqNote note;
    bool stop_flag;        // Loop boundary: if true, pattern resets to step 0
    uint8_t gate_length;   // Gate duration: 1..127 (fraction of 16th note)
    SeqPLock plock;        // Parameter lock overrides for this step
} SeqStep;

typedef struct {
    SeqStep steps[SEQ_MAX_STEPS];
    uint8_t track_idx;     // 0..7
    bool is_muted;         // Track-level mute
    bool is_audio_loop;    // true = rendered to PSRAM audio loop; false = live synth voice
    uint8_t loop_length;   // Pattern length (1..32, default 32 or set by stop_flag)
} TrackSequence;

typedef struct {
    TrackSequence tracks[SEQ_MAX_TRACKS];
    uint8_t active_track;  // Currently selected track (0..7)
    uint8_t current_step;  // Playhead step (0..31)
    uint8_t current_page;  // Visible page (0..3: steps 1-8, 9-16, 17-24, 25-32)
    bool is_playing;       // Transport running state
    bool is_recording;     // Record mode toggle
    bool step_edit_mode;   // True when editing a specific step (Combo 9 + [1..8])
    uint8_t edit_step;     // Index of step being edited (0..31)
    uint8_t bpm;           // Master tempo (40..240, default 120)
    uint32_t last_tick_us; // High-precision microsecond timestamp for clock divider
    uint32_t step_interval_us; // Microseconds per 16th note step
} SequencerState;

extern SequencerState g_seq_state;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize 8-track 32-step sequencer engine with defaults.
 */
void sequencer_init(void);

/**
 * @brief Poll sequencer clock; advances steps and triggers voices/p-locks.
 * Should be called periodically from Core 0 main loop or dedicated timer task.
 */
void sequencer_poll(void);

/**
 * @brief Start transport playback.
 */
void sequencer_start(void);

/**
 * @brief Stop transport playback and silence active notes.
 */
void sequencer_stop(void);

/**
 * @brief Toggle transport playback (Combo 9 + 10).
 */
void sequencer_toggle_play(void);

/**
 * @brief Set master tempo.
 * @param bpm Beats per minute (40..240).
 */
void sequencer_set_bpm(uint8_t bpm);

/**
 * @brief Switch visible step page (Key 10: cycles 0..3).
 */
void sequencer_next_page(void);

/**
 * @brief Set specific page (0..3).
 */
void sequencer_set_page(uint8_t page);

/**
 * @brief Toggle step active/mute state on current page (Single tap 1..8).
 * @param key_index Key index 0..7 (maps to step on current page).
 */
void sequencer_toggle_step(uint8_t key_index);

/**
 * @brief Enter or exit Step Edit Mode for parameter locking and note recording (Combo 9 + [1..8]).
 * @param key_index Key index 0..7 (maps to step on current page).
 */
void sequencer_enter_step_edit(uint8_t key_index);

/**
 * @brief Exit step edit mode.
 */
void sequencer_exit_step_edit(void);

/**
 * @brief Record a note into currently edited step.
 * @param note MIDI note (0..127).
 * @param velocity Velocity (1..127).
 * @param gate_length Gate duration (1..127).
 */
void sequencer_record_note_to_step(uint8_t note, uint8_t velocity, uint8_t gate_length);

/**
 * @brief Lock a parameter to the currently edited step.
 * @param flag PLOCK_FLAG_* bitmask.
 * @param value 0..127 parameter value.
 */
void sequencer_set_plock(uint8_t flag, uint8_t value);

/**
 * @brief Clear all parameter locks on currently edited step.
 */
void sequencer_clear_plock(void);

/**
 * @brief Set loop length / stop step on current page (Combo 9 + 10 + [1..8]).
 * @param key_index Key index 0..7.
 */
void sequencer_set_stop_step(uint8_t key_index);

/**
 * @brief Select active track (0..7).
 */
void sequencer_select_track(uint8_t track_idx);

#ifdef __cplusplus
}
#endif

#endif // SEQUENCER_MAGNETENO_H
