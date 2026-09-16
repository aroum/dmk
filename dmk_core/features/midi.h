#ifndef MIDI_H
#define MIDI_H

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(MIDI_USB) || defined(MIDI_ENABLE) || defined(MIDI_JACK)) && !defined(NO_MIDI)

/**
 * @brief Initialize MIDI subsystem state and hardware interfaces.
 */
void dmk_midi_init(void);

/**
 * @brief Process a MIDI keycode.
 * @param key 32-bit keycode
 * @param pressed True on press, false on release
 * @return true if keycode is a MIDI action and was handled, false otherwise
 */
bool dmk_midi_process_keycode(uint32_t key, bool pressed);

#else

static inline void dmk_midi_init(void) {}
static inline bool dmk_midi_process_keycode(uint32_t key, bool pressed) {
    (void)key;
    (void)pressed;
    return false;
}

#endif // defined(MIDI_USB) || defined(MIDI_ENABLE) || defined(MIDI_JACK)

#ifdef __cplusplus
}
#endif

#endif // MIDI_H
