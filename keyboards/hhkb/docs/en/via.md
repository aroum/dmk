**Language:** [English](via.md) | [Русский](../ru/via.md)

### Hybrid battery and Bluetooth LEDs

In the **Bluetooth** menu, enable the orange battery display, then click **Show battery now** or assign **BT Battery** to a key. One, two, three, or four flashes indicate 0%, 5%, 15%, or above 15%, respectively. The 100% value means all three factory low-battery thresholds are inactive; it is not a precise charge measurement. Separate toggles enable an orange low-battery warning (one flash every 30 seconds at 15%; two every 15 seconds at 5% or 0%), blue blinking while pairing (2 Hz pending, 4 Hz advertising), and 1–4 blue flashes when a profile is selected or connected. These settings are saved in EEPROM and need validation on Hybrid hardware.

**PC5 short press** (0–0.5 seconds) can show the connected profile (default), show battery level, sleep, select the next Bluetooth profile, or switch between the last Bluetooth profile and USB. **PC5 long press** can turn power off (default), sleep, or pair the last Bluetooth profile. A one-second hold while OFF always powers on. The same actions have assignable **BT Power**, **BT Sleep**, **BT Pair Last**, **BT Profile**, **BT Battery**, **BT Next**, and **BT / USB** keycodes. The default inactivity timeout is 30 minutes, or at most 10 minutes during pairing. On battery, OFF and sleep without matrix wake enter STM32 STOP and stop the nRF. PC5 or USB insertion resets the controller to wake it. With matrix wake enabled, sleep samples the matrix at 1 Hz instead. When USB is absent, selecting USB is deferred until it is connected. The STOP path needs validation on Hybrid hardware.

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
3. Open the new **Design** tab, choose **Load Draft Definition**, and select `via_classic.json` for Classic or `via_hybrid.json` for Hybrid from the keyboard directory or release assets.
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

Unplug and reconnect the keyboard's USB cable, reload VIA, and select the HHKB through **Authorize device**. Use the same `via_classic.json`.

If the error persists, open `chrome://device-log`, try connecting through VIA again, and check for entries containing `HID` or `hidraw`.

## VIA vs Vial

[VIA](https://www.usevia.app/) supports the custom **HHKB Settings** interface: Scan Rate, Calibration, Rapid Trigger, LED, and Debug. These custom controls are available only in the VIA web app. Rapid Trigger is shown only by firmware built with `HHKB_RAPID_TRIGGER`.

Vial uses `vial_classic.json` for Classic and `vial_hybrid.json` for Hybrid and supports runtime keymap editing. It does not support custom UI controls, so it cannot configure the actuation point, calibration, scan rate, LEDs, or Rapid Trigger. Use [VIA](https://www.usevia.app/) for these settings.

## Select a system

With the updated firmware and `via_classic.json`, assign **System 1** or **System 2** in the VIA keycode picker. Through **Any**, use `0x7E00` for System 1 and `0x7E01` for System 2. System 1 uses layers 0 and 1, and System 2 uses layers 2 and 3. Place each selector where it can be reached from the other system.

## HHKB Settings Menu Reference

The custom settings interface consists of 6 submenus:

### 1. Scan Rate & Timing
- **Target scan rate:** Matrix polling frequency — 50, 125, 250, 500, or 1000 Hz.
- **ADC oversampling (4x):** Hardware ADC 4x averaging toggle. Active at 50–500 Hz; automatically bypassed (1x) at 1000 Hz. Saved in EEPROM and automatically restored when the rate drops back to 500 Hz or lower.
- **Tapping term:** Hold-Tap and Layer-Tap timeout — 50 to 500 ms in 10 ms increments (default 220 ms).
- **Permissive hold:** Instant hold resolution when a second key is tapped and released within the tapping term.
- **Tap preferred:** Favors the tap action over the modifier/layer when rolling keys during fast typing.
- **Quick tap:** Quick double-tap hold interval (0–500 ms in 10 ms steps; 0 disables, default 150 ms).

### 2. Calibration and 5-Point PCHIP Travel Curve
Topre electro-capacitive switches exhibit a non-linear capacitance response curve: capacitance grows slowly during initial elastomer dome compression, then rises steeply once the dome collapses (tactile bump) around 2 mm. To accurately convert raw ADC values into linear physical travel distances, DMK implements a **5-point monotonic piecewise cubic Hermite interpolating polynomial (PCHIP)** with adjustable smoothing:
- **5 Anchor Points:**
  1. $0.0\text{ mm}$ $\rightarrow$ $0\%$ ADC signal (released key at rest, baseline).
  2. $1.0\text{ mm}$ $\rightarrow$ **ADC at 1 mm (%)** point (range 2–94%).
  3. $2.0\text{ mm}$ $\rightarrow$ **ADC at 2 mm (%)** point (range 4–96%).
  4. $3.0\text{ mm}$ $\rightarrow$ **ADC at 3 mm (%)** point (range 6–98%).
  5. **Maximum travel** $\rightarrow$ $100\%$ ADC signal (fully bottomed-out key, peak, range 3.1–4.5 mm).
- **Mathematics & PCHIP Algorithm:**
  - Slopes are computed between neighboring points while strictly preserving monotonicity: the generated curve is guaranteed never to overshoot or oscillate, and will never decrease during downward key travel.
  - **Smoothing** parameter (range $0.0$ to $1.0$): controls transition curvature. At $0.0$, the response is piecewise linear; at $1.0$, it is a smooth monotonic PCHIP curve.
  - **Steepness Guard:** The algorithm checks the maximum derivative. If slope exceeds 4x gain (more than 32 travel units per ADC quant), the curve is safely rejected with status `Curve too steep (max 4x)` and firmware falls back to standard linear travel response.
  - **Actuation Point vs. Maximum Travel Guard:** Firmware enforces an absolute safety clamp in runtime scanning: if the configured actuation point or Rapid Trigger press/release threshold exceeds the maximum travel depth (`max_travel = curve[125]`), firmware clamps it to `max_travel`:
    ```c
    uint16_t max_travel = curve ? curve[125] : 1000;
    if (scan_cfg.act_pm > max_travel) scan_cfg.act_pm = max_travel;
    ```
    This guarantees that key actuation can never be configured beyond the physically possible travel depth of the switch.

To preview and interactively tune the 5-point curve from the repository root, run the visualizer via `uv`:
```bash
uv run tools/ec_curve_visualizer.py
```

**Calibration Submenu Controls:**
- **Use travel curve:** Enables the non-linear 5-point PCHIP curve.
- **ADC at 1 mm (%), ADC at 2 mm (%), ADC at 3 mm (%):** Normalized intermediate anchor points. Points must have at least 2% spacing.
- **Maximum travel:** Physical switch bottom-out depth (3.1–4.5 mm in 0.1 mm increments).
- **Smoothing:** Curve smoothing factor (0.0 to 1.0).
- **Curve status:** RAM lookup table status (`Active`, `Disabled`, `Points need 2% spacing`, `Curve too steep (max 4x)`).
- **Use custom calibration:** Selects between PFU factory calibration constants and user calibration.
- **Calibration status:** Displays calibration progress.
- **Start calibration:** Begins baseline capacitance capture (16 matrix scan cycles).
- **Print status to HID Console:** Dumps active calibration progress and peak metrics to VIA HID Console.
- **Finish and save:** Validates and commits calibration to EEPROM.
- **Actuation point:** Actuation depth (0.1–4.0 mm in 0.1 mm steps).
- **Release hysteresis:** Classic mode release buffer (0.1–4.0 mm in 0.1 mm steps).
- **Settings EEPROM:** Availability of EEPROM storage (`Writable` / `Occupied`).

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
2. Click **Start calibration** to record baseline resting capacitance levels. The baseline capture takes exactly **16 consecutive matrix scan cycles** (taking only 16 ms at a 1000 Hz scan rate; with 4x oversampling enabled, each sample hardware-averages 4 ADC conversions). The accumulated sums are divided by 16 and saved as the zero-travel reference for each individual switch.
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

## Debugging and HID Console

The **HHKB Settings** → **Debug** submenu provides granular toggles for diagnostic logging channels. By default, all diagnostic output is disabled to prevent unnecessary MCU and USB bus traffic.

### 1. Enabling the HID Console Tab in VIA
1. Open VIA settings by clicking the top gear icon (**Settings**).
2. Enable **Show HID Console tab**.
3. A new **HID Console** tab will appear in the top navigation bar.

### 2. Logging Channels and Output Formats
- **EC Matrix Stream:** Real-time stream of capacitive sensor samples `[EC]`.
  - **Message Format:**
    ```text
    [EC] R<row> C<col> : raw=<raw> base=<base> peak=<peak> travel=<norm/10>% <DN|UP>
    ```
  - **Field Explanations:**
    - `R<row> C<col>` — Matrix coordinates of the switch row and column.
    - `raw=<raw>` — Current 12-bit raw ADC reading (0–4095).
    - `base=<base>` — Resting capacitance baseline captured during calibration.
    - `peak=<peak>` — Peak capacitance reading at full bottom-out.
    - `travel=<norm/10>%` — Computed physical key travel percentage (0–100%) normalized against the PCHIP curve.
    - `DN` / `UP` — Binary actuation state (`DN` = pressed, `UP` = released).
  - **Quiet Threshold Filtering:** To avoid flooding the console and USB buffers, intermediate travel updates are sent only when the raw ADC signal changes by more than **±40 units**, or instantly upon any transition (`DN` $\leftrightarrow$ `UP`).

- **Calibration Monitor:** Live step-by-step progress logging for key calibration. Outputs baseline capture completion, real-time peak updates per key (`[CAL] Key C3 R2 peak updated: 2650 -> 2890`), a visual progress bar `[====>    ] X% (N/60)`, and ready-to-save notification.

- **Polling Rate Monitor:** Periodic telemetry logging actual matrix scan and USB loop frequencies (e.g. `[RATE] matrix=1000 Hz (target: 1000 Hz), usb_loop=500 Hz`).
  > [!NOTE]
  > **Matrix Scan Rate (`matrix`) vs USB Task Loop (`usb_loop`):**
  > - `matrix`: The actual physical hardware scan frequency of the capacitive matrix. At 1000 Hz, scanning executes strictly every 1 ms under FreeRTOS timing.
  > - `usb_loop`: The iteration frequency of the background USB service task. The task sleeps on `ulTaskNotifyTake(..., pdMS_TO_TICKS(2))` with a **2 ms timeout**. When there is no active typing or streaming debug traffic, it wakes up on the 2 ms timeout (~500 Hz) to conserve CPU cycles and power. As soon as keystrokes or data packets arrive, it wakes up instantly on task notification without latency at full 1000 Hz USB polling (`bInterval=1`). Seeing `usb_loop=500 Hz` during idle periods is expected and indicates healthy power-efficient scheduling.

- **VIA Telemetry Diag:** Diagnostic logging of incoming and outgoing VIA v3 protocol packets (`[VIA_DIAG] rx=... tx_sub=...`).

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

## Hybrid only: Bluetooth controls and keycodes

Assign these after loading [via_hybrid.json](../../via_hybrid.json) in VIA:

- **BT Select 1–4** (`0x7E02`–`0x7E05`): select an existing bond.
- **BT Pair 1–4** (`0x7E06`–`0x7E09`): pair in the chosen slot.
- **BT Battery** (`0x7E0A`): show battery level on the orange LED.
- **BT Power** (`0x7E0B`): enter battery OFF/STOP; wake with PC5 or USB.
- **BT Sleep** (`0x7E0C`): enter battery sleep.
- **BT Pair Last** (`0x7E0D`): pair the last profile.
- **BT Profile** (`0x7E0E`): show the profile number in blue.
- **BT Next** (`0x7E0F`): select the next Bluetooth profile.
- **BT / USB** (`0x7E10`): switch between the last Bluetooth profile and USB.

**System 1** and **System 2** (`0x7E00`, `0x7E01`) are also present on Classic: they choose layer pairs, not Bluetooth profiles.

### VIA settings unique to Hybrid

In the [VIA web app](https://www.usevia.app/), open **Settings → Show Design tab → Design → Load Draft Definition** and choose [via_hybrid.json](../../via_hybrid.json). The **HHKB Settings → Bluetooth** submenu appears only in a Hybrid build. Vial supports keymap editing but cannot display custom controls.

- **Battery matrix scan rate:** 50, 125, 200 (default), 250, 500, or 1000 Hz, stored separately from the USB rate.
- **Sleep timeout:** 0–60 minutes in one-minute steps; `0` disables the inactivity timer. Pairing caps longer timeouts at 10 minutes.
- **Wake with matrix:** off by default. Off selects STOP; on scans at 1 Hz during sleep.
- **Select paired profile / Start pairing:** choose profiles 1–4; pairing may replace a bond.
- **Show battery level on orange LED / Show battery now:** enable and trigger battery flashes.
- **Blink orange on low battery:** once every 30 seconds at 15%; twice every 15 seconds at 5% or 0%.
- **Blink blue while pairing:** 2 Hz while pending, 4 Hz while advertising.
- **Blink blue for active profile:** one to four flashes for profiles 1–4.
- **Battery level (%):** reports 0, 5, 15, or 100. `100` only means all low-battery threshold inputs are inactive.
- **PC5 short press / PC5 long press:** choose independent button actions; the choices and last profile are saved in EEPROM.

PB9, PB8, and PB7 provide the 0%, 5%, and 15% battery thresholds. DMK samples them once per second and sends stable changes to the stock nRF for the BLE Battery Service. One to four manual orange flashes indicate 0%, 5%, 15%, or above 15%. These are threshold bands, not precise charge measurements. Status patterns temporarily take priority over configured base LED modes.
