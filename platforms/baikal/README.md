# Baikal BE-U1000 Platform

Support for Baikal Electronics **BE-U1000** (RISC-V 32-bit RV32IMC core) microcontroller in DMK Firmware.

## Features

- **Architecture**: RISC-V RV32IMC @ up to 100 MHz.
- **Memory**: 256 KB Flash (16 KB allocated for bootloader, 240 KB for DMK application), 64 KB RAM.
- **USB Controller**: Integrated USB 2.0 High Speed (480 Mbps) PHY powered by unified TinyUSB stack.
- **High Speed USB Polling**: Supports up to **8 kHz (125 µs polling interval)** via `-d HIGH_SPEED_USB`.
- **Bootloader**: UF2 drag-and-drop mass storage bootloader support:
  - Repository: [baikal-uf2-bootloader](https://github.com/aroum/baikal-uf2-bootloader)
  - Bootloader volume name: `BAIKAL_LI`

## Bootloader & Flashing

1. **Enter Bootloader Mode**:
   - Double-tap the hardware Reset button.
   - Or hold the Boot button while pressing Reset.
   - Or trigger `K_BOOT` from the keyboard layout (e.g. on the Adjust layer).
2. **Flash Firmware**:
   - The device enumerates as a USB Mass Storage drive labeled `BAIKAL_LI`.
   - Copy the generated `.uf2` file to `/Volumes/BAIKAL_LI/` (or run `./build_all.sh ... -u`).
