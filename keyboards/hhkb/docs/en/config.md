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
- **Battery active:** Bluetooth output. The configured frequency controls matrix scanning. Radio report timing is determined by the stock Nordic firmware. See the Bluetooth scan-rate section below.
- **Sleep:** Triggered by a keycode, configured PC5 action, or inactivity. The timeout defaults to 30 minutes and is capped at 10 minutes during pairing; `0` disables automatic sleep. With **Wake with matrix** off, sleep stops the nRF and enters STM32 STOP. PC5 held one second or USB insertion wakes by resetting the controller. With matrix wake on, the MCU keeps running at reduced duty cycle and scans once a second; PC5, a key, or USB wakes it. This mode consumes more than STOP.
- **OFF:** On batteries, the default two-second PC5 hold or **BT Power** stops the nRF, ADC, sensor circuit, and matrix timer, then enters STOP. A one-second PC5 hold or USB insertion restarts the board. LPTIM1 wakes about every 1.7 seconds to refresh an active independent watchdog. See the measured current and known wake failure below.

Short PC5 presses last 0–0.5 seconds and show the connected profile by default. VIA can assign no action, battery display, sleep, next profile, or last Bluetooth profile/USB toggle. Long presses fire after two seconds and default to OFF; VIA can assign sleep or pairing of the last profile. The one-second OFF wake action is fixed, so the keyboard remains recoverable.

The reference firmware also uses STM32 STOP for OFF and sleep, waking on PC5 or USB with a reset. DMK's [STOP implementation](../../modules/hybrid/hybrid_stop.c) adapts only this MIT-licensed wake sequence. Matrix wake at 1 Hz uses a running MCU instead.

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

On battery, the USB owner task now detaches/deinitializes TinyUSB and powers down the USB transceiver, CRS and HSI48; it polls power at 100 ms intervals and reinitializes USB after cable insertion. At matrix rates of 50, 125, 200 or 250 Hz, the CPU/AHB runs at 16 MHz; 500/1000 Hz and USB power use 32 MHz. The PLL remains enabled. TIM2 is retimed to preserve its 4 MHz counter, SysTick is retimed for the RTOS tick, and the ADC divider changes to preserve 16 MHz ADC operation. Sleep with matrix wake blanks both LEDs. These changes require hardware validation: compare current, matrix timing, held-key behavior, USB insertion/removal, and OFF/wake cycles. No new current figures are claimed.

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

Matrix scan frequency is configured independently of Nordic report delivery. The separate radio chip controls Bluetooth timing. A maximum near 125 reports/s has been reported, while the stock matrix scan rate is around 50 Hz. Supported radio intervals and firmware-specific limits still require HCI measurements. Increasing matrix frequency cannot force the radio to send faster.

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
