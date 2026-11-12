**Language:** [English](backup-restore.md) | [Русский](../ru/backup-restore.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [VIA](via.md)

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
dfu-util -a 0 -s 0x08010000 -D build/hhkb/dmk_hhkb_classic_stm32l072.bin
```

ROM DFU does not guarantee that PFU Bank 1 metadata or EEPROM flags are updated. After reset, Bank 1 can reject the image. For normal installation use the PFU updater. A readback confirms Flash contents, not Bank 1 acceptance:

```bash
dfu-util -a 0 -s 0x08010000:65536 -U bank2-readback.bin
cmp -n "$(wc -c < build/hhkb/dmk_hhkb_classic_stm32l072.bin)" \
  build/hhkb/dmk_hhkb_classic_stm32l072.bin bank2-readback.bin
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

// 4. Internal EEPROM: 6 KB, Topre calibration, serial number, and flags
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

With `HHKB_SW6_REBOOT_TO_BANK1=1`, starting DMK with SW6 ON invalidates Bank 2 and erases its first 128-byte vector page, then resets into Bank 1. The erased page contains the initial stack pointer and reset vector, so Bank 1 cannot transfer execution to Bank 2 again. Bank 2 will remain unbootable across power cycles until a complete compatible firmware image is written back to Bank 2 through the PFU updater, DFU, or SWD.

After this transition, Bank 1 can install the latest matching stock firmware from the [official HHKB download centre](https://happyhackingkb.com/download/) through Keymap Tool or `hhkb_flasher.py`. Select the correct Classic or Classic Type-S HFB. Keep SW6 OFF during normal use.
