# Milandr Platform (K1986BE92FI / MDR32F9Q2I)

Support for 32-bit ARM Cortex-M4 microcontroller **K1986BE92FI** (MDR32F9Q2I) by Milandr in DMK Firmware.

## Bootloader & Flashing

- **DFU Bootloader Repository**: [K1986BE92FI-dfu-bootloader](https://github.com/aroum/K1986BE92FI-dfu-bootloader)
- **Flashing via USB DFU**:
  1. Enter bootloader mode by holding `PB6` while resetting.
  2. Flash via `dfu-util` or `./build_all.sh ... -u`:
     ```bash
     dfu-util -a 0 -d 1209:be92 -D build/dmk_<keyboard>_milandr.bin
     ```
