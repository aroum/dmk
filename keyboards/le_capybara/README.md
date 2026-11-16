**Language / Язык:** [English](README.md) | [Русский](README_ru.md)

---

# Le Capybara DMK Firmware

Alternative high-performance electro-capacitive (EC-compatible) firmware for the **Le Capybara** keyboard designed by sporkus. Powered by the DMK engine and FreeRTOS, running natively on the **STM32F072C8T6** MCU.

**Documentation:** [Install](docs/en/install.md) • [VIA & Configuration](docs/en/via.md)

---

## Features

- **Electro-Capacitive (EC) Matrix Engine:**
  - High-frequency 1000 Hz continuous matrix scan rate.
  - Per-key baseline tracking and drift compensation.
  - Runtime capacitive calibration saved directly to MCU Flash memory (Page 62).
  - Configurable actuation point and release hysteresis.
  - **Rapid Trigger:** dynamic press and release sensitivity down to 0.1 mm increments.
- **Full Rotary Encoder Support:**
  - Dedicated hardware quadrature scanning on PB4 / PB5.
  - Discrete integrated push switch on PB3 (`KNOB_SW`).
- **Flexible Thumb Cluster Layouts:**
  - **Classique (Default):** 4-key ergonomic thumb cluster.
  - **36-Key Split:** 6-key split thumb cluster option selectable via VIA layout options.
- **Dual VIA / Vial Support:**
  - **VIA v3:** Custom interactive controls menu with Calibration, Rapid Trigger, Matrix Monitor, and Scan Rate.
  - **Vial:** Lightweight graphical configuration.
  - Seamless build-time switching via `-DVIAL_ENABLE=ON/OFF`.

---

## Hardware Specifications

| Component | Specification |
| :--- | :--- |
| **MCU** | STMicroelectronics STM32F072C8T6 (ARM Cortex-M0, 48 MHz) |
| **Flash / RAM** | 128 KB Flash / 16 KB SRAM |
| **Switch Technology** | Electro-Capacitive (Conical Spring EC-style) |
| **Key Count** | 37–40 keys (depending on thumb cluster layout) + Encoder |
| **ADC Channel** | 12-bit ADC on PA2 |
| **Discharge Pin** | PA1 (Drain / Discharge) |
| **Mux Select Pins** | PA3, PA4, PA5 (74HC4051 8:1 Multiplexer) |
| **Strobe Pins** | PB0, PB1, PB2, PB10, PB11 |
| **Rotary Encoder** | PB4 (A), PB5 (B), PB3 (Switch) |
| **Bootloader** | Built-in ST System ROM DFU Bootloader (`0x1FFFC800`) |

---

## Quick Start

### 1. Build Firmware

#### Using `build_all.sh` (Linux / macOS) or `build_all.bat` / `build_all.ps1` (Windows):

```bash
# Linux / macOS (build with clean configuration)
./build_all.sh -b le_capybara -c

# Windows (Command Prompt / Batch)
build_all.bat -b le_capybara -c

# Windows (PowerShell)
.\build_all.ps1 -Keyboard le_capybara -Clean
```

#### Using CMake directly:

```bash
# Build with Vial support (default)
cmake -B build/le_capybara -DKEYBOARD=le_capybara
cmake --build build/le_capybara

# Build with VIA v3 custom controls
cmake -B build/le_capybara_via -DKEYBOARD=le_capybara -DVIAL_ENABLE=OFF
cmake --build build/le_capybara_via
```

### 2. Flash Firmware

Put the keyboard into DFU bootloader mode (press the hardware `BOOT0` button while plugging in or resetting) and run:

```bash
# Using dfu-util directly:
dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D build/le_capybara/dmk_le_capybara_stm32f072.bin

# Or using the CMake flash target:
cmake --build build/le_capybara --target flash
```

For detailed installation instructions, see [Install Guide](docs/en/install.md).

---

## Directory Structure

- `config.h` — Matrix geometry, pin assignments, and firmware feature switches.
- `capybara_matrix.c` / `capybara_matrix.h` — Analog multiplexer scanning and discharge sequencing.
- `matrix.c` — Matrix scan task and event generation.
- `ec_settings.c` / `ec_settings.h` — Persistent calibration, thresholds, and Flash EEPROM emulation.
- `rapid_trigger.c` / `rapid_trigger.h` — Rapid Trigger algorithm implementation.
- `hhkb_via_controls.c` — VIA custom UI controls handler.
- `via.json` — VIA v3 definition with layout choices and custom menus.
- `vial.json` — Vial GUI definition.
- `docs/` — User documentation and guides.
