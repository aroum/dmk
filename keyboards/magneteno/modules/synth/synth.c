#include "synth.h"
#include "config.h"
#include "hooks.h"

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "i2s_tx.pio.h"

#define SAMPLE_RATE 48000
#define MAX_VOICES 8
#define SINE_LUT_SIZE 256

#ifndef PIN_DAC_I2S_BCK
#define PIN_DAC_I2S_BCK 6
#endif
#ifndef PIN_DAC_I2S_DATA
#define PIN_DAC_I2S_DATA 7
#endif
#ifndef PIN_DAC_I2S_LRCK
#define PIN_DAC_I2S_LRCK 8
#endif

// Precomputed Sine Wave LUT (256 samples, int16_t, full scale)
static int16_t sine_lut[SINE_LUT_SIZE];

// Precalculated phase increment per MIDI note (0..127) at 48000 Hz
static uint32_t note_phase_inc[128];

// Voice descriptor for polyphonic sine synth
typedef struct {
    bool active;
    bool releasing;
    uint8_t note;
    uint8_t velocity;       // 1..127
    uint32_t phase;         // 32-bit fixed point phase accumulator
    uint32_t phase_inc;     // Frequency phase step per sample
    uint32_t env_level;     // 16-bit envelope level (0..65535)
    uint32_t age;           // Age counter for voice stealing
} voice_t;

static voice_t voices[MAX_VOICES];
static uint32_t voice_age_counter = 0;
static volatile uint8_t master_volume = 100; // 0..100%
static PIO audio_pio = pio0;
static uint audio_sm = 0;
static bool synth_started = false;

// Internal voice management (runs on Core 1)
static void internal_note_on(uint8_t note, uint8_t vel) {
    if (note > 127) return;
    if (vel == 0) {
        // Velocity 0 is equivalent to Note-Off in MIDI
        for (int i = 0; i < MAX_VOICES; i++) {
            if (voices[i].active && voices[i].note == note) {
                voices[i].releasing = true;
            }
        }
        return;
    }

    // 1. Check if note is already playing (retrigger voice)
    for (int i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].note == note) {
            voices[i].velocity = vel;
            voices[i].releasing = false;
            voices[i].age = ++voice_age_counter;
            return;
        }
    }

    // 2. Find an empty (inactive) voice
    int target_slot = -1;
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!voices[i].active) {
            target_slot = i;
            break;
        }
    }

    // 3. If all voices busy, steal a releasing voice or the oldest voice
    if (target_slot < 0) {
        uint32_t oldest_age = 0xFFFFFFFF;
        for (int i = 0; i < MAX_VOICES; i++) {
            if (voices[i].releasing) {
                target_slot = i;
                break;
            }
            if (voices[i].age < oldest_age) {
                oldest_age = voices[i].age;
                target_slot = i;
            }
        }
    }

    if (target_slot >= 0) {
        voice_t *v = &voices[target_slot];
        v->active = true;
        v->releasing = false;
        v->note = note;
        v->velocity = vel;
        v->phase_inc = note_phase_inc[note];
        v->env_level = 0; // Soft attack starts at 0 to avoid pop
        v->age = ++voice_age_counter;
    }
}

static void internal_note_off(uint8_t note) {
    for (int i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].note == note) {
            voices[i].releasing = true;
        }
    }
}

static void internal_all_notes_off(void) {
    for (int i = 0; i < MAX_VOICES; i++) {
        voices[i].releasing = true;
    }
}

// Core 1 Audio Loop
static void __not_in_flash_func(audio_core_entry)(void) {
    multicore_lockout_victim_init();

    while (1) {
        // 1. Process pending inter-core commands from Core 0
        while (multicore_fifo_rvalid()) {
            uint32_t msg = multicore_fifo_pop_blocking();
            uint8_t cmd  = (msg >> 16) & 0xFF;
            uint8_t note = (msg >> 8) & 0xFF;
            uint8_t vel  = msg & 0xFF;

            if (cmd == 1) {
                internal_note_on(note, vel);
            } else if (cmd == 0) {
                internal_note_off(note);
            } else if (cmd == 2) {
                internal_all_notes_off();
            }
        }

        // 2. Synthesize one audio sample
        int32_t mixed_sample = 0;
        bool any_active = false;

        for (int i = 0; i < MAX_VOICES; i++) {
            voice_t *v = &voices[i];
            if (!v->active) continue;
            any_active = true;

            // Envelope progression:
            if (!v->releasing) {
                // Smooth attack (~2.5ms = 120 samples) to eliminate click
                if (v->env_level < 65000) {
                    v->env_level += 550;
                    if (v->env_level > 65535) v->env_level = 65535;
                }
            } else {
                // Exponential decay release (~50ms)
                if (v->env_level > 64) {
                    v->env_level = (v->env_level * 1021) >> 10;
                } else {
                    v->env_level = 0;
                    v->active = false;
                    continue;
                }
            }

            // Phase accumulator lookup into 256-sample sine table
            uint8_t lut_idx = (uint8_t)(v->phase >> 24);
            int16_t wave = sine_lut[lut_idx];
            v->phase += v->phase_inc;

            // Apply velocity (1..127) and envelope (0..65535)
            // (wave * env_level) >> 16 gives scaled wave
            // Then scale by velocity / 127
            int32_t scaled = ((int32_t)wave * (int32_t)v->env_level) >> 16;
            int32_t voice_out = (scaled * (int32_t)v->velocity) >> 7;

            mixed_sample += voice_out;
        }

        if (any_active) {
            // Apply master volume
            mixed_sample = (mixed_sample * (int32_t)master_volume) / 100;

            // Soft-clamp output to 16-bit dynamic range
            if (mixed_sample > 32767) mixed_sample = 32767;
            if (mixed_sample < -32768) mixed_sample = -32768;
        } else {
            mixed_sample = 0;
        }

        // 3. Pack mono sample into stereo 32-bit (Left in high 16b, Right in low 16b)
        int16_t s16 = (int16_t)mixed_sample;
        uint32_t packed = ((uint32_t)(uint16_t)s16 << 16) | (uint16_t)s16;

        // 4. Send to I2S PIO TX FIFO (blocks automatically to clock audio at 48000 Hz)
        pio_sm_put_blocking(audio_pio, audio_sm, packed);
    }
}

static inline void synth_send_event(uint8_t cmd, uint8_t note, uint8_t vel) {
    uint32_t msg = ((uint32_t)cmd << 16) | ((uint32_t)note << 8) | (vel & 0xFF);
    multicore_fifo_push_timeout_us(msg, 50);
}

void synth_init(void) {
    if (synth_started) return;

    // 1. Generate Sine Wave LUT (256 values)
    for (int i = 0; i < SINE_LUT_SIZE; i++) {
        float angle = (float)i * (2.0f * (float)M_PI) / (float)SINE_LUT_SIZE;
        sine_lut[i] = (int16_t)(sinf(angle) * 32760.0f);
    }

    // 2. Precalculate phase increments for MIDI notes 0..127
    // Formula: freq = 440.0 * 2^((note - 69) / 12)
    // phase_inc = round(freq * 2^32 / SAMPLE_RATE)
    for (int n = 0; n < 128; n++) {
        float freq = 440.0f * powf(2.0f, (float)(n - 69) / 12.0f);
        double inc = ((double)freq * 4294967296.0) / (double)SAMPLE_RATE;
        note_phase_inc[n] = (uint32_t)(inc + 0.5);
    }

    memset(voices, 0, sizeof(voices));

    // 3. Initialize I2S PIO program
    uint offset = pio_add_program(audio_pio, &i2s_tx_program);
    audio_sm = pio_claim_unused_sm(audio_pio, true);
    i2s_tx_program_init(audio_pio, audio_sm, offset, PIN_DAC_I2S_DATA, PIN_DAC_I2S_BCK, PIN_DAC_I2S_LRCK, (float)SAMPLE_RATE);

    // 4. Launch Core 1 Audio Task
    multicore_launch_core1(audio_core_entry);
    synth_started = true;
}

void synth_note_on(uint8_t note, uint8_t velocity) {
    synth_send_event(1, note, velocity);
}

void synth_note_off(uint8_t note) {
    synth_send_event(0, note, 0);
}

void synth_all_notes_off(void) {
    synth_send_event(2, 0, 0);
}

void synth_set_master_volume(uint8_t volume_percent) {
    if (volume_percent > 100) volume_percent = 100;
    master_volume = volume_percent;
}

// Hook early initialization to launch synth on boot
void hook_early_init(void) {
    synth_init();
}

// Intercept incoming host USB MIDI
void hook_midi_receive(const uint8_t packet[4]) {
    uint8_t cin  = packet[0] & 0x0F;
    uint8_t note = packet[2];
    uint8_t vel  = packet[3];

    if (cin == 0x09 && vel > 0) {
        synth_note_on(note, vel);
    } else if (cin == 0x08 || (cin == 0x09 && vel == 0)) {
        synth_note_off(note);
    }
}

// Intercept local keyboard MIDI
void hook_midi_send(const uint8_t *msg, uint8_t len) {
    if (len >= 3) {
        uint8_t status = msg[0] & 0xF0;
        uint8_t note   = msg[1];
        uint8_t vel    = msg[2];

        if (status == 0x90 && vel > 0) {
            synth_note_on(note, vel);
        } else if (status == 0x80 || (status == 0x90 && vel == 0)) {
            synth_note_off(note);
        }
    }
}

#else
// Stubs for non-RP architectures
void synth_init(void) {}
void synth_note_on(uint8_t note, uint8_t velocity) { (void)note; (void)velocity; }
void synth_note_off(uint8_t note) { (void)note; }
void synth_all_notes_off(void) {}
void synth_set_master_volume(uint8_t volume_percent) { (void)volume_percent; }
#endif
