🌐 **Language / Язык:** [English](vial.md) | [Русский](../ru/vial.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md)

---

# Vial / VIA Integration Guide

Vial is an open-source, cross-platform GUI application for configuring keyboard layouts, macros, and settings in real-time. This guide explains how to build and configure `dmk-firmware` with Vial support.

---

## 🛠️ Enabling Vial Support

Vial support can be enabled in two ways:

1. **Via Keyboard Configuration File**: Add `#define VIAL` in your keyboard's `config.h` (e.g., in [keyboards/corne/config.h](file:///home/aroum/Documents/dmk-firmware/keyboards/corne/config.h)). This is the preferred method because it preserves the configuration directly in your layout source code.
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

`dmk-firmware` supports WS2812 RGB underglow/backlight control directly through the Vial GUI:

- **Supported MCUs**: RP2040 (using PIO) and nRF52840 (using SPIM).
- **Features**: Toggle lighting, adjust brightness, change solid color (Hue & Saturation), and save/load settings to/from EEPROM.
- **Configuring RGB**:
  - Define `RGB_NUM` (number of LEDs) and `RGB_PIN` in your keyboard's `config.h` (e.g., in [keyboards/pncateho/config.h](file:///Users/aroum/Documents/GitHub/dmk-firmware/keyboards/pncateho/config.h)).
  - Optional: Define `RGB_MAP` to map logical indexes to physical LED indexes.

### 🎭 Supported RGB Effects

While the Vial interface shows a full list of QMK animations, `dmk-firmware` implements a optimized subset of effects to save flash space. Only the first index of each animation type is supported:

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

Here is the current implementation status of various Vial / VIA features in `dmk-firmware`:

| Vial / VIA Feature         | Status in dmk-firmware | Description / Limitations                                                                                   |
| -------------------------- | ---------------------- | ----------------------------------------------------------------------------------------------------------- |
| **Dynamic Keymap**         | 🟢 Supported           | Configurable in real-time. Changes persist in EEPROM (currently supported on Milandr MCUs) when saved.      |
| **Layers**                 | 🟢 Supported           | Up to 16 independent layers are supported.                                                                  |
| **Matrix Tester**          | 🟢 Supported           | Works out of the box using the standard VIA/Vial protocol.                                                  |
| **RGB Lighting**           | 🟢 Supported           | Supports toggle, brightness, animation speed, and 8 standard QMK effects/animations.                        |
| **Security (Lock/Unlock)** | 🟡 Simplified          | Keymap locking features are bypassed (the keyboard is always unlocked).                                     |
| **Macros**                 | 🟢 Supported           | Dynamic Macro configuration (up to 16 macros, 2048 bytes buffer) supported via the Vial GUI.                |
| **Encoders**               | ❌ Not Supported       | Stubbed out in the protocol. Physical encoders are not processed by Vial.                                   |
| **Tap Dance**              | ❌ Not Supported       | Dynamic Tap Dance configuration via the Vial GUI is disabled.                                               |
| **Combos**                 | 🟢 Supported           | Dynamic Combo (chording) configuration (up to 8 combos, up to 4 keys per combo) supported via the Vial GUI. |
| **Key Overrides**          | ❌ Not Supported       | Key override features are disabled.                                                                         |

---

## 🔒 Security & Matrix Tester

For this implementation, the Vial lock feature is bypassed (always unlocked for simplicity). Keymap changes and RGB configurations are saved persistently to EEPROM on supported MCUs, so they persist across power cycles.

---

## 🧪 Testing

1. Connect the keyboard to your PC.
2. Go to the [Vial Web Client](https://vial.rocks/) or download the desktop app.
3. Click "Start Vial" and select the detected keyboard.
4. Modify any key or adjust RGB settings in the GUI and check that it instantly registers on your keyboard!
