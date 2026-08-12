🌐 **Language / Язык:** [English](config.md) | [Русский](../ru/config.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md)

---

# Keyboard Configuration (`config.h`)

All hardware settings, keymaps, and macros are defined in `config.h` (e.g., `keyboards/corne/config.h`).

---

## Custom USB Config

Appending MCU name to artifact:

```c
#define VID      PA0
#define PID      P0_00
#define USB_NAME P0_00
```

## Multi-Platform Support

Configurations can dynamically target different microcontrollers using preprocessor checks (`#if defined(MCU_...)`). Supported MCUs include `MCU_milandr`, `MCU_rp2040`, `MCU_rp2350`, `MCU_nrf52840`, and `MCU_baikal`.

```c
#if defined(MCU_milandr)
    #define SERIAL_PIN      PA0
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
    #define SERIAL_PIN      GPIO0
#elif defined(MCU_nrf52840)
    #define SERIAL_PIN      P0_00
#elif defined(MCU_baikal)
    #define SERIAL_PIN      GPIO0
#endif
```

---

## Matrix Configuration

Keyboards define how switches are scanned using the `MATRIX_TYPE` and pin macros.

### Matrix Scan Types

- `MATRIX_TYPE_ROW2COL`: Rows are inputs, columns are outputs.
- `MATRIX_TYPE_COL2ROW`: Columns are inputs, rows are outputs.
- `MATRIX_TYPE_DIRECT`: Each key is connected directly to its own GPIO pin.
- `MATRIX_ACTIVE_LOW`: Define this macro to invert polarity for active‑low matrices (pull‑up on inputs, idle low on outputs).

### ROW2COL & COL2ROW Matrix

Specify row and column pins using `ROW_PINS` and `COL_PINS`.

```c
#include "pin_defs.h"

#define MATRIX_TYPE_ROW2COL
#define ROW_PINS  { GPIO0, GPIO1, GPIO2, GPIO3 }
#define COL_PINS  { GPIO4, GPIO5, GPIO6, GPIO7, GPIO8 }
```

### Direct Pins Matrix

If keys are wired directly to GPIOs, define `DIRECT_PINS` instead:

```c
#include "pin_defs.h"

#define MATRIX_TYPE_DIRECT
#define DIRECT_PINS  { GPIO0, GPIO1, GPIO2, GPIO3, GPIO4 }
```

### Debounce

Set debounce delay (in milliseconds) using the `DEBOUNCE` macro (default: `10`):

```c
#define DEBOUNCE 10
```

---

## Physical Layout and Coordinates

Physical layouts map the electrical matrix coordinate scan to the logical key position.
This mapping is specified via the `LAYOUT` coordinate map using `{ROW, COL}` pairs:

```c
#define NUM_ROWS 4
#define NUM_COLS 10

// {ROW, COL} coordinate mapping
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, \
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, \
                            {3, 3}, {3, 4}, {3, 5}, {3, 6}                  \
}
#define LAYOUT_DEFAULT LAYOUT
```

### Direct Layout Coordinate Rule

For direct layouts (`MATRIX_TYPE_DIRECT`), there is no matrix grid. Therefore, the coordinate mapped `ROW` must **always be 0**, and the `COL` matches the index of the pin:

```c
// Example for direct connection layout mapping
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4} \
}
```

### Visual Key Positioning in the Editor (`LAYOUT_EDITOR`)

If the electrical matrix schema (`LAYOUT`) differs from the physical layout of the keys on the keyboard (for example, the matrix is one-dimensional, but the keys are arranged in two rows or with an offset), you can define an optional `LAYOUT_EDITOR` macro.

It specifies the virtual `{ROW, COL}` coordinates on the editor screen for each key, in the exact same order as they are listed in `LAYOUT`:

```c
// Example for nizkoteno (10 keys): in the matrix they are in a single row,
// but on the editor screen we want to arrange them in two rows of 5 keys
#define LAYOUT_EDITOR { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}  \
}
```

---

## Macros Configuration

Macros are structured actions executed upon a keypress (mapped keycodes `0xC0` to `0xDF`). Macros are declared under the `DEFINE_KEYMAP` block:

```c
#ifdef DEFINE_KEYMAP
// Define macros steps (actions: KEY_DOWN, KEY_UP, DELAY)
const Macro keyboard_macros[] = {
    [0] = { // Macro index 0
        .steps = (MacroStep[]) {
            { KEY_DOWN, K_H },
            { KEY_UP,   K_H },
            { KEY_DOWN, K_E },
            { KEY_UP,   K_E },
            { KEY_DOWN, K_L },
            { KEY_UP,   K_L },
            { KEY_DOWN, K_L },
            { KEY_UP,   K_L },
            { KEY_DOWN, K_O },
            { KEY_UP,   K_O },
            { DELAY,    100 }, // Wait 100ms
        },
        .count = 11,
        .layer_trigger = MACRO_NO_LAYER, // Regular macro, triggered by M(0) keycode
    }
};

const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
#endif
```

### Naming Macros with `#define`

Raw numeric codes `M(0)`, `M(1)`, etc. are hard to read in keymaps. A good practice is to declare named aliases via `#define` — this makes the keymap self-documenting:

```c
#ifdef DEFINE_KEYMAP

// Named aliases for macros
#define MC_NUMDOT M(0) // Alt+46 — numpad period character
#define MC_GRTSGN M(1) // Alt+62 — greater-than sign «>»
#define MC_SNIP   M(2) // Win+Shift+S — region screenshot

const MacroStep MACRO_NUMDOT[] = {M_DN(K_LALT), M_D(40), M_DN(K_KP4), M_UP(K_KP4),
                                  M_D(40),       M_DN(K_KP6), M_UP(K_KP6), M_UP(K_LALT)};

const MacroStep MACRO_GRTSGN[] = {M_DN(K_LALT), M_D(40), M_DN(K_KP6), M_UP(K_KP6),
                                  M_D(40),       M_DN(K_KP2), M_UP(K_KP2), M_UP(K_LALT)};

const MacroStep MACRO_SNIP[] = {M_DN(K_LGUI), M_DN(K_LSFT), M_DN(K_S),
                                M_UP(K_S),    M_UP(K_LSFT), M_UP(K_LGUI)};

const Macro keyboard_macros[] = {
    [0] = {MACRO_NUMDOT, sizeof(MACRO_NUMDOT) / sizeof(MACRO_NUMDOT[0]), MACRO_NO_LAYER},
    [1] = {MACRO_GRTSGN, sizeof(MACRO_GRTSGN) / sizeof(MACRO_GRTSGN[0]), MACRO_NO_LAYER},
    [2] = {MACRO_SNIP,   sizeof(MACRO_SNIP)   / sizeof(MACRO_SNIP[0]),   MACRO_NO_LAYER},
};

const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);

// Keymap now uses readable names instead of M(0), M(1), ...:
const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = { ..., MC_NUMDOT, MC_GRTSGN, MC_SNIP, ... },
};
#endif
```

---

## Layer Trigger Macros

The `layer_trigger` field in the `Macro` struct lets you bind a macro to a specific layer activation. When the layer becomes the highest active layer, all macros with a matching `layer_trigger` are executed **automatically — without any keypress**.

- `MACRO_NO_LAYER` (value `0xFF`) — regular macro, fires only via the `M(n)` keycode.
- Any other value — the layer index that automatically fires this macro on activation.

```c
#ifdef DEFINE_KEYMAP
// Steps: press Win+1
const MacroStep STEPS_SWITCH_LAYER1[] = {
    { KEY_DOWN, K_LGUI },
    { KEY_DOWN, K_1    },
    { KEY_UP,   K_1    },
    { KEY_UP,   K_LGUI },
};

// Steps: press Win+2
const MacroStep STEPS_SWITCH_LAYER2[] = {
    { KEY_DOWN, K_LGUI },
    { KEY_DOWN, K_2    },
    { KEY_UP,   K_2    },
    { KEY_UP,   K_LGUI },
};

const Macro keyboard_macros[] = {
    // Macro 0: regular, triggered by M(0) keycode
    [0] = {
        .steps         = STEPS_SWITCH_LAYER1,
        .count         = 4,
        .layer_trigger = MACRO_NO_LAYER,
    },
    // Macro 1: fires automatically when layer 1 (LWR) is activated
    [1] = {
        .steps         = STEPS_SWITCH_LAYER1,
        .count         = 4,
        .layer_trigger = 1, // LWR
    },
    // Macro 2: fires automatically when layer 2 (RSE) is activated
    [2] = {
        .steps         = STEPS_SWITCH_LAYER2,
        .count         = 4,
        .layer_trigger = 2, // RSE
    },
};

const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
#endif
```

> [!NOTE]
> Multiple macros can share the same `layer_trigger` value. All matching macros are executed in array order when the layer activates.

> [!NOTE]
> Layer trigger macros are **not supported** in Vial dynamic keymap mode. They work only in static `config.h` configurations.

---

## Split Keyboard Settings

For split keyboards (such as Corne, Jianovka, or Nizkoteno Split), communication between halves is handled over a unified serial protocol driver architecture.

### Unified Split Connection Types & Platform Matrix

```c
#define SPLIT_CONNECTION_TYPE SOFT         // SOFT | HW_HALF_DUPLEX | HW_FULL_DUPLEX
#define SPLIT_TX_PIN          GPIO0        // Single data pin (SOFT/HW_HALF_DUPLEX) or TX pin (HW_FULL_DUPLEX)
#define SPLIT_RX_PIN          GPIO1        // RX pin (HW_FULL_DUPLEX)
```

| Platform                 | Mode / Driver (`SPLIT_CONNECTION_TYPE`) | Wire Setup                | Configuration                                                | Notes                                                             |
| :----------------------- | :-------------------------------------- | :------------------------ | :----------------------------------------------------------- | :---------------------------------------------------------------- |
| **RP2040 / RP2350**      | `HW_HALF_DUPLEX`                        | Single-Wire (1 Pin)       | `#define SPLIT_TX_PIN GPIO0`                                 | Hardware PIO state machine (`split.pio`). Non-blocking, any GPIO. |
| **RP2040 / RP2350**      | `SOFT`                                  | Single-Wire (1 Pin)       | `#define SPLIT_TX_PIN GPIO0`                                 | Software bit-bang on any GPIO.                                    |
| **Milandr (MDR32F9Q2I)** | `HW_FULL_DUPLEX`                        | Dual-Wire (2 Pins: TX+RX) | `#define SPLIT_TX_PIN PF1`<br>`#define SPLIT_RX_PIN PF0`     | Hardware UART (`MDR_UART2` on `MDR_PORTF`).                       |
| **Milandr (MDR32F9Q2I)** | `SOFT`                                  | Single-Wire (1 Pin)       | `#define SPLIT_TX_PIN PF0`                                   | Software bit-bang on any GPIO.                                    |
| **nRF52840**             | `HW_HALF_DUPLEX`                        | Single-Wire (1 Pin)       | `#define SPLIT_TX_PIN P0_00`                                 | Hardware UARTE in Open-Drain (`S0D1`) mode with single pin.       |
| **nRF52840**             | `HW_FULL_DUPLEX`                        | Dual-Wire (2 Pins: TX+RX) | `#define SPLIT_TX_PIN P0_00`<br>`#define SPLIT_RX_PIN P0_01` | Hardware UARTE dual-wire TX/RX mode.                              |
| **nRF52840**             | `SOFT`                                  | Single-Wire (1 Pin)       | `#define SPLIT_TX_PIN P0_00`                                 | Software bit-bang on any GPIO.                                    |
| **Baikal (BE-T1000)**    | `HW_HALF_DUPLEX`                        | Single-Wire (1 Pin)       | `#define SPLIT_TX_PIN PC6`                                   | Hardware RS-485 Half-Duplex (UART6 `PC6` or UART7 `PC8`).         |
| **Baikal (BE-T1000)**    | `HW_FULL_DUPLEX`                        | Dual-Wire (2 Pins: TX+RX) | `#define SPLIT_TX_PIN PA2`<br>`#define SPLIT_RX_PIN PA3`     | Hardware UART (UART0..UART7).                                     |
| **Baikal (BE-T1000)**    | `SOFT`                                  | Single-Wire (1 Pin)       | `#define SPLIT_TX_PIN PA2`                                   | Software bit-bang on any GPIO.                                    |

> [!NOTE]
> **Backwards Compatibility**: Defining `#define SERIAL_PIN GPIO0` automatically maps to `SPLIT_TX_PIN` and `SPLIT_RX_PIN`. If `SPLIT_CONNECTION_TYPE` is omitted, the firmware defaults to the safest mode for the target platform.

---

### Geometry & Side Detection Configuration

```c
#define MASTER_SIDE LEFT      // Options: LEFT, RIGHT, or AUTO
#define NUM_ROWS_SPLIT 4      // Rows on each half
#define NUM_COLS_SPLIT 5      // Columns on each half
#define SPLIT_COL_OFFSET 5    // Logical column offset for the right half
```

- **Dynamic Role Detection**: When using `AUTO` (or building without `-d`), the half connected directly via USB cable to the host system is automatically designated as `MASTER_SIDE` via `tud_mounted()` USB detection.

---

### Configuration Examples Per Platform

#### 1. RP2040 / RP2350 (PIO Hardware Single-Wire)

```c
#define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX
#define SPLIT_TX_PIN          GPIO0        // Any GPIO pin (GPIO0..GPIO29)
#define SPLIT_HARDWARE_OPTION 2            // 1 = 115200 baud, 2 = 19200 baud (default)
```

#### 2. Milandr Hardware UART (Full-Duplex / 2-Wire TX+RX)

```c
#define SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX
#define SPLIT_TX_PIN          PF1          // TX pin (automatically resolves MDR_PORTF and MDR_UART2)
#define SPLIT_RX_PIN          PF0          // RX pin
#define SPLIT_HARDWARE_OPTION 2            // 1 = 115200 baud, 2 = 19200 baud
```

#### 3. Software Bit-Bang (Universal for All MCUs: Milandr, nRF52840, Baikal, RP2040)

```c
#define SPLIT_CONNECTION_TYPE SOFT
#define SPLIT_TX_PIN          PF0          // Single GPIO pin on TRRS cable
#define SPLIT_HARDWARE_OPTION 2            // 1 = 115200 baud, 2 = 19200 baud
```

---

### Hardware UART Supported Pins Per Platform

#### 1. nRF52840 (Nordic)

- **Pin Crossbar:** Flexible pin routing allows assigning hardware UARTE (`NRF_UARTE0`) to **ANY** available GPIO pin (`P0_00`..`P0_31`, `P1_00`..`P1_15`).
- **Half-Duplex Single-Wire Mode (`HW_HALF_DUPLEX`):** `SPLIT_TX_PIN` maps both TXD and RXD to a single pin in Open-Drain (`S0D1`) mode with internal pull-up.
  - _Recommended pins:_ `P0_06`, `P0_08`, `P0_17`, `P0_20`, `P1_09`.
- **Full-Duplex Dual-Wire Mode (`HW_FULL_DUPLEX`):**
  - _Example:_ `#define SPLIT_TX_PIN P0_06`, `#define SPLIT_RX_PIN P0_08`.
- _Constraint:_ Avoid pins reserved for NFC (`P0.09`, `P0.10`) or 32kHz Low-Frequency Crystal (`P0.00`, `P0.01`) if active.

#### 2. RP2040 / RP2350 (Raspberry Pi)

- **PIO Half-Duplex Mode (`HW_HALF_DUPLEX`):** Supported on **ANY** GPIO pin (`GPIO0`..`GPIO29`).
- **Hardware Peripheral UART Pins (`HW_FULL_DUPLEX`):**
  - **RP2040 (All pin pairs via Function F2):**
    - **UART0 TX:** `GPIO0`, `GPIO12`, `GPIO16`, `GPIO28`
    - **UART0 RX:** `GPIO1`, `GPIO13`, `GPIO17`, `GPIO29`
    - **UART1 TX:** `GPIO4`, `GPIO8`, `GPIO20`, `GPIO24`
    - **UART1 RX:** `GPIO5`, `GPIO9`, `GPIO21`, `GPIO25`
  - **RP2350 (Additional pin pairs via Function F11):**
    - **UART0 TX (F11):** `GPIO2`, `GPIO14`, `GPIO18` (plus `GPIO30`, `GPIO34`, `GPIO46` on QFN-80)
    - **UART0 RX (F11):** `GPIO3`, `GPIO15`, `GPIO19` (plus `GPIO31`, `GPIO35`, `GPIO47` on QFN-80)
    - **UART1 TX (F11):** `GPIO6`, `GPIO10`, `GPIO22`, `GPIO26` (plus `GPIO38`, `GPIO42` on QFN-80)
    - **UART1 RX (F11):** `GPIO7`, `GPIO11`, `GPIO23`, `GPIO27` (plus `GPIO39`, `GPIO43` on QFN-80)

#### 3. Milandr MDR32F9Q2I (K1986BE92FI)

- **Pin Format:** Pins are passed in the same unified format as the matrix — `PF0`, `PF1`, `PA6`, `PA7`, etc.
- **Automatic Peripheral Resolution:** The firmware automatically resolves the GPIO port (`MDR_PORTF`, `MDR_PORTA`) and UART peripheral (`MDR_UART2` for `PF0`/`PF1`, `MDR_UART1` for `PA6`/`PA7`).
  - **`MDR_UART2` (Port F):** `SPLIT_TX_PIN PF1`, `SPLIT_RX_PIN PF0`
  - **`MDR_UART1` (Port A):** `SPLIT_TX_PIN PA7`, `SPLIT_RX_PIN PA6`

#### 4. Baikal (BE-T1000)

- **Software Bit-Bang Mode (`SOFT`):** Any general-purpose GPIO pin (`PA2`, `PA3`, `PB2`, `PC6`, etc.).
- **Hardware Half-Duplex Mode (`HW_HALF_DUPLEX`):** Hardware RS-485 mode with directional control (`DE`/`RE`). Supported **only on `UART6` (`PC6`) and `UART7` (`PC8`)**.
- **Hardware Full-Duplex Mode (`HW_FULL_DUPLEX`):**
  - **`UART0` (Port A):**
    - **TX Pins:** `PA2`, `PA6`, `PA8`
    - **RX Pins:** `PA3`, `PA7`, `PA9`
  - **`UART1` (Port A):**
    - **TX Pins:** `PA4`, `PA10`, `PA14`
    - **RX Pins:** `PA5`, `PA11`, `PA15`
  - **`UART2` (Port A):**
    - **TX Pin:** `PA12` | **RX Pin:** `PA13`
  - **`UART3` (Port B):**
    - **TX Pins:** `PB2`, `PB6`, `PB8`
    - **RX Pins:** `PB3`, `PB7`, `PB9`
  - **`UART4` (Port B):**
    - **TX Pins:** `PB4`, `PB10`, `PB14`
    - **RX Pins:** `PB5`, `PB11`, `PB15`
  - **`UART5` (Port B):**
    - **TX Pin:** `PB12` | **RX Pin:** `PB13`
  - **`UART6` (Port C):**
    - **TX Pin:** `PC6` | **RX Pin:** `PC7`
  - **`UART7` (Port C):**
    - **TX Pin:** `PC8` | **RX Pin:** `PC9`

---

## LEDs Indicator Configuration

`dmk-firmware` supports mapping status and diagnostics LEDs.

```c
#define LED_PINS           { PA4, PA4, PA5, PA6, PA7, PA8 }
#define LED_DEBUG           0 // Index in LED_PINS for diagnostic heartbeat LED
#define LED_HID_NUM_LOCK    1 // Index in LED_PINS for Num Lock LED
#define LED_HID_CAPS_LOCK   2 // Index in LED_PINS for Caps Lock LED
#define LED_HID_SCROLL_LOCK 3 // Index in LED_PINS for Scroll Lock LED
#define LED_HID_COMPOSE     4 // Index in LED_PINS for Compose LED
#define LED_HID_KANA        5 // Index in LED_PINS for Kana LED
```

- **Diagnostic Heartbeat (`LED_DEBUG`)**:
  - Blinks slowly (0.5 Hz) when USB is active but **not** mounted.
  - Blinks rapidly (2 Hz) when USB is **successfully mounted** (configured by PC).
  - Flashes briefly (25 ms) when a keypress event is sent.
- **Host Status Mappings (`LED_HID_*`)**: Maps keyboard lock states directly to GPIO outputs on the board.

---

## RGB WS2812 Backlight

WS2812 RGB LED strips can be controlled by defining the count, connection pin, mapping layout, and custom themes:

```c
#define RGB_NUM     10
#define RGB_PIN     GPIO25
#define RGB_MAP     { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }

#define RGB_THEME_DEFAULT { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF }
#define RGB_THEME_GAMING  { 0xFF4500, 0xFF8C00, 0xFFD700, 0xADFF2F }
#define RGB_THEMES        { RGB_THEME_DEFAULT, RGB_THEME_GAMING }
#define RGB_LIMIT_VAL     128 // Optional: Limit maximum brightness (0-255) to reduce current draw
```

- `RGB_LIMIT_VAL`: Optional maximum brightness threshold (values `0` to `255`, default: `255`). Restricts the upper bound of the LED current draw by hardware-clamping the brightness parameter inside the engine.

### Supported Effects and Animations

`dmk-firmware` supports QMK-style RGB lighting effects:

- **Mode 1 (Static Light)**: Solid color.
- **Mode 2 (Breathing)**: Smooth sinusoidal value modulation over time.
- **Mode 6 (Rainbow Mood)**: Hue shifts uniformly across all LEDs over time.
- **Mode 9 (Rainbow Swirl)**: Gradient of colors cycling/shifting across the LED strip.
- **Mode 15 (Snake)**: A segment of active LEDs shifting along the strip.
- **Mode 18 (Knight Rider)**: Scanner pattern moving back and forth.
- **Mode 21 (Christmas)**: Alternating red/green segments switching back and forth.
- **Mode 22 (Static Gradient)**: A static color wheel gradient across the length of the keyboard.
- **Mode 100 (Theme Mode)**: Custom static layouts defined under `RGB_THEMES` (only available if `RGB_THEMES` is defined).

### Controls

All settings (Mode, Hue, Saturation, Brightness, and Animation Speed) are fully configurable in real-time using **Vial GUI** (standard QMK RGBLIGHT control protocol).

Additionally, physical keys can be bound to:

- `K_RGB_TOGG`: Toggle RGB lighting on or off.
- `K_RGB_NEXT`: Cycle to the next animation mode (and next custom theme if in Theme Mode).
- `K_RGB_PREV`: Cycle to the previous animation mode (and previous custom theme).

---

## Extra Keys (Consumer/Media Control)

To enable support for multimedia keys (Consumer Page keys like play, pause, volume control, local browser, email, calculator, web search, bookmarks, etc.), define the `EXTRAKEY_ENABLE` flag:

```c
#define EXTRAKEY_ENABLE
```

This initializes a secondary HID interface on the device configured as a composite device, enabling the transmission of 16-bit Consumer Control usage codes.
