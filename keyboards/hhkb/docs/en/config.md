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
- **Sleep:** BT Sleep, the configured PC5 action, or inactivity (default 30 min; 0 disables) enters connected sleep on batteries only. Nordic stays alive. Wake with matrix only enables 1 Hz probes. A 0.1–2 s PC5 press wakes on release and suppresses the normal short action; holding 2 s performs the configured long action without waking first. USB wakes immediately. A separate deep-sleep timer offers 0 (never), 1/3/5/10/20/30/40/50/60 min; default 3 min. Expiry requests Nordic idle and enters STM32 STOP. Neither sleep timer applies on USB power.
- **OFF / deep sleep:** Request Nordic idle and wait for acknowledgement/disconnection before STM32 STOP. PD2 is not held high. To wake, hold PC5 at least approximately 1 s, then release before 10 s. Blue indicates readiness. At 10 s the attempt is cancelled, orange lights for approximately 3 s, and the MCU returns to STOP. USB insertion resets into the USB path. Ordinary OFF resumes STM32 without reset and requests the last Bluetooth profile. Timer-triggered deep sleep wakes by reset. The exact Nordic internal low-power state is not established by these measurements.

Short PC5 presses last 0–0.5 seconds and show the connected profile by default. VIA can assign no action, battery display, sleep, next profile, or last Bluetooth profile/USB toggle. Long presses fire after two seconds and default to OFF; VIA can assign sleep or pairing of the last profile. The guarded OFF wake gesture is fixed.

The SPI protocol and original STOP implementation retain their MIT attribution. The selected OFF path follows the supplied stock analysis; repeated power-button cycles, Bluetooth reconnection and direct sleep-to-OFF were confirmed by the tester in build #204. Wake with matrix affects key probing only, not PC5 behaviour.

### Differences from stock sleep and OFF

The following stock behaviour comes from the supplied static analysis of one Bank 2 image, not an independent hardware verification. Configuration defaults are unknown, USB behaviour is only partly traced, and the key-to-wake path was not followed.

- **Stock connected sleep:** state 5 keeps matrix scanning and SPI running, blanks LEDs and sends no Nordic shutdown command. Around 599 seconds of inactivity leads to state 5 when USB is present or configuration byte 5 is 1; otherwise it leads directly to OFF. In state 5, byte 5 = 0 with USB absent permits a timed transition to ordinary OFF after the minutes stored at `0x20000079`; its default value is unknown. PC5 release after 0.1–2 seconds wakes, and holding at least 2 seconds enters OFF without waking first.
- **DMK connected sleep:** defaults to 30 minutes of inactivity, or a configurable PC5 action/keycode; battery-only. Regular matrix scans stop. VIA optionally enables 1 Hz key probes. A separate VIA timer defaults to 3 minutes and enters deep STOP with reset-based wake. Thus this timer is still different from stock's transition to ordinary OFF without reset. Neither the 3-minute default nor reset-based wake is claimed to reproduce stock ordinary sleep.
- **Stock ordinary OFF:** the described Nordic idle exchange waits up to 3 seconds for a disconnected notification, then stops matrix/analog activity and enters low-power STOP without changing PD2. PC5 hold of at least 1 second arms wake; release resumes without reset. DMK follows this approach, and the tester confirmed repeated wake/reconnection and 0.21 mA OFF current.
- **Ten-second guard:** stock reportedly restarts its OFF procedure, including the Nordic exchange, after the orange warning. DMK returns directly to STOP after the warning and button release, without repeating that exchange.
- **USB:** stock reportedly allows state-5 sleep after inactivity while USB is present, but not its subsequent automatic OFF; button-driven OFF on USB follows an untraced branch. DMK intentionally disables sleep and OFF on USB power.
- **Critical battery:** the stock path at `0x08011380` reportedly handles PB9 LOW with USB absent: orange indication, deep STOP after 180 seconds or a 2-second hold, then reset on wake. DMK reads PB9 as 0%, reports the level to Nordic, and can show the configurable low-battery warning. It has no dedicated 180-second critical-battery shutdown path. Its generic sleep timers are not a replacement for that stock logic.

### Measured battery consumption and thresholds (Hybrid)

The latest comparison, received on 21 November 2026, is shown below. Earlier builds drew 4 mA in OFF (19 November) and 2.51 mA (build #196); build #204 with the selected stock-style OFF path reached 0.21 mA. These are measurements of one keyboard, not guaranteed values for every board.

On battery, Hybrid now uses the stock-style clock tree described by static firmware analysis: HSI16 /4 → PLL ×4 /4 → 4 MHz CPU/AHB/APB, voltage Range 1, and zero Flash wait states. ADC1_IN1 uses synchronous PCLK/4 = 1 MHz, 12-bit single software-triggered conversions, minimum 1.5-cycle sampling time, and **LowPowerAutoPowerOff** (automatic power-down after each conversion). The pre-conversion row settling delay is approximately 25 µs. Oversampling is off by default; the independent battery 4x toggle remains an optional departure from the stock settings.

Battery matrix scanning has a fixed **11 ms target period (about 90.9 Hz)**. Its VIA frequency dropdown and setter have been removed; previously stored battery rates are ignored. This is matrix timing, not the BLE connection interval or a guarantee of report delivery. The USB CPU/ADC clocks, scan-rate options and analog delays retain their existing DMK settings: 32 MHz CPU and 16 MHz ADC. On power-source changes, PLL, Flash latency, ADC mode, TIM2 and tickless SysTick parameters are restored for the selected path.

USB/TinyUSB, CRS, HSI48 and unused analog reference buffers are disabled on battery; cable detection is polled every 100 ms by the USB task. Unlike the analyzed stock busy loop, DMK retains tickless idle/WFI between scans. **LowPowerAutoPowerOff refers to the ADC**, independently of the keyboard inactivity timeout (30 minutes by default, configurable). Sleep still uses the VIA **Wake with matrix** choice: on enables 1 Hz matrix probes; off disables probes. Both preserve the Nordic link and use PC5/PC13 EXTI to wake the running scheduler. Both LEDs are blanked in sleep. Idle indicator checks use 250 ms, active status patterns 50 ms, sleep 1 second. Other interrupts/tasks can wake the CPU sooner.

OFF disables the ADC regulator, TIM2, USB and HSI48, clears matrix/multiplexer outputs and enters STOP. SWD markers: `hhkb_stop_stage` 1 = preparation, 2 = waiting for button release, 3 = before WFI, 4 = returned from WFI, 5 = timing the button, 6 = reset/resume. `hhkb_stop_wake_count` counts WFI returns. Both are volatile RAM diagnostics.

### Tester comparison, build #204

Bluetooth, 3.0 V; current in mA. The tester labelled the table “50 Hz scan rate”; the comparison firmware fixes battery matrix timing at 11 ms, independently of VIA. No measured matrix timing accompanied this report, so the table label does not establish an actual 50 Hz scan.

| Mode | Stock firmware | DMK #204 MIT | DMK #204 stock-style (selected) |
| --- | --- | --- | --- |
| Idle | 3.28 | 2.9 | 3.0 |
| Typing | 3.53–3.58 | 3.9 | 3.8 |
| Typing on Fn layer | 3.4 | 6.0 | 5.8 |
| OFF | 0.21 | 2.6 | 0.21 |
| Sleep | Unconfirmed | 1.81 | 1.8 |
| Sleep with matrix wake | Not available | 1.8 | 1.83 |

The source reports conflict on stock sleep: the earlier table says “Not observed”, while the later comparison lists 0.21 mA. That value is confirmed for stock OFF, but is not treated here as a verified measurement of stock state 5.

The tester confirmed repeated PC5 OFF/on cycles with Bluetooth reconnection, direct sleep-to-OFF, and automatic power-off in the stock-style build. OFF reached the stock reference; connected sleep still draws about 1.8 mA. Fn-layer current may include the configured layer LED; repeat with both LEDs disabled to separate that contribution. This cause has not been measured.

Automatic power-off is intended in both comparison variants: the battery-only inactivity timer enters connected sleep (default 30 minutes; 0 disables automatic sleep), then the deep-sleep timer enters STOP (default 3 minutes; 0 never). Manual sleep also starts the deep-sleep timer. The timer logic was the same in both builds; the difference is the OFF implementation. Neither timer applies on USB power. Timer-triggered STOP still wakes by reset.

The battery display uses discrete hardware thresholds, not a continuous voltage-to-percentage conversion. During the reported supply-voltage sweep:

- **Above-low-battery band (displayed 100%, four orange flashes):** observed from 3.00 down to 2.25 V.
- **15%, three flashes:** observed from 2.24 down to 2.14 V.
- **5%, two flashes:** observed from 2.13 down to 2.03 V.
- **0%, one flash:** observed at 2.02 V; this does not mean immediate power loss.
- The keyboard was no longer operating at 1.71 V. The exact shutdown voltage was not established.

Thus the observed threshold transitions fall between 2.25–2.24 V, 2.14–2.13 V, and 2.03–2.02 V. These are sample measurements; tolerance and hysteresis were not characterized. Displayed 100% means none of the low-battery thresholds is active, not a fully charged battery. USB power does not charge the batteries.

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

### Nordic shutdown exchange

The SPI protocol handling is adapted from the MIT reference with its attribution retained. Poll `0x02` receives `AA 40 00 00 00 00 00 00 00` plus the idle byte already on MISO. The matching Nordic follow-up `0x03` receives `AA 10 00` for idle. Control replies have priority over battery/keyboard reports; unsolicited follow-ups are ignored. Shutdown waits up to 100 ms for an existing control follow-up, then services SPI/watchdog for at least 1.5 seconds and up to roughly 3 seconds for request consumption, disconnection and SPI completion. It then enters STOP without holding PD2 high. Ordinary OFF resumes without STM32 reset; deep sleep and USB wake use reset. Nordic idle is not proof of Nordic System OFF.
