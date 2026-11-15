**Language:** [English](via.md) | [Русский](../ru/via.md)

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
  - Real-time diagnostic channels (Topre matrix stream, polling rate monitor)
- **Vial:** Lightweight graphical keymap and macro editing. Vial does not support custom proprietary vendor menus; use VIA v3 when tuning Topre capacitive and Rapid Trigger thresholds.

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

---

## Key Calibration Process

To ensure reliable actuation across all conical spring switches:

1. Navigate to **Capybara Settings** → **Calibration**.
2. Toggle on **Use custom calibration** and make sure all keys are released.
3. Click **Start calibration** to record baseline capacitance levels.
4. Fully press and bottom out all keys across the board to measure each key's peak travel capacity.
5. Click **Finish and save** to persist the calibrated values into MCU Flash memory (Page 62).

> [!TIP]
> **Visual Confirmation via Key Tester:**
> After clicking **Start calibration**, switch to VIA's top **Key Tester** tab and turn on **Test Matrix**. Each pressed key will illuminate on the visual layout, allowing you to clearly see which keys have been pressed and which ones remain. Once all keys are pressed, return to **Calibration** and click **Finish and save**.
