**Language:** [English](hybrid.md) | [Русский](hybrid_ru.md)

# HHKB Professional Hybrid: build and configuration

DMK runs on the original STM32L072 and communicates with the stock nRF52832. The reference Hybrid ANSI layout matches Classic's 4 × 15 matrix in key order. The Hybrid factory calibration layout and JIS coordinates still need verification; use DMK user calibration. The firmware has been compiled but has not been tested on Hybrid hardware.

## Build from the factory update

Obtain the exact official `HHKB800_FW_A048.hfb` separately from the [HHKB download center](https://happyhackingkb.com/download/) or your saved copy. The patcher expects SHA-256 `635b995eb5a15aa50c2cedc60da7d8998fcca8285a09d41d984c07fffcaf9d43`. Keep original and generated containers in the Git-ignored `keyboards/hhkb/local/` directory.

From the repository root:

```bash
mkdir -p keyboards/hhkb/local
# Place the unmodified HHKB800_FW_A048.hfb in keyboards/hhkb/local/ first.
./build_all.sh -b hhkb -d HHKB_HYBRID,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING -c
python3 tools/hhkb/patch_hybrid_hfb.py \
  build/hhkb-script/dmk_hhkb_stm32l072.bin \
  keyboards/hhkb/local/HHKB800_FW_A048.hfb \
  keyboards/hhkb/local/dmk_hybrid.hfb
```

`-c` requests a clean build. `HHKB_HYBRID` selects Hybrid and its SPI transport. The other definitions enable Rapid Trigger, a 16 MHz ADC clock, and the initial ×4 oversampling preference. Omit `HHKB_RAPID_TRIGGER` for a build without it; the VIA Rapid Trigger menu will be hidden. The shared [build guide](docs/en/build.md) explains the definition argument.

[patch_hybrid_hfb.py](../../tools/hhkb/patch_hybrid_hfb.py) verifies the stock hash and Bank 2 vector table, replaces only the 64 KiB STM32 application, pads it with `FF`, and recalculates both CRCs. It preserves the factory nRF image, footer, and version. The output name must be new because the script refuses to overwrite files. The [Classic flasher](../../tools/hhkb/hhkb_flasher.py) is not a verified Hybrid installer. Back up the keyboard before testing: installation and rollback of this patched Hybrid container have not been validated on hardware.

## USB, Bluetooth, and profiles

PC13 reads low on USB power and high on batteries. On battery, DMK sends six-key keyboard reports to the stock nRF over SPI and uses a separate battery matrix scan rate. On USB, reports go to USB by default and use the normal VIA scan rate. **BT / USB** can route reports to the last Bluetooth profile even while USB is connected. Selecting USB without a cable takes effect when USB is connected. Bluetooth media reports are not yet implemented. Matrix scan frequency is separate from the BLE connection interval and report frequency; the latter remain unmeasured.

Four Bluetooth profiles are available. **BT Select 1–4** selects an existing bond; **BT Pair 1–4** pairs in a chosen slot and can replace its bond. **BT Next** cycles through the four slots. The last selected slot is saved in EEPROM and requested on battery startup. **BT Pair Last** pairs that slot. DMK queues an all-keys-released report when changing output or profile; delivery over Bluetooth still needs hardware verification.

The stock nRF is SPI master on PA15/PB3–PB5 (mode 3, 125 kHz). PB6 requests a poll and PD2 controls the radio. The STM32 application occupies Bank 2 at `0x08010000`; the patcher retains the stock nRF firmware.

The common [VIA controls](docs/en/via.md) also configure USB matrix rate, calibration, actuation and hysteresis, Rapid Trigger when compiled in, ADC oversampling, hold-tap timing, and the base modes of both LEDs. Hybrid-specific battery, profile, and PC5 controls are listed below. The Classic factory calibration table must not be assumed to match Hybrid hardware.

## Power modes and PC5

- **USB active:** USB output by default. The OFF action is battery-only, as in the reference implementation.
- **Battery active:** Bluetooth output by default. Battery scanning defaults to 200 Hz; VIA offers 50, 125, 200, 250, 500, and 1000 Hz. At 1000 Hz ADC oversampling is bypassed.
- **Sleep:** Triggered by a keycode, configured PC5 action, or inactivity. The timeout defaults to 30 minutes and is capped at 10 minutes during pairing; `0` disables automatic sleep. With **Wake with matrix** off, sleep stops the nRF and enters STM32 STOP. PC5 held one second or USB insertion wakes by resetting the controller. With matrix wake on, the MCU keeps running at reduced duty cycle and scans once a second; PC5, a key, or USB wakes it. This mode consumes more than STOP.
- **OFF:** On batteries, the default two-second PC5 hold or **BT Power** stops the nRF, ADC, sensor circuit, and matrix timer, then enters STOP. A one-second PC5 hold or USB insertion restarts the board. LPTIM1 wakes about every 1.7 seconds to refresh an active independent watchdog. STOP current and wake behavior require measurement on Hybrid hardware.

Short PC5 presses last 0–0.5 seconds and show the connected profile by default. VIA can assign no action, battery display, sleep, next profile, or last Bluetooth profile/USB toggle. Long presses fire after two seconds and default to OFF; VIA can assign sleep or pairing of the last profile. The one-second OFF wake action is fixed, so the keyboard remains recoverable.

The reference firmware also uses STM32 STOP for OFF and sleep, waking on PC5 or USB with a reset. DMK's [STOP implementation](modules/hybrid/hybrid_stop.c) adapts only this MIT-licensed wake sequence. Matrix wake at 1 Hz uses a running MCU instead.

## Hybrid-only custom keycodes

Assign these after loading [via_hybrid.json](via_hybrid.json) in VIA:

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

## VIA settings unique to Hybrid

In the [VIA web app](https://www.usevia.app/), open **Settings → Show Design tab → Design → Load Draft Definition** and choose [via_hybrid.json](via_hybrid.json). The **HHKB Settings → Bluetooth** submenu appears only in a Hybrid build. Vial supports keymap editing but cannot display custom controls.

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

## Hardware and license notes

The ANSI matrix and SPI protocol follow the supplied `hhkb_hybrid` research. Its `nrf_spi.c` and `power.c` have MIT headers; only protocol handling and the STOP wake sequence were adapted. See the attribution and full upstream notice in [LICENSE.protocol](modules/hybrid/LICENSE.protocol). Factory and patched PFU containers remain outside Git.

JIS coordinates and the Hybrid factory calibration map remain unverified. A successful build does not establish that the keyboard boots, pairs, wakes from STOP, or reaches the expected battery current on real hardware.
