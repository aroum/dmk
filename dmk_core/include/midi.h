#ifndef MIDI_H
#define MIDI_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MIDI_USB) || defined(MIDI_JACK)

// Initialize MIDI subsystem (tables and hardware interfaces)
void dmk_midi_init(void);

// Process a MIDI keycode. Returns true if keycode is a MIDI action and was handled.
bool dmk_midi_process_keycode(uint32_t key, bool pressed);

#else

static inline void dmk_midi_init(void) {}
static inline bool dmk_midi_process_keycode(uint32_t key, bool pressed) {
    (void)key;
    (void)pressed;
    return false;
}

#endif // defined(MIDI_USB) || defined(MIDI_JACK)

#ifdef __cplusplus
}
#endif

#endif // MIDI_H
