**Language:** [English](via-debug.md) | [Русский](../ru/via-debug.md)

### Hybrid battery and Bluetooth LEDs

In the **Bluetooth** menu, enable the orange battery display, then click **Show battery now** or assign **BT Battery** to a key. One, two, three, or four flashes indicate 0%, 5%, 15%, or above 15%, respectively. The 100% value means all three factory low-battery thresholds are inactive; it is not a precise charge measurement. Separate toggles enable an orange low-battery warning (one flash every 30 seconds at 15%; two every 15 seconds at 5% or 0%), blue blinking while pairing (2 Hz pending, 4 Hz advertising), and 1–4 blue flashes when a profile is selected or connected. These settings are saved in EEPROM.

**PC5 short press** (0–0.5 seconds) can show the connected profile (default), show battery level, sleep, select the next Bluetooth profile, or switch the last Bluetooth profile/USB. **PC5 long press** (2 seconds) can turn power off (default), sleep, or pair the last profile. With matrix wake disabled, sleep requests enter OFF directly. With it enabled, sleep requests Nordic idle and probes the matrix at 1–10 Hz; PC5 release after 0.1–2 seconds wakes, or a 2-second hold performs the configured long action directly. The Power off after sleep timer enters ordinary OFF: Nordic idle handshake, then STM32 STOP without holding PD2 high. PC5 hold of at least about 1 second followed by release resumes without STM32 reset; a 10-second hold cancels wake. USB insertion wakes ordinary OFF by reset. Only critical-battery STOP masks PC5 and requires USB to wake by reset. These two STOP paths must not be confused. When USB is absent, selecting USB is deferred until cable connection.

### Travel curve

The ADC dropdowns use 1% steps: 2–94% at 1 mm, 4–96% at 2 mm, and 6–98% at 3 mm. To preview the actual lookup table and check the limits, run from the repository root:

```bash
uv run tools/ec_curve_visualizer.py
```

In **Calibration**, enable **Use travel curve** and enter the normalized ADC percentages measured at 1, 2 and 3 mm after calibrating the endpoints. The three points and both endpoints must be separated by at least 2 percentage points. Set maximum travel from 3.1 to 4.5 mm in 0.1 mm steps. Smoothing ranges from 0.0 (straight segments) to 1.0 (monotonic PCHIP) in steps of 0.1. A rejected curve falls back to the standard response. Refresh the menu to read **Curve status** after applying changes.

The active curve applies to actuation, classic hysteresis and Rapid Trigger. Requests above maximum travel are limited at runtime. With the curve disabled the standard 4.0 mm scale applies. Firmware saves the six parameters and regenerates the lookup table in RAM.

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA & Debug](via-debug.md)

# VIA & Debug

## Load the custom definition

1. Open the [VIA web app](https://www.usevia.app/).
2. Open **Settings** and enable **Show Design tab**.
3. Open the new **Design** tab, choose **Load Draft Definition**, and select `via.json` from the keyboard directory or release assets.
4. Connect the keyboard and open **HHKB Settings**.

All variants appear as **HHKB Professional** (`cafe:4014`). The firmware reports the physical ANSI or JIS layout to VIA and Vial. After updating from an older build, load the shared `via.json` and authorize the new USB identity.

> [!TIP]
> **Blank keycaps (missing legends) on Layer 0:**
> If the keys on Layer 0 appear blank upon connecting to VIA, this is an asynchronous WebHID rendering race in the VIA web client. Simply click **Layer 1** and then return to **Layer 0** (`Layer 1 → Layer 0`) to immediately render all key legends from the cache.

## Check USB detection in Chrome

Use a USB data cable for this check, even on Hybrid. These pages inspect USB enumeration; a Bluetooth connection alone will not show the DMK USB identity. Type each address directly into Chrome's address bar:

1. Open `chrome://device-log`, then unplug and reconnect the keyboard. Look for the new USB/HID connection and any associated errors. If investigating a VIA connection failure, leave this page open while retrying **Authorize device** in the [VIA web app](https://www.usevia.app/).
2. Open `chrome://usb-internals` and find the keyboard among the connected USB devices. Inspect its product name, Vendor ID and Product ID. Refresh the device list or reopen the page after reconnecting if necessary.

For release **2026.11.22**, all supported DMK HHKB variants identify as **HHKB Professional**, VID `0xCAFE`, PID `0x4014` (`CAFE:4014`; case is irrelevant). Seeing this identity confirms that DMK has enumerated over USB, but does not by itself prove that typing or VIA communication works.

The **HHKB Keymap Tool does not recognize running DMK** because DMK uses different USB identifiers. This is expected. The factory firmware or Bank 1 recovery interface uses VID `0x04FE`; its PID and product name depend on the model and mode. Seeing `04FE` alone does not distinguish stock operation from recovery. Follow the [installation and SW6 recovery instructions](install.md) when returning to Bank 1 for an update.

If the keyboard is missing from Chrome's list, also inspect USB devices with your operating system's tools: **System Information → USB** on macOS, **Device Manager → device Properties → Details → Hardware IDs** on Windows, or `lsusb` on Linux. Try a known USB data cable and another port before assuming the firmware failed to install. If the OS sees DMK but VIA cannot connect, check browser authorization and, on Linux, the hidraw permissions described below.

For a support report, include your operating system, keyboard model, firmware version if available, the displayed product name and VID/PID, and relevant USB/HID log entries. State whether the device appears after reconnecting and whether typing works.

## Chromium on Linux sees the HHKB, but VIA cannot connect

If VIA detects **HHKB Professional** with VID/PID `cafe:4014` but reports that the keyboard is not responding, try granting Chromium access to its `/dev/hidraw*` devices. This rule grants the active local session user access to devices with this VID/PID.

```bash
sudo tee /etc/udev/rules.d/70-hhkb-via.rules >/dev/null <<'EOF'
SUBSYSTEM=="hidraw", KERNEL=="hidraw*", ATTRS{idVendor}=="cafe", ATTRS{idProduct}=="4014", TAG+="uaccess"
EOF
sudo udevadm control --reload-rules
```

Unplug and reconnect the keyboard's USB cable, reload VIA, and select the HHKB through **Authorize device**. Use the same `via.json`.

If the error persists, open `chrome://device-log`, try connecting through VIA again, and check for entries containing `HID` or `hidraw`.

## VIA vs Vial

Vial uses `vial.json` and supports runtime keymap editing. It does not support custom UI controls, so it cannot configure the actuation point, calibration, scan rate, LEDs, or Rapid Trigger. Use [VIA](https://www.usevia.app/) for these settings.

## Select a system

With the updated firmware and `via.json`, assign **System 1** or **System 2** in the VIA keycode picker. Through **Any**, use `0x7E00` for System 1 and `0x7E01` for System 2. System 1 uses layers 0 and 1, and System 2 uses layers 2 and 3. Place each selector where it can be reached from the other system.

## HHKB Settings Menu Reference

The custom settings interface consists of 6 submenus:

### 1. Scan Rate & Timing

- **Target scan rate:** Matrix polling frequency — 50, 125, 250, 500, or 1000 Hz when operating over USB.
  > [!NOTE]
  > **Bluetooth Polling Rate Limitation (Hybrid):** Over Bluetooth LE, HID report throughput is bounded by the negotiated Connection Interval (typically ~50 Hz / 20 ms). Adjusting Target scan rate in VIA is intended for USB operation and should be locked out in firmware while connected over Bluetooth.
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

From **Start calibration** until **Finish and save**, physical keys remain visible in **Key Tester → Test Matrix**, but their presses are not sent as typing or keymap actions to USB or Bluetooth. Release all keys before starting and before finishing. Normal input resumes after Finish and save.

- **Print status to HID Console:** Dumps active calibration progress and peak metrics to VIA HID Console.
- **Finish and save:** Validates and commits calibration to EEPROM.

Outside an explicit calibration session, baseline and peak remain fixed: runtime ADC spikes do not expand the saved range. Recalibrate after changing domes, silencing rings or key travel.

- **Actuation point:** Actuation depth (0.1–4.0 mm in 0.1 mm steps).
- **Release hysteresis:** Classic mode release buffer (0.1–4.0 mm in 0.1 mm steps).
- **Settings EEPROM:** Compressed storage status: `Pending`, `Saving (compressed)`, `Saved (compressed)`, `Occupied: not saved`, `Full: not saved`, or a write error. See [compressed EEPROM storage](#compressed-eeprom-storage).

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

- **Firmware Version:** Displays the compiled DMK firmware version. Release 2026.11.22 reports `2026.11.22` here; HFB versions such as A4.50 are separate.
- **EC Matrix Stream:** Streams real-time `[EC]` capacitance readings to the HID Console.
- **VIA Telemetry Diag:** Diagnostic logging of incoming/outgoing VIA v3 packets.
- **Polling Rate Monitor:** Telemetry for matrix scan and USB report frequency.
- **Calibration Monitor:** Live tracking of sensor baseline values and capacitance drift.
- **Reset and stock recovery:** reset DMK data or return to Bank 1, with optional DMK EEPROM cleanup.

### Small readings near the released position

A reading around **5% travel when lightly resting a finger on a key or wiggling its keycap is normal** and does not, by itself, indicate a calibration problem. EC sensors detect small movements before the key reaches its actuation point; `UP` means the key is not logically pressed. The EC stream normally reports movement from about 5% travel, so the first visible value is not necessarily the resting value.

Check with both hands completely removed from the keyboard. If the readings stop, as in the reported test, this behavior is expected; silence does not prove an exact 0% reading. Persistent elevated readings with untouched keys, unintended presses, or changing calibration values after a restart need separate investigation. Keep every key fully released while capturing the baseline.

### Key Calibration Process

1. Toggle on **Use custom calibration** and ensure all keys are released.
2. Click **Start calibration** to record baseline resting capacitance levels. The baseline capture takes exactly **16 consecutive matrix scan cycles** (taking only 16 ms at a 1000 Hz scan rate; with 4x oversampling enabled, each sample hardware-averages 4 ADC conversions). The accumulated sums are divided by 16 and saved as the zero-travel reference for each individual switch.
3. Fully press all keys you want to calibrate to record peak travel.
4. Click **Finish and save** to write calibration to EEPROM (only positions with valid travel range are updated).

> [!TIP]
> **Visual Tracking via Key Tester (Test Matrix):**
> Switching between VIA tabs **does not reset** active calibration — the state remains held in MCU RAM until _Finish and save_ is clicked.
> After clicking **Start calibration**, you can navigate directly to the **Key Tester** tab in VIA's top navigation bar and toggle on **Test Matrix**. In this mode, the visual layout lights up every physically actuated switch, giving you an immediate visual confirmation of which keys have already been bottomed out and which ones remain.
> Once all keys are pressed, return to **Configure** → **HHKB Settings** → **Calibration** and click **Finish and save**.

> [!NOTE]
> **Status Label Updates in VIA (Curve status, Calibration status, Settings EEPROM):**
> Dynamic status labels (such as `Curve status`, `Calibration status`, `Settings EEPROM`, and similar fields) do not continuously poll or update live in the VIA web interface. Due to the architecture of WebHID and the VIA web app, updated values for these labels are retrieved from the controller only when switching between submenus or reloading the page in the browser (F5 / Ctrl+R / Cmd+R). If you adjust curve parameters or perform key calibration, reload the VIA page or switch menus to read the updated status.
>
> For continuous live tracking of calibration progress and sensor activity, toggle on **Calibration Monitor** under _HHKB Settings_ → _Debug_ and open the **HID Console** tab (enabled in VIA settings via _Show HID Console tab_) to observe the textual progress bar and per-key travel peak updates.

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
  >
  > - `matrix`: Measured full-matrix scan rate. Selecting 1000 Hz requests a 1 ms period; actual throughput depends on scan duration and load. Use detailed scan diagnostics to verify deadlines.
  > - `usb_loop`: USB service-task iterations, not the USB HID polling rate. An idle HHKB waits for notifications with a one-second heartbeat; pending EEPROM work uses shorter waits. Input queues and USB events wake the task immediately. The HID endpoint still advertises a 1 ms interval (`bInterval=1`).

- **VIA Telemetry Diag:** Diagnostic logging of incoming and outgoing VIA v3 protocol packets (`[VIA_DIAG] rx=... tx_sub=...`).

### Detailed scan timing (ANSI and JIS)

In **Actions → Build Firmware → Run workflow**, select the `dev` branch and enable **hhkb_scan_diagnostics**. This builds only HHKB variants with `HHKB_SCAN_DIAGNOSTICS=1`; artifact names end in `scan-debug`. Classic artifacts contain ready HFB containers. Hybrid artifacts contain STM32 binaries for local patching with the included HTML tool. Ordinary builds keep detailed scan diagnostics disabled.

After flashing, enable HID Console and **Debug → Polling Rate Monitor** in VIA. At 1000 Hz, watch `[SCAN]`:

- `raw` / `max`: current and maximum duration of acquisition plus key processing.
- `work_max`: maximum complete scan-and-publication duration, in microseconds. It should remain below 1000 µs with headroom.
- `miss`: timer deadlines missed since the previous diagnostic report; it should remain zero in steady operation.
- `q_full`: failed matrix-event queue submissions.

Compare ANSI/JIS at the same settings: RT off/on, curve off/on, simultaneous key presses, and EC Matrix Stream off/on. First calibrate, finish calibration, then leave settings unchanged for a few seconds before recording results. Threshold-cache rebuilding after calibration/settings/curve changes is one-time work and can produce a transient timing spike. USB polling remains up to 1000 Hz; battery scanning retains its stock-style interval.

The scanner caches exact raw ADC press/release thresholds in classic mode, computes normalized travel once in RT mode, and reuses it for debug output. Previous-key processing overlaps C57 discharge with interrupts enabled. Discharge/stabilization minimum delays are unchanged, and the final key is processed in the same pass. This pipeline still needs waveform and timing verification on physical JIS hardware; compilation alone does not establish 1000 Hz headroom.

### Grave Escape

In VIA → Any, assign `KC_GESC` or `0x7C16` to a key. VIA protocol 12 uses this name for QMK `QK_GESC` / `QK_GRAVE_ESCAPE`. DMK configures the output separately for each modifier. With no modifiers, the key sends Escape. With Alt, Ctrl, GUI or Shift held, that modifier's toggle selects Grave when off and Escape when on. Modifiers are preserved, so Shift + Grave normally produces `~` on a US layout, and Ctrl/Alt/GUI + Grave are sent as shortcuts. The OS layout and application settings determine the resulting character or action.

The separate **HHKB Settings → Grave Escape** tab has four toggles, **Always Escape with Alt / Ctrl / GUI / Shift**. All default off. Each toggle applies to both sides of the modifier. When multiple modifiers are held, Escape takes priority if any active modifier's toggle is on. For example, with Ctrl off and Shift on, Ctrl + GESC sends Ctrl + Grave, while Ctrl + Shift + GESC sends Ctrl + Shift + Escape.

Physical modifiers, held HT modifiers, One-Shot modifiers and active `MK(...)` modifiers are considered. Choose the output on press and release that same output even if modifiers or settings change while held. Multiple GESC bindings share one held action until all are released. The overrides persist in EEPROM without increasing the record size or erasing existing calibration/keymaps. Reload the updated `via.json` to see the tab. The behavior is supplied by the HHKB module on Classic/Hybrid, ANSI/JIS, over USB or Hybrid Bluetooth.

### Calibration and bottom-out pressure fixes (dev)

Calibration accepts resting baselines from 20 to 2000 ADC counts, including switches below 100 counts. Finish validates the same baseline and peak bounds used at load time; an invalid capture leaves that key's previous range unchanged. The ADC peak must be at most 4095 and exceed baseline by more than 150 counts. The EEPROM format is unchanged.

During calibration, acquisition stores one frame (120 bytes for ANSI, 150 for JIS). Baseline/peak bookkeeping runs after acquisition with the OpAmp off, instead of extending each inter-key interval. This does not guarantee calibration meets a 1 ms deadline: check diagnostics on hardware. Calibration-rate changes and the fork's special inter-scan OpAmp policy are not included.

Use **Print status to HID Console** to list `[CAL] missing:` positions during calibration or `[CAL] unchanged:` positions afterwards. These lists use physical keys only, including on JIS. Release every key before starting; press each to bottom-out with normal force, rather than squeezing harder after it stops moving. Check the missing list before Finish. Coordinates refer to matrix positions, not the current layer's keycodes.

RT release tracking is capped at the calibrated bottom (or the travel curve's endpoint), so extra pressure above peak does not deepen the release anchor. This is a zero-width bottom dead zone, not a configurable 0.5 mm zone. Per-key RT profiles, per-key actuation and an adjustable bottom zone from [issue #11](https://github.com/aroum/dmk/issues/11) are not included. The calibration follow-up is tracked in [issue #12](https://github.com/aroum/dmk/issues/12).

### USB report diagnostics

Enable **Debug → USB Report Diagnostics** for one `[USB]` line per second while USB is connected and active. This toggle defaults off and is saved with the other debug flags. Counters run from boot independently of logging and are cumulative (uint32 rollover); compare their differences over the same A/B capture duration. Keyboard and consumer reports on the main HID interface are included; VIA and console traffic are excluded.

- `sub`: reports accepted for transmission by TinyUSB.
- `done`: HID IN transfer completions reported by TinyUSB. A completion does not prove the application processed the event.
- `busy`: a send was rejected despite the endpoint initially appearing ready; the report remains queued for retry. This is not a dropped report or a USB NAK count.
- `drop`: reports abandoned after the bounded enqueue retries while USB was mounted and active. This can lose a short tap even though a later state resync succeeds.
- `resync`: overflow-recovery snapshots scheduled after the report queue drained. Ordinary reconnect resends are not counted here.
- `q`: current report queue depth when sampled.

`sub` and `done` may differ while a transfer is in flight or after disconnect. No suspend/disconnect event lines are printed, and these events are not counted as enqueue drops. The counters are firmware report diagnostics, not bus packet-loss measurements. Recording HID Console traffic itself can affect timing; compare runs with identical logging enabled.

### Session-only USB phase control

USB SOF phase lock is enabled by default through `HHKB_SOF_LOCK=1`. For timing comparisons, build with `HHKB_SOF_LOCK=0`. See the [Classic ANSI measurements and JIS timing headroom](config.md#usb-sof-measurements-on-classic-ansi). VIA debug controls, including USB diagnostics, remain available.

For comparisons, use a scan-diagnostic build and **Debug → Polling Rate Monitor**. `[SCAN] actual` is matrix throughput, `work_max` is worst scan/publication duration, `miss` counts missed scan deadlines, and `q_full` counts failed matrix-queue submissions. The `usb`/`usb_loop` value is USB task-loop frequency, not USB polling/report rate. `[LAT] lock=0` means phase lock is disabled or not acquired; its age/error estimates still require USB-trace validation. `[VIA_DIAG] tx_fail` concerns VIA responses, not keyboard reports. These counters do not measure lost USB packets or prove end-to-end delivery; use a USB trace for bus-level failures. Keep console settings identical in A/B runs and discard the initial reacquisition interval.

### 1000 Hz scan timing

[Issue #8](https://github.com/aroum/dmk/issues/8) reports scan overruns on Hybrid ANSI in release `2026.11.22` and dev at `6ef2a5e`. Dev now places RT scan functions in RAM on ARM and processes the previous key during C57 discharge, preserving cached classic thresholds, ADC timeout handling and same-pass final-key processing. Discharge lasts at least 2 µs; release/mux selection to the next strobe lasts at least 4 µs. This extends the discharge interval; it needs hardware validation on Classic and JIS. `HHKB_SCAN_1KHZ` has been removed: timer scheduling is always available and VIA selects the USB rate up to 1000 Hz. Battery scan settings are unchanged.

Use `[RATE]` / `[SCAN]` to verify actual throughput: a 1 ms USB HID interval does not guarantee 1000 fresh matrix scans/s. The author's approximately 850 µs result is from their fork on Hybrid ANSI, not a measurement of this implementation. Compare builds with the optional emulator (MIT attribution and license: `tools/hhkb/LICENSE.scan_emu`):

```bash
uv run --with unicorn --with pyelftools tools/hhkb/scan_emu.py time build/hhkb-script/dmk_hhkb_stm32l072
```

Install `arm-none-eabi-nm` on PATH. The emulator estimates peripheral timing and instruction cost; use it for relative comparisons, not hardware timing guarantees. Calibration buffering and per-key RT profiles from the fork are not included.

### Hold-Tap configuration compatibility (dev)

DMK uses one `HT(hold, tap)` action internally; VIA/Vial Mod-Tap and Layer-Tap are converted to this format. Legacy source forms `HT(1, K_A)`, `HT(FN1, K_A)` and `HT(MOD_LSHIFT, K_A)` remain supported. New configs can use explicit `HT(MO(1), K_A)` or `HT(K_RSFT, K_RGUI)`. Use `KEY(...)` for custom HID aliases or numeric HID usages. See [Universal Hold-Tap](../../../../docs/en/keycodes.md#universal-hold-tap) for supported actions and encoding limits.

The dev fixes preserve readable legacy EEPROM records; no factory reset is required. Reassign keys that were already saved incorrectly. VIA cannot represent every native HT pair: ordinary key/key and tap-layer pairs read as unassigned, so configure those in source. These dev changes are not part of release `2026.11.22`.

### Reset DMK data or return to stock Bank 1

In the [VIA web app](https://www.usevia.app/), open **HHKB Settings** → **Debug**:

Set SW6 to OFF and enable **Arm reset / stock recovery**. Choose one of the following actions.

| Scenario | VIA action | DMK EEPROM | Firmware after reboot |
| --- | --- | --- | --- |
| Return to stock, retain DMK data | Leave **Erase DMK EEPROM when returning to stock** off, click **Return to stock Bank 1** | Preserved | Stock Bank 1 |
| Return to stock and clear DMK data | Enable **Erase DMK EEPROM when returning to stock**, click **Return to stock Bank 1** | Recognized DMK records cleared | Stock Bank 1 |
| Reset DMK data | Click **Reset DMK data and restart** | Recognized DMK records cleared, regardless of the recovery checkbox | DMK with compiled defaults |

Returning to Bank 1 uses the SW6 recovery procedure. It updates PFU boot metadata and erases the first 128-byte vector page of DMK in Bank 2. Reinstall DMK through the PFU updater to run it again. The checkbox defaults off each session and applies to the Bank 1 button. EEPROM cleanup uses the SW5+SW6 routine and clears recognized DMK calibration, curves, settings, Grave Escape flags, keymaps, macros and combos. Factory PFU calibration is preserved. Previously overwritten PFU bytes require a same-board backup or the stock Keymap Tool recovery described in [backup and restore](backup-restore.md).

Both buttons require arming and consume the armed state. Commands are executed after a 150 ms acknowledgement delay. EEPROM erase failure prevents the return to Bank 1 and can be retried after rearming. Keep power connected until the operation completes.

**Reset status** describes the current session's last command. `Ready` means no action is queued, `Armed` means the buttons are enabled, `Resetting DMK...` or `Returning to Bank 1...` means an action is queued, and `Restarting...` means its pre-reboot work succeeded. `Set SW6 to OFF` blocks the request, and `Reset failed; retry` reports an EEPROM cleanup failure. USB disconnects during reboot, so the final status may disappear before VIA displays it. Verify the resulting PFU or DMK USB device after reconnecting. EEPROM save status is shown separately under **Settings EEPROM**.

## Hybrid only: Bluetooth controls and keycodes

### Media keys over Bluetooth

Hybrid routes consumer/media reports to the stock Nordic module when Bluetooth output is selected, including when powered by USB. Selecting USB output uses the normal USB consumer-report path.

The factory BLE report map supports **32 consumer usages**. These include volume up/down, mute, play/pause, next/previous track, stop, eject, brightness up/down, and browser/application controls. DMK encodes every usage in that map into Nordic's four consumer bitmask bytes (packet type `0x02`); it does not require changes to the Nordic firmware. The mapping is adapted from the MIT [l3tnun/hhkb_hybrid](https://github.com/l3tnun/hhkb_hybrid) reference and retains its attribution in [LICENSE.protocol](../../modules/hybrid/LICENSE.protocol).

Supported HID Consumer usage IDs (hexadecimal): `006F`, `0070`, `0030`, `00B7`, `00E2`, `00E9`, `00EA`, `00B8`, `0087`, `018A`, `0192`, `0088`, `0196`, `0221`, `0223`, `0224`, `0225`, `0226`, `0227`, `022A`, `00CD`, `00B5`, `00B6`, `0153`, `0154`, `0155`, `0206`, `00B2`, `00B3`, `00B4`, `0003`, `0152`.

Press and release reports share the ordered keyboard FIFO. Transport switches and sleep/OFF release both keyboard and consumer state. If the radio stalls and the FIFO fills, DMK resynchronizes the latest state of both report types, including releases. The existing consumer API sends one active usage at a time, as on USB; the four bytes do not mean all 32 buttons are simultaneously exposed by DMK.

Arbitrary Consumer usages outside the factory map, HID System Control reports, and mouse reports are not supported by this Bluetooth path. USB consumer handling is unchanged. The factory module understands HID report types and usages, not DMK/QMK keycode numbers; DMK performs that translation.

Assign these after loading [via.json](../../via.json) in VIA:

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

In the [VIA web app](https://www.usevia.app/), open **Settings → Show Design tab → Design → Load Draft Definition** and choose [via.json](../../via.json). The **HHKB Settings → Bluetooth** submenu appears only in a Hybrid build. Vial supports keymap editing but cannot display custom controls.

- Battery matrix rate is fixed at an 11 ms target period (about 90.9 Hz); there is no battery scan-rate control.
- **Battery ADC oversampling 4x:** independent battery-mode toggle, off by default and saved in EEPROM. USB-powered operation uses the separate ADC oversampling setting, even when sending reports over Bluetooth. Older EEPROM records default this new setting to off.
- **Sleep after inactivity:** 0–60 minutes; default 30; 0 disables both inactivity and the 60-second disconnected-standby trigger. With Wake with matrix off, enters OFF directly; with it on, enters matrix-wake sleep. Manual sleep actions follow the same choice.
- **Power off after sleep:** 0/1/3/5/10/20/30/40/50/60 min; default 3, 0 never. Battery only. Visible only when Wake with matrix is enabled; saved in EEPROM independently of inactivity.
- **Wake with matrix:** off by default: sleep actions enter OFF directly. When enabled, sleep disables sensors and LEDs between matrix probes; a key or PC5 wakes the keyboard. Ordinary OFF always requires its guarded PC5 wake gesture. Critical-battery protection is independent.
- **Sleep matrix scan rate (Hz):** dropdown 1–10 Hz, step 1; default 1 Hz. Visible only with matrix wake enabled. Saved in EEPROM without changing calibration or keymaps; older records default to 1 Hz. Higher rates shorten the wait for key wake but may consume more power.

These sleep controls are Hybrid-only (including Type-S and JIS). Changes are saved automatically to EEPROM after about 0.5 seconds without further changes and survive a power cycle. Disabling matrix wake hides the scan rate and sleep-to-OFF timer without clearing their saved values; re-enabling it restores those choices. Classic does not expose or use these controls.

- **Select paired profile / Start pairing:** choose profiles 1–4; pairing may replace a bond.
- **Show battery level on orange LED / Show battery now:** enable and trigger battery flashes.
- **Blink orange on low battery:** once every 30 seconds at 15%; twice every 15 seconds at 5%. At 0%, the critical-battery warning below takes priority.
- **Blink blue while pairing:** 2 Hz while pending, 4 Hz while advertising.
- **Blink blue for active profile:** one to four flashes for profiles 1–4.
- **Battery level (%):** reports 0, 5, 15, or 100. `100` only means all low-battery threshold inputs are inactive.
- **PC5 short press / PC5 long press:** choose independent button actions; the choices and last profile are saved in EEPROM.

**Critical battery (Hybrid):** PB9 LOW on battery forces a steady orange warning even when the low-battery LED toggle is off. Matrix scanning stops; after 180 seconds or a PC5 hold of at least 2 seconds, DMK requests Nordic idle and enters deep STOP with PC5 disabled and reset-based wake on USB. This overrides configured PC5 actions and sleep/matrix-wake settings. Connecting USB or recovering PB9 HIGH cancels the warning. The configurable blink patterns above apply to noncritical battery warnings.

PB9, PB8, and PB7 provide the 0%, 5%, and 15% battery thresholds. DMK samples them once per second and sends stable changes to the stock nRF for the BLE Battery Service. One to four manual orange flashes indicate 0%, 5%, 15%, or above 15%. These are threshold bands, not precise charge measurements. Status patterns temporarily take priority over configured base LED modes.

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
   - **EC Matrix Stream** — prints state transition events (press/release) with matrix coordinates `(col, row)`, raw ADC count (`raw`), normalized value (`norm` from 0 to 1000 permille), and current key status.
   - **Calibration Monitor** — real-time progress bar and calibration stream: baseline capture, peak registration per key (`[CAL] Key C3 R2 peak updated: 2650 -> 2890`), completion percentage `[====>    ] X% (N/60)` for ANSI or `(N/69)` for JIS, and save readiness notifications.

     > [!NOTE]
     > Switching between the **Configure** tab and **HID Console** does not interrupt or reset calibration — the state remains active in MCU RAM until _Finish and save_ is clicked. Console monitoring provides uninterrupted real-time visibility without having to manually refresh the status counter in the calibration menu.
   - **Polling Rate Monitor** — outputs periodic reports of the actual matrix scan rate and USB task frequency (e.g. `[RATE] Matrix: 1000 Hz, USB: 500 Hz`).
   - **VIA Telemetry Diag** — prints incoming VIA/Vial requests, commands, and parameter modifications.

> [!TIP]
> You can save logs to a text file or clear the output area directly using the buttons in the upper-right corner of the **HID Console** tab in VIA.

---

## Non-linear EC Capacitive Sensor Response

The EC capacitive switch relies on a conical spring compressing against PCB sensing pads. The resulting capacitance change follows a distinctly non-linear response relative to physical key travel:

| Physical Key Travel | Sensor Value (% / ‰) | Mechanism Behavior & Physics                                      |
| ------------------- | -------------------- | ----------------------------------------------------------------- |
| **0 mm**            | **0%** (0 ‰)         | Key at rest (baseline level); conical spring fully retracted.     |
| **1 mm**            | **15%** (150 ‰)      | Initial rubber dome compression; slow initial capacitance growth. |
| **2 mm**            | **50%** (500 ‰)      | Peak tactile resistance (tactile bump).                           |
| **3 mm**            | **80%** (800 ‰)      | Tactile drop-off; spring rapidly compresses toward the PCB pads.  |
| **4 mm**            | **99%** (990–1000 ‰) | Bottom-out; maximum measurable capacitance (peak).                |

### DMK Defaults:

- **Default Actuation Point:** **75% of travel** (750 ‰, approximately 2.8 mm). The key triggers reliably right after collapsing the tactile dome, avoiding accidental triggers on light key rests.
- **Release Hysteresis:** **0.4 mm** (100 ‰). Provides a solid deadband preventing chatter or repeated triggers near the actuation boundary.
- **Rapid Trigger:** Disabled by default. Can be enabled on demand under **HHKB Settings → Rapid Trigger**.

USB soft-OFF keeps USB enumerated and VIA accessible, stops the matrix/sensors and blanks normal LEDs. Hold PC5 about 1 second and release to resume without reset; a 10-second hold cancels wake and repeats Nordic idle after the orange warning. Power off after sleep is battery-only; sleep itself is available on USB. Matrix wake and PC5 mappings keep their existing EEPROM settings.


## Compressed EEPROM storage

The development firmware uses one atomic snapshot for all four keymap layers, eight combos, the full macro buffer, calibration, travel curve parameters and VIA settings. This applies to Classic and Hybrid, ANSI and JIS, including Type-S. Hybrid-only settings remain exclusive to Hybrid.

Storage occupies **1,772 bytes**: two 880-byte slots and a 12-byte ownership/reset header. Each slot has a 20-byte header and up to 860 compressed payload bytes. Calibration stores each real key's baseline and peak as a lossless pair of 12-bit ADC values (216 bytes for ANSI settings including calibration; 243 for JIS). Empty matrix positions are omitted. Keymaps retain all 32 bits of every DMK keycode; byte-plane ordering and a bounded-window lossless codec compress repeated bytes. No keycodes, macro capacity or ADC precision are discarded. Compression runs only on save/load; scanning uses the existing RAM arrays.

A generation number, checksum and commit marker validate each snapshot. The previous snapshot remains intact until the replacement is complete. Writes are spread across storage polls, and unchanged snapshots are skipped. A failed EEPROM write is retried. If a configuration cannot compress into a slot, **Full: not saved** preserves the previous complete configuration; simplify macros/combos or the keymap before saving again. Do not disconnect power until **Settings EEPROM** reports **Saved (compressed)**. VIA status labels require switching menus or reloading the page to refresh.

The fragmented physical ranges are `0x08080480–0x080804FF`, `0x08080524–0x080806FF`, `0x08080D00–0x08080FFF`, `0x08081370–0x080813FF`, and `0x08081700–0x080817FF`. These avoid the known PFU keymap, factory sensor, Bluetooth configuration and boot/update metadata regions. Enrollment checks every candidate word: unknown nonzero data blocks writing with **Occupied: not saved**. ANSI and JIS ownership markers differ; one layout never reinterprets the other's records.

On first startup, compatible legacy DMK records are imported automatically (ANSI DMK1/DMK2 or JIS DMK2, plus compatible settings). When necessary, the newest legacy DMK2 snapshot is first copied and committed into its first legacy slot. Only after the new compact snapshot commits are owned legacy bytes in PFU table regions cleared. Power cuts during migration are recoverable. This does not reconstruct PFU bytes overwritten by older DMK firmware: restoring stock may still require the same-board backup or resetting the stock keymap in HHKB Keymap Tool. Older DMK builds cannot read the compressed format.

Normal Bank 1 firmware updates should preserve the new storage because it avoids the known update metadata and stock keymap regions. This layout is a development implementation based on the available EEPROM map and Classic backup; it still needs a complete Hybrid/JIS EEPROM backup and hardware verification. Keep a [same-board EEPROM backup](backup-restore.md) before migration, and verify Settings EEPROM after updating.
