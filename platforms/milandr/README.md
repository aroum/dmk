# Milandr Platform (K1986BE92FI / MDR32F9Q2I)

Support for 32-bit ARM Cortex-M4 microcontroller **K1986BE92FI** (MDR32F9Q2I) by Milandr in DMK Firmware.

## Bootloaders & Flashing

The platform supports two bootloader modes: **UF2 Mass Storage** (default for USB flashing) and **USB DFU**.

### 1. UF2 Bootloader (Recommended)
- **Repository**: [K1986BE92FI-uf2-bootloader](https://github.com/aroum/K1986BE92FI-uf2-bootloader)
- **Drive volume label**: `K1986VE92FI`
- **Memory Layout**: 16 KB offset (`0x08004000`), 112 KB Flash available for application.
- **Entering Bootloader Mode**:
  - Double-tap the Reset button (within 500 ms).
  - Or hold the `PB6` button while resetting.
  - Or trigger `K_BOOT` from the keyboard layout (e.g. on the Adjust layer).
- **Flashing**:
  1. Build firmware in UF2 format (enabled automatically with `-u` or `--uf2`):
     ```bash
     ./build_all.sh -b corne --mcu milandr -cu --uf2
     ```
  2. Drag and drop `build/dmk_corne_milandr.uf2` onto the `K1986VE92FI` drive (or let `-u` copy it automatically).

### 2. DFU Bootloader
- **Repository**: [K1986BE92FI-dfu-bootloader](https://github.com/aroum/K1986BE92FI-dfu-bootloader)
- **Memory Layout**: 4 KB offset (`0x08001000`), 124 KB Flash available for application.
- **Flashing via USB DFU**:
  1. Enter bootloader mode by holding `PB6` while resetting.
  2. Flash via `dfu-util` using `--dfu` flag:
     ```bash
     ./build_all.sh -b corne --mcu milandr -cu --dfu
     ```
     Or manually:
     ```bash
     dfu-util -a 0 -d 1209:be92 -D build/dmk_<keyboard>_milandr.bin
     ```
