**Language:** [English](via.md) | [Русский](../ru/via.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [VIA](via.md)

# [VIA](https://www.usevia.app/)

## Load the custom definition

1. Open the [VIA web app](https://www.usevia.app/).
2. Open **Settings** and enable **Show Design tab**.
3. Open the new **Design** tab, choose **Load Draft Definition**, and select `via.json` from this directory or the release assets.
4. Connect the keyboard and open **HHKB Settings**.

## VIA vs Vial

[VIA](https://www.usevia.app/) supports the custom **HHKB Settings** interface: Scan Rate, Calibration, Rapid Trigger, and LED. These custom controls are available only in the VIA web app. Rapid Trigger is shown only by firmware built with `HHKB_RAPID_TRIGGER`.

Vial uses `vial.json` and supports runtime keymap editing. It does not support custom UI controls, so it cannot configure the actuation point, calibration, scan rate, LEDs, or Rapid Trigger. Use [VIA](https://www.usevia.app/) for these settings.

## Settings

- Scan rate: 50, 125, 250, 500, or 1000 Hz.
- ADC oversampling ×4: a toggle in Scan Rate. At 50–500 Hz it averages four hardware conversions per key; at 1000 Hz the ADC always uses ×1. The preference is saved and applies again when the rate drops to 500 Hz or less.
- Tapping term: 50–500 ms in 10 ms steps; default 220 ms.
- Quick tap: 0–500 ms in 10 ms steps; default 150 ms; zero disables it.
- Actuation, classic hysteresis, and RT press/release sensitivity: 0.1–4.0 mm in 0.1 mm steps.
- RT top dead zone: 0.0–0.6 mm in 0.1 mm steps.
- LED PA8 and PA9: Off, Debug heartbeat, Caps Lock, Fn layer, or System 2.

The millimetre scale maps a calibrated signal range to a nominal 4 mm key travel. It is a configuration scale rather than a mechanical measurement.

Calibration Start captures released baselines. Fully press the keys to update, then use Finish and save. Only positions with a valid signal range are changed.

Settings and calibration are stored in EEPROM at `0x08081500`. [VIA](https://www.usevia.app/) keymaps use the reserved EEPROM slots separately.
