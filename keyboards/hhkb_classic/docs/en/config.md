**Language:** [English](config.md) | [Русский](../ru/config.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Configuration

## Build definitions

- `HHKB_RAPID_TRIGGER` enables Rapid Trigger and its [VIA](https://www.usevia.app/) menu.
- `HHKB_TYPE_S` selects the Type-S HFB model.
- `HHKB_ADC_16MHZ` clocks ADC from HCLK/2 instead of HCLK/4.
- `HHKB_ADC_OVERSAMPLING` sets the initial [VIA](https://www.usevia.app/) preference for ×4 hardware averaging. ×4 operates only at 50–500 Hz; 1000 Hz always uses ×1. The user's preference is saved in EEPROM.
- `HHKB_ADC_DMA` transfers each synchronized ADC1 result through DMA1 Channel 1. The scanner still triggers one conversion after each row strobe and waits for completion. On the tested board with ×4 oversampling, this did not improve scan speed.
- `HHKB_FACTORY_THRESHOLDS` loads each board's factory threshold and offset tables from EEPROM for classic mode.
- `HHKB_SCAN_DIAGNOSTICS` adds diagnostic counters for SWD measurement.

## Matrix scanning

The Classic matrix has 15 columns and 4 rows. DMK selects a multiplexer column, pulses PC8 to discharge the sampling path, drives one row LOW, waits for the analog path, performs one ADC conversion, releases the row, and repeats for all rows and columns. PC14 powers the sensor circuit, PC7 enables the amplifier, and PA1 is ADC1_IN1.

The timer scanner is enabled by default. [VIA](https://www.usevia.app/) selects 50, 125, 250, 500, or 1000 Hz; the 1000 Hz choice uses a 950 µs target period. The scanner runs from SRAM and uses TIM2 for analog delays.

## Matrix and peripheral pinout

- **Matrix:** 4 rows × 15 columns.
- **Row drive:** `PB0`, `PB1`, `PB2`, and `PB10` (`GPIOB` masks `0x001`, `0x002`, `0x004`, `0x400`).
- **Column select:** two 4051 analog multiplexers. `PC6` selects U1 for columns 0–7; `PB15` selects U2 for columns 8–14; `PB12`, `PB13`, and `PB14` are the channel-address lines.
- **Analog path:** `PC14` powers the sensor circuit; `PC7` enables the op-amp; `PC8` discharges sampling capacitor C57; `PA1` is ADC1 input `ADC_IN1`.
- **DIP switches:** `PA3`–`PA7` and `PC4`, active-low inputs.
- **LEDs:** `PA8` and `PA9`. [VIA](https://www.usevia.app/) independently assigns Off, Debug heartbeat, Caps Lock, Fn layer, or System 2; the defaults are Caps Lock on PA8 and Fn layer on PA9.

## Factory data and calibration

Factory classic-mode thresholds are read from the keyboard's EEPROM at startup. They therefore remain specific to each board when the same DMK HFB is installed elsewhere. User calibration records a baseline and peak per key in a separate EEPROM record. Disabling user calibration selects the default calibrated range; it does not overwrite factory tables.

## DIP switches

SW1–SW5 are read but have no fixed DMK layout action. SW6 is assigned to Bank 1 recovery by default. Assign the System 1 and System 2 keycodes in VIA to select a base/Fn layer pair. Fn + Tab remains Caps Lock.
