**Language:** [English](backup-restore.md) | [Русский](../ru/backup-restore.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Backup & restore

Back up Flash, EEPROM, and Option Bytes before any update. EEPROM includes board-specific factory thresholds and user settings; do not copy EEPROM from another keyboard.

## STM32 ROM DFU

STM32 system-ROM DFU is independent of DMK, Bank 1, and the installed Bank 2 image. It works without a debugger.

1. Open the keyboard and remove the metal shield on the controller side.
2. While connecting USB, short the **right-hand pads of R6 and R85**. This raises BOOT0 and starts the STM32 system bootloader. Determine “right-hand” from the photograph, not from an arbitrary board orientation; verify the resistor markings and pads before shorting.
3. The source observation reports an orange LED. Confirm the mode over USB: STM32 DFU normally appears as `0483:df11`, then remove the short.

![R6 and R85 pads for ROM DFU](https://i.imgur.com/oTwQLPg.jpeg)

Check the device:

```bash
dfu-util -l
```

### Back up with DFU

```bash
dfu-util -a 0 -s 0x08000000:131072 -U hhkb-flash-full.bin
dfu-util -a 2 -s 0x08080000:6144 -U hhkb-eeprom.bin
dfu-util -a 1 -s 0x1FF80000:32 -U hhkb-option-bytes.bin
```

The full Flash dump includes Bank 1 and Bank 2. It does not include EEPROM or Option Bytes.

### Restore with DFU

DFU writes a raw `.bin`, not an `.hfb`. To restore only Bank 2 from a backup of the **same keyboard**:

```bash
dfu-util -a 0 -s 0x08010000 -D hhkb-bank2-stock-firmware_64k.bin
```

To restore a complete same-board backup, write Flash and EEPROM separately. Option Bytes should be restored only when necessary and only from that same backup.

```bash
dfu-util -a 0 -s 0x08000000 -D hhkb-full-flash_128k.bin
dfu-util -a 2 -s 0x08080000 -D hhkb-eeprom_6k.bin
# Only if needed, from the same board:
dfu-util -a 1 -s 0x1FF80000 -D hhkb-option-bytes.bin
```

Writing a DMK Bank 2 image directly is possible:

```bash
dfu-util -a 0 -s 0x08010000 -D build/hhkb/dmk_hhkb_stm32l072.bin
```

ROM DFU does not guarantee that PFU Bank 1 metadata or EEPROM flags are updated. After reset, Bank 1 can reject the image. For normal installation use the PFU updater. A readback confirms Flash contents, not Bank 1 acceptance:

```bash
dfu-util -a 0 -s 0x08010000:65536 -U bank2-readback.bin
cmp -n "$(wc -c < build/hhkb/dmk_hhkb_stm32l072.bin)" \
  build/hhkb/dmk_hhkb_stm32l072.bin bank2-readback.bin
```

## Full SWD / J-Link backup

The HHKB Classic SWD connection uses five pads beside connector CN3, arranged in two rows:

```text
GND  NRST  VCC
 SWCLK SWDIO
```

#### J-Link wiring for the five SWD pads beside CN3

| Pad   | STM32L072 signal          | J-Link signal    |
| ----- | ------------------------- | ---------------- |
| GND   | Ground                    | GND              |
| NRST  | NRST                      | RESET (optional) |
| VCC   | 3.3 V / reference voltage | VTref            |
| SWCLK | PA14                      | SWCLK            |
| SWDIO | PA13                      | SWDIO            |

> [!TIP]
> The keyboard may be powered by USB. In that case connect its VCC to J-Link VTref only to sense the 3.3 V reference voltage. Alternatively supply 3.3 V from the programmer. Do not use both power sources at once.

### Automatic full-backup script

Create `backup_jlink.jlink`:

```text
r
halt
// 1. Full Flash dump: 128 KB = Bank 1 + Bank 2
savebin hhkb_full_flash_128k.bin 0x08000000 0x00020000

// 2. Original Bank 1: 64 KB
savebin hhkb_bank1_bootloader_64k.bin 0x08000000 0x00010000

// 3. Original Bank 2: 64 KB
savebin hhkb_bank2_stock_firmware_64k.bin 0x08010000 0x00010000

// 4. Internal EEPROM: 6 KB, EC calibration, serial number, and flags
savebin hhkb_eeprom_6k.bin 0x08080000 0x00001800

// 5. STM32L072 Option Bytes: protection and boot parameters
savebin hhkb_option_bytes.bin 0x1FF80000 0x00000020

q
```

Run the backup:

```bash
JLinkExe -device STM32L072RB -if SWD -speed 4000 -autoconnect 1 -CommanderScript backup_jlink.jlink
```

### Restore through J-Link

To restore Flash and EEPROM for the same keyboard, create `restore_jlink.jlink`:

```text
r
halt
loadbin hhkb_full_flash_128k.bin 0x08000000
loadbin hhkb_eeprom_6k.bin 0x08080000
r
g
q
```

This restores Flash and EEPROM from the full backup. It intentionally does not write Option Bytes; restore those only when necessary and only from a backup of the same board.

## Bank layout

| Area   | Address                 | Purpose                                                 |
| ------ | ----------------------- | ------------------------------------------------------- |
| Bank 1 | `0x08000000–0x0800FFFF` | Original PFU updater, B4.16 in the saved Classic backup |
| Bank 2 | `0x08010000–0x0801FFFF` | Original A4.29 or DMK                                   |
| EEPROM | `0x08080000–0x080817FF` | Factory data and DMK settings                           |

## Restore

For normal recovery, use SW6 to enter Bank 1 and install the matching official `.hfb` with the [HHKB download centre (Keymap Tool and firmware)](https://happyhackingkb.com/download/) or `hhkb_flasher.py`. The official tool does not recognize a running DMK USB device.

DFU or SWD can restore a saved Bank 2 image. Restoring all Flash, EEPROM, or Option Bytes is an advanced operation: use backups from the same keyboard and keep the original files unchanged.

## SW6 recovery

SW6 recovery is enabled by default for Classic and Hybrid, including ANSI and JIS. Return SW6 to OFF after entering Bank 1. The Hybrid recovery path still needs testing on hardware.

With `HHKB_SW6_REBOOT_TO_BANK1=1`, starting DMK with SW6 ON invalidates Bank 2 and erases its first 128-byte vector page, then resets into Bank 1. The erased page contains the initial stack pointer and reset vector, so Bank 1 cannot transfer execution to Bank 2 again. Bank 2 will remain unbootable across power cycles until a complete compatible firmware image is written back to Bank 2 through the PFU updater, DFU, or SWD.

After this transition, Bank 1 can install the latest matching stock firmware from the [official HHKB download centre](https://happyhackingkb.com/download/) through Keymap Tool or `hhkb_flasher.py`. Select the correct Classic or Classic Type-S HFB. Keep SW6 OFF during normal use.

## Hybrid only: complete backup before update experiments

The local patch retains proprietary PFU nRF firmware, so the generated HFB cannot be distributed as a DMK release. The MIT-licensed reverse engineering of the STM32–Nordic SPI protocol and STOP wake sequence is credited in [LICENSE.protocol](../../modules/hybrid/LICENSE.protocol); it does not remove the proprietary code from the patched image.

A possible future approach is to supply an intentionally invalid nRF update payload so the radio rejects it and keeps its original firmware, while the STM32 update proceeds. This has **not** been tested: the PC updater may reject the container first, or the radio update may behave differently. Before attempting any firmware modifications or experimental updates, **complete backups of both chips (STM32 and Nordic nRF52832) are critically required** to prevent permanently bricking the Bluetooth subsystem.

### Hybrid hardware interfaces and pinout for backup and debugging

The HHKB Hybrid PCB features dedicated programming and debug access points for both microcontrollers:

#### 1. STM32 Debug Interface (CN3 Pads)
The five test pads near connector CN3 arranged in two rows:
```text
GND   NRST  VCC
 SWCLK SWDIO
```
- **SWDIO:** PA13
- **SWCLK:** PA14
- **NRST:** STM32 Reset pin
- **VCC / GND:** 3.3V power and ground

STM32 backup is captured standardly via J-Link or OpenOCD (128 KB Flash + 6 KB EEPROM + Option Bytes), exactly as on the Classic.

#### 2. Factory Interface CN6
Connector / test pads **CN6** on the board are likely the UART interface used during factory testing or programming of the Nordic Bluetooth chip.

#### 3. Nordic nRF52832 Debug Interface (Taiyo Yuden EYSHCNZWZ)
According to the official [Taiyo Yuden EYSHCNZWZ Datasheet (PDF)](https://docs.rs-online.com/2d1f/0900766b81699ea1.pdf) (*Pin Descriptions* and module footprint), the following pins are used for debug access and dumps (via J-Link / nrfjprog / OpenOCD):
- **Pin 14:** `SWDIO`
- **Pin 15:** `SWDCLK` (Pin 15 is marked directly on the PCB silkscreen, serving as a reference point to locate and count surrounding pins)
- **Pin 20:** `RESET` (nRF52 P0.21 / nRESET)
- **Power:** VDD (Pin 11/12, 1.7–3.6V) and GND (Pins 42–45, 47, 48)

> [!IMPORTANT]
> To dump the full Flash (512 KB) and UICR from the nRF52832, connect a programmer (such as J-Link) to `SWDIO`, `SWDCLK`, `GND`, and `VDD`. Verify whether readback protection (APPROTECT) is active prior to attempting full dumps.

---

### Inter-chip SPI Connection (STM32 ↔ Nordic nRF52832)

In the HHKB Hybrid architecture, **Nordic acts as the SPI Master**, while **STM32 operates as the SPI Slave**:

- **SPI Configuration:**
  - **STM32:** `SPI1` peripheral in Slave mode
  - **Nordic:** `SPI0` peripheral in Master mode
  - **SPI Mode:** Mode 3 (`CPOL = 1`, `CPHA = 1`)
  - **Format:** 8-bit, MSB first
  - **Nordic SPI Clock:** 125 kHz

- **SPI Pin Connections (mapped to [EYSHCNZWZ datasheet](https://docs.rs-online.com/2d1f/0900766b81699ea1.pdf)):**
  - **SCK:** STM32 `PB3` ↔ Nordic `P0.25` (**Module Pin 27**)
  - **MISO:** STM32 `PB4` ↔ Nordic `P0.24` (**Module Pin 26**)
  - **MOSI:** STM32 `PB5` ↔ Nordic `P0.23` (**Module Pin 25**)
  - **CS / NSS:** STM32 `PA15` ↔ Nordic `P0.22` (**Module Pin 24**)

- **Control and Interrupt Signals:**
  - **STM32 `PB6` → Nordic `P0.18`:** Attention / Request line. When the STM32 has pending key reports or battery status updates, it issues a short `LOW` pulse to prompt Nordic into initiating an SPI transaction.
  - **STM32 `PD2`:** Nordic enable control line. Driven `LOW` to enable the radio module during normal keyboard operation.

> [!TIP]
> If you have a logic analyzer or an RP2040 configured as a logic probe, capturing these signals (SCK, MISO, MOSI, CS, Attention PB6) during bootup and during official firmware updates will provide valuable insight into protocol transactions.
