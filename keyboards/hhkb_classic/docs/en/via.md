**Language:** [English](via.md) | [Русский](../ru/via.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [VIA](via.md)

# VIA

Load `via.json` through the VIA Design tab. The custom interface is grouped under **HHKB Settings**: Scan Rate, Calibration, Rapid Trigger, and LED. Rapid Trigger is shown only by firmware built with `HHKB_RAPID_TRIGGER`.

Vial uses `vial.json` and supports keymap editing. Vial does not expose VIA v3 custom controls, so use VIA for analog, LED, and scan-rate settings.

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

Settings and calibration are stored in EEPROM at `0x08081500`. VIA keymaps use the reserved EEPROM slots separately.
