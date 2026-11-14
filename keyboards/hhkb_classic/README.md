**Language / Язык:** [English](README.md) | [Русский](README_ru.md)

---

# HHKB Classic DMK Firmware — Uses the Original Controller, No Hardware Mods Required

Alternative firmware for the HHKB Professional Classic. It runs directly on the original STM32L072 controller: no controller replacement, soldering, or keyboard disassembly is required.

**Documentation:** [Install](docs/en/install.md) • [Supported Models](docs/en/models.md) • [Backup & restore](docs/en/backup-restore.md) • [Build](docs/en/build.md) • [Config](docs/en/config.md) • [VIA](docs/en/via.md)

Prebuilt firmware for Classic and Classic Type-S is published on the [GitHub Releases page](https://github.com/aroum/dmk/releases).

## Supported keyboards

- HHKB Professional Classic (`PD-KB401W`, `PD-KB401B`, `PD-KB401WN`, `PD-KB401BN`)
- HHKB Professional Classic Type-S (`PD-KB401WSC`, `PD-KB401BSC`, `PD-KB401YSC`)

See the detailed [Supported Models Guide](docs/en/models.md) for part numbers, color variants, and container IDs.

## Features

- [VIA](https://www.usevia.app/) and Vial support for runtime keymap changes.
- Configurable matrix scan rate: 50, 125, 250, 500, or 1000 Hz. The stock firmware's matrix loop runs at about 50 Hz. At 50–500 Hz, optional ×4 hardware ADC oversampling improves measurement precision and reduces noise.
- Factory thresholds, or per-key user calibration through [VIA](https://www.usevia.app/).
- Adjustable actuation point and classic release hysteresis.
- Rapid Trigger, including independent press and release sensitivity.
- Optional hardware ADC oversampling to reduce measurement noise.
- Two configurable LEDs; [VIA](https://www.usevia.app/) assigns Off, Debug heartbeat, Caps Lock, Fn layer, or System 2 to PA8 and PA9.
- Mod-Tap and Layer-Tap, with configurable tapping term, permissive hold, tap preference, and quick tap.
- Two base/Fn layer pairs, selectable with the System 1 and System 2 keycodes in VIA.

## Scan rate and power

Running the controller at a higher scan rate is within the MCU's normal operating specifications. The 32 MHz clock and the supported scan rates are standard operating modes, not overclocking; the chip remains cool.

Measured current consumption:

- Stock firmware, 50 Hz, 16 MHz: 7.5 mA.
- DMK, 500 Hz with ADC oversampling, 32 MHz: 10 mA.
- DMK, 1000 Hz without oversampling, 32 MHz: 12 mA.

At 1000 Hz the increase is only 4.5 mA, which is negligible for a USB-powered keyboard. If power consumption matters, select 50 Hz in [VIA](https://www.usevia.app/) to return to the stock 7.5 mA level.

PFU's 50 Hz choice may reflect the Classic's shared design lineage with the battery-powered Bluetooth Hybrid. The Classic is effectively the wired-only version, and the scan rate apparently drew little attention for years.

## Installation and recovery

The firmware is installed on the original controller without opening the keyboard. The supported paths are the Python flasher or the official HHKB Keymap Tool after returning to the stock Bank 1 updater with SW6. DFU and SWD are also available for backup, recovery, and development.

The original firmware can be restored using the documented Bank 1/SW6 recovery path, or by writing a saved DFU backup. Read [Installation, backup, and recovery](docs/en/backup-restore.md) before flashing.

## VIA vs Vial

[VIA](https://www.usevia.app/) provides the full **HHKB Settings** menu: Scan Rate, Calibration, Rapid Trigger, and LED. The custom controls are available only in the VIA web app, not in Vial or other VIA clients. To load them, open **Settings**, enable **Show Design tab**, then use **Design → Load Draft Definition** to select `via.json`.

Vial supports runtime keymap editing through `vial.json`, but it does not provide custom UI controls. Use [VIA](https://www.usevia.app/) for calibration, scan rate, LED assignments, and Rapid Trigger settings.

## DIP switches

DMK reads all six DIP switches. SW6 currently provides the recovery transition to the stock Bank 1 updater. The remaining switches are intentionally not bound to fixed layout behaviour yet: a user-editable [VIA](https://www.usevia.app/)/Vial keymap makes automatic layout rewrites ambiguous.

Possible future uses include assigning switch functions through [VIA](https://www.usevia.app/) dropdowns. Suggestions are welcome.

## Other HHKB models

### HHKB Professional Hybrid / Hybrid JIS

USB mode can run DMK; Bluetooth needs separate work. A test keyboard is needed for reliable support. If you can test or provide board information, please send a Reddit DM.

### HHKB Professional Classic JIS

PFU does not provide a separate public firmware file for this model, and board photos are scarce. It may share the regular Classic hardware, but this still needs confirmation.

### HHKB Professional 2 and other models

Firmware development is possible, but installation is different. Pro2 has no built-in USB firmware update path, so it must be opened and flashed through SWD with a programmer.

### Realforce R3 and newer

Realforce R3 and newer models use a similar firmware-update container format, so firmware for the original controller should be possible. I do not have a compatible keyboard for hardware testing or for reverse-engineering the controller pinout, so this is not supported yet.

## Directory structure

- `modules/topre/` — matrix scanning, ADC, calibration, and Rapid Trigger.
- `modules/dip_switch/` — DIP switch input.
- `modules/via_settings/` — VIA commands and persistent settings.
- `modules/indicators_system/` — LEDs and the two-system layer selector.
- `via.json` — VIA v3 definition with custom controls.
- `vial.json` — Vial definition without VIA v3 custom controls.
- `docs/` — board documentation and research notes.
