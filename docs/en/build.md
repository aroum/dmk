🌐 **Language / Язык:** [English](build.md) | [Русский](../ru/build.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md)

---

## Prerequisites

Building the firmware requires the `ARM GCC` toolchain and `uv`.

### Linux (Ubuntu/Debian)

1. Install system utilities and build tools:

   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake ninja-build git python3 python3-pip
   ```

2. Install ARM GCC and RISC-V GCC (for Baikal MCUs) toolchains:

   ```bash
   sudo apt install -y gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi gcc-riscv64-unknown-elf
   ```

3. Install `uv` (recommended Python package installer):

   ```bash
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```

### macOS

1. Install Homebrew (if not already installed):

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. Install tools and compiler toolchain:

   ```bash
   brew install cmake ninja git python
   brew tap osx-cross/arm
   brew install arm-gcc-bin
   ```

3. (Optional) Install USB flashing tools:

   ```bash
   brew install dfu-util
   brew install picotool
   brew install --cask nrfutil
   ```

4. Install `uv`:

   ```bash
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```

### Windows

1. **Using Scoop (Recommended)**:
   Open PowerShell and run:

   ```powershell
   # Install Scoop (if not installed)
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression

   # Install required tools
   scoop install git cmake ninja python uv

   # Install ARM GCC toolchain
   scoop bucket add extras
   scoop install gcc-arm-none-eabi
   ```

2. **Using winget**:
   Open Command Prompt / PowerShell and run:

   ```cmd
   winget install Kitware.CMake
   winget install Python.Python.3
   winget install Git.Git
   winget install Ninja-build.Ninja
   ```

   For the ARM GCC toolchain, download the installer directly from [ARM Developer website](https://developer.arm.com/downloads/-/gnu-rm) and add its `bin` directory to your System PATH.
3. Install `uv`:

   ```powershell
   powershell -c "irm https://astral.sh/uv/install.ps1 | iex"
   ```

---

## Usage

To run the build, use the `build_all.sh` script:

```
./build_all.sh [FLAGS]
```

## Description of Arguments and Flags

### Main Parameters

- `--keyboard [NAME]` / `-b [NAME]` — Keyboard selection. `NAME` corresponds to the name of the subfolder in the `keyboards` directory.
- `--probe [PROBE]` / `-p [PROBE]` — Debugger selection for flashing `milandr`. Default is `j-link`.
- `--mcu [MCU]` — Target microcontroller selection (`milandr`, `rp2040`, `rp2350`, `nrf52840`, `baikal`). Default is `milandr`.
- `--memory [SIZE]` — Flash memory size selection (e.g., `256KB`, `512KB`, `2MB`, `4MB`, `16MB`). Used for boards with external flash memory.
- `--define [DEFS]` / `-d [DEFS]` — Compiler definitions, e.g., `-d LEFT, 6_COL_LAYOUT, KEEPER_KEYMAP`.
- `--uf2` — Generate a UF2 file (for RP2040/RP2350/nRF52840).
- `--lang [LANG]` — Set script language (`ru` or `en`).

### Combined Flags

You can combine flags (e.g., `-cf`):

- `-c`, `--clean` — Perform a "clean" build (clear temporary files).
- `-f`, `--flash` — Flash via external debugger.
- `-u`, `--usb-flash` — Flash via USB (for RP2040/RP2350, nRF52, and Milandr).
- `-h`, `--help` — Show help.

## Flashing Methods & Tools

You can flash the target microcontroller using different methods depending on your platform and hardware debugger availability:

### 1. UF2 Flashing (Drag-and-Drop Mass Storage)

For development boards with UF2 bootloaders (RP2040, RP2350, nRF52840):

1. Hold the `BOOTSEL` / `RESET` button while connecting USB to enter bootloader mode. The board will mount as a USB flash drive.
2. Drag and drop the generated `.uf2` file from the `build/` directory onto the drive. The board will flash and reboot automatically.

### 2. Automated USB Flashing Tools (`-u`, `--usb-flash`)

- **RP2040 / RP2350**: [`picotool`](https://github.com/raspberrypi/picotool) — Automated USB bootloader flashing.
  - macOS: `brew install picotool`
- **nRF52840**: [`nrfutil`](https://pypi.org/project/nrfutil/) / `adafruit-nrfutil` — Flashing via Adafruit DFU bootloader.
  - macOS: `brew install --cask nrfutil`
  - Linux / Windows: `pip install nrfutil` / `pip install adafruit-nrfutil`
- **Milandr (K1986BE92FI)**: [`dfu-util`](http://dfu-util.sourceforge.net/) — Flashing with [K1986BE92FI-dfu-bootloader](https://github.com/aroum/K1986BE92FI-dfu-bootloader).
  - macOS: `brew install dfu-util`
  - Linux: `sudo apt install dfu-util`

### 3. External Debugger Flashing (`-f`, `--flash`)

- **[OpenOCD](https://openocd.org/)**: Used for direct SWD/JTAG flashing (J-Link, ST-Link, CMSIS-DAP). Supports Milandr, RP2040, nRF52840, and Baikal.

Install OpenOCD via your package manager:

- **macOS (Homebrew)**: `brew install open-ocd`
- **Debian / Ubuntu**: `sudo apt install openocd`
- **Fedora**: `sudo dnf install openocd`
- **Windows (MSYS2)**: `pacman -S mingw-w64-x86_64-openocd`

Utility Links:

- **picotool (RP2040/RP2350)**: <https://github.com/raspberrypi/picotool>
- **nrfutil (nRF52840)**: <https://pypi.org/project/nrfutil/>
- **dfu-util (USB DFU)**: <http://dfu-util.sourceforge.net/>
- **OpenOCD (SWD/JTAG)**: <https://openocd.org/>
- **Milandr DFU Bootloader**: <https://github.com/aroum/K1986BE92FI-dfu-bootloader>

## Usage Examples

### Milandr

**Full build with flashing via debugger and clean for a split keyboard:**

```bash
./build_all.sh -b corne -d LEFT -cf
./build_all.sh -b corne -d RIGHT -cf
```

**Clean build and flash via USB DFU:**

```bash
./build_all.sh -b corne -d LEFT -cu
```

### Raspberry Pi (RP2040/RP2350)

**Build with UF2 generation:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040
```

**Clean build and flash via USB:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040 -cu
```

**Clean build, UF2 and USB flash at the same time:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040 -cu --uf2
```

**Clean build specifying memory size:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040 -cf --memory 512KB
```

## Automated Build in Custom Repository (GitHub Actions)

You can maintain your keyboard configuration in a separate lightweight repository and automatically build the firmware binary on every `git push` via GitHub Actions.

### Repository Directory Structure

Your configuration repository should have the following file layout:

```text
my-dmk-config/
├── .github/
│   └── workflows/
│       └── build.yml               # GitHub Actions workflow file
└── keyboards/
    └── my_keyboard/                # Your keyboard name (passed to -b)
        ├── config.h                # Matrix and pin configurations
        ├── keymap.c                # Keymaps definition
        └── vial.json               # (Optional) Vial layout configuration
```

### Workflow File Template (`.github/workflows/build.yml`)

Here is a ready-to-use workflow template:

```yaml
name: Build DMK Firmware

on:
  push:
    branches: [ main, master ]
  pull_request:
    branches: [ main, master ]
  workflow_dispatch:

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout User Configuration
        uses: actions/checkout@v4
        with:
          path: user_config

      - name: Checkout DMK Firmware Core
        uses: actions/checkout@v4
        with:
          repository: aroum/dmk
          path: dmk
          submodules: recursive

      - name: Install Toolchain and Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi

      - name: Install uv
        uses: astral-sh/setup-uv@v5

      - name: Build Firmware
        run: |
          # Copy user keyboard configuration into DMK firmware tree
          mkdir -p dmk/keyboards
          cp -r user_config/keyboards/* dmk/keyboards/

          cd dmk
          # Replace 'my_keyboard' and 'rp2040' with your keyboard name and target MCU
          ./build_all.sh -b my_keyboard --mcu rp2040 -c --uf2

      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: dmk-binaries
          path: |
            dmk/build/dmk_*
          if-no-files-found: error
```

A reference template repository is available at `https://github.com/aroum/dmk-config-template`.

## Unified uv Environment

All tools (`tools/nrfutil`, `tools/uf2`) share a single unified environment defined in the root `pyproject.toml` file.

To synchronize dependencies, run:

```bash
uv sync
```

This creates a single `.venv` folder in the project root. The build scripts (`build_all.sh` and `build_all.ps1`) automatically call `uv run --project . ...` using this root environment.

## Automatic Port Detection for nice!nano

When flashing nRF52840, you can specify `--nrf-port auto` to automatically detect and select the serial port containing `nice!nano` in its description/name. If the device is not found, the script will fall back to listing all available ports for interactive selection.

```bash
./build_all.sh -b kabarga --mcu nrf52840 -cu --nrf-port auto
```
