#include "magneteno_ui.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hooks.h"
#include "config.h"
#include "vial.h"

#include "../display/magneteno_display.h"
#include "../hall_calibration/include/hall_calibration.h"
#include "../imu/magneteno_imu.h"
#include "../leds/magneteno_leds.h"
#include "../sequencer/sequencer.h"
#include "../synth/synth.h"
#include <math.h>

static bool s_synth_enabled = true;

bool magneteno_ui_is_synth_enabled(void) {
    return s_synth_enabled;
}

void magneteno_ui_set_synth_enabled(bool enabled) {
    if (s_synth_enabled == enabled) return;
    s_synth_enabled = enabled;
    if (!s_synth_enabled) {
        synth_all_notes_off();
    }
}

void magneteno_ui_toggle_synth_enabled(void) {
    magneteno_ui_set_synth_enabled(!s_synth_enabled);
}

// Hook called before core keycode resolution in keyboard.c
bool hook_process_key(uint32_t keycode, bool pressed) {
    if (keycode == KC_SYNTH_TOGGLE || keycode == 0x7102) {
        if (pressed) {
            magneteno_ui_toggle_synth_enabled();
        }
        return true;
    }
    return false;
}

MagnetenoUIMode g_ui_mode = UI_MODE_PIANO;
int8_t g_piano_octave = 0;
uint8_t g_mixer_bank = 0; // 0 = Bank A (Tracks 1-4), 1 = Bank B (Tracks 5-8)

// Diatonic C-major scale intervals: C, D, E, F, G, A, B, C
static const uint8_t s_scale_intervals[8] = {0, 2, 4, 5, 7, 9, 11, 12};

// Currently pressed physical keys mask (bit 0..9)
static uint16_t s_pressed_keys_mask = 0;

// Active MIDI notes for physical piano keys 0..7
static int16_t s_active_key_notes[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

// Selected parameter index in submenus (0..7)
static uint8_t s_selected_submenu_param = 0;

// Flag indicating whether Key 9 was used as part of a combination
static bool s_key9_combo_used = false;

// Submenu to SynthParamID mapping for 8 keys
static const SynthParamID s_menu_param_map[7][8] = {
    // UI_MODE_MENU_OSC (1): Wave1, Wave2, Mix, Detune, Sub, Noise, -, -
    {PARAM_OSC1_WAVE, PARAM_OSC2_WAVE, PARAM_OSC_MIX, PARAM_OSC2_DETUNE, PARAM_SUB_LEVEL, PARAM_NOISE_LEVEL, PARAM_COUNT, PARAM_COUNT},
    // UI_MODE_MENU_VCF (2): Type, Cutoff, Reso, Drive, Env Depth, Hall Depth, -, -
    {PARAM_VCF_TYPE, PARAM_VCF_CUTOFF, PARAM_VCF_RESONANCE, PARAM_VCF_DRIVE, PARAM_VCF_ENV_DEPTH, PARAM_VCF_HALL_DEPTH, PARAM_COUNT, PARAM_COUNT},
    // UI_MODE_MENU_EG (3): Attack, Decay, Sustain, Release, -, -, -, -
    {PARAM_AMP_ATTACK, PARAM_AMP_DECAY, PARAM_AMP_SUSTAIN, PARAM_AMP_RELEASE, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT},
    // UI_MODE_MENU_LFO (4): Rate, Depth, -, -, -, -, -, -
    {PARAM_LFO1_RATE, PARAM_LFO1_DEPTH, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT},
    // UI_MODE_MENU_ADV (5): -, -, AT Dest, AT Depth, -, -, -, Rapid Trigger
    {PARAM_COUNT, PARAM_COUNT, PARAM_AT_DEST, PARAM_AT_DEPTH, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_RAPID_TRIGGER},
    // UI_MODE_MENU_PRESETS (6): 1..8 preset slots
    {PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT},
    // UI_MODE_MENU_FX (7): Delay Time, Feedback, Mix, Reverb Size, Mix, -, -, -
    {PARAM_FX_DELAY_TIME, PARAM_FX_DELAY_FEEDBACK, PARAM_FX_DELAY_MIX, PARAM_FX_REVERB_SIZE, PARAM_FX_REVERB_MIX, PARAM_COUNT, PARAM_COUNT, PARAM_COUNT}
};

static inline uint8_t get_piano_note(uint8_t col) {
    if (col > 7) return 60;
    int base = 60 + (g_piano_octave * 12) + s_scale_intervals[col];
    if (base < 0) base = 0;
    if (base > 127) base = 127;
    return (uint8_t)base;
}

bool magneteno_ui_process_key(uint8_t col, bool pressed) {
    if (col >= 10) return false;

    // Check if the physical key on layer 0 is mapped to KC_SYNTH_TOGGLE or MIDI_TOGGLE (0x7102)
    if (dynamic_keymap[0][col] == KC_SYNTH_TOGGLE || dynamic_keymap[0][col] == 0x7102) {
        if (pressed) {
            magneteno_ui_toggle_synth_enabled();
        }
        return true;
    }

    if (pressed) {
        s_pressed_keys_mask |= (1 << col);
    } else {
        s_pressed_keys_mask &= ~(1 << col);
    }

    // 1. Check simultaneous 9 + 10 chord: Toggles Main Menu from ANY mode
    if ((s_pressed_keys_mask & (1 << 8)) && (s_pressed_keys_mask & (1 << 9))) {
        if (pressed) {
            g_ui_mode = UI_MODE_MENU_MAIN;
            s_key9_combo_used = true;
            return true;
        }
    }

    // 2. Process according to current UI mode
    switch (g_ui_mode) {
        case UI_MODE_MENU_MAIN: {
            if (!pressed) return true;
            // Keys 1..10 select submode:
            switch (col) {
                case 0: g_ui_mode = UI_MODE_MENU_OSC; break;
                case 1: g_ui_mode = UI_MODE_MENU_VCF; break;
                case 2: g_ui_mode = UI_MODE_MENU_EG; break;
                case 3: g_ui_mode = UI_MODE_MENU_LFO; break;
                case 4: g_ui_mode = UI_MODE_MENU_ADV; break;
                case 5: g_ui_mode = UI_MODE_MENU_PRESETS; break;
                case 6: g_ui_mode = UI_MODE_MENU_FX; break;
                case 7: g_ui_mode = UI_MODE_MIXER; break;
                case 8: g_ui_mode = UI_MODE_PIANO; break;
                case 9: g_ui_mode = UI_MODE_SEQ; break;
                default: break;
            }
            return true;
        }

        case UI_MODE_PIANO: {
            if (col < 8) {
                if (pressed) {
                    uint8_t note = get_piano_note(col);
                    uint8_t vel = hall_get_velocity(col);
                    s_active_key_notes[col] = note;
                    synth_note_on(note, vel);

                    // If sequencer is in step edit mode, record note into the edited step
                    if (g_seq_state.step_edit_mode) {
                        sequencer_record_note_to_step(note, vel, 64);
                    }
                } else {
                    if (s_active_key_notes[col] >= 0) {
                        synth_note_off((uint8_t)s_active_key_notes[col]);
                        s_active_key_notes[col] = -1;
                    }
                }
                return true;
            }

            // Key 9 (col 8): Octave Down
            if (col == 8) {
                if (pressed) {
                    s_key9_combo_used = false;
                } else if (!s_key9_combo_used) {
                    if (g_piano_octave > -4) g_piano_octave--;
                }
                return true;
            }

            // Key 10 (col 9): Octave Up
            if (col == 9) {
                if (pressed) {
                    if (g_piano_octave < 4) g_piano_octave++;
                }
                return true;
            }
            break;
        }

        case UI_MODE_SEQ: {
            if (col < 8) {
                if (pressed) {
                    // Check Combo 9 + [1..8]: Enter Step Edit Mode
                    if (s_pressed_keys_mask & (1 << 8)) {
                        s_key9_combo_used = true;
                        sequencer_enter_step_edit(col);
                    } else if ((s_pressed_keys_mask & (1 << 8)) && (s_pressed_keys_mask & (1 << 9))) {
                        // 9 + 10 + [1..8]: Set loop length / stop step
                        s_key9_combo_used = true;
                        sequencer_set_stop_step(col);
                    } else {
                        // Single tap: toggle step active/mute
                        sequencer_toggle_step(col);
                    }
                }
                return true;
            }

            // Key 9 (col 8): Transport Play / Stop
            if (col == 8) {
                if (pressed) {
                    s_key9_combo_used = false;
                } else if (!s_key9_combo_used) {
                    if (g_seq_state.step_edit_mode) {
                        sequencer_exit_step_edit();
                    } else {
                        sequencer_toggle_play();
                    }
                }
                return true;
            }

            // Key 10 (col 9): Cycle pages 0..3
            if (col == 9) {
                if (pressed) {
                    sequencer_next_page();
                }
                return true;
            }
            break;
        }

        case UI_MODE_MIXER: {
            // Key 10 (col 9): Toggle Bank A/B
            if (col == 9) {
                if (pressed) g_mixer_bank = !g_mixer_bank;
                return true;
            }
            // Key 9 (col 8): Toggle between Mixer Faders and Looper
            if (col == 8) {
                if (pressed) g_ui_mode = UI_MODE_LOOPER;
                return true;
            }
            // Keys 1..4: Volume + for channels in bank
            if (col < 4) {
                if (pressed) {
                    // Increment channel volume
                }
                return true;
            }
            // Keys 5..8: Volume - for channels in bank
            if (col >= 4 && col < 8) {
                if (pressed) {
                    // Decrement channel volume
                }
                return true;
            }
            break;
        }

        case UI_MODE_LOOPER: {
            // Key 10 (col 9): Toggle Bank A/B
            if (col == 9) {
                if (pressed) g_mixer_bank = !g_mixer_bank;
                return true;
            }
            // Key 9 (col 8): Toggle back to Mixer Faders
            if (col == 8) {
                if (pressed) g_ui_mode = UI_MODE_MIXER;
                return true;
            }
            // Keys 1..4: Rec / Play / Overdub on Track
            if (col < 4) {
                if (pressed) {
                    uint8_t track = col + (g_mixer_bank * 4);
                    sequencer_select_track(track);
                    // Switch to sequencer mode for this track
                    g_ui_mode = UI_MODE_SEQ;
                }
                return true;
            }
            // Keys 5..8: Mute / Clear Track
            if (col >= 4 && col < 8) {
                if (pressed) {
                    uint8_t track = (col - 4) + (g_mixer_bank * 4);
                    synth_looper_clear_track(track);
                }
                return true;
            }
            break;
        }

        case UI_MODE_MENU_OSC:
        case UI_MODE_MENU_VCF:
        case UI_MODE_MENU_EG:
        case UI_MODE_MENU_LFO:
        case UI_MODE_MENU_ADV:
        case UI_MODE_MENU_PRESETS:
        case UI_MODE_MENU_FX: {
            uint8_t menu_idx = g_ui_mode - UI_MODE_MENU_OSC;
            if (col < 8) {
                if (pressed) {
                    s_selected_submenu_param = col;
                }
                return true;
            }
            // Key 9 (col 8): Decrement value or Back
            if (col == 8) {
                if (pressed) {
                    SynthParamID pid = s_menu_param_map[menu_idx][s_selected_submenu_param];
                    if (pid < PARAM_COUNT) {
                        uint8_t val = synth_get_param(pid);
                        if (val >= 8) val -= 8; else val = 0;
                        synth_set_param(pid, val);

                        // If in step edit mode, record parameter lock
                        if (g_seq_state.step_edit_mode) {
                            if (pid == PARAM_VCF_CUTOFF) sequencer_set_plock(PLOCK_FLAG_CUTOFF, val);
                            else if (pid == PARAM_VCF_RESONANCE) sequencer_set_plock(PLOCK_FLAG_RESONANCE, val);
                            else if (pid == PARAM_AMP_DECAY) sequencer_set_plock(PLOCK_FLAG_DECAY, val);
                            else if (pid == PARAM_VCF_DRIVE) sequencer_set_plock(PLOCK_FLAG_DRIVE, val);
                        }
                    }
                }
                return true;
            }
            // Key 10 (col 9): Increment value or Return to Main Menu
            if (col == 9) {
                if (pressed) {
                    SynthParamID pid = s_menu_param_map[menu_idx][s_selected_submenu_param];
                    if (pid < PARAM_COUNT) {
                        uint8_t val = synth_get_param(pid);
                        if (val <= 119) val += 8; else val = 127;
                        synth_set_param(pid, val);

                        // If in step edit mode, record parameter lock
                        if (g_seq_state.step_edit_mode) {
                            if (pid == PARAM_VCF_CUTOFF) sequencer_set_plock(PLOCK_FLAG_CUTOFF, val);
                            else if (pid == PARAM_VCF_RESONANCE) sequencer_set_plock(PLOCK_FLAG_RESONANCE, val);
                            else if (pid == PARAM_AMP_DECAY) sequencer_set_plock(PLOCK_FLAG_DECAY, val);
                            else if (pid == PARAM_VCF_DRIVE) sequencer_set_plock(PLOCK_FLAG_DRIVE, val);
                        }
                    }
                }
                return true;
            }
            break;
        }

        default:
            break;
    }

    return false;
}

// Hook called by DMK keyboard.c
bool hook_matrix_change(uint8_t row, uint8_t col, bool pressed) {
    (void)row;
    if (!s_synth_enabled) {
        // When synthesizer is disabled, let DMK handle matrix events as a standard keyboard
        return false;
    }
    return magneteno_ui_process_key(col, pressed);
}

// FreeRTOS periodic UI, Display, IMU & Aftertouch update task
static void magneteno_ui_task(void *pvParameters) {
    (void)pvParameters;
    magneteno_leds_init();
    magneteno_display_init();
    magneteno_imu_init();
    sequencer_init();
    synth_init();

    TickType_t last_wake = xTaskGetTickCount();
    uint8_t display_div = 0;

    while (1) {
        // 1. Advance sequencer clock (if synth is enabled)
        if (s_synth_enabled) {
            sequencer_poll();
        }

        // 2. Poll 6-axis IMU (tilt, shake, tap tempo)
        magneteno_imu_poll();

        if (s_synth_enabled) {
            // Feed IMU spatial gestures to synth:
            // Roll tilt -> vibrato / pitch bend (-2.0 .. +2.0 semitones)
            if (fabsf(g_imu_state.roll_deg) > 3.0f) {
                float bend = (g_imu_state.roll_deg / 45.0f) * 2.0f;
                if (bend > 2.0f) bend = 2.0f;
                if (bend < -2.0f) bend = -2.0f;
                synth_set_imu_pitch_bend(bend);
            } else {
                synth_set_imu_pitch_bend(0.0f);
            }

            // Pitch tilt -> master filter cutoff modulation
            if (fabsf(g_imu_state.pitch_deg) > 3.0f) {
                float mod = g_imu_state.pitch_deg * 80.0f;
                synth_set_imu_cutoff_mod(mod);
            } else {
                synth_set_imu_cutoff_mod(0.0f);
            }

            // 3. Process continuous polyphonic aftertouch in Piano mode
            if (g_ui_mode == UI_MODE_PIANO) {
                for (uint8_t i = 0; i < 8; i++) {
                    if (s_active_key_notes[i] >= 0) {
                        uint8_t pressure = hall_get_pressure(i);
                        synth_set_aftertouch((uint8_t)s_active_key_notes[i], pressure);
                    }
                }
            }
        }

        // 4. Update WS2812 LEDs (~60Hz rate)
        magneteno_leds_update(g_ui_mode);

        // 5. Update Sharp Memory LCD display (~30Hz rate: every 2nd tick)
        if (++display_div >= 2) {
            display_div = 0;
            magneteno_display_update(g_ui_mode);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(16)); // ~60Hz
    }
}

static StaticTask_t s_ui_task_tcb;
static StackType_t s_ui_task_stack[1024];

void magneteno_ui_init(void) {
    xTaskCreateStatic(magneteno_ui_task, "magneteno_ui", 1024, NULL, tskIDLE_PRIORITY + 2, s_ui_task_stack, &s_ui_task_tcb);
}
