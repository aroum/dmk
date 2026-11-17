**Language:** [English](config.md) | [Русский](../ru/config.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Configuration

## Build definitions

- `HHKB_RAPID_TRIGGER` enables Rapid Trigger and its [VIA](https://www.usevia.app/) menu.
- `HHKB_CLASSIC_TYPE_S` selects the Type-S HFB model.
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

## Hybrid only: transport, profiles, and power

PC13 reads low on USB power and high on batteries. On battery, DMK sends six-key keyboard reports to the stock nRF over SPI and uses a separate battery matrix scan rate. On USB, reports go to USB by default and use the normal VIA scan rate. **BT / USB** can route reports to the last Bluetooth profile even while USB is connected. Selecting USB without a cable takes effect when USB is connected. Bluetooth media reports are not yet implemented. Matrix scan frequency is separate from the BLE connection interval and report frequency; the latter remain unmeasured.

Four Bluetooth profiles are available. **BT Select 1–4** selects an existing bond; **BT Pair 1–4** pairs in a chosen slot and can replace its bond. **BT Next** cycles through the four slots. The last selected slot is saved in EEPROM and requested on battery startup. **BT Pair Last** pairs that slot. DMK queues an all-keys-released report when changing output or profile; delivery over Bluetooth still needs hardware verification.

The stock nRF is SPI master on PA15/PB3–PB5 (mode 3, 125 kHz). PB6 requests a poll and PD2 controls the radio. The STM32 application occupies Bank 2 at `0x08010000`; the patcher retains the stock nRF firmware.

The common [VIA controls](via.md) also configure USB matrix rate, calibration, actuation and hysteresis, Rapid Trigger when compiled in, ADC oversampling, hold-tap timing, and the base modes of both LEDs. Hybrid-specific battery, profile, and PC5 controls are listed below. The Classic factory calibration table must not be assumed to match Hybrid hardware.

### Power modes and PC5

- **USB active:** USB output by default. The OFF action is battery-only, as in the reference implementation.
- **Battery active:** Bluetooth output by default. Battery scanning defaults to 200 Hz; VIA offers 50, 125, 200, 250, 500, and 1000 Hz. At 1000 Hz ADC oversampling is bypassed.
- **Sleep:** Triggered by a keycode, configured PC5 action, or inactivity. The timeout defaults to 30 minutes and is capped at 10 minutes during pairing; `0` disables automatic sleep. With **Wake with matrix** off, sleep stops the nRF and enters STM32 STOP. PC5 held one second or USB insertion wakes by resetting the controller. With matrix wake on, the MCU keeps running at reduced duty cycle and scans once a second; PC5, a key, or USB wakes it. This mode consumes more than STOP.
- **OFF:** On batteries, the default two-second PC5 hold or **BT Power** stops the nRF, ADC, sensor circuit, and matrix timer, then enters STOP. A one-second PC5 hold or USB insertion restarts the board. LPTIM1 wakes about every 1.7 seconds to refresh an active independent watchdog. STOP current and wake behavior require measurement on Hybrid hardware.

Short PC5 presses last 0–0.5 seconds and show the connected profile by default. VIA can assign no action, battery display, sleep, next profile, or last Bluetooth profile/USB toggle. Long presses fire after two seconds and default to OFF; VIA can assign sleep or pairing of the last profile. The one-second OFF wake action is fixed, so the keyboard remains recoverable.

The reference firmware also uses STM32 STOP for OFF and sleep, waking on PC5 or USB with a reset. DMK's [STOP implementation](../../modules/hybrid/hybrid_stop.c) adapts only this MIT-licensed wake sequence. Matrix wake at 1 Hz uses a running MCU instead.

### Hardware and license notes

The ANSI matrix and SPI protocol follow the supplied `hhkb_hybrid` research. Its `nrf_spi.c` and `power.c` have MIT headers; only protocol handling and the STOP wake sequence were adapted. See the attribution and full upstream notice in [LICENSE.protocol](../../modules/hybrid/LICENSE.protocol). Factory and patched PFU containers remain outside Git.

JIS coordinates and the Hybrid factory calibration map remain unverified. A successful build does not establish that the keyboard boots, pairs, wakes from STOP, or reaches the expected battery current on real hardware.
