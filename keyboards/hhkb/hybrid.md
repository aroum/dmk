# HHKB Hybrid ANSI development build

The Classic and Hybrid share the EC scan engine. The reference Hybrid ANSI layout matches the Classic's 60 matrix coordinates in key order. Both use four rows (PB0, PB1, PB2, PB10), fifteen columns selected through PB12–PB14, PC6 and PB15, and PA1 / ADC1_IN1. Analog power, op amp enable and capacitor discharge use PC14, PC7 and PC8. The Hybrid factory EEPROM calibration map and the JIS key coordinates still need verification. Run DMK user calibration on Hybrid.

Build the Bank 2 STM32 application and patch a privately supplied PFU A0.48 container:

```sh
./build_all.sh -b hhkb -d HHKB_HYBRID,HHKB_RAPID_TRIGGER -c
mkdir -p keyboards/hhkb/local
python3 tools/hhkb/patch_hybrid_hfb.py \
  build/hhkb-script/dmk_hhkb_stm32l072.bin \
  keyboards/hhkb/local/HHKB800_FW_A048.hfb \
  keyboards/hhkb/local/dmk_hybrid.hfb
```

The patcher verifies the stock template SHA-256 and the new application's vector table. It replaces only the first 64 KiB of payload, pads with FF, and recalculates the two CRC fields. The original nRF image, footer and version are preserved. Keep the downloaded image, patched image and backups in the ignored `local/` directory. The Classic flasher is not a Hybrid installer. This package has not been tested on hardware.

## Bluetooth and power
When PC13 indicates USB power, the firmware uses USB and applies the VIA scan rate. On battery it routes 6-key keyboard reports to the stock nRF52832 and scans at 200 Hz (5 ms, as in the reference low-power build). In VIA, choose **Bluetooth → Battery matrix scan rate**: 50, 125, 200, 250, 500 or 1000 Hz. The selection applies live and is saved in EEPROM. At 1000 Hz ADC oversampling is bypassed. Sustained throughput and current draw still need to be measured on Hybrid hardware. The matrix cadence is separate from the BLE connection interval.

The stock nRF is the SPI master on PA15/PB3–PB5. It uses mode 3 and sends a query before reading a reply starting with AA. PB6 requests a poll and PD2 enables the radio. The current transport supports the startup handshake, keyboard reports, profile selection and explicit pairing. The VIA **Bluetooth** menu exposes profile selection and pairing. The **BT Select 1–4** and **BT Pair 1–4** custom keycodes provide the same actions on battery. Pairing may replace the selected bond.

The Bluetooth sleep timeout defaults to 10 minutes (0 disables it). The case power button wakes the keyboard. With **Wake with matrix** enabled, the sensor circuit is powered briefly for a scan once a second. Otherwise matrix scanning stops, and the ADC, sensor board, op amp and scan timer interrupts are disabled. The firmware still polls the case button and USB power. STM32 STOP, radio shutdown, media reports and host LED feedback over SPI await hardware validation.

Protocol reference: l3tnun's locally supplied `hhkb_hybrid`, `docs/reverse-engineering/bluetooth-protocol.md` and MIT-licensed `nrf_spi.c`. The small protocol adaptation carries the upstream MIT notice in `modules/hybrid/LICENSE.protocol`.
