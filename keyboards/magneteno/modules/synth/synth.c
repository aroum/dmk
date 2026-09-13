#include "synth.h"
#include "config.h"
#include "hooks.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/sync.h"
#include "i2s_tx.pio.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#define SAMPLE_RATE 44100.0f
#define SINE_LUT_SIZE 256
#define MAX_VOICES 8

#ifndef PIN_DAC_I2S_BCK
#define PIN_DAC_I2S_BCK 6
#endif
#ifndef PIN_DAC_I2S_DATA
#define PIN_DAC_I2S_DATA 7
#endif
#ifndef PIN_DAC_I2S_LRCK
#define PIN_DAC_I2S_LRCK 8
#endif

// Precomputed Sine Wave LUT (256 samples, int16_t, amplitude 32000)
static int16_t sine_lut[SINE_LUT_SIZE];

// Precalculated phase increments for MIDI notes 0..127 at 44100 Hz
static uint32_t note_phase_inc[128];

// Use dedicated PIO1 for I2S audio (pio0 is used by WS2812 and split)
static PIO audio_pio = pio1;
static uint audio_sm = 0;
static bool synth_started = false;
static volatile uint8_t master_volume = 80; // 0..100%

// Polyphonic voice state
typedef struct {
    bool active;
    bool releasing;
    uint8_t note;
    uint8_t velocity;
    uint32_t phase;
    uint32_t phase_inc;
    uint32_t env_level; // 0..65535 (Q16)
    uint32_t age;
} voice_t;

static voice_t voices[MAX_VOICES];
static uint32_t voice_age_counter = 0;

// Inter-core lock-free ring buffer (Core 0 producer, Core 1 consumer)
typedef struct {
    uint8_t cmd; // 1=NoteOn, 0=NoteOff, 2=AllNotesOff
    uint8_t note;
    uint8_t vel;
} synth_event_t;

#define SYNTH_QUEUE_SIZE 64
static volatile synth_event_t s_synth_queue[SYNTH_QUEUE_SIZE];
static volatile uint32_t s_synth_head = 0;
static volatile uint32_t s_synth_tail = 0;

static inline void synth_send_event(uint8_t cmd, uint8_t note, uint8_t vel) {
    uint32_t next_head = (s_synth_head + 1) % SYNTH_QUEUE_SIZE;
    if (next_head != s_synth_tail) {
        s_synth_queue[s_synth_head].cmd = cmd;
        s_synth_queue[s_synth_head].note = note;
        s_synth_queue[s_synth_head].vel = vel;
        __dmb();
        s_synth_head = next_head;
    }
}

// Internal voice allocators (run on Core 1)
static void internal_note_on(uint8_t note, uint8_t vel) {
    if (note > 127)
        return;
    if (vel == 0) {
        for (int i = 0; i < MAX_VOICES; i++) {
            if (voices[i].active && voices[i].note == note) {
                voices[i].releasing = true;
            }
        }
        return;
    }

    // 1. Retrigger voice if note is already active
    for (int i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].note == note) {
            voices[i].velocity = vel;
            voices[i].releasing = false;
            voices[i].age = ++voice_age_counter;
            return;
        }
    }

    // 2. Find inactive voice slot
    int target = -1;
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!voices[i].active) {
            target = i;
            break;
        }
    }

    // 3. Voice stealing: prioritize releasing voice, else steal oldest voice
    if (target < 0) {
        uint32_t oldest_age = 0xFFFFFFFF;
        for (int i = 0; i < MAX_VOICES; i++) {
            if (voices[i].releasing) {
                target = i;
                break;
            }
            if (voices[i].age < oldest_age) {
                oldest_age = voices[i].age;
                target = i;
            }
        }
    }

    if (target >= 0) {
        voice_t *v = &voices[target];
        v->active = true;
        v->releasing = false;
        v->note = note;
        v->velocity = vel;
        v->phase_inc = note_phase_inc[note];
        v->env_level = 0; // Smooth attack starts from 0 to prevent audio clicks
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

// Core 1 Audio Loop: synthesizes active voices and continuously streams I2S to DAC
static void __not_in_flash_func(audio_core_entry)(void) {
    multicore_lockout_victim_init();

    while (1) {
        // 1. Drain pending inter-core commands from Core 0
        while (s_synth_tail != s_synth_head) {
            synth_event_t evt = s_synth_queue[s_synth_tail];
            __dmb();
            s_synth_tail = (s_synth_tail + 1) % SYNTH_QUEUE_SIZE;

            if (evt.cmd == 1) {
                internal_note_on(evt.note, evt.vel);
            } else if (evt.cmd == 0) {
                internal_note_off(evt.note);
            } else if (evt.cmd == 2) {
                internal_all_notes_off();
            }
        }

        // 2. Synthesize audio across all active polyphonic voices
        int32_t mixed_sample = 0;
        bool any_active = false;

        for (int i = 0; i < MAX_VOICES; i++) {
            voice_t *v = &voices[i];
            if (!v->active)
                continue;
            any_active = true;

            // Attack & Release Envelope
            if (!v->releasing) {
                // Smooth attack (~2.5ms = 120 samples) to eliminate click
                if (v->env_level < 65000) {
                    v->env_level += 550;
                    if (v->env_level > 65535)
                        v->env_level = 65535;
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

            // Scale by envelope (0..65535) and velocity (1..127)
            int32_t scaled = ((int32_t)wave * (int32_t)v->env_level) >> 16;
            int32_t voice_out = (scaled * (int32_t)v->velocity) >> 8;

            mixed_sample += voice_out;
        }

        if (any_active) {
            // Master volume scaling
            mixed_sample = (mixed_sample * (int32_t)master_volume) / 100;

            // Smooth musical soft-saturation (prevents harsh digital flat-topping on multi-voice chords)
            if (mixed_sample > 24000) {
                int32_t excess = mixed_sample - 24000;
                mixed_sample = 24000 + (excess * 8000) / (excess + 8000);
                if (mixed_sample > 32000)
                    mixed_sample = 32000;
            } else if (mixed_sample < -24000) {
                int32_t excess = -mixed_sample - 24000;
                int32_t compressed = 24000 + (excess * 8000) / (excess + 8000);
                if (compressed > 32000)
                    compressed = 32000;
                mixed_sample = -compressed;
            }
        } else {
            // Silence when no voices active, but keep writing to keep I2S clocks active
            mixed_sample = 0;
        }

        // 3. Pack mono sample into stereo 32-bit (Left in high 16b, Right in low 16b)
        int16_t s16 = (int16_t)mixed_sample;
        uint32_t packed_sample = ((uint32_t)(uint16_t)s16 << 16) | (uint16_t)s16;

        // 4. Send to I2S PIO TX FIFO (automatically clocks DAC at 44100 Hz)
        pio_sm_put_blocking(audio_pio, audio_sm, packed_sample);
    }
}

void synth_init(void) {
    if (synth_started)
        return;

    // 1. Generate Sine Wave LUT (256 values)
    for (int i = 0; i < SINE_LUT_SIZE; i++) {
        float angle = (float)i * (2.0f * (float)M_PI) / (float)SINE_LUT_SIZE;
        sine_lut[i] = (int16_t)(sinf(angle) * 32000.0f);
    }

    // 2. Precalculate phase increments for MIDI notes 0..127 at 44100 Hz
    for (int n = 0; n < 128; n++) {
        float freq = 440.0f * powf(2.0f, (float)(n - 69) / 12.0f);
        double inc = ((double)freq * 4294967296.0) / (double)SAMPLE_RATE;
        note_phase_inc[n] = (uint32_t)(inc + 0.5);
    }

    memset((void *)voices, 0, sizeof(voices));

    // 3. Initialize I2S PIO program on dedicated pio1
    uint offset = pio_add_program(audio_pio, &i2s_tx_program);
    audio_sm = pio_claim_unused_sm(audio_pio, true);
    i2s_tx_program_init(audio_pio, audio_sm, offset, PIN_DAC_I2S_DATA, PIN_DAC_I2S_BCK, PIN_DAC_I2S_LRCK, SAMPLE_RATE);

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
    if (volume_percent > 100)
        volume_percent = 100;
    master_volume = volume_percent;
}

// Hook early initialization to launch synth on boot
void hook_early_init(void) {
    synth_init();
}

// Intercept incoming host USB MIDI
void hook_midi_receive(const uint8_t packet[4]) {
    uint8_t cin = packet[0] & 0x0F;
    uint8_t status = packet[1] & 0xF0;
    uint8_t note = packet[2];
    uint8_t vel = packet[3];

    if ((cin == 0x09 || status == 0x90) && vel > 0) {
        synth_note_on(note, vel);
    } else if (cin == 0x08 || status == 0x80 || ((cin == 0x09 || status == 0x90) && vel == 0)) {
        synth_note_off(note);
    } else if ((cin == 0x0B || status == 0xB0) && (packet[2] >= 120 && packet[2] <= 123)) {
        synth_all_notes_off();
    }
}

// Intercept local keyboard MIDI
void hook_midi_send(const uint8_t *msg, uint8_t len) {
    if (len >= 3) {
        uint8_t status = msg[0] & 0xF0;
        uint8_t note = msg[1];
        uint8_t vel = msg[2];

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
void synth_note_on(uint8_t note, uint8_t velocity) {
    (void)note;
    (void)velocity;
}
void synth_note_off(uint8_t note) {
    (void)note;
}
void synth_all_notes_off(void) {}
void synth_set_master_volume(uint8_t volume_percent) {
    (void)volume_percent;
}
#endif
