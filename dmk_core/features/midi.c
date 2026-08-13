#include "midi.h"
#include "FreeRTOS.h"
#include "config.h"
#include "keys.h"
#include "queue.h"
#include "usb.h"

#if defined(MIDI_USB) || defined(MIDI_JACK)

// Active MIDI configuration state (channel, octave, transpose, velocity)
typedef struct {
    uint8_t octave;
    int8_t transpose;
    uint8_t velocity;
    uint8_t channel;
} dmk_midi_config_t;

static dmk_midi_config_t midi_config = {.octave = 2, .transpose = 0, .velocity = 127, .channel = 0};

// Tracks active notes sent per tone position (0xFF = inactive)
static uint8_t tone_status[72];
static uint8_t midi_cc_values[128];
static bool midi_initialized = false;

#ifdef MIDI_USB
extern void usb_send_midi_noteon(uint8_t chan, uint8_t note, uint8_t vel);
extern void usb_send_midi_noteoff(uint8_t chan, uint8_t note, uint8_t vel);
extern void usb_send_midi_cc(uint8_t chan, uint8_t cc, uint8_t val);
extern void usb_send_midi_pitchbend(uint8_t chan, int16_t val);
#endif

#ifdef MIDI_JACK
extern void midi_jack_init(void);
extern void midi_jack_write(uint8_t byte);

static void jack_send_midi_noteon(uint8_t chan, uint8_t note, uint8_t vel) {
    midi_jack_write(0x90 | (chan & 0x0F));
    midi_jack_write(note & 0x7F);
    midi_jack_write(vel & 0x7F);
}
static void jack_send_midi_noteoff(uint8_t chan, uint8_t note, uint8_t vel) {
    midi_jack_write(0x80 | (chan & 0x0F));
    midi_jack_write(note & 0x7F);
    midi_jack_write(vel & 0x7F);
}
static void jack_send_midi_cc(uint8_t chan, uint8_t cc, uint8_t val) {
    midi_jack_write(0xB0 | (chan & 0x0F));
    midi_jack_write(cc & 0x7F);
    midi_jack_write(val & 0x7F);
}
static void jack_send_midi_pitchbend(uint8_t chan, int16_t val) {
    uint16_t pb = (uint16_t)(val + 8192);
    midi_jack_write(0xE0 | (chan & 0x0F));
    midi_jack_write(pb & 0x7F);
    midi_jack_write((pb >> 7) & 0x7F);
}
#endif

/**
 * @brief Dispatch MIDI Note-On message across enabled transports (USB and/or DIN5/TRS Jack).
 */
static void send_midi_noteon(uint8_t chan, uint8_t note, uint8_t vel) {
#ifdef MIDI_USB
    usb_send_midi_noteon(chan, note, vel);
#endif
#ifdef MIDI_JACK
    jack_send_midi_noteon(chan, note, vel);
#endif
}

/**
 * @brief Dispatch MIDI Note-Off message across enabled transports.
 */
static void send_midi_noteoff(uint8_t chan, uint8_t note, uint8_t vel) {
#ifdef MIDI_USB
    usb_send_midi_noteoff(chan, note, vel);
#endif
#ifdef MIDI_JACK
    jack_send_midi_noteoff(chan, note, vel);
#endif
}

/**
 * @brief Dispatch MIDI Control Change (CC) message across enabled transports.
 */
static void send_midi_cc(uint8_t chan, uint8_t cc, uint8_t val) {
#ifdef MIDI_USB
    usb_send_midi_cc(chan, cc, val);
#endif
#ifdef MIDI_JACK
    jack_send_midi_cc(chan, cc, val);
#endif
}

/**
 * @brief Dispatch MIDI Pitch Bend message across enabled transports.
 */
static void send_midi_pitchbend(uint8_t chan, int16_t val) {
#ifdef MIDI_USB
    usb_send_midi_pitchbend(chan, val);
#endif
#ifdef MIDI_JACK
    jack_send_midi_pitchbend(chan, val);
#endif
}

/**
 * @brief Initialize MIDI state, CC lookup tables, and hardware UART/Jack output if enabled.
 */
void dmk_midi_init(void) {
    for (int i = 0; i < 72; i++) {
        tone_status[i] = 0xFF;
    }
    for (int i = 0; i < 128; i++) {
        midi_cc_values[i] = 64;
    }
#ifdef MIDI_JACK
    midi_jack_init();
#endif
    midi_initialized = true;
}

/**
 * @brief Process MIDI keycodes (notes, octaves, CC, sustain, velocity, pitch bend).
 * @param key 32-bit composite keycode.
 * @param pressed True for keydown, false for keyup.
 * @return true if key was handled as a MIDI message.
 */
bool dmk_midi_process_keycode(uint32_t key, bool pressed) {
    if (!midi_initialized) {
        dmk_midi_init();
    }

    // 1. Check custom MIDI CC static macro (0x21000000 prefix)
    if ((key & 0xFF000000) == 0x21000000) {
        uint8_t cc = (key >> 8) & 0x7F;
        uint8_t val = key & 0x7F;
        send_midi_cc(midi_config.channel, cc, pressed ? val : 0);
        return true;
    }

    // 2. Check custom MIDI CC fixed 127 trigger range
    if (key >= MIDI_CC_VAL_127_BASE && key < MIDI_CC_VAL_127_BASE + 16) {
        uint8_t cc = key - MIDI_CC_VAL_127_BASE;
        send_midi_cc(midi_config.channel, cc, pressed ? 127 : 0);
        return true;
    }

    // 3. Check custom MIDI CC toggle range
    if (key >= MIDI_CC_TOGGLE_BASE && key < MIDI_CC_TOGGLE_BASE + 16) {
        if (pressed) {
            uint8_t cc = key - MIDI_CC_TOGGLE_BASE;
            midi_cc_values[cc] = (midi_cc_values[cc] == 127) ? 0 : 127;
            send_midi_cc(midi_config.channel, cc, midi_cc_values[cc]);
        }
        return true;
    }

    // 4. Check custom MIDI CC Increment/Decrement
    if (key >= MIDI_CC_INC_BASE && key < MIDI_CC_INC_BASE + 16) {
        if (pressed) {
            uint8_t cc = key - MIDI_CC_INC_BASE;
            if (midi_cc_values[cc] < 127) {
                midi_cc_values[cc]++;
            }
            send_midi_cc(midi_config.channel, cc, midi_cc_values[cc]);
        }
        return true;
    }
    if (key >= MIDI_CC_DEC_BASE && key < MIDI_CC_DEC_BASE + 16) {
        if (pressed) {
            uint8_t cc = key - MIDI_CC_DEC_BASE;
            if (midi_cc_values[cc] > 0) {
                midi_cc_values[cc]--;
            }
            send_midi_cc(midi_config.channel, cc, midi_cc_values[cc]);
        }
        return true;
    }

    // 5. Check standard QMK MIDI keycodes (0x7100 - 0x71FF)
    if (key >= MIDI_ON && key <= 0x71FF) {
        switch (key) {
        case MIDI_NOTE_C_0 ... MIDI_NOTE_B_5: {
            uint8_t tone = key - MIDI_NOTE_C_0;
            if (pressed) {
                if (tone_status[tone] == 0xFF) {
                    int note_val = 12 * midi_config.octave + tone + midi_config.transpose;
                    if (note_val < 0)
                        note_val = 0;
                    if (note_val > 127)
                        note_val = 127;
                    tone_status[tone] = (uint8_t)note_val;
                    send_midi_noteon(midi_config.channel, (uint8_t)note_val, midi_config.velocity);
                }
            } else {
                uint8_t note_val = tone_status[tone];
                if (note_val != 0xFF) {
                    send_midi_noteoff(midi_config.channel, note_val, 0);
                    tone_status[tone] = 0xFF;
                }
            }
            break;
        }
        case MIDI_OCTAVE_N2 ... MIDI_OCTAVE_7:
            if (pressed) {
                midi_config.octave = (uint8_t)(key - MIDI_OCTAVE_N2);
            }
            break;
        case MIDI_OCTAVE_DOWN:
            if (pressed && midi_config.octave > 0) {
                midi_config.octave--;
            }
            break;
        case MIDI_OCTAVE_UP:
            if (pressed && midi_config.octave < 9) {
                midi_config.octave++;
            }
            break;
        case MIDI_TRANSPOSE_N6 ... MIDI_TRANSPOSE_6:
            if (pressed) {
                midi_config.transpose = (int8_t)((int)key - 0x715D);
            }
            break;
        case MIDI_TRANSPOSE_DOWN:
            if (pressed && midi_config.transpose > -12) {
                midi_config.transpose--;
            }
            break;
        case MIDI_TRANSPOSE_UP:
            if (pressed && midi_config.transpose < 12) {
                midi_config.transpose++;
            }
            break;
        case MIDI_VELOCITY_0 ... MIDI_VELOCITY_10:
            if (pressed) {
                midi_config.velocity = (uint8_t)((key - MIDI_VELOCITY_0) * 12.7f);
            }
            break;
        case MIDI_VELOCITY_DOWN:
            if (pressed && midi_config.velocity >= 13) {
                midi_config.velocity -= 13;
            }
            break;
        case MIDI_VELOCITY_UP:
            if (pressed && midi_config.velocity <= 114) {
                midi_config.velocity += 13;
            }
            break;
        case MIDI_CHANNEL_1 ... MIDI_CHANNEL_16:
            if (pressed) {
                midi_config.channel = (uint8_t)(key - MIDI_CHANNEL_1);
            }
            break;
        case MIDI_CHANNEL_DOWN:
            if (pressed && midi_config.channel > 0) {
                midi_config.channel--;
            }
            break;
        case MIDI_CHANNEL_UP:
            if (pressed && midi_config.channel < 15) {
                midi_config.channel++;
            }
            break;
        case MIDI_ALL_NOTES_OFF:
            if (pressed) {
                send_midi_cc(midi_config.channel, 123, 0);
            }
            break;
        case MIDI_SUSTAIN:
            send_midi_cc(midi_config.channel, 64, pressed ? 127 : 0);
            break;
        case MIDI_PORTAMENTO:
            send_midi_cc(midi_config.channel, 65, pressed ? 127 : 0);
            break;
        case MIDI_SOSTENUTO:
            send_midi_cc(midi_config.channel, 66, pressed ? 127 : 0);
            break;
        case MIDI_SOFT:
            send_midi_cc(midi_config.channel, 67, pressed ? 127 : 0);
            break;
        case MIDI_LEGATO:
            send_midi_cc(midi_config.channel, 68, pressed ? 127 : 0);
            break;
        case MIDI_PITCH_BEND_DOWN:
            send_midi_pitchbend(midi_config.channel, pressed ? -8192 : 0);
            break;
        case MIDI_PITCH_BEND_UP:
            send_midi_pitchbend(midi_config.channel, pressed ? 8191 : 0);
            break;
        default:
            break;
        }
        return true;
    }

    return false;
}

#endif // defined(MIDI_USB) || defined(MIDI_JACK)
