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

## HHKB Settings Menu Reference

The custom settings interface consists of 6 submenus:

### 1. Scan Rate & Timing
- **Target scan rate:** Matrix polling frequency — 50, 125, 250, 500, or 1000 Hz.
- **ADC oversampling (4x):** Hardware ADC 4x averaging toggle. Active at 50–500 Hz; automatically bypassed (1x) at 1000 Hz. Saved in EEPROM and automatically restored when the rate drops back to 500 Hz or lower.
- **Tapping term:** Hold-Tap and Layer-Tap timeout — 50 to 500 ms in 10 ms increments (default 220 ms).
- **Permissive hold:** Instant hold resolution when a second key is tapped and released within the tapping term.
- **Tap preferred:** Favors the tap action over the modifier/layer when rolling keys during fast typing.
- **Quick tap:** Quick double-tap hold interval (0–500 ms in 10 ms steps; 0 disables, default 150 ms).

### 2. Calibration
- **Use travel curve:** Enables non-linear ADC travel curve transformation.
- **ADC at 1 mm (%), ADC at 2 mm (%), ADC at 3 mm (%):** Normalized curve anchor points (2–94%, 4–96%, 6–98%).
- **Maximum travel:** Physical total switch travel (3.1–4.5 mm in 0.1 mm increments).
- **Smoothing:** Curve interpolation from 0.0 (piecewise linear) to 1.0 (monotonic PCHIP spline).
- **Curve status:** Status of RAM lookup table validation (`Valid curve` or error description).
- **Use custom calibration:** Toggles between PFU factory calibration thresholds and user-calibrated values.
- **Calibration status:** Shows active calibration state (`In progress: X/60 keys` / `Calibrated`).
- **Start calibration:** Captures resting baseline capacitance levels.
- **Print status to HID Console:** Dumps active calibration progress and peak travel metrics to the VIA HID Console.
- **Finish and save:** Validates key travels and commits calibration to EEPROM.
- **Actuation point:** Travel depth required to trigger keypress (0.1–4.0 mm in 0.1 mm steps).
- **Release hysteresis:** Release buffer travel below actuation point in classic mode (0.1–4.0 mm in 0.1 mm steps).
- **Settings EEPROM:** User EEPROM block availability status (`Writable` / `Occupied`).

### 3. Rapid Trigger
- **Rapid Trigger:** Toggles continuous dynamic keystroke tracking without requiring keys to return to the top.
- **RT press sensitivity:** Travel distance downward to re-actuate after release (0.1–4.0 mm in 0.1 mm steps).
- **RT release sensitivity:** Travel distance upward from lowest point to de-actuate (0.1–4.0 mm in 0.1 mm steps).
- **Top dead zone:** Upper dead zone (0.0–0.6 mm in 0.1 mm steps) where returning resets keystroke and disarms RT.

### 4. LED Indicators
- **Blue LED (PA8):** Sets behavior for the blue LED:
  - `0`: Off
  - `1`: Debug heartbeat (1 Hz blink)
  - `2`: Caps Lock indicator
  - `3`: Fn layer activity indicator
  - `4`: System 2 profile indicator
- **Orange LED (PA9):** Identical 5 modes for the orange LED.

### 5. Power Saving
- **Power Saving Mode:** Deep power-saving toggle when no USB host is present (charger-only, unmounted, or suspended):
  - When disconnected from USB host, turns off all LEDs and completely cuts power to the Topre capacitive sensor board (`PC14` and `PC7` pulled LOW) while suspending matrix scanning.
  - When a USB host connects, sensor power is restored and normal scanning resumes automatically.

### 6. Debug
- **Firmware Version:** Displays the compiled DMK firmware version.
- **EC Matrix Stream:** Streams real-time `[EC]` capacitance readings to the HID Console.
- **VIA Telemetry Diag:** Diagnostic logging of incoming/outgoing VIA v3 packets.
- **Polling Rate Monitor:** Telemetry for matrix scan and USB report frequency.
- **Calibration Monitor:** Live tracking of sensor baseline values and capacitance drift.
- **Factory reset (Arm settings reset & Erase all DMK settings):** Safe complete reset of all DMK custom settings, calibrations, and keymaps.

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
> **Status Label Updates in VIA (Curve status, Calibration status, Settings EEPROM):**
> Dynamic status labels (such as `Curve status`, `Calibration status`, `Settings EEPROM`, and similar fields) do not continuously poll or update live in the VIA web interface. Due to the architecture of WebHID and the VIA web app, updated values for these labels are retrieved from the controller only when switching between submenus or reloading the page in the browser (F5 / Ctrl+R / Cmd+R). If you adjust curve parameters or perform key calibration, reload the VIA page or switch menus to read the updated status.
>
> For continuous live tracking of calibration progress and sensor activity, toggle on **Calibration Monitor** under *HHKB Settings* → *Debug* and open the **HID Console** tab (enabled in VIA settings via *Show HID Console tab*) to observe the textual progress bar and per-key travel peak updates.

## Debug

The **HHKB Settings** → **Debug** submenu provides granular toggles for diagnostic logging channels (EC Matrix Stream, Calibration Monitor, Polling Rate Monitor, VIA Telemetry Diag).

For a complete guide on configuring the VIA HID Console, inspecting real-time matrix logs, and EC sensor response curves, see [Debugging and HID Console](debug.md).

### Reset all DMK settings (Factory reset)

In the [VIA web app](https://www.usevia.app/), open **HHKB Settings** → **Debug**:
1. Enable the safety switch **Arm settings reset**.
2. Click the button **Erase all DMK settings** that appears.
3. Observe the operational state in **Reset status** (`Ready` → `Armed` → `Resetting...`).

**How reset works and what it erases:**
- **Accidental trigger protection:** The erase button is hidden and disarmed until the *Arm settings reset* toggle is engaged.
- **EEPROM configuration & calibration erase:** Firmware clears the user EEPROM region (starting at `0x08081500`), wiping custom capacitance baseline/peak calibrations, travel curve lookup tables, actuation point, Rapid Trigger sensitivities, scan rate, debug flags, and indicator modes.
- **Dynamic VIA/Vial keymap reset:** All dynamic keymap layers, macros, combos, and key overrides stored in EEPROM are erased, reverting the keyboard to its default compiled-in ROM keymap.
- **Bootloader & factory calibration safety:** The reset operation affects **only** DMK runtime settings. The original PFU Topre factory calibration data (stored at `0x08081100`/`0x08081200`) and both firmware banks (Bank 1 recovery bootloader and Bank 2) remain untouched.
- **Automatic reboot:** Once the EEPROM regions are cleared, the microcontroller automatically executes a software system reset (`platform_system_reset()`) and boots up cleanly with default parameters.
- **DIP SW6 safety check:** DIP switch **SW6** on the rear of the keyboard must be in the **OFF** position to allow settings reset. If SW6 is ON, the reset status will show `Set SW6 to OFF` and the operation will be aborted.
