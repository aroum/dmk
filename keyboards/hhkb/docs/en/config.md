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

### Hybrid power-off behaviour

All Hybrid builds use the stock-style OFF implementation: Nordic idle over SPI, STM32 STOP without holding PD2 high, and resume without STM32 reset after ordinary OFF. No additional build flag is required. The earlier MIT/stock comparison builds have been removed from CI. Hybrid ANSI and JIS artifacts include the same implementation, including Type-S.

### Power modes and PC5

- **USB active:** USB output by default. Configured inactivity or BT Sleep can enter sleep. PC5/BT Power can enter USB soft-OFF: the USB device remains enumerated, while matrix scanning, sensor power and normal LEDs are disabled. STM32 keeps the scheduler, USB and SPI alive; the idle request is repeated about every 3 seconds without blocking PC5 polling. Unplugging USB while OFF enters battery STOP. Inserting USB into battery OFF still resets into the USB path.
- **Battery active:** Bluetooth output. Matrix scanning targets a fixed 11 ms period. Radio report timing is determined by the stock Nordic firmware. See the Bluetooth scan-rate section below.
- **Sleep:** BT Sleep, the configured PC5 action, or inactivity (default 30 min; 0 disables automatic sleep) enters Nordic-idle sleep on either power source. With automatic sleep enabled, a no-connection notification lasting 60 seconds also enters sleep. DMK releases keys and sends the disconnect/idle SPI exchange, repeating a consumed request at 200 ms intervals while disconnection is unconfirmed (up to six retries; total wait bounded to about 3 seconds), then blanks LEDs and stops regular matrix scans. VIA Wake with matrix enables 1–10 Hz probes (default 1 Hz); off enters ordinary OFF directly. PC5 release after 0.1–2 seconds, an enabled matrix probe, a power-source change or a new Nordic slot-connected notification wakes. Wake requests the last BT profile when Bluetooth output is selected. Holding PC5 2 seconds performs the configured long action without an intermediate wake. The power-off-after-sleep timer offers 0 (never), 1/3/5/10/20/30/40/50/60 min; default 3 min. It enters ordinary OFF only on battery; USB sleep does not automatically enter OFF.
- **Ordinary OFF:** request Nordic idle and wait for request consumption/disconnection, then battery STOP or USB soft-OFF without raising PD2. Hold PC5 at least about 1 second and release before 10 seconds to resume without STM32 reset; blue indicates readiness. Holding 10 seconds cancels wake: orange for about 3 seconds, wait for release, repeat the Nordic idle exchange and remain OFF. Critical-battery STOP is separate: PC5 is masked and USB wake resets STM32.

Short PC5 presses last 0–0.5 seconds and show the connected profile by default. VIA can assign no action, battery display, sleep, next profile, or last Bluetooth profile/USB toggle. Long presses fire after two seconds and default to OFF; VIA can assign sleep or pairing of the last profile. The guarded OFF wake gesture is fixed.

The SPI protocol and original STOP implementation retain their MIT attribution. The selected ordinary OFF path follows the supplied stock analysis; repeated power-button cycles, Bluetooth reconnection and direct sleep-to-OFF were confirmed by the tester. Wake with matrix affects key probing only, not PC5 behaviour.

### Differences from stock sleep and OFF

On battery, DMK aims to reproduce stock clocks, ADC operation, Nordic idle signalling, PC5 handling and STOP to obtain stock-level consumption. OFF reached 0.21 mA on the tested keyboard. Stock powers off directly; DMK adds optional sleep with matrix wake. With matrix wake disabled, a sleep request enters OFF immediately. With it enabled, sensors are powered only for probes at 1–10 Hz (default 1 Hz), until a key wakes the board or the configured battery sleep-to-OFF timer expires.

Additional VIA settings control inactivity, sleep-to-OFF timing, sleep scan rate, PC5 actions, battery oversampling and LEDs. The critical-battery USB-only wake path remains separate. PC5 handling does not depend on the matrix-wake checkbox. See the [current consumption comparison](../../README.md#hybrid--battery-power).

### Measured battery consumption and thresholds (Hybrid)

The current measurements are published in the README, separately for Classic and Hybrid. They were received on 21 November 2026 and apply to one keyboard; higher sleep probe rates still require hardware measurements.

On battery, Hybrid now uses the stock-style clock tree described by static firmware analysis: HSI16 /4 → PLL ×4 /4 → 4 MHz CPU/AHB/APB, voltage Range 1, and zero Flash wait states. ADC1_IN1 uses synchronous PCLK/4 = 1 MHz, 12-bit single software-triggered conversions, minimum 1.5-cycle sampling time, and **LowPowerAutoPowerOff** (automatic power-down after each conversion). The pre-conversion row settling delay is approximately 25 µs. Oversampling is off by default; the independent battery 4x toggle remains an optional departure from the stock settings.

Battery matrix scanning has a fixed **11 ms target period (about 90.9 Hz)**. Its VIA frequency dropdown and setter have been removed; previously stored battery rates are ignored. This is matrix timing, not the BLE connection interval or a guarantee of report delivery. The USB CPU/ADC clocks, scan-rate options and analog delays retain their existing DMK settings: 32 MHz CPU and 16 MHz ADC. On power-source changes, PLL, Flash latency, ADC mode, TIM2 and tickless SysTick parameters are restored for the selected path.

USB/TinyUSB, CRS, HSI48 and unused analog reference buffers are disabled on battery; cable detection is polled every 100 ms by the USB task. Unlike the analyzed stock busy loop, DMK retains tickless idle/WFI between scans. **LowPowerAutoPowerOff refers to the ADC**, independently of the keyboard inactivity timeout (30 minutes by default, configurable). VIA **Wake with matrix** enables sleep with 1–10 Hz probes (default 1 Hz); when off, sleep requests enter ordinary OFF directly. Both paths request Nordic idle. Sleep keeps the scheduler running between probes; battery OFF enters STOP. Both LEDs are blanked in sleep. Idle indicator checks use 250 ms, active status patterns 50 ms, sleep 1 second. Other interrupts/tasks can wake the CPU sooner.

Battery OFF disables the ADC regulator, TIM2, USB and HSI48, clears matrix/multiplexer outputs and enters STOP. USB soft-OFF keeps USB/CPU/SPI alive while TIM2 scanning and analog sensor outputs are disabled. SWD markers: `hhkb_stop_stage` 1 = preparation, 2 = waiting for button release, 3 = before WFI, 4 = returned from WFI, 5 = timing the button, 6 = resume. `hhkb_stop_wake_count` counts WFI returns. Both are volatile RAM diagnostics.

The [README](../../README.md#scan-rate-and-power) contains the current Classic and Hybrid consumption tables.

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

The SPI protocol handling is adapted from the MIT reference with its attribution retained. Poll `0x02` receives `AA 40 00 00 00 00 00 00 00` plus the idle byte already on MISO. The matching Nordic follow-up `0x03` receives `AA 10 00` for idle. Control replies have priority over battery/keyboard reports; unsolicited follow-ups are ignored. Shutdown waits up to 100 ms for an existing control follow-up, then services SPI/watchdog for at least 1.5 seconds and up to roughly 3 seconds for request consumption, disconnection and SPI completion. It then enters STOP without holding PD2 high. Ordinary OFF resumes without STM32 reset; critical-battery STOP wakes only on USB and resets; ordinary OFF also resets on USB insertion. Nordic idle is not proof of Nordic System OFF.

PD2 is observed HIGH after GPIO initialization and LOW after peripheral setup in the supplied stock analysis, including reset-based boots. DMK experiments indicate that HIGH stops the stock Nordic, but the dump alone does not establish whether this pin controls reset or power. A Nordic restart after a pulse is an inference, not a verified pin function.

DMK reproduces this PD2 sequence for Hybrid: after the early SW6 check, Bank 2 enables the GPIOD clock, preloads HIGH and selects push-pull output without pull resistors. PD2 stays HIGH through platform and peripheral initialization, then goes LOW after the SPI slave and its IRQ are ready. Each matrix scan reasserts PC14 HIGH and PD2 LOW. Sleep and OFF do not raise PD2, and ordinary OFF resumes without a new startup pulse. Reset-based boots execute the startup sequence again. No fixed pulse duration is assumed; the HIGH interval follows initialization time and needs measurement on hardware. Classic builds do not execute this PD2 sequence; Bank 1 is not modified.
