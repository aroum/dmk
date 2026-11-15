**Language:** [English](install.md) | [Русский](../ru/install.md)

---

# Installation & Flashing Guide

This guide describes how to build, flash, and perform the initial calibration of the DMK firmware on the **Le Capybara** keyboard.

> [!WARNING]
> Flashing microcontrollers carries inherent risks. Ensure that your board is supplied with stable USB power during the flashing process.

---

## Memory Map & Flashing Parameters

The Le Capybara keyboard utilizes an **STM32F072C8T6** MCU with 128 KB of on-chip Flash memory and 16 KB of SRAM.

- **Base Flash Address:** `0x08000000` (Offset 0)
- **Vector Table Address:** `0x08000000`
- **Bootloader:** ST System ROM DFU Bootloader (`0x1FFFC800`, USB VID:PID `0483:df11`)
- **Settings & Calibration Storage:** Flash Page 62 (`0x0801F000`, 2 KB reserved for EEPROM emulation)

---

## Prerequisites

Ensure you have the required ARM toolchain and flashing utility installed:

```bash
# macOS (via Homebrew)
brew install arm-none-eabi-gcc cmake ninja dfu-util

# Debian / Ubuntu
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi cmake ninja-build dfu-util
```

### Linux udev Rules

If you are using Linux, allow non-root users access to the ST DFU bootloader:

```bash
sudo tee /etc/udev/rules.d/50-dfu-util.rules << 'EOF'
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", MODE="0666"
EOF
sudo udevadm control --reload-rules
sudo udevadm trigger
```

---

## Entering DFU Bootloader Mode

There are two primary methods to put the Le Capybara into DFU mode:

### 1. Hardware Method
- Hold down the **BOOT0** button (or short the BOOT0 solder pads).
- Connect the USB cable to your computer (or press the RESET button while holding BOOT0).
- Release BOOT0 after connecting.
- Verify that the device is detected in DFU mode:
  ```bash
  dfu-util -l
  # Output should list: Found DFU: [0483:df11] ver=2200, devnum=..., cfg=1, intf=0, path=..., alt=0, name="@Internal Flash  /0x08000000/064*0002Kg"
  ```

### 2. Software Method
- If the keyboard is already running DMK firmware:
  - Press the dedicated bootloader keycode (e.g. `K_BOOT` / `QK_BOOT`) if assigned in your keymap.
  - Or click the **Reset into Bootloader** action from the VIA / Vial interface.

---

## Flashing the Firmware

### Using CMake

From the root of the DMK repository:

```bash
# Build the target
cmake -B build/le_capybara -DKEYBOARD=le_capybara
cmake --build build/le_capybara

# Flash directly to hardware
cmake --build build/le_capybara --target flash
```

### Using `dfu-util` Directly

You can also invoke `dfu-util` manually on the generated binary:

```bash
dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D build/le_capybara/dmk_le_capybara_stm32f072.bin
```

The `:leave` flag instructs the STM32 to automatically exit DFU mode and execute the new firmware upon completion.

---

## Post-Flashing Initial Calibration

Topre / electro-capacitive keyboards rely on capacitive sensing influenced by PCB tolerances and conical spring seating.

> [!TIP]
> After your first flash, perform key calibration in VIA to ensure optimal switch responsiveness. Follow the step-by-step instructions in the [VIA Configuration Guide](via.md#key-calibration-process).
