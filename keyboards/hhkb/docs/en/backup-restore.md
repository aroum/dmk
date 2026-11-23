**Language:** [English](backup-restore.md) | [Русский](../ru/backup-restore.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA & Debug](via-debug.md)

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

SW6 recovery is enabled by default for Classic and Hybrid, including ANSI and JIS, and is verified on hardware. Return SW6 to OFF after entering Bank 1.

With `HHKB_SW6_REBOOT_TO_BANK1=1`, starting DMK with SW6 ON invalidates Bank 2 and erases its first 128-byte vector page, then resets into Bank 1. The erased page contains the initial stack pointer and reset vector, so Bank 1 cannot transfer execution to Bank 2 again. Bank 2 will remain unbootable across power cycles until a complete compatible firmware image is written back to Bank 2 through the PFU updater, DFU, or SWD.

After this transition, Bank 1 can install the latest matching stock firmware from the [official HHKB download centre](https://happyhackingkb.com/download/) through Keymap Tool or `hhkb_flasher.py`. Select the correct Classic or Classic Type-S HFB. Keep SW6 OFF during normal use.

### SW5 + SW6: erase DMK settings and enter Bank 1 (dev)

This combination is implemented in the **dev** branch. For a normal update that keeps DMK settings, use **SW5 OFF + SW6 ON**. With **SW5 ON + SW6 ON**, startup first clears recognized DMK EEPROM records, then performs the usual Bank 1 recovery. Both switches must remain ON through the debounce interval; SW5 alone never erases anything.

At startup:

| SW5 | SW6 | Recovery / erase action |
| --- | --- | --- |
| OFF | OFF | Normal DMK startup; no EEPROM erase or Bank 1 transition |
| ON | OFF | Normal DMK startup; no EEPROM erase or Bank 1 transition |
| OFF | ON | Enter Bank 1, keeping DMK settings |
| ON | ON | Erase recognized DMK records, then enter Bank 1 |

With SW6 OFF, SW5 retains its normal DIP-switch function; it never triggers recovery or erasure by itself.

1. Disconnect USB; on Hybrid also remove the batteries to force a complete startup.
2. Set both SW5 and SW6 to ON, then connect USB.
3. Keep power connected until the PFU Bank 1 USB device appears. Then set SW6 to OFF and restore SW5 to its usual position before flashing.

The erase removes the owned compressed slots and their ownership header, plus recognized older DMK1/DMK2 keymap records and DMK settings/calibration records. ANSI and JIS formats are recognized independently of the running build. Legacy snapshots and versioned calibration records require valid checksums; unknown records and records with invalid checksums are left untouched. This deliberately does **not** erase all 6 KiB of EEPROM or reset PFU keymaps. Factory sensor tables, PFU Bluetooth connection records, model/version metadata and Nordic firmware are preserved. The normal SW6 recovery path still updates its own Bank 2 status/CRC fields and erases the Bank 2 vector page.

If an EEPROM write fails, Bank 2 is not invalidated or erased: startup stops, and `hhkb_recovery_flash_error` is `0x40000000` for SWD inspection. Reconnect stable power and retry SW5+SW6. Do not disconnect power during erasure. This new erase combination has host tests and build checks; hardware verification is still required.

### Classic and Hybrid EEPROM differences

The available full EEPROM backup (`hhkb-eeprom.bin`, 6,144 bytes; paired Flash identifies **PD-KB401W**) is Classic. No separate complete Hybrid EEPROM dump was found, so the Hybrid column below is based on the stock A0.48 reverse engineering in [l3tnun/hhkb_hybrid](https://github.com/l3tnun/hhkb_hybrid), not a byte-for-byte comparison of two backups.

| Region | Available Classic backup | Hybrid A0.48 EEPROM map |
| --- | --- | --- |
| `0x08080380–0x0808047F` | All zero | Bluetooth connection journal: 16 × 16 bytes |
| `0x08080700–0x08080CFF` | All zero | Stock keymap table B, 1,536 bytes |
| `0x08081400–0x080816FF` | All zero | Second half of stock keymap table A, 768 bytes |
| `0x08081000–0x080812FF` | Individual nonzero sensor tables | Individual sensor/calibration tables; preserve the same-board data |
| Recovery fields | Backup has `0x348=0`, `0x360=1`, CRC bytes at `0x370=07 DE`; DMK Classic recovery changes `0x348` and CRC | DMK Hybrid recovery clears boot-enable byte `0x360` |

The **DMK** compressed record format is the same for Classic and Hybrid with the same matrix geometry. Hybrid power, button and LED options occupy packed reserved fields; they do not require another EEPROM region. ANSI and JIS have different ownership identifiers and calibration geometry. A zero Classic region must not be treated as free Hybrid EEPROM, and a Classic EEPROM backup must never be copied onto Hybrid.

## Flashing Methods and Limitations

Currently, there are three ways to flash DMK onto the keyboard:

1. **Hardware programmer / debugger (SWD):** connecting directly via CN3 pads (STM32) to write Bank 2 at `0x08010000`. Requires opening the case and soldering/connecting an external SWD programmer.
2. **Built-in ROM DFU mode:** shorting the right-hand pads of R6 and R85 while plugging in USB (pulling `BOOT0` high). Also requires disassembling the keyboard and bridging board pads.
3. **Official PFU updater or `hhkb_flasher.py` with a patched official `.hfb`:** replacing the STM32 section (Bank 2) while keeping the original nRF Bluetooth section intact (for Hybrid).

All three methods have trade-offs and significantly narrow down the target audience (they require hardware disassembly or access to proprietary official vendor files).

### nRF Update Validation Mechanism and Bank 1 Behavior (Hybrid)

Testing on physical hardware has revealed the following behavior:

- When attempting to flash a "clean" DMK build generated with an empty/test nRF section (such as via `patch_hybrid_hfb.py --make-stm32-only`):
  1. The Nordic nRF52832 bootloader inspects the staging area (header, CRC16, and MD5).
  2. Upon detecting an invalid CRC or MD5 mismatch, the nRF rejects the update, clears the staging area, and **safely retains its current stock firmware**.
  3. However, once the Nordic rejects the update, **STM32 Bank 1 also rejects its own update** and refuses to transfer execution to Bank 2 (it does not set the boot flag `0x08080360 = 1`). Consequently, clean standalone CI-built images cannot boot via the stock updater.

In theory, one could generate a clean firmware image that the nRF would accept. That would yield a fully working USB keyboard, but the Bluetooth module would become useless until stock PFU firmware is reflashed. A potential multi-stage bypass concept is documented in the [Installation Guide](install.md#theoretical-multi-stage-workaround).

## Backup & Restore: STM32L072CZY6 (Classic & Hybrid)

The primary host microcontroller across HHKB Classic and Hybrid is the **STMicroelectronics STM32L072CZY6** (ARM Cortex-M0+, 192 KB Flash, 20 KB SRAM, 6 KB embedded EEPROM).

### STM32 Memory Map: Differences Between Classic and Hybrid

Both families employ a dual-bank Flash structure, but boot validation flags and EEPROM layout differ:

| Region                    | Address Range                      | Classic Purpose                                   | Hybrid Purpose                                                         |
| :------------------------ | :--------------------------------- | :------------------------------------------------ | :--------------------------------------------------------------------- |
| **Bank 1**                | `0x08000000–0x0800FFFF` (64 KB)    | Factory PFU recovery bootloader                   | Factory PFU recovery bootloader (coordinates both STM32 & nRF via SPI) |
| **Bank 2**                | `0x08010000–0x0801FFFF` (64 KB)    | Application firmware (stock A4.xx / A8.xx or DMK) | Application firmware (stock A0.xx / A2.xx or DMK)                      |
| **Bank 3 / Reserved**     | `0x08020000–0x0802FFFF` (64 KB)    | Unused by PFU scheme                              | Unused by PFU scheme                                                   |
| **Embedded EEPROM**       | `0x08080000–0x080817FF` (6 KB)     | Factory calibrations, Bank 2 boot flags           | Factory calibrations, Bank 2 & Bluetooth boot flags                    |
| ↳ _Bank 2 status flags_   | `0x08080348` and `0x08080370`      | Status (`0x01` = broken) and 2-byte CRC tag       | Hybrid uses byte `0x08080360` (`0x01` = Bank 2 valid/boot-enabled)     |
| ↳ _EC Matrix calibration_ | `0x08081000–0x080812FF`            | Factory baselines and thresholds                  | Factory thresholds (structure differs)                                 |
| ↳ _User DMK Data_ | Fragmented, 1,772 bytes | [Compressed snapshots](via-debug.md#compressed-eeprom-storage): calibration, keymaps, macros, combos, settings | Same format, plus Hybrid power/button/indicator settings; PFU pairing data stays separate |
| **Option Bytes**          | `0x1FF80000–0x1FF8001F` (32 bytes) | Readout protection (RDP Level 0/1) & boot flags   | Readout protection (RDP Level 0/1) & boot flags                        |

### STM32 SWD Pinout (CN3 Pads)

The STM32 debug interface is accessible via five test pads adjacent to connector CN3 in two staggered rows:

```text
GND   NRST  VCC
 SWCLK SWDIO
```

- **SWDIO:** Pin PA13 of STM32L072
- **SWCLK:** Pin PA14 of STM32L072
- **NRST:** STM32 hardware reset line
- **VCC:** 3.3V power (or VTref reference input for programmer)
- **GND:** Ground (0V)

> [!TIP]
> The keyboard can remain powered via USB-C. In this configuration, connect VCC to the programmer's VTref pin solely for voltage sensing (do not power the board simultaneously from both USB and debugger).

### STM32 Backup via J-Link

Create a command script `backup_stm32.jlink`:

```text
r
halt
// 1. Full Flash dump: 128 KB = Bank 1 + Bank 2
savebin stm32_full_flash_128k.bin 0x08000000 0x00020000
// 2. Factory Bank 1 (PFU Bootloader): 64 KB
savebin stm32_bank1_bootloader_64k.bin 0x08000000 0x00010000
// 3. Current Bank 2: 64 KB
savebin stm32_bank2_app_64k.bin 0x08010000 0x00010000
// 4. Embedded EEPROM: 6 KB (calibration tables, serial, boot flags)
savebin stm32_eeprom_6k.bin 0x08080000 0x00001800
// 5. Option Bytes: 32 bytes
savebin stm32_option_bytes.bin 0x1FF80000 0x00000020
q
```

Execute the dump:

```bash
JLinkExe -device STM32L072RB -if SWD -speed 4000 -autoconnect 1 -CommanderScript backup_stm32.jlink
```

### STM32 Restore via J-Link

Create `restore_stm32.jlink`:

```text
r
halt
loadbin stm32_full_flash_128k.bin 0x08000000
loadbin stm32_eeprom_6k.bin 0x08080000
r
g
q
```

Execute restore:

```bash
JLinkExe -device STM32L072RB -if SWD -speed 4000 -autoconnect 1 -CommanderScript restore_stm32.jlink
```

---

## Backup & Restore: Nordic nRF52832 (Hybrid Only)

On HHKB Hybrid, the Bluetooth subsystem is integrated on a **Taiyo Yuden EYSHCNZWZ** module based on the **Nordic Semiconductor nRF52832** SoC ([Module Datasheet](https://docs.rs-online.com/2d1f/0900766b81699ea1.pdf)) (32-bit ARM Cortex-M4F @ 64 MHz, 512 KB Flash, 64 KB RAM).

### Nordic nRF52832 Memory Map

The 512 KB Flash is structured for Nordic SoftDevice S132 and PFU's custom bootloader:

| Region              | Address Range                    | Purpose                                                                                   |
| :------------------ | :------------------------------- | :---------------------------------------------------------------------------------------- |
| **MBR**             | `0x00000000–0x00000FFF` (4 KB)   | Master Boot Record                                                                        |
| **SoftDevice S132** | `0x00001000–0x00025FFF` (148 KB) | Nordic BLE protocol stack v6.0.0                                                          |
| **Application**     | `0x00026000–0x00031FFF` (48 KB)  | Stock HHKB Hybrid Bluetooth firmware                                                      |
| **Staging Area**    | `0x00036000–...`                 | SPI firmware update buffer received from STM32 (header at `0x36000`, binary at `0x37000`) |
| **FDS Storage**     | `0x00075000–0x00077FFF` (12 KB)  | Pairing & bonding data (Flash Data Storage, magic `0xDEADC0DE`)                           |
| **Bootloader**      | `0x00078000–0x0007FFFF` (32 KB)  | Vendor update bootloader `FwUpdate_*` (`UICR.BOOTLOADERADDR = 0x78000`)                   |
| **UICR**            | `0x10001000–0x100013FF` (1 KB)   | User Information Configuration Registers                                                  |

### SWD Debug Interface for Nordic nRF52832 (CN6 and Module Pins)

Debug access to the Nordic chip is provided through:

- **Connector / pads CN6:** Dedicated factory **SWD interface for the Nordic nRF52832**.
- **Module Edge Contacts (Taiyo Yuden EYSHCNZWZ):**
  - **Pin 14:** `SWDIO`
  - **Pin 15:** `SWDCLK` (Pin 15 is marked directly on PCB silkscreen as a primary reference)
  - **Pin 20:** `RESET` (nRF52 P0.21 / nRESET)
  - **VDD:** Pins 11/12 (1.7–3.6V)
  - **GND:** Pins 42–45, 47, 48

### Nordic nRF52832 Backup via J-Link / nrfjprog

Ensure that readout protection (APPROTECT) is disabled before reading.

Create `backup_nrf52.jlink`:

```text
r
halt
// 1. Full Flash dump: 512 KB (MBR + SoftDevice + App + FDS + Bootloader)
savebin nrf52_full_flash_512k.bin 0x00000000 0x00080000
// 2. UICR: 1 KB configuration registers
savebin nrf52_uicr_1k.bin 0x10001000 0x00000400
q
```

Execute the dump:

```bash
JLinkExe -device nRF52832_xxAA -if SWD -speed 4000 -autoconnect 1 -CommanderScript backup_nrf52.jlink
```

Or using Nordic's official CLI:

```bash
nrfjprog --family NRF52 --readcode nrf52_full_flash_512k.hex
nrfjprog --family NRF52 --readuicr nrf52_uicr.hex
```

### Nordic nRF52832 Restore

To restore the factory Bluetooth state:

```text
r
halt
loadbin nrf52_full_flash_512k.bin 0x00000000
loadbin nrf52_uicr_1k.bin 0x10001000
r
g
q
```

Execute restore:

```bash
JLinkExe -device nRF52832_xxAA -if SWD -speed 4000 -autoconnect 1 -CommanderScript restore_nrf52.jlink
```

Or via `nrfjprog`:

```bash
nrfjprog --family NRF52 --program nrf52_full_flash_512k.hex --chiperase --reset
```

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
