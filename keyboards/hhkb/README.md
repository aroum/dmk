**Language / Язык:** [English](README.md) | [Русский](README_ru.md)

---

# HHKB Classic & Hybrid DMK Firmware — Uses the Original Controller, No Hardware Mods Required

Alternative firmware for HHKB Professional Classic and Hybrid keyboards. It runs directly on the original STM32L072 controller: no controller replacement, soldering, or keyboard disassembly is required.

**Documentation:** [Install](docs/en/install.md) • [Supported Models](docs/en/models.md) • [Backup & restore](docs/en/backup-restore.md) • [Build](docs/en/build.md) • [Config](docs/en/config.md) • [VIA](docs/en/via-debug.md)

Prebuilt firmware for Classic and Classic Type-S is published on the [GitHub Releases page](https://github.com/aroum/dmk/releases).

## Supported keyboards

- HHKB Classic (ANSI)
- HHKB Classic Type-S (ANSI and JIS)
- HHKB Hybrid (ANSI and JIS)
- HHKB Hybrid Type-S (ANSI and JIS)

See the detailed [Supported Models Guide](docs/en/models.md) for part numbers, color variants, and container IDs.

## Features

- [VIA](https://www.usevia.app/) or Vial support for runtime keymap changes.
- Configurable matrix scan rate: 50, 125, 250, 500, or 1000 Hz. The stock firmware's matrix loop runs at about 50 Hz. At 50–500 Hz, optional ×4 hardware ADC oversampling improves measurement precision and reduces noise.
- Factory thresholds, or per-key user calibration through [VIA](https://www.usevia.app/).
- Adjustable actuation point and classic release hysteresis.
- Rapid Trigger, including independent press and release sensitivity.
- Optional hardware ADC oversampling to reduce measurement noise.
- Two configurable LEDs; [VIA](https://www.usevia.app/) assigns Off, Debug heartbeat, Caps Lock, Fn layer, or System 2 to PA8 and PA9.
- Mod-Tap and Layer-Tap, with configurable tapping term, permissive hold, tap preference, and quick tap.
- Two base/Fn layer pairs, selectable with the System 1 and System 2 keycodes in VIA.

Hybrid Bluetooth also supports all 32 consumer usages in the factory Nordic report map, including volume, playback, brightness and browser controls. See [Bluetooth media support and limits](docs/en/via-debug.md#media-keys-over-bluetooth).

## Scan rate and power

Selecting 1000 Hz requests that matrix rate; it is not a guarantee. Hybrid ANSI scan overruns are reported in [issue #8](https://github.com/aroum/dmk/issues/8). See [scan diagnostics and the current limitation](docs/en/via-debug.md#known-1000-hz-scan-limitation) to check actual throughput.

### Classic and Hybrid — USB power

The 32 MHz CPU clock and supported scan rates are within the MCU's normal operating specifications, not overclocking. The chip remains cool.

| Firmware | Matrix scan rate | CPU    | ADC oversampling | Current |
| -------- | ---------------- | ------ | ---------------- | ------- |
| Stock    | 50 Hz            | 16 MHz | —                | 7.5 mA  |
| DMK      | 50 Hz            | 32 MHz | On               | 7.5 mA  |
| DMK      | 500 Hz           | 32 MHz | On               | 10 mA   |
| DMK      | 1000 Hz          | 32 MHz | Off              | 12 mA   |

At 1000 Hz the increase over stock is 4.5 mA. At 50 Hz DMK matches the measured stock consumption.

On USB power, HHKB Hybrid consumption is essentially identical to Classic, though baseline current may be slightly higher due to the dedicated Bluetooth controller being powered; the relative ratio between stock and DMK remains identical to Classic across all scan rates.

### Hybrid — battery power

On USB power, the matrix scan rate remains configurable in [VIA](https://www.usevia.app/) up to **1000 Hz**. Battery operation uses the stock-style matrix period of approximately **11 ms, or about 91–92 Hz**. This is key scanning, not the Bluetooth report rate: report timing is controlled by the stock Nordic firmware and connection parameters.

Bluetooth, 3.0 V, measured on one keyboard. DMK aims to match stock battery consumption. The stock firmware powers off rather than providing DMK's optional matrix-wake sleep.

| Mode                           | Stock   | DMK     |
| ------------------------------ | ------- | ------- |
| OFF                            | 0.21 mA | 0.21 mA |
| Sleep with matrix wake at 1 Hz | —       | 1.44 mA |
| Sleep with matrix wake at 10 Hz | — | 1.68 mA |
| Idle                           | 3.28 mA | 3.0 mA  |
| Typing                         | 3.55 mA | 3.8 mA  |
| Typing on Fn layer, LED off    | 3.4 mA  | 3.5 mA  |
| Typing on Fn layer, LED on     | —       | 5.8 mA  |

Stock has no Fn-layer LED indication: its 3.4 mA measurement belongs in the LED-off row. The extra DMK Fn-layer current with the LED on was confirmed to come from that indicator. The tester measured 1.68 mA at 10 Hz and confirmed reconnection on the first key press.

With **Wake with matrix** off, inactivity or a sleep action enters OFF directly. With it on, sleep powers down sensors and LEDs between matrix probes at a configurable **1–10 Hz** (default 1 Hz). A detected key wakes the keyboard; holding a key briefly helps at low probe rates. **Power off after sleep** sets the battery sleep-to-OFF timeout (default 3 min; 0 never). Both controls are hidden when matrix wake is off. The tester confirmed that the timer reduces current to 0.21 mA after OFF. See [Hybrid power settings](docs/en/via-debug.md#via-settings-unique-to-hybrid).

## Installation and recovery

The firmware is installed on the original controller without opening the keyboard. The supported paths are the Python flasher or the official HHKB Keymap Tool after returning to the stock Bank 1 updater with SW6. DFU and SWD are also available for backup, recovery, and development.

The original firmware can be restored using the documented Bank 1/SW6 recovery path, or by writing a saved DFU backup. Read [Installation, backup, and recovery](docs/en/backup-restore.md) before flashing.

## VIA or Vial

[VIA](https://www.usevia.app/) provides the full **HHKB Settings** menu: Scan Rate, Calibration, Rapid Trigger, and LED, as well as a **Debug** menu displaying the dynamic **Firmware Version** read directly from controller memory. The custom controls are available only in the VIA web app, not in Vial or other VIA clients. To load them, open **Settings**, enable **Show Design tab**, then use **Design → Load Draft Definition** to select `via.json`.

Vial supports runtime keymap editing through `vial.json`, but it does not provide custom UI controls. Use [VIA](https://www.usevia.app/) for calibration, scan rate, LED assignments, Rapid Trigger settings, and runtime version queries.

## DIP switches

DMK reads all six DIP switches. SW6 currently provides the recovery transition to the stock Bank 1 updater. The remaining switches are intentionally not bound to fixed layout behaviour yet: a user-editable [VIA](https://www.usevia.app/) or Vial keymap makes automatic layout rewrites ambiguous.

Possible future uses include assigning switch functions through [VIA](https://www.usevia.app/) dropdowns. Suggestions are welcome.

## Other HHKB models

### HHKB Professional 2 and other models

Firmware development is possible, but installation is different. Pro2 has no built-in USB firmware update path, so it must be opened and flashed through SWD with a programmer.

### HHKB Studio

Studio uses a different controller and architecture, with mechanical switches, a pointing stick and gesture pads. It needs a separate port rather than the Classic/Hybrid firmware. More information about the controller pinout, peripheral protocols and update format is needed. See the [Studio hardware and support notes](docs/en/models.md#hhkb-studio).

### Realforce R3 and newer

Realforce R3 and newer models use a similar firmware-update container format, so firmware for the original controller should be possible. I do not have a compatible keyboard for hardware testing or for reverse-engineering the controller pinout, so this is not supported yet.

## Directory structure

- `modules/ec/` — matrix scanning, ADC, calibration, and Rapid Trigger.
- `modules/dip_switch/` — DIP switch input.
- `modules/via_settings/` — VIA commands and persistent settings.
- `modules/indicators_system/` — LEDs and the two-system layer selector.
- `via.json` — shared VIA definition with ANSI and JIS layouts and custom controls.
- `vial.json` — shared Vial definition with ANSI and JIS layouts.
- `docs/` — board documentation and research notes.

## Acknowledgements

- [SuperEnduro](https://www.reddit.com/user/SuperEnduro/) — for testing and helping reverse-engineer the Hybrid.
- [guyguilty](https://www.reddit.com/user/guyguilty/) — for testing the Classic.
