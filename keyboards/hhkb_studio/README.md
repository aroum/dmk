# HHKB Studio

**Status:** Exploratory Research Phase (Hardware Investigation & Architectural Scouting)  
**Target Platform:** Multi-Controller Mechanical Architecture (STMicroelectronics STM32L4 + Nordic nRF52 + Touch Controllers + TrackPoint)

---

## Overview

The **HHKB Studio** represents a radical departure from the traditional Topre electro-capacitive architecture. It is an all-in-one workstation keyboard featuring hot-swappable mechanical switches, an integrated center Pointing Stick (TrackPoint) with three physical mouse buttons, four optical/capacitive Gesture Pads on the chassis bezels, and dual-mode USB-C / Bluetooth Low Energy connectivity.

Porting the open-source **DMK (Dynamic Matrix Keyboard)** firmware to the HHKB Studio requires substantial reverse-engineering due to the high density of peripheral subsystems and auxiliary sensor controllers.

---

## Preliminary Hardware Architecture

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

### 1. Main Controller (MCU)
* **Suspected Core:** STMicroelectronics **STM32L4** series (ARM Cortex-M4 @ 80 MHz, hardware FPU, ultra-low-power modes, high-speed ADC, and flexible I2C/SPI peripherals).
* **Workload:** High-frequency digital matrix scanning, mouse pointing stick acceleration / ballistics filtering, multi-touch gesture decoding, USB composite HID stack (Keyboard + Mouse + Consumer + System Control), and inter-chip coordination.

### 2. Pointing Stick (TrackPoint) & Mouse Keys
* **Sensor Mechanism:** Isometric strain-gauge or Hall-effect analog pointing stick positioned between the `G`, `H`, and `B` key switches.
* **Buttons:** Three dedicated mechanical mouse switches located directly below the spacebar row (Left, Middle/Scroll, Right).
* **Processing:** Dual-axis analog signals digitized through high-resolution differential ADC channels or an auxiliary TrackPoint controller, requiring calibration drift compensation, non-linear acceleration curves, and dead-zone filtering.

### 3. Gesture Pads (Touch Sliders)
* **Sensors:** Four touch-sensitive navigation strips positioned on the chassis exterior:
  * Left side bezel (default: arrow navigation / window switching).
  * Right side bezel (default: scrolling / zooming).
  * Left front bezel (default: app switching / tab switching).
  * Right front bezel (default: document navigation / volume control).
* **Auxiliary Controllers:** Driven by dedicated capacitive touch slider controllers (e.g. Azoteq IQS or Microchip CAP series) communicating with the main MCU over shared I2C or SPI buses with interrupt-on-touch signaling.

### 4. Switch Matrix
* **Mechanism:** Unlike classic Topre capacitive sensing, the Studio uses **mechanical MX-compatible hot-swappable switches** (factory custom 45g linear silent switches) on a conventional digital switch matrix with switching diodes.
* No analog charge-integrator or Topre multiplexer frontend is present.

### 5. Wireless Subsystem
* Powered by a Nordic Semiconductor **nRF52** series SoC managing Bluetooth 5.0 Low Energy, multi-device host pairing (up to 4 profiles), and power profiles powered by 4× AA batteries or USB-C bus power.

---

## Technical Comparison: Classic vs. Hybrid vs. Studio

| Feature | HHKB Professional Classic | HHKB Professional HYBRID | HHKB Studio |
| :--- | :--- | :--- | :--- |
| **Switch Technology** | Electro-capacitive Topre 45g | Electro-capacitive Topre 45g | Mechanical Hot-Swap MX (Linear 45g) |
| **Main MCU** | STM32L072CZ (ARM Cortex-M0+) | STM32 series (similar to Classic) | STM32L4 series (ARM Cortex-M4) |
| **Wireless Transceiver** | None (Wired USB-C only) | Taiyo Yuden EYSHCNZWZ (nRF52832) | Nordic nRF52 series (BLE 5.0) |
| **Pointing Device** | None | None | Isometric Pointing Stick + 3 Mouse Buttons |
| **Gesture Controls** | None | None | 4× Capacitive Touch Gesture Pads |
| **Power Source** | USB-C Bus Power (5V) | 2× AA Batteries / USB-C | 4× AA Batteries / USB-C |
| **DMK Porting Complexity** | Baseline (Fully Supported) | Moderate (Dual-MCU Interconnect) | High (Multi-Peripheral Architecture) |

---

## Known Model List

The HHKB Studio lineup comprises 4 core models across ANSI and JIS layouts, available in Charcoal and Snow finishes:

| Commercial Name | Model Number | Layout | Case Color | Keycap Color | Switch Type | Hardware Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **HHKB Studio (Charcoal, ANSI)** | `PD-ID100B` | ANSI 60-key + 3 Mouse | Charcoal | Charcoal | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Snow, ANSI)** | `PD-ID100Y` | ANSI 60-key + 3 Mouse | Snow White | Snow White | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Charcoal, JIS)** | `PD-ID120B` | JIS 69-key + 3 Mouse | Charcoal | Charcoal | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |
| **HHKB Studio (Snow, JIS)** | `PD-ID120Y` | JIS 69-key + 3 Mouse | Snow White | Snow White | Mechanical Linear 45g (Hot-swap) | Hardware Investigation |

---

## Call for Hardware Contributors & Reverse-Engineers

Because hardware information and component schematics for the HHKB Studio are extremely limited, community assistance is crucial.

If you own an HHKB Studio and have access to basic lab or debugging tools, we would greatly appreciate:

1. **High-Resolution PCB Photography:**
   * Sharp, glare-free macro photos of both sides of the main controller PCB, sub-boards, and sensor breakout flex cables.
   * Legible photos of all IC markings (MCU, touch sensor controllers, power management ICs, flash memory).
2. **Component Identification:**
   * Confirmation of exact part numbers for the MCU, BLE module, and capacitive gesture slider ICs.
3. **Trace & Bus Mapping:**
   * Tracing connections between the main MCU and the 4 gesture pad ribbons (identifying I2C / SPI lines, interrupts, power rails).
   * Pinout tracing for the pointing stick module and mouse switch signals.
   * Locating debug test pads (`SWDIO`, `SWCLK`, `NRST`, UART TX/RX).
4. **Firmware & Communication Dumps:**
   * Non-destructive firmware flash backups via SWD (if readout protection permits).
   * Logic analyzer captures of communication between the main MCU, gesture controllers, and Bluetooth module.

If you are interested in contributing hardware data or collaborating on reverse-engineering, please open an issue or start a discussion on our GitHub repository.

---

## External References & Community Teardowns

* **Qiita Teardown & Disassembly Walkthrough (Japanese):**  
  [目覚めるとHHKB Studioのポッチから霊圧が消えていた #分解 (FrozenVoice / Qiita)](https://qiita.com/FrozenVoice/items/1d7dbad19d621e8d434c)  
  Detailed disassembly steps showing the lower chassis screw layout (under rubber feet and battery compartment), flat ribbon cable connectors, multi-layer stacked PCB architecture, switch removal requirement, and pointing stick module mounting.

* **Reddit Modding & PCB Discussion:**  
  [Modding HHKB Studio (r/HHKB)](https://www.reddit.com/r/HHKB/comments/1khi71b/modding_hhkb_studio/)  
  Community discussion covering teardown observations, internal construction, switch swapping, and hardware modding notes.
