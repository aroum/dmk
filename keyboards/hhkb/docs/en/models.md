**Language:** [English](models.md) | [Русский](../ru/models.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# HHKB models and support status

## Comparison

| Feature                  | HHKB Professional Classic    | HHKB Professional HYBRID         | HHKB Studio                                |
| :----------------------- | :--------------------------- | :------------------------------- | :----------------------------------------- |
| **Switch Technology**    | Electro-capacitive EC 45g    | Electro-capacitive EC 45g        | Mechanical Hot-Swap MX (Linear 45g)        |
| **Main MCU**             | STM32L072CZ (ARM Cortex-M0+) | STM32L072CZ (ARM Cortex-M0+)     | STM32L4 series (ARM Cortex-M4)             |
| **Wireless Transceiver** | None (Wired USB-C only)      | Taiyo Yuden EYSHCNZWZ (nRF52832) | Nordic nRF52 series (BLE 5.0)              |
| **Pointing Device**      | None                         | None                             | Isometric Pointing Stick + 3 Mouse Buttons |
| **Gesture Controls**     | None                         | None                             | 4× Capacitive Touch Gesture Pads           |
| **Power Source**         | USB-C Bus Power (5V)         | 2× AA Batteries / USB-C          | 4× AA Batteries / USB-C                    |
| **DMK Firmware Status**  | Native Support (1000 Hz, RT) | Standard ANSI build locally; hardware unverified     | Exploratory Research                       |

## HHKB Classic

DMK firmware runs natively on the factory STM32L072CZ microcontroller of the wired USB-C Happy Hacking Keyboard (HHKB Professional Classic series). No hardware modifications, soldering, or disassembly are required.

### Supported ANSI Models (60-key US Layout)

The following 7 models represent the physical 60-key ANSI (US Layout) revisions fully supported out of the box:

| Commercial Name                                 | Model Number  | Layout      | Color            | Keycaps         | Switch Type       | Container ID | Firmware Target |
| :---------------------------------------------- | :------------ | :---------- | :--------------- | :-------------- | :---------------- | :----------- | :-------------- |
| **HHKB Professional Classic (White)**           | `PD-KB401W`   | ANSI 60-key | White / Gray     | Printed         | Classic EC 45g    | `AHHX01`     | `classic`       |
| **HHKB Professional Classic (Charcoal)**        | `PD-KB401B`   | ANSI 60-key | Charcoal         | Printed         | Classic EC 45g    | `AHHX01`     | `classic`       |
| **HHKB Professional Classic (White, Blank)**    | `PD-KB401WN`  | ANSI 60-key | White / Gray     | Blank (無刻印)  | Classic EC 45g    | `AHHX01`     | `classic`       |
| **HHKB Professional Classic (Charcoal, Blank)** | `PD-KB401BN`  | ANSI 60-key | Charcoal         | Blank (無刻印)  | Classic EC 45g    | `AHHX01`     | `classic`       |
| **HHKB Classic Type-S (White)**                 | `PD-KB401WSC` | ANSI 60-key | White / Gray     | Printed         | Silent Type-S 45g | `AHHS01`     | `type-s`        |
| **HHKB Classic Type-S (Charcoal)**              | `PD-KB401BSC` | ANSI 60-key | Charcoal         | Printed         | Silent Type-S 45g | `AHHS01`     | `type-s`        |
| **HHKB Classic Type-S (Snow)**                  | `PD-KB401YSC` | ANSI 60-key | Snow (All-White) | Printed / Blank | Silent Type-S 45g | `AHHS01`     | `type-s`        |

---

### Hardware Architecture & Compatibility Notes

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

### Japanese JIS Models (PD-KB421 Series) — Pending Matrix Verification

| Commercial Name                            | Model Number  | Color         | Layout     | Status                                               |
| :----------------------------------------- | :------------ | :------------ | :--------- | :--------------------------------------------------- |
| **HHKB Pro Classic Type-S JIS (Charcoal)** | `PD-KB421BSC` | Charcoal (墨) | 69-key JIS | Builds with `HHKB_JIS`; hardware testing required    |
| **HHKB Pro Classic Type-S JIS (White)**    | `PD-KB421WSC` | White (白)    | 69-key JIS | Builds with `HHKB_JIS`; hardware testing required    |
| **HHKB Pro Classic Type-S JIS (Snow)**     | `PD-KB421YSC` | Snow (雪)     | 69-key JIS | Builds with `HHKB_JIS`; hardware testing required    |

### Technical Analysis & Hardware Auto-Detection

- **Unified Factory Firmware Distribution:** Crucially, PFU does not provide separate firmware packages for JIS models on their official website. The official updates (`HHKB410_FW_A429.hfb` for Classic and `HHKB401_FW_A803.hfb` for Classic Type-S) are identical for both ANSI and JIS models. The stock firmware automatically detects whether it is running on an ANSI or JIS board at startup (likely via a hardware strapping resistor on an unpopulated GPIO, an EEPROM descriptor flag, or sensing line pull).
- **Core Hardware Architecture:** The `PD-KB421` series shares the exact same core controller architecture as the ANSI models (STMicroelectronics STM32L072, identical analog sensing frontend, dual-bank bootloader, and USB flasher protocol).
- **Matrix Routing Difference:** The Japanese layout features **69 physical keys** (incorporating arrow cluster, dedicated Kana/Henkan keys, and split spacebar row). Because of the higher key count, the analog multiplexer addressing (U1/U2) and row strobe lines differ from the standard 60-key ANSI matrix (15 columns × 4 rows).
- **Current Status:** Not yet supported out-of-the-box in the DMK firmware target pending matrix map reverse-engineering.

### Call for Hardware Testers & Volunteers

If you own a `PD-KB421` model, you can assist in enabling support:

1. **EEPROM Calibration Dump:** Read factory baseline/threshold data at `0x08081000–0x080813FF` to extract factory EC calibration values for the JIS layout.
2. **Matrix Pin Tracing:** Run a diagnostic build with HID Console logging enabled to map each physical switch press to its corresponding (Col, Row) ADC coordinate.
3. **Layout Mapping:** Assist in drafting the 69-key `via_classic.json` / `vial_classic.json` matrix coordinate definition.

Interested volunteers can open an issue or reach out via GitHub Discussions.

---

## HHKB Hybrid and Hybrid Type-S

The HHKB HYBRID family shares the same core controller architecture as the Classic, supplemented by a Nordic Semiconductor nRF52832 Bluetooth Low Energy subsystem and dual-source battery power management.

**Status:** Standard ANSI Hybrid firmware builds locally; physical Hybrid hardware remains unverified. The patched HFB retains proprietary PFU nRF firmware and cannot be published.
**Target Platform:** Dual-chip Architecture (STMicroelectronics STM32 + Taiyo Yuden EYSHCNZWZ / Nordic nRF52832)

The [local build and patch procedure](build.md#hybrid-only-local-build-and-factory-image-patch) replaces the STM32 application in an original PFU update. That preserves proprietary nRF code, so only the build instructions and source are distributable. A proposed nRF rejection experiment could avoid shipping that code, but it requires complete backups of both chips and has not been tested.


---

### Overview

The **HHKB Professional HYBRID** and **HYBRID Type-S** represent the dual-mode (USB-C & Bluetooth Low Energy) generation of the Happy Hacking Keyboard family.

The primary goal of this subproject is to port the open-source **DMK** firmware to run natively on the keyboard's internal STM32 microcontroller. This brings Vial / VIA runtime configuration, high-speed EC scanning, customizable actuation points, and Rapid Trigger capabilities to the HYBRID series, while preserving the factory Bluetooth transceiver functionality.

According to hardware teardowns and community contributions, the controller motherboard architecture is closely related to the HHKB Professional Classic, with the addition of a dedicated Bluetooth subsystem, battery power management, and wireless antennas. Crucially, the physical controller PCB is shared across both ANSI and JIS models.

---

### Hardware architecture

#### Dual-chip processing

Unlike simple single-MCU keyboards, the HHKB HYBRID employs an asymmetric dual-controller design:

- **Application & Matrix Processor (STM32):**
  - Microcontroller: STMicroelectronics STM32 series (similar to the STM32L072 ARM Cortex-M0+ found in the Classic).
  - Responsibilities: Driving the capacitive EC matrix multiplexers, high-speed analog ADC sensing, key travel calculations, debounce/filtering, USB-C device stack, and power-saving sleep logic.
- **Wireless & Bluetooth Subsystem (Nordic nRF52832):**
  - Module: **Taiyo Yuden EYSHCNZWZ** ([Datasheet](https://docs.rs-online.com/2d1f/0900766b81699ea1.pdf)).
  - Core: **Nordic Semiconductor nRF52832** (32-bit ARM Cortex-M4F @ 64 MHz, 512 KB Flash, 64 KB RAM, Bluetooth 5.0 Low Energy).
  - Responsibilities: Bluetooth LE protocol stack, HID-over-GATT profile, multi-device host pairing storage (up to 4 devices), and RF power control.
- **Inter-Chip Interconnect:**
  - Dedicated high-speed serial bus (UART or SPI with hardware flow control and handshake interrupt lines) connecting the STM32 and nRF52832.
  - The STM32 sends formatted key state reports and battery status to the Nordic module; the Nordic module relays host pairing commands, connection states, and wake-up events to the STM32.

#### Analog EC sensing

- Shares the same discrete capacitive sensing circuit topology as the Classic:
  - Discrete operational amplifier / comparator for integration and charge amplification.
  - Analog multiplexers (e.g. 74HC4051) for column selection.
  - Active-low row drive strobe lines on GPIO.
  - Hardware discharge cycle on capacitor C57 between switch samples.
- ANSI models utilize a 15-column × 4-row matrix (60 keys).
- JIS models utilize an expanded matrix accommodating 69 physical switches.

#### Power and USB

- Dual power sources: 2× AA alkaline / Ni-MH batteries (battery compartment hump) and 5V USB-C.
- Onboard boost/buck DC-DC regulators providing clean 3.3V rails for digital logic and the analog sensing circuit.
- Low-quiescent-current sleep circuitry with auto-off timers and keypress wake-up interrupts.

---

### Development and verification

#### STM32 and stock nRF

The engineering target is to **retain the factory Nordic nRF52832 firmware** and exclusively replace the STM32 application firmware:

1. **Zero RF Certification Risk:** Keeping the factory Bluetooth stack avoids any need for re-certifying RF behavior, Bluetooth qualification, or reimplementing complex Bluetooth LE pairing state machines.
2. **Native DMK Core on STM32:** The STM32 will execute the full DMK firmware (Vial/VIA protocol, dynamic layers, macros, Rapid Trigger, and 1000 Hz USB wired scanning).
3. **Bridge Protocol:** DMK will speak the factory inter-chip protocol over the internal UART/SPI lines to feed keystroke reports to the Nordic module during wireless operation.

#### Remaining hardware checks

1. **Full Non-Destructive Flash Backup:**
   - Dump Flash, EEPROM, and Option Bytes from both the STM32 and the nRF52832 via SWD before modifying any memory contents (verifying Readout Protection / RDP level).
2. **Protocol Analysis:**
   - Tap the internal communication traces between STM32 and EYSHCNZWZ using a logic analyzer or RP2040.
   - Decode the packet framing during:
     - USB connected vs. disconnected transitions.
     - Bluetooth pairing and switching across the 4 paired devices (`Fn + Ctrl + 1..4`).
     - Keystroke press and release transmissions.
     - Battery telemetry reports.
     - Deep sleep entry and wake-up signaling.
3. **Firmware Update Protocol Reversal:**
   - Analyze the PFU Keymap Tool update routines to determine if the official updater flashes both chips or only the STM32.

### Hardware volunteers

We are actively looking for community members owning an HHKB Professional HYBRID or HYBRID Type-S who are willing to participate in reverse-engineering.

#### Equipment

- **SWD Programmer / Debugger:** Segger J-Link, ST-Link v2/v3, or an inexpensive Raspberry Pi Pico / RP2040 running [picoprobe](https://github.com/raspberrypi/picoprobe).
- **Logic Analyzer:** Saleae, Sigrok-compatible 8-channel analyzer (24 MHz), or an RP2040 running an analyzer firmware (e.g., [pico-logic](https://github.com/gusmanb/logicanalyzer)).
- **Basic Lab Tools:** Multimeter, fine probes, and soldering iron for attaching temporary test points.

#### Needed measurements

1. **High-Resolution PCB Scans:** Both top and bottom sides of the motherboard with readable IC part markings.
2. **SWD Pinout Identification:** Locating test pads for the STM32 SWD (`SWDIO`, `SWCLK`, `NRST`, `GND`) and the Nordic module SWD pins.
3. **Firmware Memory Readback:** Verifying if Readout Protection (RDP Level 1 or 0) allows dumping factory Flash and EEPROM images.
4. **Logic Captures:** Protocol dumps of inter-chip traffic during normal operation.

If you have the hardware and want to collaborate, please open a GitHub Issue or reach out via GitHub Discussions.

### Hybrid ANSI and JIS

#### Standard HHKB Professional Hybrid (Wired USB-C + Bluetooth 5.0)

| Commercial Name                       | Model Number | Layout      | Case Color | Keycap Color | Legends        | Switch Type     | Hardware Status |
| :------------------------------------ | :----------- | :---------- | :--------- | :----------- | :------------- | :-------------- | :-------------- |
| **HHKB Pro HYBRID (White)**           | `PD-KB800W`  | ANSI 60-key | White      | White / Gray | Dyesub Printed | Standard EC 45g | Local build; unverified |
| **HHKB Pro HYBRID (Charcoal)**        | `PD-KB800B`  | ANSI 60-key | Charcoal   | Charcoal     | Dyesub Printed | Standard EC 45g | Local build; unverified |
| **HHKB Pro HYBRID (White, Blank)**    | `PD-KB800WN` | ANSI 60-key | White      | White / Gray | Blank (無刻印) | Standard EC 45g | Local build; unverified |
| **HHKB Pro HYBRID (Charcoal, Blank)** | `PD-KB800BN` | ANSI 60-key | Charcoal   | Charcoal     | Blank (無刻印) | Standard EC 45g | Local build; unverified |
| **HHKB Pro HYBRID JIS (White)**           | `PD-KB820W`  | JIS 69-key  | White      | White / Gray | Dyesub Printed | Standard EC 45g | Builds with `HHKB_JIS`; hardware testing required |
| **HHKB Pro HYBRID JIS (Charcoal)**        | `PD-KB820B`  | JIS 69-key  | Charcoal   | Charcoal     | Dyesub Printed | Standard EC 45g | Builds with `HHKB_JIS`; hardware testing required |

#### HHKB Professional Hybrid Type-S (Silenced EC + Bluetooth 5.0)

| Commercial Name                              | Model Number  | Layout      | Case Color | Keycap Color | Legends        | Switch Type       | Hardware Status |
| :------------------------------------------- | :------------ | :---------- | :--------- | :----------- | :------------- | :---------------- | :-------------- |
| **HHKB Pro HYBRID Type-S (White)**           | `PD-KB800WS`  | ANSI 60-key | White      | White / Gray | Dyesub Printed | Silent Type-S 45g | Variant unverified |
| **HHKB Pro HYBRID Type-S (Charcoal)**        | `PD-KB800BS`  | ANSI 60-key | Charcoal   | Charcoal     | Dyesub Printed | Silent Type-S 45g | Variant unverified |
| **HHKB Pro HYBRID Type-S (White, Blank)**    | `PD-KB800WNS` | ANSI 60-key | White      | White / Gray | Blank (無刻印) | Silent Type-S 45g | Variant unverified |
| **HHKB Pro HYBRID Type-S (Charcoal, Blank)** | `PD-KB800BNS` | ANSI 60-key | Charcoal   | Charcoal     | Blank (無刻印) | Silent Type-S 45g | Variant unverified |
| **HHKB Pro HYBRID Type-S (Snow)**            | `PD-KB800YS`  | ANSI 60-key | Snow White | Snow White   | Dyesub Printed | Silent Type-S 45g | Variant unverified |
| **HHKB Pro HYBRID Type-S (Snow, Blank)**     | `PD-KB800YNS` | ANSI 60-key | Snow White | Snow White   | Blank (無刻印) | Silent Type-S 45g | Variant unverified |
| **HHKB Pro HYBRID Type-S JIS (White)**           | `PD-KB820WS`  | JIS 69-key  | White      | White / Gray | Dyesub Printed | Silent Type-S 45g | Builds with `HHKB_JIS`; hardware testing required |
| **HHKB Pro HYBRID Type-S JIS (Charcoal)**        | `PD-KB820BS`  | JIS 69-key  | Charcoal   | Charcoal     | Dyesub Printed | Silent Type-S 45g | Builds with `HHKB_JIS`; hardware testing required |
| **HHKB Pro HYBRID Type-S JIS (Snow)**            | `PD-KB820YS`  | JIS 69-key  | Snow White | Snow White   | Dyesub Printed | Silent Type-S 45g | Builds with `HHKB_JIS`; hardware testing required |
| **HHKB Pro HYBRID Type-S JIS (Snow, Blank)**     | `PD-KB820YNS` | JIS 69-key  | Snow White | Snow White   | Blank (無刻印) | Silent Type-S 45g | Builds with `HHKB_JIS`; hardware testing required |

#### HHKB Professional Hybrid Type-S 30th Anniversary

| Commercial Name                                  | Model Number   | Layout      | Case Color | Keycap Color | Legends        | Switch Type       | Hardware Status |
| :----------------------------------------------- | :------------- | :---------- | :--------- | :----------- | :------------- | :---------------- | :-------------- |
| **HHKB HYBRID Type-S 30th Anniv (Charcoal)**     | `PD-KB800BSC1` | ANSI 60-key | Charcoal   | Charcoal     | Dyesub Printed | Silent Type-S 45g | Variant unverified |
| **HHKB HYBRID Type-S 30th Anniv (White)**        | `PD-KB800WSC1` | ANSI 60-key | White      | White / Gray | Dyesub Printed | Silent Type-S 45g | Variant unverified |
| **HHKB HYBRID Type-S 30th Anniv (Snow)**         | `PD-KB800YSC1` | ANSI 60-key | Snow White | Snow White   | Dyesub Printed | Silent Type-S 45g | Variant unverified |
| **HHKB HYBRID Type-S JIS 30th Anniv (Charcoal)** | `PD-KB820BSC1` | JIS 69-key  | Charcoal   | Charcoal     | Dyesub Printed | Silent Type-S 45g | Builds with `HHKB_JIS`; hardware testing required |
| **HHKB HYBRID Type-S JIS 30th Anniv (White)**    | `PD-KB820WSC1` | JIS 69-key  | White      | White / Gray | Dyesub Printed | Silent Type-S 45g | Builds with `HHKB_JIS`; hardware testing required |
| **HHKB HYBRID Type-S JIS 30th Anniv (Snow)**     | `PD-KB820YSC1` | JIS 69-key  | Snow White | Snow White   | Dyesub Printed | Silent Type-S 45g | Builds with `HHKB_JIS`; hardware testing required |

---

## HHKB Studio

**Status:** Exploratory Research Phase (Hardware Investigation & Architectural Scouting)
**Target Platform:** Multi-Controller Mechanical Architecture (STMicroelectronics STM32L4 + Nordic nRF52 + Touch Controllers + TrackPoint)

---

#### Overview

The **HHKB Studio** represents a radical departure from the traditional EC electro-capacitive architecture. It is an all-in-one workstation keyboard featuring hot-swappable mechanical switches, an integrated center Pointing Stick (TrackPoint) with three physical mouse buttons, four capacitive Gesture Pads on the chassis bezels, and dual-mode USB-C / Bluetooth Low Energy connectivity.

Porting the open-source **DMK** firmware to the HHKB Studio requires substantial reverse-engineering due to the high density of peripheral subsystems and auxiliary sensor controllers.

---

### Preliminary Hardware Architecture

Based on initial teardowns and community documentation, the HHKB Studio utilizes a sophisticated multi-chip topology:

```
                       ┌────────────────────────┐
                       │  Nordic nRF52 Series   │
                       │   Bluetooth LE Stack   │
                       └───────────┬────────────┘
                                   │ Serial / UART
                       ┌───────────┴────────────┐
                       │    Main Controller     │
                       │ STMicroelectronics     │
                       │ STM32L4 Series (ARM M4)│
                       └─────┬──────┬─────┬─────┘
                             │      │     │
         ┌───────────────────┘      │     └────────────────────┐
         │                          │                          │
┌────────┴────────┐      ┌──────────┴─────────┐     ┌──────────┴─────────┐
│ Mechanical Keys │      │   Pointing Stick   │     │  4× Gesture Pads   │
│ Hot-swap Matrix │      │  (TrackPoint +     │     │ (Capacitive Touch  │
│ (Diodes + GPIO) │      │  3 Mouse Buttons)  │     │   Slider ICs)      │
└─────────────────┘      └────────────────────┘     └────────────────────┘
```

#### 1. Main Controller (MCU)

- **Suspected Core:** STMicroelectronics **STM32L4** series (ARM Cortex-M4 @ 80 MHz, hardware FPU, ultra-low-power modes, high-speed ADC, and flexible I2C/SPI peripherals).
- **Workload:** High-frequency digital matrix scanning, mouse pointing stick acceleration / ballistics filtering, multi-touch gesture decoding, USB composite HID stack (Keyboard + Mouse + Consumer + System Control), and inter-chip coordination.

#### 2. Pointing Stick (TrackPoint) & Mouse Keys

- **Sensor Mechanism:** Isometric strain-gauge or Hall-effect analog pointing stick positioned between the `G`, `H`, and `B` key switches.
- **Buttons:** Three dedicated mechanical mouse switches located directly below the spacebar row (Left, Middle/Scroll, Right).
- **Processing:** Dual-axis analog signals digitized through high-resolution differential ADC channels or an auxiliary TrackPoint controller, requiring calibration drift compensation, non-linear acceleration curves, and dead-zone filtering.

#### 3. Gesture Pads (Touch Sliders)

- **Sensors:** Four touch-sensitive navigation strips positioned on the chassis exterior:
  - Left side bezel (default: arrow navigation / window switching).
  - Right side bezel (default: scrolling / zooming).
  - Left front bezel (default: app switching / tab switching).
  - Right front bezel (default: document navigation / volume control).
- **Auxiliary Controllers:** Driven by dedicated capacitive touch slider controllers (e.g. Azoteq IQS or Microchip CAP series) communicating with the main MCU over shared I2C or SPI buses with interrupt-on-touch signaling.

#### 4. Switch Matrix

- **Mechanism:** Unlike classic EC capacitive sensing, the Studio uses **mechanical MX-compatible hot-swappable switches** (factory custom 45g linear silent switches) on a conventional digital switch matrix with switching diodes.
- No analog charge-integrator or EC multiplexer frontend is present.

#### 5. Wireless Subsystem

- Powered by a Nordic Semiconductor **nRF52** series SoC managing Bluetooth 5.0 Low Energy, multi-device host pairing (up to 4 profiles), and power profiles powered by 4× AA batteries or USB-C bus power.

---

---

### Call for Hardware Contributors & Reverse-Engineers

Because hardware information and component schematics for the HHKB Studio are extremely limited, community assistance is crucial.

If you own an HHKB Studio and have access to basic lab or debugging tools, we would greatly appreciate:

1. **High-Resolution PCB Photography:**
   - Sharp, glare-free macro photos of both sides of the main controller PCB, sub-boards, and sensor breakout flex cables.
   - Legible photos of all IC markings (MCU, touch sensor controllers, power management ICs, flash memory).
2. **Component Identification:**
   - Confirmation of exact part numbers for the MCU, BLE module, and capacitive gesture slider ICs.
3. **Trace & Bus Mapping:**
   - Tracing connections between the main MCU and the 4 gesture pad ribbons (identifying I2C / SPI lines, interrupts, power rails).
   - Pinout tracing for the pointing stick module and mouse switch signals.
   - Locating debug test pads (`SWDIO`, `SWCLK`, `NRST`, UART TX/RX).
4. **Firmware & Communication Dumps:**
   - Non-destructive firmware flash backups via SWD (if readout protection permits).
   - Logic analyzer captures of communication between the main MCU, gesture controllers, and Bluetooth module.

If you are interested in contributing hardware data or collaborating on reverse-engineering, please open an issue or start a discussion on our GitHub repository.

---

### External References & Community Teardowns

- **FCC ID Official Internal Photos (PDF):**  
  [FCC ID Internal Photographs (fccid.io)](https://fccid.io/m/c43d7d2438f06f734587671a0ccda4a33eedf70136552befb534f8075f53212a.pdf)  
  High-resolution laboratory teardown photographs of internal PCBs, wireless antennas, trackpoint daughterboards, and component layouts submitted for FCC certification.

- **Qiita Teardown & Disassembly Walkthrough (Japanese):**
  [目覚めるとHHKB Studioのポッチから霊圧が消えていた #分解 (FrozenVoice / Qiita)](https://qiita.com/FrozenVoice/items/1d7dbad19d621e8d434c)
  Detailed disassembly steps showing the lower chassis screw layout (under rubber feet and battery compartment), flat ribbon cable connectors, multi-layer stacked PCB architecture, switch removal requirement, and pointing stick module mounting.

- **Reddit Modding & PCB Discussion:**
  [Modding HHKB Studio (r/HHKB)](https://www.reddit.com/r/HHKB/comments/1khi71b/modding_hhkb_studio/)
  Community discussion covering teardown observations, internal construction, switch swapping, and hardware modding notes.

The HHKB Studio represents a mechanical switch architecture featuring hot-swappable MX-compatible switches, an integrated pointing stick (TrackPoint), 3 dedicated mouse buttons, and 4 capacitive gesture pads.

| Commercial Name                  | Model Number | Layout                | Case Color | Keycap Color | Switch Type                      | Hardware Status        |
| :------------------------------- | :----------- | :-------------------- | :--------- | :----------- | :------------------------------- | :--------------------- |
| **HHKB Studio (Charcoal, ANSI)** | `PD-ID100B`  | ANSI 60-key + 3 Mouse | Charcoal   | Charcoal     | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Snow, ANSI)**     | `PD-ID100Y`  | ANSI 60-key + 3 Mouse | Snow White | Snow White   | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Charcoal, JIS)**  | `PD-ID120B`  | JIS 69-key + 3 Mouse  | Charcoal   | Charcoal     | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Snow, JIS)**      | `PD-ID120Y`  | JIS 69-key + 3 Mouse  | Snow White | Snow White   | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |

---

## Technical Comparison Across Generations

---

## Legacy Incompatible Keyboards

The following legacy keyboards are **not supported**:

- **HHKB Professional 2 / Pro 2 Type-S:** Legacy models powered by Fujitsu MB90F395 / Cypress controllers with USB mini-B connectors.
- **HHKB Lite / Lite 2:** Membrane-based entry-level keyboards with PS/2 or USB interfaces.
