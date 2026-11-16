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

## Capybara Settings Menu

### 1. Actuation & Hysteresis
- **Actuation Point:** Sets the travel depth required to register a keypress (0.1 mm – 4.0 mm, default 2.2 mm).
- **Classic Hysteresis:** Sets the release distance buffer required before a key de-actuates (0.1 mm – 4.0 mm, default 0.4 mm).

### 2. Rapid Trigger (Dynamic Keystroke)
- **Enable Rapid Trigger:** Toggles continuous dynamic tracking.
- **Press Sensitivity:** Distance the switch must travel downward from any point to re-trigger (0.1 mm – 4.0 mm, default 0.4 mm).
- **Release Sensitivity:** Distance the switch must travel upward from any point to reset (0.1 mm – 4.0 mm, default 0.4 mm).
- **Top Dead Zone:** Protects against unintentional actuations near key rest position (0.0 mm – 0.6 mm, default 0.2 mm).

### 3. Scan Rate & Timing
- **Scan Rate:** Choose between 50, 125, 250, 500, or 1000 Hz.
- **Tapping Term:** Configures Mod-Tap and Layer-Tap timeout (50 ms – 500 ms).
- **Quick Tap:** Configures quick double-tap hold repeat (0 ms – 500 ms).

### 4. Debug & Diagnostics
- **Firmware Version:** Dynamically queries and displays the exact runtime firmware build version (`DMK_FW_VERSION`, e.g. `26.11.15` or `26.11.15-b8c21fe`) directly from controller memory over VIA protocol (channel 0, ID 28).
- **EC Matrix Stream:** Logs press and release transitions immediately, then reports travel changes of the most recently pressed key while it moves. Changes smaller than 40 ADC counts are suppressed to limit console traffic. Each `[EC]` line includes matrix coordinates, the raw ADC sample, and the travel value used by the firmware.
- **VIA Telemetry Diag:** Diagnostic logging of VIA communication packets.
- **Polling Rate Monitor:** Real-time matrix scanning frequency telemetry.
- **Calibration Monitor:** Live monitoring of baseline capacitance and tracking drift.

---

### Reset all DMK settings

In the [VIA web app](https://www.usevia.app/), open **Debug**, enable **Arm settings reset**, and click **Erase all DMK settings**. The keyboard restarts with the default layout, calibration, and settings. Custom keymaps, macros, and combos are erased.

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
