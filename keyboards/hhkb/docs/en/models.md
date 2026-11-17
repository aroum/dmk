**Language:** [English](models.md) | [Русский](../ru/models.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Supported HHKB Classic Models

DMK firmware runs natively on the factory STM32L072CZ microcontroller of the wired USB-C Happy Hacking Keyboard (HHKB Professional Classic series). No hardware modifications, soldering, or disassembly are required.

## Supported ANSI Models (60-key US Layout)

The following 7 models represent the physical 60-key ANSI (US Layout) revisions fully supported out of the box:

| Commercial Name | Model Number | Layout | Color | Keycaps | Switch Type | Container ID | Firmware Target |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Professional Classic (White)** | `PD-KB401W` | ANSI 60-key | White / Gray | Printed | Classic EC 45g | `AHHX01` | `classic` |
| **HHKB Professional Classic (Charcoal)** | `PD-KB401B` | ANSI 60-key | Charcoal | Printed | Classic EC 45g | `AHHX01` | `classic` |
| **HHKB Professional Classic (White, Blank)** | `PD-KB401WN` | ANSI 60-key | White / Gray | Blank (無刻印) | Classic EC 45g | `AHHX01` | `classic` |
| **HHKB Professional Classic (Charcoal, Blank)** | `PD-KB401BN` | ANSI 60-key | Charcoal | Blank (無刻印) | Classic EC 45g | `AHHX01` | `classic` |
| **HHKB Classic Type-S (White)** | `PD-KB401WSC` | ANSI 60-key | White / Gray | Printed | Silent Type-S 45g | `AHHS01` | `type-s` |
| **HHKB Classic Type-S (Charcoal)** | `PD-KB401BSC` | ANSI 60-key | Charcoal | Printed | Silent Type-S 45g | `AHHS01` | `type-s` |
| **HHKB Classic Type-S (Snow)** | `PD-KB401YSC` | ANSI 60-key | Snow (All-White) | Printed / Blank | Silent Type-S 45g | `AHHS01` | `type-s` |

---

## Hardware Architecture & Compatibility Notes

1. **Architecture Uniformity:**
   All 7 models share the exact same hardware motherboard architecture:
   - Microcontroller: **STMicroelectronics STM32L072CZY6** (ARM Cortex-M0+, 192 KB Flash, 20 KB SRAM, 6 KB EEPROM).
   - Sensor Board: Capacitive EC matrix (15 columns × 4 rows = 60 keys).
   - Factory Updater: Dual-bank flash layout (Bank 1 recovery bootloader + Bank 2 application firmware).

2. **Keycaps & Color Variants:**
   The suffix `N` in part numbers (e.g. `PD-KB401BN`, `PD-KB401WN`) denotes blank keycaps ("無刻印"). Keycap legends and case colors are purely aesthetic; the underlying electronics and firmware container format (`AHHX01`) are identical.

3. **Classic vs. Type-S:**
   - **Classic (`AHHX01`):** Standard EC switches with full 4.0 mm travel. Uses the `classic` firmware build.
   - **Type-S (`AHHS01`):** Factory silenced EC switches with internal dampening rings and ~3.8 mm travel. Uses the `type-s` firmware build (`HHKB_CLASSIC_TYPE_S` compile flag).

---

## Japanese JIS Models (PD-KB421 Series) — Pending Matrix Verification

| Commercial Name | Model Number | Color | Layout | Status |
| :--- | :--- | :--- | :--- | :--- |
| **HHKB Pro Classic Type-S JIS (Charcoal)** | `PD-KB421BSC` | Charcoal (墨) | 69-key JIS | Hardware identical; matrix pending |
| **HHKB Pro Classic Type-S JIS (White)** | `PD-KB421WSC` | White (白) | 69-key JIS | Hardware identical; matrix pending |
| **HHKB Pro Classic Type-S JIS (Snow)** | `PD-KB421YSC` | Snow (雪) | 69-key JIS | Hardware identical; matrix pending |

### Technical Analysis & Hardware Auto-Detection:
- **Unified Factory Firmware Distribution:** Crucially, PFU does not provide separate firmware packages for JIS models on their official website. The official updates (`HHKB410_FW_A429.hfb` for Classic and `HHKB401_FW_A803.hfb` for Classic Type-S) are identical for both ANSI and JIS models. The stock firmware automatically detects whether it is running on an ANSI or JIS board at startup (likely via a hardware strapping resistor on an unpopulated GPIO, an EEPROM descriptor flag, or sensing line pull).
- **Core Hardware Architecture:** The `PD-KB421` series shares the exact same core controller architecture as the ANSI models (STMicroelectronics STM32L072, identical analog sensing frontend, dual-bank bootloader, and USB flasher protocol).
- **Matrix Routing Difference:** The Japanese layout features **69 physical keys** (incorporating arrow cluster, dedicated Kana/Henkan keys, and split spacebar row). Because of the higher key count, the analog multiplexer addressing (U1/U2) and row strobe lines differ from the standard 60-key ANSI matrix (15 columns × 4 rows).
- **Current Status:** Not yet supported out-of-the-box in the DMK firmware target pending matrix map reverse-engineering.

### Call for Hardware Testers & Volunteers:
If you own a `PD-KB421` model, you can assist in enabling support:
1. **EEPROM Calibration Dump:** Read factory baseline/threshold data at `0x08081000–0x080813FF` to extract factory EC calibration values for the JIS layout.
2. **Matrix Pin Tracing:** Run a diagnostic build with HID Console logging enabled to map each physical switch press to its corresponding (Col, Row) ADC coordinate.
3. **Layout Mapping:** Assist in drafting the 69-key `via_classic.json` / `vial_classic.json` matrix coordinate definition.

Interested volunteers can open an issue or reach out via GitHub Discussions.

---

## HHKB Professional HYBRID & HYBRID Type-S (Bluetooth LE Models)

The HHKB HYBRID family shares the same core controller architecture as the Classic, supplemented by a Nordic Semiconductor nRF52832 Bluetooth Low Energy subsystem and dual-source battery power management.

### Standard HHKB Professional HYBRID (Wired USB-C + Bluetooth 5.0)

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Legends | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Pro HYBRID (White)** | `PD-KB800W` | ANSI 60-key | White | White / Gray | Dyesub Printed | Standard EC 45g | In Development |
| **HHKB Pro HYBRID (Charcoal)** | `PD-KB800B` | ANSI 60-key | Charcoal | Charcoal | Dyesub Printed | Standard EC 45g | In Development |
| **HHKB Pro HYBRID (White, Blank)** | `PD-KB800WN` | ANSI 60-key | White | White / Gray | Blank (無刻印) | Standard EC 45g | In Development |
| **HHKB Pro HYBRID (Charcoal, Blank)** | `PD-KB800BN` | ANSI 60-key | Charcoal | Charcoal | Blank (無刻印) | Standard EC 45g | In Development |
| **HHKB Pro HYBRID JIS (White)** | `PD-KB820W` | JIS 69-key | White | White / Gray | Dyesub Printed | Standard EC 45g | Planned |
| **HHKB Pro HYBRID JIS (Charcoal)** | `PD-KB820B` | JIS 69-key | Charcoal | Charcoal | Dyesub Printed | Standard EC 45g | Planned |

### HHKB Professional HYBRID Type-S (Silenced EC + Bluetooth 5.0)

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Legends | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Pro HYBRID Type-S (White)** | `PD-KB800WS` | ANSI 60-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | In Development |
| **HHKB Pro HYBRID Type-S (Charcoal)** | `PD-KB800BS` | ANSI 60-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | In Development |
| **HHKB Pro HYBRID Type-S (White, Blank)** | `PD-KB800WNS` | ANSI 60-key | White | White / Gray | Blank (無刻印) | Silent Type-S 45g | In Development |
| **HHKB Pro HYBRID Type-S (Charcoal, Blank)** | `PD-KB800BNS` | ANSI 60-key | Charcoal | Charcoal | Blank (無刻印) | Silent Type-S 45g | In Development |
| **HHKB Pro HYBRID Type-S (Snow)** | `PD-KB800YS` | ANSI 60-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | In Development |
| **HHKB Pro HYBRID Type-S (Snow, Blank)** | `PD-KB800YNS` | ANSI 60-key | Snow White | Snow White | Blank (無刻印) | Silent Type-S 45g | In Development |
| **HHKB Pro HYBRID Type-S JIS (White)** | `PD-KB820WS` | JIS 69-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S JIS (Charcoal)** | `PD-KB820BS` | JIS 69-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S JIS (Snow)** | `PD-KB820YS` | JIS 69-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S JIS (Snow, Blank)** | `PD-KB820YNS` | JIS 69-key | Snow White | Snow White | Blank (無刻印) | Silent Type-S 45g | Planned |

### HHKB Professional HYBRID Type-S 30th Anniversary Special Editions

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Legends | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB HYBRID Type-S 30th Anniv (Charcoal)** | `PD-KB800BSC1` | ANSI 60-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | In Development |
| **HHKB HYBRID Type-S 30th Anniv (White)** | `PD-KB800WSC1` | ANSI 60-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | In Development |
| **HHKB HYBRID Type-S 30th Anniv (Snow)** | `PD-KB800YSC1` | ANSI 60-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | In Development |
| **HHKB HYBRID Type-S JIS 30th Anniv (Charcoal)** | `PD-KB820BSC1` | JIS 69-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB HYBRID Type-S JIS 30th Anniv (White)** | `PD-KB820WSC1` | JIS 69-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB HYBRID Type-S JIS 30th Anniv (Snow)** | `PD-KB820YSC1` | JIS 69-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | Planned |

---

## HHKB Studio (Mechanical & Gesture Architecture)

The HHKB Studio represents a mechanical switch architecture featuring hot-swappable MX-compatible switches, an integrated pointing stick (TrackPoint), 3 dedicated mouse buttons, and 4 capacitive gesture pads.

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Studio (Charcoal, ANSI)** | `PD-ID100B` | ANSI 60-key + 3 Mouse | Charcoal | Charcoal | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Snow, ANSI)** | `PD-ID100Y` | ANSI 60-key + 3 Mouse | Snow White | Snow White | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Charcoal, JIS)** | `PD-ID120B` | JIS 69-key + 3 Mouse | Charcoal | Charcoal | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Snow, JIS)** | `PD-ID120Y` | JIS 69-key + 3 Mouse | Snow White | Snow White | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |

---

## Technical Comparison Across Generations

| Feature | HHKB Professional Classic | HHKB Professional HYBRID | HHKB Studio |
| :--- | :--- | :--- | :--- |
| **Switch Technology** | Electro-capacitive EC 45g | Electro-capacitive EC 45g | Mechanical Hot-Swap MX (Linear 45g) |
| **Main MCU** | STM32L072CZ (ARM Cortex-M0+) | STM32L072CZ (ARM Cortex-M0+) | STM32L4 series (ARM Cortex-M4) |
| **Wireless Transceiver** | None (Wired USB-C only) | Taiyo Yuden EYSHCNZWZ (nRF52832) | Nordic nRF52 series (BLE 5.0) |
| **Pointing Device** | None | None | Isometric Pointing Stick + 3 Mouse Buttons |
| **Gesture Controls** | None | None | 4× Capacitive Touch Gesture Pads |
| **Power Source** | USB-C Bus Power (5V) | 2× AA Batteries / USB-C | 4× AA Batteries / USB-C |
| **DMK Firmware Status** | Native Support (1000 Hz, RT) | In Development (Wired + BLE) | Exploratory Research |

---

## Legacy Incompatible Keyboards

The following legacy keyboards are **not supported**:

- **HHKB Professional 2 / Pro 2 Type-S:** Legacy models powered by Fujitsu MB90F395 / Cypress controllers with USB mini-B connectors.
- **HHKB Lite / Lite 2:** Membrane-based entry-level keyboards with PS/2 or USB interfaces.
