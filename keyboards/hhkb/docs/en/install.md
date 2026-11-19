**Language:** [English](install.md) | [Русский](../ru/install.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Install

> [!WARNING]
> You flash the keyboard and use DFU/SWD at your own risk. The author accepts no responsibility for keyboard damage, data loss, or consequences of selecting an incorrect firmware image, model, or recovery method.

> [!TIP]
> **Recommended initial key calibration after first flashing:**
> While the firmware includes robust default capacitive baseline thresholds, it is strongly recommended to calibrate all keys (60 keys for ANSI or 69 for JIS) after initial flashing to match your specific PCB and conical spring tolerances. Step-by-step instructions and visual verification via the *Key Tester* (*Test Matrix*) tab are provided in the [VIA guide](via.md#key-calibration-process).

## Before flashing

Build or download the matching firmware for your model first:
- **Classic (ANSI):** `AHHX01` container (A4.50)
- **Classic Type-S (ANSI):** `AHHS01` container (A8.50)
- **Classic Type-S (JIS):** `AHKS01` container (A9.50)
- **Hybrid (ANSI / JIS):** patched `AHUX01` or `AHJX01` container produced via `patch_hybrid_hfb.py`

Keep a [full backup](backup-restore.md) before the first update (Flash + EEPROM for STM32, and Flash + UICR for Nordic nRF52832 on Hybrid).

DMK occupies Flash Bank 2 (`0x08010000–0x0801FFFF`). The original Bank 1 updater remains in place and provides the PFU update interface.

DMK uses a different USB identity, so Keymap Tool cannot flash it directly. If DMK is already installed, first return to the stock Bank 1 updater.

With the keyboard powered off, set SW6 to ON, connect power, and return SW6 to OFF after Bank 1 starts. This recovery action erases the first 128-byte page of Bank 2. See [SW6 recovery](backup-restore.md#sw6-recovery) for the detailed description of this irreversible transition to Bank 1. Then flash the generated `.hfb` with the official updater and reconnect the keyboard after it completes.

If the keyboard is currently running the original PFU firmware, its PFU update interface is already available: do **not** use SW6.

> [!WARNING]
> Make sure SW6 is OFF before flashing.

## Install through the PFU updater

Open the matching `.hfb` in HHKB Keymap Tool and complete the normal update.

After a successful DMK update, Keymap Tool displays the following message:

> An error occurred during writing to HHKB. Check the connection with HHKB and try again. You cannot write anything to HHKB while pressing a key.

This is expected and does **not** mean that the update failed. At this point Bank 1 has written Bank 2 and transfers execution to DMK. DMK then reconnects with its own USB VID/PID, which the PFU Keymap Tool does not recognize, so the tool reports that its original PFU device disappeared.

## Install with the DMK flasher

You do not need the official Keymap Tool. `tools/hhkb/hhkb_flasher.py` talks to the PFU Raw HID update interface (VID `0x04FE`, UsagePage `0xFF00`, Usage `0x01`) exposed by Bank 1 and provides a cross-platform alternative for Linux, macOS, and Windows. Stock firmware already exposes this interface; a running DMK image has a different USB identity, so it must first be returned to Bank 1 through SW6 as described above.

```bash
uv run --with hidapi python3 tools/hhkb/hhkb_flasher.py --info
uv run --with hidapi python3 tools/hhkb/hhkb_flasher.py build/hhkb-script/<firmware>.hfb
```

Confirm with `--info` that the PFU updater sees the expected keyboard model.

### Image Checks

The local flasher validates the container strictly before transmission by default:
- **Two CRC-16 checksums:** validates both the outer container CRC-16/MCRF4XX (first 2 bytes) and the inner payload CRC-16 over the 64 KB binary.
- **Keyboard model:** validates that the hardware model (e.g. `AHHX01` for Classic, `AHHS01` for Type-S) matches the model string in the container header/trailer.
- **Reset vector address:** validates that the reset vector points into the Flash Bank 2 range (`0x08010000 <= (reset_vector & ~1) < 0x08020000`) with the Thumb bit set.

Optional CLI flags:
- `--skip-crc`: disables local CRC-16 validation.
- `--skip-model-check`: disables local model compatibility checks.
  > [!IMPORTANT]
  > `--skip-crc` and `--skip-model-check` affect only those local checks in the Python tool; they do **not** bypass validation performed by the keyboard bootloader itself.
- `--force`: independently disables reset-handler address validation for raw or custom test images.

Bank 1 validates Bank 2 before transfer of control. Because exact internal PFU metadata rules are proprietary, direct Bank 2 writes that bypass Bank 1 (such as via DFU) require hardware verification.

### Verified PFU HID Exchange

The PFU firmware update sequence over USB Raw HID has been fully analyzed and verified on physical hardware:
- **`0xE0` (Firmup Mode):** switches the controller into firmware update mode and waits for the new HID interface to re-enumerate (~10 seconds).
- **`0xE1` (Firmup Start):** transmits total container size (65,570 bytes: `0x00010022` in little-endian) and outer CRC-16, triggering an erase of Bank 2.
- **`0xE2` (Firmup Send):** transmits 65,568 container bytes (the entire container excluding the first two CRC bytes) in 57-byte payload chunks inside 64-byte HID reports with a 7-byte header `[0xAA, 0xAA, 0xE2, 0x00, len, seq_lo, seq_hi]`.
- **`0xE3` (Firmup End):** finalizes transfer, requests verification, and triggers MCU reboot.

> [!NOTE]
> On 11 November 2026, the tool wrote the original Classic A4.29 container (`AHHX01`) to hardware. A SWD readback of Bank 2 matched the container payload byte-for-byte, confirming the exactness of the transfer protocol.

### Image Layout and Versioning

The `.hfb` container is exactly **65,570 bytes**:
- `0x0000–0x0001`: outer CRC-16 (MCRF4XX) of the remaining container bytes.
- `0x0002–0x0003`: offset / header.
- `0x0004–0x10003`: 64 KB firmware payload (Flash Bank 2).
- `0x10004–0x10005`: inner CRC-16 (MCRF4XX) of the 64 KB payload.
- `0x10006–0x1000B`: ASCII model identifier (`AHHX01` for Classic, `AHHS01` for Classic Type-S).
- `0x1000C–0x1000F`: 4-byte BCD firmware version. The current build scripts (`make_hfb.py`) emit **A4.50** for `AHHX01` and **A8.50** for `AHHS01`.
- `0x10010–0x10021`: 18 bytes of 0xFF padding.

For full layout breakdown and manual packaging commands, see [Build](build.md#hfb-container-format-for-hhkb-classic).

## DFU and SWD

ROM DFU and SWD are development and recovery paths. DFU can write a raw Bank 2 `.bin`, but it does not update PFU Bank 1 metadata. Use the PFU updater for normal installation. The five SWD pads beside connector CN3 are arranged in two rows:

```text
top:     GND | RST | VCC
bottom:    CLK | DIO
```

Read [Backup & restore](backup-restore.md) before using either path.

## Hybrid installation status

A Hybrid firmware package can be [built and patched locally](build.md#hybrid-only-local-build-and-factory-image-patch). The resulting HFB contains proprietary PFU nRF firmware and is not published. Flashing via local patching of an official vendor HFB has been verified and works on physical HHKB Hybrid hardware. The DMK HID flasher accepts a complete model-matched Hybrid HFB:

```bash
uv run --with hidapi python3 tools/hhkb/hhkb_flasher.py keyboards/hhkb/local/dmk_hybrid.hfb
```

Clean standalone builds from CI (without vendor blobs) cannot be accepted by the stock updater: the Nordic nRF52832 bootloader rejects the blank radio section, after which STM32 Bank 1 aborts the entire update.

### Theoretical Multi-stage Workaround

To distribute clean DMK firmware without proprietary PFU blobs, a multi-stage flashing procedure has been proposed:

1. **Stage 1 (Temporary firmware):** Flash a temporary firmware image containing a dummy Bluetooth firmware accepted by the nRF. The running STM32 code then modifies Bank 1 to bypass the nRF update success check.
2. **Stage 2 (Restore Bluetooth):** Flash the official PFU firmware to restore the original Bluetooth firmware on the nRF module.
3. **Stage 3 (Install DMK):** Flash DMK with an invalid Bluetooth firmware image. The nRF rejects the update and keeps its stock firmware, while the modified Bank 1 considers DMK in Bank 2 valid and boots it.
4. **Stage 4 (Revert & Cleanup):** When SW6 is toggled ON to return to stock firmware, DMK restores the original validation code in Bank 1.

> [!CAUTION]
> **High risk of unrecoverable bricking:**
> This procedure carries significant risk because it involves writing to Bank 1. Bank 1 is the primary hardware recovery path through USB. If anything goes wrong during Bank 1 modification, USB recovery is permanently lost, requiring an SWD debugger to unbrick. A simpler and safer solution is preferred.
