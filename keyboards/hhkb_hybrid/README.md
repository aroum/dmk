# HHKB Professional HYBRID & HYBRID Type-S

**Status:** Research & Reverse-Engineering Phase (Pre-Alpha / Hardware Investigation)  
**Target Platform:** Dual-chip Architecture (STMicroelectronics STM32 + Taiyo Yuden EYSHCNZWZ / Nordic nRF52832)

---

## Overview

The **HHKB Professional HYBRID** and **HYBRID Type-S** represent the dual-mode (USB-C & Bluetooth Low Energy) generation of the Happy Hacking Keyboard family. 

The primary goal of this subproject is to port the open-source **DMK** firmware to run natively on the keyboard's internal STM32 microcontroller. This brings Vial / VIA runtime configuration, high-speed EC scanning, customizable actuation points, and Rapid Trigger capabilities to the HYBRID series, while preserving the factory Bluetooth transceiver functionality.

According to hardware teardowns and community contributions, the controller motherboard architecture is closely related to the HHKB Professional Classic, with the addition of a dedicated Bluetooth subsystem, battery power management, and wireless antennas. Crucially, the physical controller PCB is shared across both ANSI and JIS models.

---

## Known Hardware Architecture

### 1. Dual-MCU Processing Strategy
Unlike simple single-MCU keyboards, the HHKB HYBRID employs an asymmetric dual-controller design:
* **Application & Matrix Processor (STM32):**
  * Microcontroller: STMicroelectronics STM32 series (similar to the STM32L072 ARM Cortex-M0+ found in the Classic).
  * Responsibilities: Driving the capacitive EC matrix multiplexers, high-speed analog ADC sensing, key travel calculations, debounce/filtering, USB-C device stack, and power-saving sleep logic.
* **Wireless & Bluetooth Subsystem (Nordic nRF52832):**
  * Module: **Taiyo Yuden EYSHCNZWZ** ([Datasheet](https://docs.rs-online.com/2d1f/0900766b81699ea1.pdf)).
  * Core: **Nordic Semiconductor nRF52832** (32-bit ARM Cortex-M4F @ 64 MHz, 512 KB Flash, 64 KB RAM, Bluetooth 5.0 Low Energy).
  * Responsibilities: Bluetooth LE protocol stack, HID-over-GATT profile, multi-device host pairing storage (up to 4 devices), and RF power control.
* **Inter-Chip Interconnect:**
  * Dedicated high-speed serial bus (UART or SPI with hardware flow control and handshake interrupt lines) connecting the STM32 and nRF52832.
  * The STM32 sends formatted key state reports and battery status to the Nordic module; the Nordic module relays host pairing commands, connection states, and wake-up events to the STM32.

### 2. Analog EC Sensing Frontend
* Shares the same discrete capacitive sensing circuit topology as the Classic:
  * Discrete operational amplifier / comparator for integration and charge amplification.
  * Analog multiplexers (e.g. 74HC4051) for column selection.
  * Active-low row drive strobe lines on GPIO.
  * Hardware discharge cycle on capacitor C57 between switch samples.
* ANSI models utilize a 15-column × 4-row matrix (60 keys).
* JIS models utilize an expanded matrix accommodating 69 physical switches.

### 3. Power Management & Dual-Source Switching
* Dual power sources: 2× AA alkaline / Ni-MH batteries (battery compartment hump) and 5V USB-C.
* Onboard boost/buck DC-DC regulators providing clean 3.3V rails for digital logic and the analog sensing circuit.
* Low-quiescent-current sleep circuitry with auto-off timers and keypress wake-up interrupts.

---

## Firmware Development & Reverse-Engineering Plan

### Proposed Architecture: Hybrid Coexistence
The engineering target is to **retain the factory Nordic nRF52832 firmware** and exclusively replace the STM32 application firmware:
1. **Zero RF Certification Risk:** Keeping the factory Bluetooth stack avoids any need for re-certifying RF behavior, Bluetooth qualification, or reimplementing complex Bluetooth LE pairing state machines.
2. **Native DMK Core on STM32:** The STM32 will execute the full DMK firmware (Vial/VIA protocol, dynamic layers, macros, Rapid Trigger, and 1000 Hz USB wired scanning).
3. **Bridge Protocol:** DMK will speak the factory inter-chip protocol over the internal UART/SPI lines to feed keystroke reports to the Nordic module during wireless operation.

### Investigation Steps:
1. **Full Non-Destructive Flash Backup:**
   * Dump Flash, EEPROM, and Option Bytes from both the STM32 and the nRF52832 via SWD before modifying any memory contents (verifying Readout Protection / RDP level).
2. **Protocol Analysis:**
   * Tap the internal communication traces between STM32 and EYSHCNZWZ using a logic analyzer or RP2040.
   * Decode the packet framing during:
     - USB connected vs. disconnected transitions.
     - Bluetooth pairing and switching across the 4 paired devices (`Fn + Ctrl + 1..4`).
     - Keystroke press and release transmissions.
     - Battery telemetry reports.
     - Deep sleep entry and wake-up signaling.
3. **Firmware Update Protocol Reversal:**
   * Analyze the PFU Keymap Tool update routines to determine if the official updater flashes both chips or only the STM32.

---

## Supported & Target Model List

All models share the same underlying controller PCB architecture:

### 1. Standard HHKB Professional HYBRID (Wired USB-C + Bluetooth 5.0)

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Legends | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Pro HYBRID (White)** | `PD-KB800W` | ANSI 60-key | White | White / Gray | Dyesub Printed | Standard EC 45g | Planned |
| **HHKB Pro HYBRID (Charcoal)** | `PD-KB800B` | ANSI 60-key | Charcoal | Charcoal | Dyesub Printed | Standard EC 45g | Planned |
| **HHKB Pro HYBRID (White, Blank)** | `PD-KB800WN` | ANSI 60-key | White | White / Gray | Blank (無刻印) | Standard EC 45g | Planned |
| **HHKB Pro HYBRID (Charcoal, Blank)** | `PD-KB800BN` | ANSI 60-key | Charcoal | Charcoal | Blank (無刻印) | Standard EC 45g | Planned |
| **HHKB Pro HYBRID JIS (White)** | `PD-KB820W` | JIS 69-key | White | White / Gray | Dyesub Printed | Standard EC 45g | Planned |
| **HHKB Pro HYBRID JIS (Charcoal)** | `PD-KB820B` | JIS 69-key | Charcoal | Charcoal | Dyesub Printed | Standard EC 45g | Planned |

### 2. HHKB Professional HYBRID Type-S (Silenced EC + Bluetooth 5.0)

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Legends | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Pro HYBRID Type-S (White)** | `PD-KB800WS` | ANSI 60-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S (Charcoal)** | `PD-KB800BS` | ANSI 60-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S (White, Blank)** | `PD-KB800WNS` | ANSI 60-key | White | White / Gray | Blank (無刻印) | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S (Charcoal, Blank)** | `PD-KB800BNS` | ANSI 60-key | Charcoal | Charcoal | Blank (無刻印) | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S (Snow)** | `PD-KB800YS` | ANSI 60-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S (Snow, Blank)** | `PD-KB800YNS` | ANSI 60-key | Snow White | Snow White | Blank (無刻印) | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S JIS (White)** | `PD-KB820WS` | JIS 69-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S JIS (Charcoal)** | `PD-KB820BS` | JIS 69-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S JIS (Snow)** | `PD-KB820YS` | JIS 69-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB Pro HYBRID Type-S JIS (Snow, Blank)** | `PD-KB820YNS` | JIS 69-key | Snow White | Snow White | Blank (無刻印) | Silent Type-S 45g | Planned |

### 3. HHKB Professional HYBRID Type-S 30th Anniversary Special Editions

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Legends | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB HYBRID Type-S 30th Anniv (Charcoal)** | `PD-KB800BSC1` | ANSI 60-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB HYBRID Type-S 30th Anniv (White)** | `PD-KB800WSC1` | ANSI 60-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB HYBRID Type-S 30th Anniv (Snow)** | `PD-KB800YSC1` | ANSI 60-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB HYBRID Type-S JIS 30th Anniv (Charcoal)** | `PD-KB820BSC1` | JIS 69-key | Charcoal | Charcoal | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB HYBRID Type-S JIS 30th Anniv (White)** | `PD-KB820WSC1` | JIS 69-key | White | White / Gray | Dyesub Printed | Silent Type-S 45g | Planned |
| **HHKB HYBRID Type-S JIS 30th Anniv (Snow)** | `PD-KB820YSC1` | JIS 69-key | Snow White | Snow White | Dyesub Printed | Silent Type-S 45g | Planned |

---

## Call for Hardware Contributors & Testers

We are actively looking for community members owning an HHKB Professional HYBRID or HYBRID Type-S who are willing to participate in reverse-engineering.

### Desired Equipment:
* **SWD Programmer / Debugger:** Segger J-Link, ST-Link v2/v3, or an inexpensive Raspberry Pi Pico / RP2040 running [picoprobe](https://github.com/raspberrypi/picoprobe).
* **Logic Analyzer:** Saleae, Sigrok-compatible 8-channel analyzer (24 MHz), or an RP2040 running an analyzer firmware (e.g., [pico-logic](https://github.com/gusmanb/logicanalyzer)).
* **Basic Lab Tools:** Multimeter, fine probes, and soldering iron for attaching temporary test points.

### What We Need:
1. **High-Resolution PCB Scans:** Both top and bottom sides of the motherboard with readable IC part markings.
2. **SWD Pinout Identification:** Locating test pads for the STM32 SWD (`SWDIO`, `SWCLK`, `NRST`, `GND`) and the Nordic module SWD pins.
3. **Firmware Memory Readback:** Verifying if Readout Protection (RDP Level 1 or 0) allows dumping factory Flash and EEPROM images.
4. **Logic Captures:** Protocol dumps of inter-chip traffic during normal operation.

If you have the hardware and want to collaborate, please open a GitHub Issue or reach out via GitHub Discussions.
