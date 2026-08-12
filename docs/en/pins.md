🌐 **Language / Язык:** [English](pins.md) | [Русский](../ru/pins.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md)

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

Port A: PA0-PA15
Port B: PB0-PB15
Port C: PC0-PC15

## RGB LED Pin Assignments

Depending on the microcontroller platform, the pins allowed for WS2812 RGB data output are constrained by the hardware peripheral used:

- **Milandr (K1986BE92FI)**: RGB uses the SSP (SPI) hardware blocks. You **must** connect the RGB data line to one of these specific hardware TXD pins:
  - **SSP1**: `PA15` or `PF0`
  - **SSP2**: `PD3` or `PD15`
- **Raspberry Pi (RP2040/RP2350)**: RGB uses PIO (programmable I/O) state machines. You can configure **any** GPIO pin (`GPIO0`-`GPIO29` / `GPIO0`-`GPIO47`) for the RGB connection.
- **nRF52840**: RGB uses SPIM1 (SPI master). You can configure **any** GPIO pin (`P0_00`-`P0_31` / `P1_00`-`P1_15`) for the RGB connection.
- **Baikal (be-u1000)**: RGB uses programmable I/O (PIO) state machines. You can configure **any** GPIO pin (`PA0`-`PA15`, `PB0`-`PB15`, `PC0`-`PC15`) for the RGB connection.

## Split Keyboard (UART) Pin Assignments

For split keyboards, the communication line (`SERIAL_PIN`) works over a single-wire half-duplex connection. There are two modes available on the supported platforms:

1. **Bit-Bang (default)**: Software-emulated UART.
   - **Features**:
     - Uses **exactly 1 free GPIO pin**.
     - On Milandr and nRF52840, this mode runs synchronously inside a processor critical section, leading to higher peak CPU load during packet transmission/reception.
     - Does not require external pull-up resistors (uses internal pull-up).
   - **Supported Platforms**: Milandr, Raspberry Pi, nRF52840, Baikal.

2. **Hardware / PIO Half-Duplex Mode (Hardware / PIO)**:
   Depending on the microcontroller architecture, hardware-based split communication can operate over a single pin or require tying two pins (TX and RX) together on the PCB:

   - **Single-Pin Configurations**:
     - **nRF52840**: **Supports single-pin operation**. Thanks to the fully routable pin configuration (GPIO crossbar), both the TXD and RXD signals of the UARTE peripheral can be assigned to the **same physical GPIO pin** (e.g., `P0_06`). The pin must be set to open-drain mode with a pull-up resistor.
     - **Raspberry Pi (RP2040/RP2350) & Baikal (via PIO)**: **Support single-pin operation**. They leverage Programmable I/O (PIO) blocks to dynamically switch the direction of a single GPIO pin between input and output on the fly.
     - **STM32**: **Supports single-pin operation**. The built-in USART has a native Single-wire mode (enabled via the `USART_CR3_HDSEL` bit in QMK/ChibiOS), which internally routes the receiver channel to the TX pin. Only the TX pin is connected externally.

   - **Tied TX/RX Configurations**:
     - **Milandr (K1986BE92FI)**: **Requires physically connecting two pins**. The hardware UART block (ARM PL011) does not support a native single-wire mode, and the GPIO multiplexer does not allow mapping both TX and RX to the same physical pin. To use hardware UART, you **must physically connect the TX and RX pins together on the PCB** and run that single node to the split communication line. The TX pin must be set to open-drain with a pull-up resistor.
     - **Available Hardware Pin Pairs on Milandr**:
       - **UART1**: TX `PC3` + RX `PC4` (Main) or TX `PB5` + RX `PB6` (Alternate)
       - **UART2**: TX `PF1` + RX `PF0` (Override) or TX `PD1` + RX `PD0` (Alternate)

   - **Configuration**: Enabled by defining `#define SPLIT_UART_HARDWARE` in the keyboard's `config.h`.
   - **Pull-up Resistor**: For all open-drain hardware configurations, a pull-up resistor is mandatory (either internal using `#define SPLIT_UART_PULLUP 1` or an external 1.5kΩ–8.2kΩ resistor to VCC) to hold the line high when idle. (Exception: RP2040 configured with PIO where external pull-up is not required).

---

### ProMicro

[dmk_core/include/proMicro_pins.h](../dmk_core/include/proMicro_pins.h)

```c
#include "../../dmk_core/include/proMicro_pins.h"
```

| proMicro | rp2040 | rp2350 | nRF52840 | milandr | baikal |
| -------- | ------ | ------ | -------- | ------- | ------ |
| PM_0     | —      | —      | P0_08    | —       | —      |
| PM_1     | —      | —      | P0_06    | —       | —      |
| PM_2     | —      | —      | P0_17    | —       | —      |
| PM_3     | —      | —      | P0_20    | —       | —      |
| PM_4     | —      | —      | P0_22    | —       | —      |
| PM_5     | —      | —      | P0_24    | —       | —      |
| PM_6     | —      | —      | P1_00    | —       | —      |
| PM_7     | —      | —      | P0_11    | —       | —      |
| PM_8     | —      | —      | P1_04    | —       | —      |
| PM_9     | —      | —      | P1_06    | —       | —      |
| PM_10    | —      | —      | P0_09    | —       | —      |
| PM_14    | —      | —      | P1_11    | —       | —      |
| PM_15    | —      | —      | P1_13    | —       | —      |
| PM_16    | —      | —      | P0_10    | —       | —      |
| PM_18    | —      | —      | P1_15    | —       | —      |
| PM_19    | —      | —      | P0_02    | —       | —      |
| PM_20    | —      | —      | P0_29    | —       | —      |
| PM_21    | —      | —      | P0_31    | —       | —      |
| PM_VCC   | —      | —      | P0_13    | —       | —      |
| PM_LED   | —      | —      | P0_15    | —       | —      |
