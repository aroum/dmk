#ifndef MIDI_JACK_H
#define MIDI_JACK_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PIN_MIDI_JACK_OUT
#define PIN_MIDI_JACK_OUT 0
#endif

#ifndef PIN_MIDI_JACK_IN
#define PIN_MIDI_JACK_IN 1
#endif

#ifndef MIDI_JACK_BAUD
#define MIDI_JACK_BAUD 31250
#endif

/**
 * @brief Initialize hardware UART for MIDI at 31250 baud and spawn background RX task.
 */
void midi_jack_init(void);

/**
 * @brief Transmit a raw byte over hardware MIDI UART.
 */
void midi_jack_write(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif // MIDI_JACK_H
