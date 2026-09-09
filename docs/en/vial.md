🌐 **Language / Язык:** [English](vial.md) | [Русский](../ru/vial.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md) • [Modules](modules.md) • [MIDI](midi.md)

---

# Vial / VIA Integration Guide

Vial is an open-source, cross-platform GUI application for configuring keyboard layouts, macros, and settings in real-time. This guide explains how to build and configure `dmk` with Vial support.

---

## 🛠️ Enabling Vial Support

Vial support can be enabled in two ways:

1. **Via Keyboard Configuration File**: Add `#define VIAL` in your keyboard's `config.h` (e.g., in [keyboards/corne/config.h](../../keyboards/corne/config.h)). This is the preferred method because it preserves the configuration directly in your layout source code.
2. **Via CMake Build Option**: Pass `-DVIAL=ON` when configuring the build.

When Vial support is enabled, a secondary **Raw HID** USB interface is initialized to facilitate communication between the keyboard and the Vial GUI.

### Build Commands Examples

- **Build using settings in `config.h`** (if `#define VIAL` is present):

  ```bash
  ./build_all.sh -b corne --mcu rp2040 -c --uf2
  ```

- **Build specifying the CMake flag explicitly**:

  ```bash
  cmake -DVIAL=ON -DKEYBOARD=corne -DMCU=rp2040 build && cmake --build build
  ```

---

## 🗺️ How it Works

1. **Vial Layout Definition (`vial.json`)**:
   Each keyboard that supports Vial must define its physical layouts, vendor ID, and product ID in a `vial.json` file inside the keyboard directory (e.g., `keyboards/corne/vial.json`).

2. **Automatic Compression**:
   The CMake build system automatically invokes a Python tool (`tools/vial/compress.py`) to compress `vial.json` using LZMA (`FORMAT_ALONE` container format). This generates a C header `vial_generated.h` in the keyboard directory containing:
   - `vial_keyboard_def[]`: The compressed JSON layout array.
   - `vial_keyboard_id[]`: A stable 8-byte ID hashed from the layout file.

3. **Dynamic Keymap & EEPROM Storage**:
   On startup, the firmware initializes the keymap and loads saved configurations from EEPROM if available.
   - All matrix key resolution logic reads from the writeable dynamic keymap.
   - Vial can instantly edit any keycode on-the-fly.
   - Configuration changes (keymap, macros, and RGB settings) are saved persistently to EEPROM (currently supported on Milandr MCUs) when you trigger a save in the Vial GUI.

---

## 💡 RGB Lighting Integration

`dmk` supports WS2812 RGB underglow/backlight control directly through the Vial GUI:

- **Supported MCUs**: RP2040 (using PIO) and nRF52840 (using SPIM).
- **Features**: Toggle lighting, adjust brightness, change solid color (Hue & Saturation), and save/load settings to/from EEPROM.
- **Configuring RGB**:
  - Define `RGB_NUM` (number of LEDs) and `RGB_PIN` in your keyboard's `config.h` (e.g., in [keyboards/pncateho/config.h](../../keyboards/pncateho/config.h)).
  - Optional: Define `RGB_MAP` to map logical indexes to physical LED indexes.

### 🎭 Supported RGB Effects

While the Vial interface shows a full list of QMK animations, `dmk` implements a optimized subset of effects to save flash space. Only the first index of each animation type is supported:

1. **Solid Color** (`RGBLIGHT_MODE_STATIC_LIGHT`)
2. **Breathing 1** (`RGBLIGHT_MODE_BREATHING`)
3. **Rainbow Mood 1** (`RGBLIGHT_MODE_RAINBOW_MOOD`)
4. **Rainbow Swirl 1** (`RGBLIGHT_MODE_RAINBOW_SWIRL`)
5. **Snake 1** (`RGBLIGHT_MODE_SNAKE`)
6. **Knight 1** (`RGBLIGHT_MODE_KNIGHT`)
7. **Christmas** (`RGBLIGHT_MODE_CHRISTMAS`)
8. **Gradient 1** (`RGBLIGHT_MODE_STATIC_GRADIENT`)

> [!NOTE]
> Other variations shown in Vial (such as _Breathing 2–4_, _Rainbow Mood 2–3_, _Rainbow Swirl 2–6_, _Snake 2–6_, _Knight 2–3_, _Gradient 2–10_, _RGB Test_, and _Alternating_) are **not implemented** and will not animate.

---

## 📊 Vial Features Support Matrix

Here is the current implementation status of various Vial / VIA features in `dmk`:

| Vial / VIA Feature         | Status in dmk   | Description / Limitations                                                                                        |
| -------------------------- | --------------- | ---------------------------------------------------------------------------------------------------------------- |
| **Dynamic Keymap**         | 🟢 Supported     | Configurable in real-time. Changes persist in EEPROM (currently supported on Milandr MCUs) when saved.           |
| **Layers**                 | 🟢 Supported     | Up to 16 independent layers are supported.                                                                       |
| **Matrix Tester**          | 🟢 Supported     | Works out of the box using the standard VIA/Vial protocol.                                                       |
| **RGB Lighting**           | 🟢 Supported     | Supports toggle, brightness, animation speed, and 8 standard QMK effects/animations.                             |
| **Security (Lock/Unlock)** | 🟡 Simplified    | Keymap locking features are bypassed (the keyboard is always unlocked).                                          |
| **Macros**                 | 🟢 Supported     | Dynamic Macro configuration (up to 16 macros, 2048 bytes buffer) supported via the Vial GUI.                     |
| **Mousekeys**              | 🟢 Supported     | Mouse buttons, 2D cursor motion (with smooth acceleration physics), and wheel scrolling (vertical & horizontal pan) assignable in Vial GUI (v6 protocol, `0xED01`–`0xED13` mapped to `dmk`). |
| **Encoders**               | 🟢 Supported     | Rotary encoders (clockwise & counter-clockwise actions per layer) configurable in Vial GUI via `vial_get_encoder` and `vial_set_encoder` with EEPROM persistence. |
| **Tap Dance**              | ❌ Not Supported | Dynamic Tap Dance configuration via the Vial GUI is disabled.                                                    |
| **Combos**                 | 🟢 Supported     | Dynamic Combo (chording) configuration (up to 8 combos, up to 4 keys per combo) supported via the Vial GUI.      |
| **Key Overrides**          | ❌ Not Supported | Key override features are disabled.                                                                              |

---

## 🖱️ Mousekeys & Movement Support

`dmk` implements full USB HID Mousekeys emulation integrated into the Vial protocol (protocol v6, keycodes `0xED01`–`0xED13`):

- **2D Cursor Motion**: Supports continuous, fluid directional movement (`KC_MS_UP`, `KC_MS_DOWN`, `KC_MS_LEFT`, `KC_MS_RIGHT`).
- **Smooth Acceleration Physics**: Instead of abrupt static steps, `dmk` utilizes dynamic acceleration math (`MOUSEKEY_TIME_TO_MAX`, `MOUSEKEY_MAX_SPEED`, and `MOUSEKEY_DECELERATION_TIME_MS`) to ramp cursor velocity progressively, providing pinpoint precision at first press and fast traversal over long holds.
- **Mouse Buttons**: Up to 5 discrete mouse buttons (`KC_MS_BTN1` through `KC_MS_BTN5`) for left, right, middle, back, and forward clicks.
- **Wheel Scrolling & Panning**: Supports vertical scrolling (`KC_MS_WH_UP`, `KC_MS_WH_DOWN`) and horizontal panning (`KC_MS_WH_LEFT`, `KC_MS_WH_RIGHT`).
- **Dynamic Acceleration Profiles**: On-the-fly acceleration switching via `KC_MS_ACCEL0`, `KC_MS_ACCEL1`, and `KC_MS_ACCEL2`.

---

## 🔄 Rotary Encoders Support

Physical rotary encoders are first-class citizens in `dmk` and can be remapped directly in Vial:

- **Per-Layer Actions**: Each encoder supports independent actions for clockwise (CW) and counter-clockwise (CCW) rotations on every layer.
- **Vial GUI Customization**: Configurable in real time via Vial's encoder interface (`vial_get_encoder` and `vial_set_encoder` commands) or VIA (`0x14`/`0x15`).
- **Synthetic Pulse Generation**: Each detent produces a debounced 20ms pulse with immediate release on rapid subsequent rotations, preventing key stickiness during fast continuous spinning.
- **Layer Transparency**: If an encoder key is unassigned or set to transparent (`K_TRNS` / `0x0001`) on an upper active layer, `dmk` falls through down the active layer stack until an assigned action is resolved.
- **Persistence**: Remapped encoder actions are written to EEPROM or onboard flash when saving in Vial.

---

## 🔒 Security & Matrix Tester

- **Vial Lock**: The lock feature is intentionally bypassed (`VIAL_GET_UNLOCK_STATUS` always reports unlocked), making real-time layout adjustments frictionless.
- **Matrix Tester (macOS vs Windows)**:
  - **Windows**: Matrix Tester works immediately out-of-the-box via `hid.dll`.
  - **macOS Considerations**:
    - `vial-gui` calls `grabKeyboard()` to prevent keystrokes from typing into other windows while testing. Under macOS (Catalina through Sequoia), global keyboard grabbing requires explicit permission under **System Settings → Privacy & Security → Input Monitoring** (or **Accessibility**). Without this permission, the Qt application may fail to intercept inputs or stall.
    - `dmk` incorporates response queuing for the Raw HID interface so that the high polling rate of Matrix Tester (50–100 Hz) does not drop packets even if the USB IN endpoint is momentarily busy.
    - If desktop `vial-gui` encounters permission limitations on macOS, the [Vial WebHID Client](https://vial.rocks/) in Chrome or Edge can be used as an alternative without requiring OS-level keyboard grab permissions.

---

## 🧪 Testing

1. Connect the keyboard to your PC.
2. Go to the [Vial Web Client](https://vial.rocks/) or download the desktop app.
3. Click "Start Vial" and select the detected keyboard.
4. Modify any key or adjust RGB settings in the GUI and check that it instantly registers on your keyboard!
