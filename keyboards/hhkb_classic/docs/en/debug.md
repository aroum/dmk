**Language:** [English](debug.md) | [Русский](../ru/debug.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Debug and HID Console

DMK firmware for HHKB Classic provides real-time diagnostic logging via standard USB HID Console interface. This enables tracking Topre capacitive matrix state, measuring raw ADC values, monitoring matrix scan rates, and inspecting VIA configurator communication.

By default, all debug logging channels are **disabled** to avoid unnecessary MCU and USB bus overhead.

---

## Enabling Debug Logging

### 1. Enable HID Console in the VIA Web App
1. Open the [VIA web application](https://www.usevia.app/).
2. Click the **Settings** gear icon in the top navigation bar.
3. Toggle on **Show HID Console tab**.
4. A new **HID Console** tab will appear in the top menu bar.

### 2. Enable Logging Channels in Keyboard Settings
1. Navigate to the **Configure** tab.
2. Open **HHKB Settings** → **Debug** submenu.
3. Toggle the desired logging flags:
   - **Topre Matrix Stream** — prints state transition events (press/release) with matrix coordinates `(col, row)`, raw ADC count (`raw`), normalized value (`norm` from 0 to 1000 permille), and current key status.
   - **Calibration Monitor** — real-time progress bar and calibration stream: baseline capture, peak registration per key, completion percentage `[====>    ] X% (N/60)`, and save readiness notifications.
   - **Polling Rate Monitor** — outputs periodic reports of the actual matrix scan rate and USB task frequency (e.g. `[RATE] Matrix: 1000 Hz, USB: 500 Hz`).
   - **VIA Telemetry Diag** — prints incoming VIA/Vial requests, commands, and parameter modifications.

> [!TIP]
> You can save logs to a text file or clear the output area directly using the buttons in the upper-right corner of the **HID Console** tab in VIA.

---

## Non-linear Topre Capacitive Sensor Response

The Topre capacitive switch relies on a conical spring compressing against PCB sensing pads. The resulting capacitance change follows a distinctly non-linear response relative to physical key travel:

| Physical Key Travel | Sensor Value (% / ‰) | Mechanism Behavior & Physics |
|---|---|---|
| **0 mm** | **0%** (0 ‰) | Key at rest (baseline level); conical spring fully retracted. |
| **1 mm** | **15%** (150 ‰) | Initial rubber dome compression; slow initial capacitance growth. |
| **2 mm** | **50%** (500 ‰) | Peak tactile resistance (tactile bump). |
| **3 mm** | **80%** (800 ‰) | Tactile drop-off; spring rapidly compresses toward the PCB pads. |
| **4 mm** | **99%** (990–1000 ‰) | Bottom-out; maximum measurable capacitance (peak). |

### DMK Defaults:
- **Default Actuation Point:** **75% of travel** (750 ‰, approximately 2.8 mm). The key triggers reliably right after collapsing the tactile dome, avoiding accidental triggers on light key rests.
- **Release Hysteresis:** **0.4 mm** (100 ‰). Provides a solid deadband preventing chatter or repeated triggers near the actuation boundary.
- **Rapid Trigger:** Disabled by default. Can be enabled on demand under **HHKB Settings → Rapid Trigger**.
