**Language:** [English](config.md) | [Русский](../ru/config.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA & Debug](via-debug.md)

# Configuration

## Build Definitions

- `HHKB_CLASSIC` builds firmware for wired HHKB Classic (ANSI).
- `HHKB_CLASSIC_TYPE_S` builds firmware for wired Classic Type-S (ANSI or JIS when paired with `HHKB_JIS`).
- `HHKB_HYBRID` builds firmware for wireless Hybrid and Hybrid Type-S (enables SPI communications with Nordic nRF, battery management, profiles, and STOP mode).
- `HHKB_JIS` enables support for the 69-key Japanese JIS matrix (sparse matrix, 15 columns, Kana/Henkan/Muhenkan keys, and arrow cluster).
- `HHKB_SCAN_DIAGNOSTICS` adds measurement counters accessible over SWD.

## Matrix Scanning

- **ANSI (60 keys):** 15 columns × 4 rows matrix (all positions populated).
- **JIS (69 keys):** 15 columns × 5 logical rows matrix (69 physical switches, active `populated` mask).

DMK selects the multiplexer column, pulses `PC8` to discharge measurement capacitor `C57`, pulls the target row active `LOW`, waits for analog settling, performs ADC conversion, restores the row to `HIGH`, and proceeds to the next key. `PC14` powers the analog matrix circuit, `PC7` enables the op-amp, and `PA1` serves as `ADC1_IN1`.

The timer-driven scanner is enabled by default. [VIA](https://www.usevia.app/) selects 50, 125, 250, 500, or 1000 Hz (1000 Hz targets a 950 µs loop period). The scanner runs directly from SRAM, with TIM2 providing precision analog delays.

## Matrix and Peripheral Pinout

- **Matrix:** 4 rows × 15 columns (ANSI) or up to 5 rows × 15 columns (JIS).
- **Rows:** `PB0`, `PB1`, `PB2`, `PB10` (`GPIOB` masks `0x001`, `0x002`, `0x004`, `0x400`).
- **Columns:** Two 74HC4051 analog multiplexers. `PC6` selects U1 (columns 0–7), `PB15` selects U2 (columns 8–14); `PB12`, `PB13`, `PB14` drive channel addresses.
- **Analog frontend:** `PC14` sensor power, `PC7` op-amp enable, `PC8` C57 capacitor discharge, `PA1` ADC input (`ADC_IN1`).
- **DIP switches:** `PA3`–`PA7` and `PC4`, active low.
- **LEDs:** `PA8` and `PA9` (Classic and Hybrid). [VIA](https://www.usevia.app/) assigns Off, Debug heartbeat, Caps Lock, Fn layer, or System 2 independently; by default PA8 indicates Caps Lock, PA9 indicates Fn layer.

## Factory Data and Calibration

Factory classic-mode thresholds are read from the keyboard's EEPROM at startup. A single DMK build preserves individual thresholds when installed across different boards. User calibration records per-key baseline and peak in an EEPROM record (`0x08081500`). Disabling user calibration selects default calibrated ranges without overwriting factory tables.

## DIP Switches

SW1–SW5 are read by firmware but have no hardcoded actions in DMK. SW6 triggers recovery into Bank 1. To select base/Fn layer pairs, use VIA System 1 and System 2 keycodes. Fn + Tab toggles Caps Lock.

## Hybrid Only: Transport, Profiles, and Power

PC13 reads LOW on USB power and HIGH on battery. On battery, DMK sends 6-key HID reports over SPI to the stock nRF module and uses dedicated battery scan rates. On USB, reports default to USB. **BT / USB** switches to the last Bluetooth profile even with USB plugged in. Bluetooth report timing is controlled by the Nordic chip and is independent of matrix scanning.

Four Bluetooth profiles are supported. **BT Select 1–4** selects a bond; **BT Pair 1–4** starts pairing in a chosen slot. **BT Next** cycles through slots. The last selected slot is saved in EEPROM.

The factory nRF is SPI master on PA15/PB3–PB5 (Mode 3, 125 kHz). PB6 requests polling (Attention), PD2 controls module enable. STM32 application lives in Bank 2 starting at `0x08010000`.

The common [VIA controls](via-debug.md) also configure USB matrix rate, calibration, actuation and hysteresis, Rapid Trigger when compiled in, ADC oversampling, hold-tap timing, and the base modes of both LEDs. Hybrid-specific battery, profile, and PC5 controls are listed below. The Classic factory calibration table must not be assumed to match Hybrid hardware.

### Power modes and PC5

- **USB active:** USB output by default. The OFF action is battery-only, as in the reference implementation.
- **Battery active:** Bluetooth output. Matrix scanning targets a fixed 11 ms period. Radio report timing is determined by the stock Nordic firmware. See the Bluetooth scan-rate section below.
- **Sleep:** BT Sleep, the configured PC5 action, or inactivity (default 30 min; 0 disables) enters connected sleep on batteries only. Nordic stays alive. Wake with matrix only enables 1 Hz probes. A 0.1–2 s PC5 press wakes on release and suppresses the normal short action; holding 2 s performs the configured long action without waking first. USB wakes immediately. A separate deep-sleep timer offers 0 (never), 1/3/5/10/20/30/40/50/60 min; default 3 min. Expiry enters the common Nordic-idle/PD2/STOP path. Neither sleep timer applies on USB power.
- **OFF / deep sleep:** Request Nordic idle and wait for acknowledgement/disconnection before PD2 and STM32 STOP, following the MIT implementation. To wake, hold PC5 at least approximately 1 s, then release before 10 s. Blue indicates readiness. At 10 s the attempt is cancelled, orange lights for approximately 3 s, and the MCU returns to STOP. USB insertion resets into the USB path. DMK retains reset-based wake; stock resume without reset and the exact Nordic low-power state remain unverified.

Short PC5 presses last 0–0.5 seconds and show the connected profile by default. VIA can assign no action, battery display, sleep, next profile, or last Bluetooth profile/USB toggle. Long presses fire after two seconds and default to OFF; VIA can assign sleep or pairing of the last profile. The guarded OFF wake gesture is fixed.

The supplied static stock analysis distinguishes connected sleep from deep STOP/OFF. DMK uses STOP only for OFF, with the MIT SPI/PD2 sequence; DMK adds configurable connected sleep before deep sleep. DMK's [STOP implementation](../../modules/hybrid/hybrid_stop.c) adapts only this MIT-licensed wake sequence. Matrix wake at 1 Hz uses a running MCU instead.

### Measured battery consumption and thresholds (Hybrid)

User measurements supplied on 20 November 2026 were taken over Bluetooth with a 3.0 V supply. These results describe the tested Hybrid and firmware, not guaranteed figures for every board. Values below are in mA; peaks are noted separately.

| Mode | DMK, 1000 Hz | DMK, 50 Hz | DMK, 50 Hz + 4x oversampling | Stock |
| --- | --- | --- | --- | --- |
| Boot | 12.5–14 | 12.5–15 | 13 | 3.3 |
| Idle | 10.33–10.7 | 10.5–12 | 10.9–11.7 | 3.28 |
| Typing | 11–13; one peak at 14 | 13 | 13–13.5; peaks at 15 | 3.53–3.58 |
| OFF | 4–4.5 | 4 | 4.2 | 0.21 |
| Sleep | 4 | 4 | 4 | Not observed |
| Sleep with matrix wake | 7.4 | 7.5 | 7.3 | Not measured |

DMK currently draws substantially more than stock on battery, including roughly 19–21 times the stock OFF current. Lowering matrix rate alone did not reduce idle consumption in these measurements. OFF/sleep power saving is not yet validated: the tester also reproduced failure to wake using PC5, requiring battery removal. The documented wake sequence above describes intended behavior. A pre-STOP SWD capture confirmed the PC5 input and EXTI routing and a counting LPTIM, but debug access was lost after continuing into STOP; no valid post-STOP snapshot is available.

The updated OFF path explicitly powers down the USB transceiver and HSI48, stops TIM2, clears row/multiplexer outputs, and waits for the OFF button to be released before entering STOP. These changes are compiled but have not yet been measured on hardware; the figures above remain the earlier measurements. For SWD diagnosis, `hhkb_stop_stage` records 1 = preparation, 2 = waiting for button release, 3 = before WFI, 4 = returned from WFI, 5 = timing a held button, 6 = requesting reset. `hhkb_stop_wake_count` counts WFI returns. Both variables are volatile RAM diagnostics and are cleared on reset.

On battery, Hybrid now uses the stock-style clock tree described by static firmware analysis: HSI16 /4 → PLL ×4 /4 → 4 MHz CPU/AHB/APB, voltage Range 1, and zero Flash wait states. ADC1_IN1 uses synchronous PCLK/4 = 1 MHz, 12-bit single software-triggered conversions, minimum 1.5-cycle sampling time, and **LowPowerAutoPowerOff** (automatic power-down after each conversion). The pre-conversion row settling delay is approximately 25 µs. Oversampling is off by default; the independent battery 4x toggle remains an optional departure from the stock settings.

Battery matrix scanning has a fixed **11 ms target period (about 90.9 Hz)**. Its VIA frequency dropdown and setter have been removed; previously stored battery rates are ignored. This is matrix timing, not the BLE connection interval or a guarantee of report delivery. The USB CPU/ADC clocks, scan-rate options and analog delays retain their existing DMK settings: 32 MHz CPU and 16 MHz ADC. On power-source changes, PLL, Flash latency, ADC mode, TIM2 and tickless SysTick parameters are restored for the selected path.

USB/TinyUSB, CRS, HSI48 and unused analog reference buffers are disabled on battery; cable detection is polled every 100 ms by the USB task. Unlike the analyzed stock busy loop, DMK retains tickless idle/WFI between scans. **LowPowerAutoPowerOff refers to the ADC**, independently of the keyboard inactivity timeout (30 minutes by default, configurable). Sleep still uses the VIA **Wake with matrix** choice: on enables 1 Hz matrix probes; off disables probes. Both preserve the Nordic link and use PC5/PC13 EXTI to wake the running scheduler. Both LEDs are blanked in sleep. Idle indicator checks use 250 ms, active status patterns 50 ms, sleep 1 second. Other interrupts/tasks can wake the CPU sooner.

OFF disables the ADC regulator and TIM2. The terminal STOP path clears pending SysTick/PendSV and limits EXTI to PC5, PC13 and LPTIM; this removes possible extra wake sources but does not prove the reported wake failure is fixed. Requests to an unresponsive Nordic slow from 5 to 100 ms after five attempts. The calibration and Rapid Trigger algorithms remain DMK algorithms; this does not copy the stock threshold/reconfirmation or calibration routines. Verify raw ADC ranges and calibration after changing analog timings. Static stock analysis and successful builds are not hardware validation, and the older current measurements above are not results for this new configuration.

**Acceptance measurements:** use the same Hybrid at 3.0 V, Bluetooth-connected to the same host, USB disconnected, debug channels and decorative/heartbeat LEDs off. Close OpenOCD and power-cycle before measurement; low-power debug settings change STOP consumption. Record boot, idle, typing, OFF, and sleep with/without matrix wake, then repeat with battery oversampling off/on at the fixed 11 ms scan period. Targets from the stock measurement are 3.28 mA idle, 3.53–3.58 mA typing, and 0.21 mA OFF; these are goals, not achieved DMK results. Verify ten OFF/PC5 wake cycles and USB insertion/removal, key releases, calibration and ADC ranges after each clock transition. If idle remains high, measure the STM32, Nordic and analog supply contributions separately before further clock reductions.

### Updated tester measurements, 20 November 2026

The tester reported these values for an updated build; its exact commit was not supplied. Debugger wires were connected, but OpenOCD was not attached. Supply voltage was not restated for this update (the earlier measurements used 3.0 V). Treat this separately from measurements after a full power removal with no debugger. Values are in mA.

| Mode | Updated DMK build | Stock reference |
| --- | --- | --- |
| Boot peak | 8.5 | 3.3 |
| Idle | 2.89 | 3.28 |
| Typing | 3.5; peak 3.7 | 3.53–3.58 |
| OFF | 2.51 | 0.21 |
| Sleep without matrix wake | 2.51 | Not observed |
| Sleep with matrix wake | 1.75 | Not measured |

The tester confirmed the two sleep readings were not swapped. Idle/typing have reached the stock reference level in this report; terminal OFF/STOP consumption remains unresolved. A separate SWD observation found startup blocked waiting for HSI48 with its reference disabled; enabling the reference allowed boot to continue. Startup now explicitly enables that reference before waiting for HSI48 ready.

PC5 wake is now release-based in both sleep modes; a 2 s hold executes the configured long action without an intermediate wake. The earlier immediate-wake and special release timing variants have been replaced. OFF uses the 1–10 s guard described above. The measured 2.51 mA still requires verification after changes.

The battery display uses discrete hardware thresholds, not a continuous voltage-to-percentage conversion. During the reported supply-voltage sweep:

- **Above-low-battery band (displayed 100%, four orange flashes):** observed from 3.00 down to 2.25 V.
- **15%, three flashes:** observed from 2.24 down to 2.14 V.
- **5%, two flashes:** observed from 2.13 down to 2.03 V.
- **0%, one flash:** observed at 2.02 V; this does not mean immediate power loss.
- The keyboard was no longer operating at 1.71 V. The exact shutdown voltage was not established.

Thus the observed threshold transitions fall between 2.25–2.24 V, 2.14–2.13 V, and 2.03–2.02 V. These are sample measurements; tolerance and hysteresis were not characterized. Displayed 100% means none of the low-battery thresholds is active, not a fully charged battery. USB power does not charge the batteries.

For an alternative OFF path without holding Nordic in reset and with STM32 resume after ordinary OFF, see [comparing OFF implementations](build.md#hybrid-comparing-off-implementations). Select it with `HHKB_STOCK_POWER_OFF`; the descriptions above refer to the default MIT-derived path.

### Hardware and license notes

The ANSI matrix and SPI protocol follow the supplied `hhkb_hybrid` research. Its `nrf_spi.c` and `power.c` have MIT headers; only protocol handling and the STOP wake sequence were adapted. See the attribution and full upstream notice in [LICENSE.protocol](../../modules/hybrid/LICENSE.protocol). Factory and patched PFU containers remain outside Git.

JIS coordinates and the Hybrid factory calibration map remain unverified. A successful build does not establish that the keyboard boots, pairs, wakes from STOP, or reaches the expected battery current on real hardware.


## Bluetooth scan rate and report delivery (Hybrid)

Matrix scan frequency is configured independently of Nordic report delivery. The separate radio chip controls Bluetooth timing. The tester reports a negotiated BLE connection interval of 15 ms (about 66.67 connection intervals/s) on their host. This is not a universal 50 Hz Bluetooth limit or a guarantee of one HID report per interval. The HID measurement script observed 21.9 reports/s during typing, including typing pauses and host scheduling; this does not measure the connection interval. An HCI trace is needed to verify connection parameters and changes. The supplied static analysis of Hybrid stock firmware finds an approximately 11 ms battery matrix period; this still needs timing verification on hardware. Increasing matrix frequency cannot force the radio to send faster.

## Bluetooth HID report intervals on macOS (Hybrid)

Unplug the HHKB USB cable, connect it over Bluetooth, and run from the repository root:

```bash
uv run --with hidapi python3 tools/hhkb/measure_bt_hid.py --list
uv run --with hidapi python3 tools/hhkb/measure_bt_hid.py --device 0 --seconds 20
```

Replace 0 with the HHKB index. Press and release several keys during capture. If macOS blocks the input reports, grant the terminal Input Monitoring permission and restart it. The script keeps timestamps only. Typing cadence and host buffering affect the reported intervals, so use an HCI trace for the BLE connection interval.

### PacketLogger

Sign in to [Apple Developer Downloads](https://developer.apple.com/download/all/) and download **Additional Tools for Xcode**. Copy `PacketLogger.app` from the DMG's Hardware folder to Applications.

```bash
open -a PacketLogger
mdfind 'kMDItemFSName == "PacketLogger.app"'
```

If capture is unavailable, follow [Apple's Bluetooth logging profile instructions](https://developer.apple.com/bug-reporting/profiles-and-logs/) for macOS. Start capture before reconnecting the Hybrid and then press and release a few keys. Find **LE Connection Complete** or **LE Enhanced Connection Complete** and later **LE Connection Update Complete** events for the same handle. Multiply the raw interval by 1.25 ms. If the tool already displays milliseconds, use the displayed value.

DMK firmware for HHKB (Classic & Hybrid, ANSI & JIS) provides real-time diagnostic logging via standard USB HID Console interface. This enables tracking EC capacitive matrix state, measuring raw ADC values, monitoring matrix scan rates, and inspecting VIA configurator communication.

By default, all debug logging channels are **disabled** to avoid unnecessary MCU and USB bus overhead.

### MIT reference shutdown exchange

The MIT reference handles SPI idle, PD2 and STOP. DMK extends it with separate inactivity-to-sleep and sleep-to-deep timers, optional matrix wake and configurable PC5 actions. The earlier claim that the stock 180 s path was ordinary sleep was corrected: the supplied analysis identifies it as critical-battery handling. The default 3 min sleep-to-deep timer is an explicit DMK setting, not a confirmed stock inactivity value. SW6 keeps DMK recovery behavior.

SPI poll `0x02` receives the full nine-byte reply `AA 40 00 00 00 00 00 00 00` (plus the idle byte already on MISO). Control replies have priority over battery/keyboard reports. After that poll, only the corresponding follow-up is accepted: Nordic `0x03` receives `AA 10 00` for idle. The earlier short poll variant has been removed. Before replacing a control request, shutdown waits up to 100 ms for an in-flight follow-up. It keeps the orange indication on for at least 1.5 seconds, services the watchdog and SPI, and waits up to roughly 3 seconds for request consumption, link disconnection and SPI completion. It then raises PD2 and enters STOP. A 1–10 s PC5 hold followed by release, or USB insertion, wakes OFF by reset; 10 s cancels the attempt. The reference also uses STOP for this path; it does not establish that DMK has reached the stock OFF current.

The actual production ISR is host-tested for the full response, priority and rejection of unsolicited follow-ups. Electrical timing, Nordic acceptance and power consumption still need hardware verification.
