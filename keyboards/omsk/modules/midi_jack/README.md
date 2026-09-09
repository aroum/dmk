# MIDI Jack Module for DMK

External modular hardware MIDI DIN-5 / TRS Jack (31250 baud serial UART) transport for DMK.

## Features

- Transmits all DMK keypress-generated MIDI messages (Notes, Control Change, Pitch Bend) over hardware UART at the standard 31250 baud rate.
- Intercepts outgoing MIDI messages automatically via `hook_midi_send()`.
- Spawns a background FreeRTOS task (`midi_rx_task`) for incoming MIDI stream handling and optional `MIDI_THRU` pass-through.
- Hardware implementations for **Raspberry Pi (RP2040 / RP2350)** and **Milandr (K1986BE92FI)**.

## Configuration (`config.h`)

```c
// Define UART TX and RX pins (RP2040 defaults to GPIO0 and GPIO1):
#define PIN_MIDI_JACK_OUT GPIO0
#define PIN_MIDI_JACK_IN  GPIO1

// Optional: Enable MIDI-Thru (repeats input stream to output)
#define MIDI_THRU

// Optional: Override baud rate (defaults to standard 31250)
#define MIDI_JACK_BAUD 31250
```

## Build Usage

Enable the module with `-m tests/modules/midi_jack` (or `-m midi_jack`):

```bash
./build_all.sh -b omsk --mcu rp2040 -m midi_jack -c
```
