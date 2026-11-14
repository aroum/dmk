# Supported HHKB Classic Models

DMK firmware runs natively on the factory STM32L072CZ microcontroller of the wired USB-C Happy Hacking Keyboard (HHKB Professional Classic series). No hardware modifications, soldering, or disassembly are required.

## Model Matrix

| Commercial Name | Model Number | Color | Keycaps | Switch Type | Container ID | Firmware Target |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Professional Classic (White)** | `PD-KB401W` | White / Gray | Printed | Classic Topre 45g | `AHHX01` | `classic` |
| **HHKB Professional Classic (Charcoal)** | `PD-KB401B` | Charcoal | Printed | Classic Topre 45g | `AHHX01` | `classic` |
| **HHKB Professional Classic (White, Blank)** | `PD-KB401WN` | White / Gray | Blank (無刻印) | Classic Topre 45g | `AHHX01` | `classic` |
| **HHKB Professional Classic (Charcoal, Blank)** | `PD-KB401BN` | Charcoal | Blank (無刻印) | Classic Topre 45g | `AHHX01` | `classic` |
| **HHKB Classic Type-S (White)** | `PD-KB401WSC` | White / Gray | Printed | Silent Type-S 45g | `AHHS01` | `type-s` |
| **HHKB Classic Type-S (Charcoal)** | `PD-KB401BSC` | Charcoal | Printed | Silent Type-S 45g | `AHHS01` | `type-s` |
| **HHKB Classic Type-S (Snow)** | `PD-KB401YSC` | Snow (All-White) | Printed / Blank | Silent Type-S 45g | `AHHS01` | `type-s` |

---

## Hardware Architecture & Compatibility Notes

1. **Architecture Uniformity:**
   All 7 models share the exact same hardware motherboard architecture:
   - Microcontroller: **STMicroelectronics STM32L072CZY6** (ARM Cortex-M0+, 192 KB Flash, 20 KB SRAM, 6 KB EEPROM).
   - Sensor Board: Capacitive Topre matrix (15 columns × 4 rows = 60 keys).
   - Factory Updater: Dual-bank flash layout (Bank 1 recovery bootloader + Bank 2 application firmware).

2. **Keycaps & Color Variants:**
   The suffix `N` in part numbers (e.g. `PD-KB401BN`, `PD-KB401WN`) denotes blank keycaps ("無刻印"). Keycap legends and case colors are purely aesthetic; the underlying electronics and firmware container format (`AHHX01`) are identical.

3. **Classic vs. Type-S:**
   - **Classic (`AHHX01`):** Standard Topre switches with full 4.0 mm travel. Uses the `classic` firmware build.
   - **Type-S (`AHHS01`):** Factory silenced Topre switches with internal dampening rings and ~3.8 mm travel. Uses the `type-s` firmware build (`HHKB_TYPE_S` compile flag).

---

## Japanese JIS Models (PD-KB421 Series) — Pending Matrix Verification

| Commercial Name | Model Number | Color | Layout | Status |
| :--- | :--- | :--- | :--- | :--- |
| **HHKB Pro Classic Type-S JIS (Charcoal)** | `PD-KB421BSC` | Charcoal (墨) | 69-key JIS | Hardware identical; matrix pending |
| **HHKB Pro Classic Type-S JIS (White)** | `PD-KB421WSC` | White (白) | 69-key JIS | Hardware identical; matrix pending |
| **HHKB Pro Classic Type-S JIS (Snow)** | `PD-KB421YSC` | Snow (雪) | 69-key JIS | Hardware identical; matrix pending |

### Technical Analysis:
- **Hardware Architecture:** The `PD-KB421` series shares the exact same core controller architecture as the ANSI models (STMicroelectronics STM32L072, identical analog sensing frontend, dual-bank bootloader, and USB flasher protocol).
- **Matrix Routing Difference:** The Japanese layout features **69 physical keys** (incorporating arrow cluster, dedicated Kana/Henkan keys, and split spacebar row). Because of the higher key count, the analog multiplexer addressing (U1/U2) and row strobe lines differ from the standard 60-key ANSI matrix (15 columns × 4 rows).
- **Current Status:** Not yet supported out-of-the-box in the main firmware target to prevent improper key mapping.

### Call for Hardware Testers & Volunteers:
If you own a `PD-KB421` model, you can assist in enabling support:
1. **EEPROM Calibration Dump:** Read factory baseline/threshold data at `0x08081000–0x080813FF` to extract factory Topre calibration values for the JIS layout.
2. **Matrix Pin Tracing:** Run a diagnostic build with HID Console logging enabled to map each physical switch press to its corresponding (Col, Row) ADC coordinate.
3. **Layout Mapping:** Assist in drafting the 69-key `via.json` / `vial.json` matrix coordinate definition.

Interested volunteers can open an issue or reach out via GitHub Discussions.

---

## Incompatible Keyboards

The following keyboards are **not supported**:

- **HHKB Professional HYBRID / HYBRID Type-S:** Dual-board design with Bluetooth LE transceiver and different pinout / power management.
- **HHKB Professional 2 / Pro 2 Type-S:** Legacy models powered by Fujitsu MB90F395 / Cypress controllers with USB mini-B connectors.
- **HHKB Studio:** Mechanical switch model with pointing stick and separate MCU architecture.
