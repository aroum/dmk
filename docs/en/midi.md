# MIDI Support

The firmware supports sending MIDI messages (Notes, Control Change, Pitch Bend, Octave/Channel/Transpose controls) directly from keypresses and rotary encoders.

## MIDI Configuration and Compilation

MIDI support is flexible and can be customized in the keyboard's `config.h` using the following definitions:

- `#define MIDI_USB`: Enables USB MIDI support (using TinyUSB on RP2040 or native hardware USB driver on Milandr).
- `#define MIDI_JACK`: Enables physical MIDI port (DIN-5) support via UART.
- `#define MIDI_THRU`: (When `MIDI_JACK` is enabled) automatically routes incoming MIDI data from the input port to the output port.

### Physical MIDI Port Pins Definition

If `MIDI_JACK` is enabled, you must also define the UART TX and RX pins:

```c
#define PIN_MIDI_JACK_OUT 0 // GPIO/pin number for transmission (TX)
#define PIN_MIDI_JACK_IN 1  // GPIO/pin number for reception (RX)
```

Example build command:

```bash
./build_all.sh --keyboard omsk --mcu rp2040 -cs -d MIDI_USB -d MIDI_JACK
```

If neither `MIDI_USB` nor `MIDI_JACK` is defined, all MIDI features and descriptors are omitted, keeping the binary size as small as possible.

### Mapping Fixed MIDI CC Values to Keys

In your static keymap in `config.h`, you can map a key to send a specific MIDI CC value on press and `0` on release using the `MIDI_CC(cc, value)` macro:

- **Key Press**: Sends a MIDI CC message with number `cc` (0–127) and value `value` (0–127).
- **Key Release**: Sends a MIDI CC message with number `cc` and value `0`.

The following manual mapping macros are also available:

- `MIDI_CC_X_VAL_127(cc)`: Sends value 127 on press, and 0 on release for CC `cc`.
- `MIDI_CC_X_TOGGLE(cc)`: Toggles CC `cc` parameter state between 127 and 0 on each press.

Example usage in keymap:

```c
MIDI_CC(10, 127),     // Sends CC 10 with value 127 on press, and value 0 on release
MIDI_CC_X_VAL_127(11), // Same as above for CC 11
MIDI_CC_X_TOGGLE(12),  // Toggles state of CC 12 (127 -> 0 -> 127) on press
```

## MIDI Keycodes

Standard MIDI keycodes are supported (corresponding to QMK MIDI):

- **Notes**: `MIDI_NOTE_C_0` to `MIDI_NOTE_B_5` (6 octaves of notes, dynamically transposed by octave/transpose controls).
- **Octave Controls**: `MIDI_OCTAVE_DOWN` / `MIDI_OCTAVE_UP` and direct octave selection (`MIDI_OCTAVE_N2` to `7`).
- **Transpose Controls**: `MIDI_TRANSPOSE_DOWN` / `MIDI_TRANSPOSE_UP` and direct transpose selection.
- **Velocity Controls**: `MIDI_VELOCITY_DOWN` / `MIDI_VELOCITY_UP` and direct velocity selection (`0` to `10`).
- **Channel Controls**: `MIDI_CHANNEL_DOWN` / `MIDI_CHANNEL_UP` and direct channel selection (`1` to `16`).
- **Other Controls**:
  - `MIDI_ALL_NOTES_OFF`: Sends a panic message to turn off all playing notes.
  - `MIDI_SUSTAIN` / `MIDI_PORTAMENTO` / `MIDI_SOSTENUTO` / `MIDI_SOFT` / `MIDI_LEGATO`: Standard MIDI pedals / CC actions.
  - `MIDI_PITCH_BEND_DOWN` / `MIDI_PITCH_BEND_UP`: Pitch bend wheel simulation.

## Advanced MIDI & Vial Integration

Advanced MIDI mapping is fully integrated with **Vial**.

### Custom MIDI CC Increment & Decrement

Vial allows you to map keys or encoder rotations to increment or decrement absolute MIDI Control Change (CC) parameters:

- **MIDI CC 0-15 Inc**: `MIDI_CC_X_INC` (values `0x7E00` to `0x7E0F`)
- **MIDI CC 0-15 Dec**: `MIDI_CC_X_DEC` (values `0x7E10` to `0x7E1F`)
- **MIDI CC 0-15 Val 127**: `MIDI_CC_X_VAL_127` (values `0x7E20` to `0x7E2F`)
- **MIDI CC 0-15 Toggle**: `MIDI_CC_X_TOGGLE` (values `0x7E30` to `0x7E3F`)

These are available in the **Custom** tab in the Vial GUI.

### Encoder Setup (Absolute CC Mode)

To set up an encoder to control a CC value (e.g. Volume/CC 10) in absolute mode:

1. Map the **Clockwise (CW)** rotation of the encoder to `MIDI CC 10 Inc`.
2. Map the **Counter-Clockwise (CCW)** rotation of the encoder to `MIDI CC 10 Dec`.

The encoder will track the absolute value of CC 10 (0 to 127, starting at 64) and transmit the exact absolute value upon rotation.

### MIDI Keycode Reference Table

| Group / Function        | Keycode / Format                                                                    | Value Range               | Description                                             |
| ----------------------- | ----------------------------------------------------------------------------------- | ------------------------- | ------------------------------------------------------- |
| **Notes**               | `MIDI_NOTE_C_0` .. `MIDI_NOTE_B_5`                                                  | 6 octaves (72 notes)      | Plays note at current velocity                          |
| **Octave Selection**    | `MIDI_OCTAVE_N2` .. `MIDI_OCTAVE_7`                                                 | -2 to 7                   | Sets the base octave for notes                          |
| **Octave Shift**        | `MIDI_OCTAVE_DOWN` / `MIDI_OCTAVE_UP`                                               | —                         | Lowers or raises base octave by 1                       |
| **Transpose Selection** | `MIDI_TRANSPOSE_N6` .. `MIDI_TRANSPOSE_6`                                           | -6 to 6 semitones         | Shifts pitch transposition                              |
| **Transpose Shift**     | `MIDI_TRANSPOSE_DOWN` / `MIDI_TRANSPOSE_UP`                                         | —                         | Lowers or raises transposition by 1 semitone            |
| **Velocity Selection**  | `MIDI_VELOCITY_0` .. `MIDI_VELOCITY_10`                                             | 0 to 127                  | Sets the key velocity (strength)                        |
| **Velocity Shift**      | `MIDI_VELOCITY_DOWN` / `MIDI_VELOCITY_UP`                                           | —                         | Decreases or increases velocity                         |
| **Channel Selection**   | `MIDI_CHANNEL_1` .. `MIDI_CHANNEL_16`                                               | channels 1–16             | Sets the active MIDI channel                            |
| **Channel Shift**       | `MIDI_CHANNEL_DOWN` / `MIDI_CHANNEL_UP`                                             | —                         | Decreases or increases active channel number            |
| **Pitch Bend**          | `MIDI_PITCH_BEND_DOWN` / `MIDI_PITCH_BEND_UP`                                       | —                         | Modulates pitch downwards or upwards                    |
| **CC Effects**          | `MIDI_SUSTAIN` / `MIDI_PORTAMENTO` / `MIDI_SOSTENUTO` / `MIDI_SOFT` / `MIDI_LEGATO` | —                         | Standard MIDI CC effect controls (pedals, legato, etc.) |
| **Panic**               | `MIDI_ALL_NOTES_OFF`                                                                | —                         | Turns off all currently sounding notes                  |
| **CC Increment (Vial)** | `MIDI_CC_X_INC`                                                                     | X = 0 .. 15               | Increments absolute CC X value by 1 on press            |
| **CC Decrement (Vial)** | `MIDI_CC_X_DEC`                                                                     | X = 0 .. 15               | Decrements absolute CC X value by 1 on press            |
| **CC Toggle**           | `MIDI_CC_X_TOGGLE(cc)` / `MIDI_CC_X_TOGGLE`                                         | cc = 0 .. 15              | Toggles CC parameter `cc` between 127 and 0 on press    |
| **CC Static Trigger**   | `MIDI_CC_X_VAL_127(cc)` / `MIDI_CC_X_VAL_127`                                       | cc = 0 .. 15              | Sends 127 on press, and 0 on release                    |
| **Static CC (Key)**     | `MIDI_CC(cc, value)`                                                                | cc: 0..127, value: 0..127 | Sends `value` on press, and `0` on release              |
