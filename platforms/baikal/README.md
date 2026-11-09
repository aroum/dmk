# Baikal BE-U1000 Platform

Support for Baikal Electronics **BE-U1000** (RISC-V 32-bit RV32IMC core) microcontroller in DMK Firmware.

## Features

- **Architecture**: RISC-V RV32IMC @ up to 100 MHz.
- **Memory**: 256 KB Flash (16 KB for the bootloader, 240 KB for the application); 32 KiB SRAM and 160 KiB TCM shared by cores 0 and 1. Core 2 has another 10 KiB of TCM.
- **USB Controller**: Integrated USB 2.0 High Speed (480 Mbps) PHY powered by unified TinyUSB stack.
- **High Speed USB Polling**: Supports up to **8 kHz (125 µs polling interval)** via `-d HIGH_SPEED_USB`.
- **Bootloader**: UF2 drag-and-drop mass storage bootloader support:
  - Repository: [baikal-uf2-bootloader](https://github.com/aroum/baikal-uf2-bootloader)
  - Bootloader volume name: `BAIKAL_LI`

The FreeRTOS dynamic heap, newlib heap and interrupt stack are placed in shared TCM; ordinary static data remains in SRAM. The FreeRTOS heap defaults to 3 KiB and can be enlarged with `-d 'configTOTAL_HEAP_SIZE=...'`. USB DMA uses core 0's front-port alias for TCM addresses. The linker reports SRAM and TCM usage separately.

DMK executes on core 0 and does not start cores 1 or 2. Their actual reset and clock states after the bootloader hands over control have not been verified on hardware.

## Bootloader & Flashing

1. **Enter Bootloader Mode**:
   - Double-tap the hardware Reset button.
   - Or hold the Boot button while pressing Reset.
   - Or trigger `K_BOOT` from the keyboard layout (e.g. on the Adjust layer).
2. **Flash Firmware**:
   - The device enumerates as a USB Mass Storage drive labeled `BAIKAL_LI`.
   - Copy the generated `.uf2` file to `/Volumes/BAIKAL_LI/` (or run `./build_all.sh ... -u`).
