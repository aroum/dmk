**Language:** [English](via.md) | [Русский](../ru/via.md)

### Travel curve

The ADC dropdowns use 1% steps: 2–94% at 1 mm, 4–96% at 2 mm, and 6–98% at 3 mm. To preview the actual lookup table and check the limits, run from the repository root:

```bash
uv run tools/ec_curve_visualizer.py
```

In **Calibration**, enable **Use travel curve** and enter the normalized ADC percentages measured at 1, 2 and 3 mm after calibrating the endpoints. The three points and both endpoints must be separated by at least 2 percentage points. Set maximum travel from 3.1 to 4.5 mm in 0.1 mm steps. Smoothing ranges from 0.0 (straight segments) to 1.0 (monotonic PCHIP) in steps of 0.1. A rejected curve falls back to the standard response. Refresh the menu to read **Curve status** after applying changes.

The active curve applies to actuation, classic hysteresis and Rapid Trigger. Requests above maximum travel are limited at runtime. With the curve disabled the standard 4.0 mm scale applies. Firmware saves the six parameters and regenerates the lookup table in RAM.

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# [VIA](https://www.usevia.app/)

## Load the custom definition

1. Open the [VIA web app](https://www.usevia.app/).
2. Open **Settings** and enable **Show Design tab**.
3. Open the new **Design** tab, choose **Load Draft Definition**, and select `via.json` from this directory or the release assets.
4. Connect the keyboard and open **HHKB Settings**.

> [!TIP]
> **Blank keycaps (missing legends) on Layer 0:**
> If the keys on Layer 0 appear blank upon connecting to VIA, this is an asynchronous WebHID rendering race in the VIA web client. Simply click **Layer 1** and then return to **Layer 0** (`Layer 1 → Layer 0`) to immediately render all key legends from the cache.

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

[VIA](https://www.usevia.app/) supports the custom **HHKB Settings** interface: Scan Rate, Calibration, Rapid Trigger, LED, and Debug. These custom controls are available only in the VIA web app. Rapid Trigger is shown only by firmware built with `HHKB_RAPID_TRIGGER`.

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

### Key Calibration Process

1. Toggle on **Use custom calibration** and ensure all keys are released.
2. Click **Start calibration** to record baseline capacitance levels.
3. Fully press all keys you want to calibrate to record peak travel.
4. Click **Finish and save** to write calibration to EEPROM (only positions with valid travel range are updated).

> [!TIP]
> **Visual Tracking via Key Tester (Test Matrix):**
> Switching between VIA tabs **does not reset** active calibration — the state remains held in MCU RAM until *Finish and save* is clicked.
> After clicking **Start calibration**, you can navigate directly to the **Key Tester** tab in VIA's top navigation bar and toggle on **Test Matrix**. In this mode, the visual layout lights up every physically actuated switch, giving you an immediate visual confirmation of which keys have already been bottomed out and which ones remain.
> Once all keys are pressed, return to **Configure** → **HHKB Settings** → **Calibration** and click **Finish and save**.

> [!NOTE]
> **Calibration Status Counter and Real-Time Monitoring in VIA:**
> The `Calibration status` label (e.g. `In progress: 10/60 keys`) does not update automatically in real time while typing because the VIA web client does not poll custom controls continuously.
> To refresh the displayed count, click the **Refresh status** button or switch between submenus.
>
> For continuous live tracking, toggle on **Calibration Monitor** under *HHKB Settings* → *Debug* and open the **HID Console** tab (enabled in VIA settings via *Show HID Console tab*) to observe the textual progress bar and per-key travel peak updates.

## Debug

The **HHKB Settings** → **Debug** submenu provides granular toggles for diagnostic logging channels (EC Matrix Stream, Calibration Monitor, Polling Rate Monitor, VIA Telemetry Diag).

For a complete guide on configuring the VIA HID Console, inspecting real-time matrix logs, and Topre sensor response curves, see [Debugging and HID Console](debug.md).

Settings and calibration are stored in EEPROM at `0x08081500`. [VIA](https://www.usevia.app/) keymaps use the reserved EEPROM slots separately.
