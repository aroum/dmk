# DMK Firmware Documentation

Welcome to the **DMK** (Dmitry's Mechanical Keyboard firmware) documentation.

🌐 **Language / Язык:** [English](README.md) | [Русский](../ru/README.md)

---

## Documentation Topics

1. ⚙️ **[Configuration (`config.h`)](config.md)**\
   Keyboard configuration, USB VID/PID, cross-platform MCU support (RP2040, RP2350, Milandr, nRF52840, Baikal), RGB lighting, encoders, and status LEDs.

2. 🔌 **[Pins & Ports (`pins.md`)](pins.md)**\
   Pin assignment rules, port naming conventions for all supported microcontrollers, matrix and direct wiring.

3. ⌨️ **[Keycodes Reference (`keycodes.md`)](keycodes.md)**\
   Complete keycode reference: basic keys, modifiers, system keys, media keys, and advanced functions.

4. 🗺️ **[Keymaps & Layers (`keymap.md`)](keymap.md)**\
   Layer switching (MO, TO, TG, TT, OSL), macros, chords (combos), and Tap Dance.

5. 🎛️ **[Modules (`modules.md`)](modules.md)**\
   Supported hardware modules: RGB lighting (WS2812 / SK6812), displays (OLED SSD1306, LCD ST7789), and EC11 rotary encoders.

6. 🎹 **[MIDI (`midi.md`)](midi.md)**\
   USB MIDI device configuration: sending notes, control change (CC), Pitch Bend, and channel switching.

7. 🛠️ **[Building Firmware (`build.md`)](build.md)**\
   Compiling firmware via `build_all.sh` / `build_all.ps1`, toolchain setup (CMake, Ninja, GCC ARM), and GitHub Actions CI.

8. 🔧 **[Vial / VIA Integration (`vial.md`)](vial.md)**\
   Generating `vial.json`, dynamic EEPROM/Flash keymaps, security unlock, and layout definitions.

---

🧙‍♂️ **[DMK Config Wizard (Web Editor)](https://htmlpreview.github.io/?https://github.com/aroum/dmk/blob/main/editor/wizard.html)**
