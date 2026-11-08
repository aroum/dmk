#include "sequencer.h"
#include "../synth/synth.h"
#include <string.h>
#include <stdlib.h>

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "pico/stdlib.h"
#include "hardware/timer.h"
#else
// Host testing stub
static inline uint32_t time_us_32(void) { return 0; }
#endif

SequencerState g_seq_state;

static void update_step_interval(void) {
    if (g_seq_state.bpm == 0) g_seq_state.bpm = 120;
    // 16th notes: 4 steps per beat. 1 beat = 60,000,000 / BPM microseconds.
    g_seq_state.step_interval_us = (60000000UL / (uint32_t)g_seq_state.bpm) / 4UL;
}

void sequencer_init(void) {
    memset(&g_seq_state, 0, sizeof(SequencerState));
    g_seq_state.bpm = 120;
    g_seq_state.active_track = 0;
    g_seq_state.current_page = 0;
    g_seq_state.current_step = 0;
    g_seq_state.is_playing = false;
    g_seq_state.is_recording = false;
    g_seq_state.step_edit_mode = false;
    update_step_interval();

    for (int t = 0; t < SEQ_MAX_TRACKS; t++) {
        TrackSequence *track = &g_seq_state.tracks[t];
        track->track_idx = t;
        track->is_muted = false;
        track->is_audio_loop = false;
        track->loop_length = SEQ_MAX_STEPS;

        for (int s = 0; s < SEQ_MAX_STEPS; s++) {
            track->steps[s].note.note = 60; // Middle C
            track->steps[s].note.velocity = 100;
            track->steps[s].note.probability = 100;
            track->steps[s].note.enabled = false;
            track->steps[s].stop_flag = false;
            track->steps[s].gate_length = 64; // 50% gate
            track->steps[s].plock.flags = PLOCK_FLAG_NONE;
        }
    }
}

void sequencer_set_bpm(uint8_t bpm) {
    if (bpm < 40) bpm = 40;
    if (bpm > 240) bpm = 240;
    g_seq_state.bpm = bpm;
    update_step_interval();
}

void sequencer_start(void) {
    g_seq_state.is_playing = true;
    g_seq_state.current_step = 0;
    g_seq_state.last_tick_us = time_us_32();
}

void sequencer_stop(void) {
    g_seq_state.is_playing = false;
    synth_all_notes_off();
}

void sequencer_toggle_play(void) {
    if (g_seq_state.is_playing) {
        sequencer_stop();
    } else {
        sequencer_start();
    }
}

void sequencer_next_page(void) {
    g_seq_state.current_page = (g_seq_state.current_page + 1) % SEQ_PAGES;
}

void sequencer_set_page(uint8_t page) {
    if (page < SEQ_PAGES) {
        g_seq_state.current_page = page;
    }
}

void sequencer_toggle_step(uint8_t key_index) {
    if (key_index >= SEQ_STEPS_PER_PAGE) return;
    uint8_t step_idx = (g_seq_state.current_page * SEQ_STEPS_PER_PAGE) + key_index;
    TrackSequence *track = &g_seq_state.tracks[g_seq_state.active_track];

    track->steps[step_idx].note.enabled = !track->steps[step_idx].note.enabled;
}

void sequencer_enter_step_edit(uint8_t key_index) {
    if (key_index >= SEQ_STEPS_PER_PAGE) return;
    uint8_t step_idx = (g_seq_state.current_page * SEQ_STEPS_PER_PAGE) + key_index;

    if (g_seq_state.step_edit_mode && g_seq_state.edit_step == step_idx) {
        // Toggle off if already editing this exact step
        g_seq_state.step_edit_mode = false;
    } else {
        g_seq_state.step_edit_mode = true;
        g_seq_state.edit_step = step_idx;
    }
}

void sequencer_exit_step_edit(void) {
    g_seq_state.step_edit_mode = false;
}

void sequencer_record_note_to_step(uint8_t note, uint8_t velocity, uint8_t gate_length) {
    if (!g_seq_state.step_edit_mode) return;
    TrackSequence *track = &g_seq_state.tracks[g_seq_state.active_track];
    SeqStep *step = &track->steps[g_seq_state.edit_step];

    step->note.note = note;
    step->note.velocity = (velocity > 0) ? velocity : 100;
    step->note.enabled = true;
    if (gate_length > 0) {
        step->gate_length = gate_length;
    }
}

void sequencer_set_plock(uint8_t flag, uint8_t value) {
    if (!g_seq_state.step_edit_mode) return;
    TrackSequence *track = &g_seq_state.tracks[g_seq_state.active_track];
    SeqStep *step = &track->steps[g_seq_state.edit_step];

    step->plock.flags |= flag;
    switch (flag) {
        case PLOCK_FLAG_CUTOFF:      step->plock.cutoff = value; break;
        case PLOCK_FLAG_RESONANCE:   step->plock.resonance = value; break;
        case PLOCK_FLAG_DECAY:       step->plock.decay = value; break;
        case PLOCK_FLAG_WAVETABLE:   step->plock.wavetable_pos = value; break;
        case PLOCK_FLAG_DRIVE:       step->plock.drive = value; break;
        case PLOCK_FLAG_FX_SEND:     step->plock.fx_send = value; break;
        default: break;
    }
}

void sequencer_clear_plock(void) {
    if (!g_seq_state.step_edit_mode) return;
    TrackSequence *track = &g_seq_state.tracks[g_seq_state.active_track];
    track->steps[g_seq_state.edit_step].plock.flags = PLOCK_FLAG_NONE;
}

void sequencer_set_stop_step(uint8_t key_index) {
    if (key_index >= SEQ_STEPS_PER_PAGE) return;
    uint8_t step_idx = (g_seq_state.current_page * SEQ_STEPS_PER_PAGE) + key_index;
    TrackSequence *track = &g_seq_state.tracks[g_seq_state.active_track];

    track->steps[step_idx].stop_flag = !track->steps[step_idx].stop_flag;
    if (track->steps[step_idx].stop_flag) {
        track->loop_length = step_idx + 1;
    } else {
        track->loop_length = SEQ_MAX_STEPS;
    }
}

void sequencer_select_track(uint8_t track_idx) {
    if (track_idx < SEQ_MAX_TRACKS) {
        g_seq_state.active_track = track_idx;
    }
}

void sequencer_poll(void) {
    if (!g_seq_state.is_playing) return;

    uint32_t now = time_us_32();
    if (now - g_seq_state.last_tick_us < g_seq_state.step_interval_us) {
        return;
    }
    g_seq_state.last_tick_us = now;

    // Process all tracks for current step
    for (int t = 0; t < SEQ_MAX_TRACKS; t++) {
        TrackSequence *track = &g_seq_state.tracks[t];
        if (track->is_muted) continue;

        uint8_t step_idx = g_seq_state.current_step % track->loop_length;
        SeqStep *step = &track->steps[step_idx];

        if (step->note.enabled) {
            bool should_trigger = true;
            if (step->note.probability < 100) {
                should_trigger = ((rand() % 100) < step->note.probability);
            }

            if (should_trigger) {
                // If this is the active live synth track, dispatch note and parameter locks
                if (t == g_seq_state.active_track && !track->is_audio_loop) {
                    synth_note_on(step->note.note, step->note.velocity);
                }
            }
        }
    }

    // Advance step
    TrackSequence *active = &g_seq_state.tracks[g_seq_state.active_track];
    if (active->steps[g_seq_state.current_step].stop_flag || 
        g_seq_state.current_step + 1 >= active->loop_length) {
        g_seq_state.current_step = 0;
    } else {
        g_seq_state.current_step = (g_seq_state.current_step + 1) % SEQ_MAX_STEPS;
    }
}
