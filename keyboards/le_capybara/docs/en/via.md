**Language:** [English](via.md) | [Русский](../ru/via.md)

### Travel curve

The ADC dropdowns use 1% steps: 2–94% at 1 mm, 4–96% at 2 mm, and 6–98% at 3 mm. To preview the actual lookup table and check the limits, run from the repository root:

```bash
uv run tools/ec_curve_visualizer.py
```

In **Calibration**, enable **Use travel curve** and enter the normalized ADC percentages measured at 1, 2 and 3 mm after calibrating the endpoints. The three points and both endpoints must be separated by at least 2 percentage points. Set maximum travel from 3.1 to 4.5 mm in 0.1 mm steps. Smoothing ranges from 0.0 (straight segments) to 1.0 (monotonic PCHIP) in steps of 0.1. A rejected curve falls back to the standard response. Refresh the menu to read **Curve status** after applying changes.

The active curve applies to actuation, classic hysteresis and Rapid Trigger. Requests above maximum travel are limited at runtime. With the curve disabled the standard 4.0 mm scale applies. Firmware saves the six parameters and regenerates the lookup table in RAM.

---

# [VIA](https://www.usevia.app/) & Configuration Guide

This guide covers how to load the definition file, configure layouts, tune electro-capacitive settings, and calibrate keys for the **Le Capybara** keyboard in VIA.

---

## Loading the Custom Definition

1. Open the [VIA web app](https://www.usevia.app/) (supported in Chromium-based browsers like Chrome, Edge, Brave).
2. Go to **Settings** (gear icon) and toggle on **Show Design tab**.
3. Go to the newly visible **Design** tab.
4. Click **Load Draft Definition** and select `via.json` from `keyboards/le_capybara/via.json` (or downloaded from release assets).
5. Return to the **Configure** tab and click **Authorize device** to connect to your keyboard.
6. The custom **Capybara Settings** menu will appear in the left sidebar.

> [!TIP]
> **Blank keycaps on initial connection:**
> If keycaps appear blank upon connecting, simply click **Layer 1** and then click back to **Layer 0** (`Layer 1 → Layer 0`) to trigger VIA's rendering update.

> [!NOTE]
> **Status Label Updates in VIA (Curve status, Calibration status, Settings EEPROM):**
> Dynamic status labels (such as `Curve status`, `Calibration status`, `Settings EEPROM`, and similar fields) do not continuously poll or update live in the VIA web interface. Due to the architecture of WebHID and the VIA web app, updated values for these labels are retrieved from the controller only when switching between submenus or reloading the page in the browser (F5 / Ctrl+R / Cmd+R). If you adjust curve parameters or perform key calibration, reload the VIA page or switch menus to read the updated status.

---

## Linux udev Rules for WebHID

If VIA detects the keyboard on Linux but cannot communicate with it, grant access to the `hidraw` device node:

```bash
sudo tee /etc/udev/rules.d/70-le-capybara-via.rules >/dev/null <<'EOF'
SUBSYSTEM=="hidraw", KERNEL=="hidraw*", ATTRS{idVendor}=="cafe", ATTRS{idProduct}=="4020", TAG+="uaccess"
EOF
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Reconnect the USB cable and reload the VIA page.

---

## VIA vs Vial

- **VIA v3:** Full support for the custom **Capybara Settings** interface:
  - Matrix Scan Rate (50 Hz – 1000 Hz)
  - Key Calibration and Live Progress Monitor
  - Rapid Trigger (Press / Release sensitivity, dead zone)
  - Layout options (Classique 4-key vs 36-Key Split 6-key thumb cluster)
  - Real-time diagnostic channels (EC matrix stream, polling rate monitor)
- **Vial:** Lightweight graphical keymap and macro editing. Vial does not support custom proprietary vendor menus; use VIA v3 when tuning EC capacitive and Rapid Trigger thresholds.

---

## Layout Options

Under the **LAYOUT** tab in VIA:
- **Classique (Default):** 4-key ergonomic thumb cluster with rotary encoder in the center.
- **36-Key Split:** 6-key split thumb cluster layout (3 thumb keys per half).

---

## Capybara Settings Menu Reference

The custom settings interface consists of 6 submenus:

### 1. Scan Rate & Timing
- **Target scan rate:** Operating matrix scan rate (50, 125, 250, 500, or 1000 Hz).
- **ADC oversampling (4x):** Four-times hardware ADC conversion averaging at 50–500 Hz. Automatically drops to 1x at 1000 Hz; automatically restored when rate is lowered.
- **Tapping term:** Mod-Tap and Layer-Tap hold timeout (50–500 ms in 10 ms steps, default 220 ms).
- **Permissive hold:** Instant hold activation when a second key is pressed and released during the tapping term.
- **Tap preferred:** Favors the tap keycode over the modifier/layer during fast typing.
- **Quick tap:** Quick double-tap hold interval (0–500 ms, 0 disables).

### 2. Calibration
- **Use travel curve:** Enables non-linear ADC travel curve transformation.
- **ADC at 1 mm (%), ADC at 2 mm (%), ADC at 3 mm (%):** Normalized curve anchor points (2–94%, 4–96%, 6–98%).
- **Maximum travel:** Total physical switch travel (3.1–4.5 mm in 0.1 mm increments).
- **Smoothing:** Curve smoothing factor from 0.0 (piecewise linear) to 1.0 (monotonic PCHIP spline).
- **Curve status:** RAM lookup table validation status (`Valid curve` or error description).
- **Use custom calibration:** Enables user-level sensor calibration.
- **Calibration status:** Shows active calibration state (`In progress: X/36 keys` / `Calibrated`).
- **Start calibration:** Captures resting baseline capacitance levels.
- **Print status to HID Console:** Dumps active calibration progress and peak travel metrics to the VIA HID Console.
- **Finish and save:** Validates key travels and commits calibration to persistent memory.
- **Actuation point:** Travel depth required to trigger keypress (0.1–4.0 mm, default 2.2 mm).
- **Release hysteresis:** Release buffer travel below actuation point in classic mode (0.1–4.0 mm, default 0.4 mm).
- **Settings EEPROM:** User settings storage availability status (`Writable` / `Occupied`).

### 3. Rapid Trigger (Dynamic Keystroke)
- **Enable Rapid Trigger:** Toggles continuous dynamic tracking.
- **Press Sensitivity:** Distance the switch must travel downward from any point to re-trigger (0.1–4.0 mm, default 0.4 mm).
- **Release Sensitivity:** Distance the switch must travel upward from any point to reset (0.1–4.0 mm, default 0.4 mm).
- **Top Dead Zone:** Upper dead zone protecting against unintentional actuations near key rest position (0.0–0.6 mm, default 0.2 mm).

### 4. RGB & Status Indicators (RGB / Indicators)
- **Front Indicator 1:** Mode for the first front indicator LED:
  - `0`: Off
  - `1`: Debug heartbeat (1 Hz blink)
  - `2`: Caps Lock indicator
  - `3`: Fn layer indicator
  - `4`: System 2 profile indicator
- **Front Indicator 2:** Mode for the second front indicator LED (same 5 options).

### 5. Power Saving
- **Power Saving Mode:** Deep power-saving toggle when no USB host is detected (power-only connection or USB suspend):
  - When disconnected from USB host, turns off all RGB/LED indicators, disables matrix multiplexers (EN0 `PA8` and EN1 `PC14` driven HIGH), and suspends matrix scanning.
  - When a USB host is detected, matrix scanning and lighting resume automatically.

### 6. Debug & Diagnostics
- **Firmware Version:** Displays the compiled DMK firmware build version (`DMK_FW_VERSION`).
- **EC Matrix Stream:** Real-time stream of keystroke events and travel samples `[EC]` to the HID Console.
- **VIA Telemetry Diag:** Diagnostic logging of incoming/outgoing VIA v3 packets.
- **Polling Rate Monitor:** Real-time matrix scanning frequency telemetry.
- **Calibration Monitor:** Live monitoring of baseline capacitance and tracking drift.
- **Factory reset (Arm settings reset & Erase all DMK settings):** Safe complete reset of all DMK custom settings, calibrations, and keymaps.

---

### Reset all DMK settings (Factory reset)

In the [VIA web app](https://www.usevia.app/), open **Capybara Settings** → **Debug**:
1. Enable the safety switch **Arm settings reset**.
2. Click the button **Erase all DMK settings** that appears.
3. Observe the operational state in **Reset status** (`Ready` → `Armed` → `Resetting...`).

**How reset works and what it erases:**
- **Accidental trigger protection:** The erase button is hidden and disarmed until the *Arm settings reset* toggle is engaged.
- **EEPROM configuration & calibration erase:** Firmware clears the emulation Flash sector on STM32F072 (Page 62 at `0x0801F000`), wiping custom baseline/peak capacitance calibrations, travel curve lookup tables, actuation point, Rapid Trigger sensitivities, scan rate, debug flags, and RGB indicator modes.
- **Dynamic VIA/Vial keymap reset:** All dynamic keymap layers, macros, combos, and key overrides are erased, reverting the keyboard to its default compiled-in ROM keymap.
- **Automatic reboot:** Once memory sectors are cleared, the microcontroller automatically executes a software system reset (`platform_system_reset()`) and boots up cleanly with default parameters.

## Key Calibration Process

To ensure reliable actuation across all conical spring switches:

1. Navigate to **Capybara Settings** → **Calibration**.
2. Toggle on **Use custom calibration** and make sure all keys are released.
3. Click **Start calibration** to record baseline capacitance levels.
4. Fully press and bottom out all keys across the board to measure each key's peak travel capacity.
5. Release all keys, then click **Finish and save**. A key is saved only if it had a stable baseline, moved far enough, and returned to rest. Other keys keep their previous calibration.

> [!TIP]
> **Visual Confirmation via Key Tester:**
> After clicking **Start calibration**, switch to VIA's top **Key Tester** tab and turn on **Test Matrix**. Each pressed key will illuminate on the visual layout, allowing you to clearly see which keys have been pressed and which ones remain. Once all keys are pressed, return to **Calibration** and click **Finish and save**.
