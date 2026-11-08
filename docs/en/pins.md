🌐 **Language / Язык:** [English](pins.md) | [Русский](../ru/pins.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md) • [Modules](modules.md) • [MIDI](midi.md) • [Wizard](https://htmlpreview.github.io/?https://github.com/aroum/dmk/blob/main/editor/wizard.html)

---

# Pins

## Milandr (K1986BE92FI)

- Port A: PA0-PA15
- Port B: PB0-PB15
- Port C: PC0-PC15
- Port D: PD0-PD15
- Port E: PE0-PE15
- Port F: PF0-PF15

## Raspberry Pi (RP2040/RP2350)

### RP2040

- GPIO0-GPIO29

### RP2350

- GPIO0-GPIO47

## nRF52840

- Port 0: P0_00-P0_31
- Port 1: P1_00-P1_15

## Baikal (be-u1000)

- Port A: PA0-PA15
- Port B: PB0-PB15
- Port C: PC0-PC15

### Baikal BE-U1000 PIO Interface Architecture
The Baikal BE-U1000 microcontroller incorporates an independent auxiliary 32-bit RISC-V core: **CloudBEAR BM-310 (Core 2)**, clocked at up to 100 MHz with 10 KB dedicated TCM memory (`TCMA` 2 KB + `TCMB` 8 KB).

* **Purpose**: Software-defined implementation of custom external digital interfaces (Programmable Input/Output, PIO).
* **Hardware Register**: Core 2 accesses its fast dedicated 16-bit register `CORE2_PIO` (`IN`, `EN`, `OU`) for single-cycle pin manipulation.
* **Pin Routing Constraints**: The 16 lines `PIO[0]`..`PIO[15]` are hardware-mapped via the chip's internal crossbar **strictly to Port PC** (`PC[0]` through `PC[15]` as Alternate Function `AF 1`).
* **Port PA & PB Limitation**: Pins on **Port PA** and **Port PB** do not have physical connectivity to the Core 2 PIO peripheral. Any protocols implemented via the Core 2 PIO coprocessor must use pins on Port PC.

## RGB LED Pin Assignments

Depending on the microcontroller platform, the pins allowed for WS2812 RGB data output are constrained by the hardware peripheral used:

- **Milandr (K1986BE92FI)**: RGB uses the SSP (SPI) hardware blocks. You **must** connect the RGB data line to one of these specific hardware TXD pins:
  - **SSP1**: `PA15` or `PF0`
  - **SSP2**: `PD3` or `PD15`
- **Raspberry Pi (RP2040/RP2350)**: RGB uses hardware PIO (Programmable I/O) state machines. You can configure **any** GPIO pin (`GPIO0`-`GPIO29` / `GPIO0`-`GPIO47`) for the RGB connection.
- **nRF52840**: RGB uses SPIM1 (SPI master). You can configure **any** GPIO pin (`P0_00`-`P0_31` / `P1_00`-`P1_15`) for the RGB connection.
- **Baikal (be-u1000)**: RGB uses a cycle-calibrated GPIO bit-bang driver executing within a FreeRTOS critical section (`taskENTER_CRITICAL()`) on the 200 MHz CloudBEAR BR-350 core (giving 5 ns timing resolution). This allows connecting WS2812 to **any** GPIO pin across `PA0`-`PA15`, `PB0`-`PB15`, or `PC0`-`PC15` (such as `PA6` / `PM_1` on Kabarga CE) without requiring hardware SPI or PIO routing.

## Split Keyboard (UART) Pin Assignments

Both halves must use firmware with the same split packet format. The current format includes a sequence byte and CRC and is incompatible with older firmware; update both halves together.

The slave sends matrix positions; the master resolves keycodes and layers. After each matrix packet the slave opens a reply window in which the master can send one LED mask or RGB pixel. Values are repeated so a lost reply can recover. Ordinary LEDs correspond to the first eight entries in `LED_PINS`. RGB output defaults to mirroring master logical pixels starting at index 0; define `SPLIT_RGB_OFFSET` on the master to select another starting index and `SPLIT_REMOTE_RGB_COUNT` to set the number of remote pixels (default `RGB_NUM`, maximum 255). Modules may also call `split_set_remote_leds(mask)` and `split_set_remote_rgb(index, color)`. Both halves must be updated together. Physical timing and reconnect behavior should be verified on the target hardware.

For split keyboards, the communication line (`SERIAL_PIN`) works over a single-wire half-duplex connection. There are two modes available on the supported platforms:

1. **Bit-Bang (default)**: Software-emulated UART.
   - **Features**:
     - Uses **exactly 1 free GPIO pin**.
     - Transmission is queued outside the matrix task. Each UART byte uses a short critical section for bit timing; the line uses a wake pulse and a reply window.
     - Does not require external pull-up resistors (uses internal pull-up).
   - **Supported Platforms**: Milandr, Raspberry Pi, nRF52840, Baikal.

2. **Hardware / PIO Half-Duplex Mode (Hardware / PIO)**:
   Depending on the microcontroller architecture, hardware-based split communication can operate over a single pin or require tying two pins (TX and RX) together on the PCB:

   - **Single-Pin Configurations**:
     - **nRF52840**: **Supports single-pin operation**. Thanks to the fully routable pin configuration (GPIO crossbar), both the TXD and RXD signals of the UARTE peripheral can be assigned to the **same physical GPIO pin** (e.g., `P0_06`). The pin must be set to open-drain mode with a pull-up resistor.
     - **Raspberry Pi (RP2040/RP2350)**: **Supports single-pin operation**. Uses Programmable I/O (PIO) state machines to dynamically switch the direction of a single GPIO pin between input and output on the fly.
     - **Baikal (be-u1000)**: Single-wire split is supported via **Software Bit-Bang (`SOFT`)** on any free GPIO pin. For hardware UART (`HW_FULL_DUPLEX`), two physical pins (`SPLIT_TX_PIN` and `SPLIT_RX_PIN`) are used, as Baikal's hardware UART does not feature a single-wire half-duplex transceiver mode.
     - **STM32**: **Supports single-pin operation**. The built-in USART has a native Single-wire mode (enabled via the `USART_CR3_HDSEL` bit in QMK/ChibiOS), which internally routes the receiver channel to the TX pin. Only the TX pin is connected externally.

   - **Tied TX/RX Configurations**:
     - **Milandr (K1986BE92FI)**: **Requires physically connecting two pins**. The hardware UART block (ARM PL011) does not support a native single-wire mode, and the GPIO multiplexer does not allow mapping both TX and RX to the same physical pin. To use hardware UART, you **must physically connect the TX and RX pins together on the PCB** and run that single node to the split communication line. The TX pin must be set to open-drain with a pull-up resistor.
     - **Available Hardware Pin Pairs on Milandr**:
       - **UART1**: TX `PC3` + RX `PC4` (Main) or TX `PB5` + RX `PB6` (Alternate)
       - **UART2**: TX `PF1` + RX `PF0` (Override) or TX `PD1` + RX `PD0` (Alternate)

   - **Configuration**: Enabled by defining `#define SPLIT_UART_HARDWARE` in the keyboard's `config.h`.
   - **Pull-up Resistor**: For all open-drain hardware configurations, a pull-up resistor is mandatory (either internal using `#define SPLIT_UART_PULLUP 1` or an external 1.5kΩ–8.2kΩ resistor to VCC) to hold the line high when idle. (Exception: RP2040 configured with PIO where external pull-up is not required).

---

### ProMicro Pin Mapping

ProMicro alias pins (`PM_0`..`PM_21`) allow writing keyboards compatible with ProMicro footprints across all supported microcontroller platforms. They are automatically available when including `"pin_defs.h"`:

```c
#include "pin_defs.h"
```

| proMicro | rp2040 | rp2350 | nRF52840 | milandr | baikal |
| -------- | ------ | ------ | -------- | ------- | ------ |
| PM_0     | GPIO8  | GPIO8  | P0_08    | PB6     | PA7    |
| PM_1     | GPIO6  | GPIO6  | P0_06    | PB5     | PA6    |
| PM_2     | GPIO17 | GPIO17 | P0_17    | PB4     | PA12   |
| PM_3     | GPIO20 | GPIO20 | P0_20    | PB3     | PA13   |
| PM_4     | GPIO22 | GPIO22 | P0_22    | PB2     | PA5    |
| PM_5     | GPIO24 | GPIO24 | P0_24    | PB1     | PC9    |
| PM_6     | GPIO0  | GPIO0  | P1_00    | PB0     | PC8    |
| PM_7     | GPIO11 | GPIO11 | P0_11    | PC0     | PC4    |
| PM_8     | GPIO4  | GPIO4  | P1_04    | PC1     | PC6    |
| PM_9     | GPIO2  | GPIO2  | P1_06    | PC2     | PC7    |
| PM_10    | GPIO1  | GPIO1  | P0_09    | PD6     | PA8    |
| PM_14    | GPIO19 | GPIO19 | P1_11    | PD5     | PA3    |
| PM_15    | GPIO15 | GPIO15 | P1_13    | PD2     | PA2    |
| PM_16    | GPIO10 | GPIO10 | P0_10    | PD4     | PA1    |
| PM_18    | GPIO27 | GPIO27 | P1_15    | PD7     | PA0    |
| PM_19    | GPIO3  | GPIO3  | P0_02    | PE0     | PA9    |
| PM_20    | GPIO16 | GPIO16 | P0_29    | PB6     | PA11   |
| PM_21    | GPIO18 | GPIO18 | P0_31    | PE2     | PA10   |
| PM_VCC   | GPIO13 | GPIO13 | P0_13    | PB8     | PA4    |
| PM_LED   | GPIO9  | GPIO9  | P0_15    | PB7     | PA15   |
