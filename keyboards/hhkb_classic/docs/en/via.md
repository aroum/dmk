**Language:** [English](via.md) | [Русский](../ru/via.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [VIA](via.md)

# [VIA](https://www.usevia.app/)

## Load the custom definition

1. Open the [VIA web app](https://www.usevia.app/).
2. Open **Settings** and enable **Show Design tab**.
3. Open the new **Design** tab, choose **Load Draft Definition**, and select `via.json` from this directory or the release assets.
4. Connect the keyboard and open **HHKB Settings**.

## Chromium on Linux sees the HHKB, but VIA cannot connect

If VIA detects **HHKB Professional Classic** with VID/PID `cafe:4010` but reports that the keyboard is not responding, try granting Chromium access to its `/dev/hidraw*` devices. This rule grants the active local session user access to devices with this VID/PID.

```bash
sudo tee /etc/udev/rules.d/70-hhkb-via.rules >/dev/null <<'EOF'
SUBSYSTEM=="hidraw", KERNEL=="hidraw*", ATTRS{idVendor}=="cafe", ATTRS{idProduct}=="4010", TAG+="uaccess"
EOF
sudo udevadm control --reload-rules
```

Unplug and reconnect the keyboard's USB cable, reload VIA, and select the HHKB through **Authorize device**. Use the same `via.json`.

If the error persists, open `chrome://device-log`, try connecting through VIA again, and check for entries containing `HID` or `hidraw`.

## VIA vs Vial

[VIA](https://www.usevia.app/) supports the custom **HHKB Settings** interface: Scan Rate, Calibration, Rapid Trigger, and LED. These custom controls are available only in the VIA web app. Rapid Trigger is shown only by firmware built with `HHKB_RAPID_TRIGGER`.

Vial uses `vial.json` and supports runtime keymap editing. It does not support custom UI controls, so it cannot configure the actuation point, calibration, scan rate, LEDs, or Rapid Trigger. Use [VIA](https://www.usevia.app/) for these settings.

## Select a system

With the updated firmware and `via.json`, assign **System 1** or **System 2** in the VIA keycode picker. Through **Any**, use `0x7E00` for System 1 and `0x7E01` for System 2. System 1 uses layers 0 and 1, and System 2 uses layers 2 and 3. Place each selector where it can be reached from the other system.

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
