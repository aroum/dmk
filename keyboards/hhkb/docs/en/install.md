**Language:** [English](install.md) | [Русский](../ru/install.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA & Debug](via-debug.md)

# Install

> [!WARNING]
> You flash the keyboard and use DFU/SWD at your own risk. The author accepts no responsibility for keyboard damage, data loss, or consequences of selecting an incorrect firmware image, model, or recovery method.

> [!TIP]
> **Recommended initial key calibration after first flashing:**
> While the firmware includes robust default capacitive baseline thresholds, it is strongly recommended to calibrate all keys (60 keys for ANSI or 69 for JIS) after initial flashing to match your specific PCB and conical spring tolerances. Step-by-step instructions and visual verification via the *Key Tester* (*Test Matrix*) tab are provided in the [VIA guide](via-debug.md#key-calibration-process).

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

## Hybrid Installation Status: Why a Local Build Is Required

Unlike the HHKB Classic, where the update file targets only the single STM32 controller, the official HHKB Hybrid update container (`HHKB800_FW_A048.hfb`, 282,672 bytes) is a **dual-target composite container**:
1. The first 64 KB (`0x0004–0x10003`) contains firmware for the main STM32 controller (Flash Bank 2).
2. The trailing ~216 KB contains the proprietary update payload for the **Nordic nRF52832** Bluetooth SoC (SoftDevice + BLE application stack) with its own staging header and checksums.

### Why Prebuilt Hybrid HFB Releases Cannot Be Distributed
Because the nRF payload contains proprietary, closed-source code owned by PFU Limited, distributing a precompiled full `.hfb` image for Hybrid keyboards would violate copyright restrictions. The DMK repository is strictly open-source and cannot distribute vendor binary blobs.

### Why Clean CI-Generated Builds Failed on Hardware
An experimental attempt was made to generate fully clean standalone images in GitHub Actions:
- The STM32 section was populated with open-source DMK firmware.
- The nRF payload section was replaced with empty padding or dummy data carrying an intentionally invalid checksum (CRC16/MD5).

**The hypothesis:** The Nordic nRF52832 bootloader would detect the corrupt payload, reject it, and safely preserve its installed factory Bluetooth firmware, while the STM32 update would succeed.

**Actual hardware test results:**
1. The Nordic nRF bootloader behaves as predicted: it verifies the staging area, detects the MD5/CRC mismatch, purges the staging buffer, and **safely preserves the existing stock Bluetooth firmware**.
2. However, the update transaction is orchestrated by the **STM32 Bank 1 bootloader**, which polls the Nordic chip over SPI at finalization (`commandE3`).
3. Upon receiving an error/rejection status from the Nordic SoC, **STM32 Bank 1 rejects the overall update process**: it refuses to mark Bank 2 as valid (leaving boot flag `0x08080360 = 0`) and never passes execution to DMK in Bank 2.

As a result, a standalone "clean" HFB cannot be accepted by the stock bootloader without modifying Bank 1.

### The Proven Working Solution: Local Vendor Image Patching
To satisfy the Bank 1 update validator without redistributing vendor binaries, DMK provides the local patching utility `tools/hhkb/patch_hybrid_hfb.py`:
1. The user downloads the official update file directly from [PFU's official download center](https://happyhackingkb.com/download/) (`HHKB800_FW_A048.hfb` for ANSI or `HHKB820_FW_JP_A249.hfb` for JIS).
2. The patcher replaces **only the first 64 KB STM32 payload** with the freshly compiled DMK binary, preserves the authentic factory nRF payload untouched, and recalculates the outer container CRCs.
3. Because the nRF section remains authentic and valid, both microcontrollers validate successfully, and Bank 1 boots DMK on the STM32 while keeping full Bluetooth functionality.

```bash
uv run --with hidapi python3 tools/hhkb/hhkb_flasher.py keyboards/hhkb/local/dmk_hybrid.hfb
```

**Flashing via this local patching workflow is verified and fully functional on physical HHKB Hybrid hardware.**

---

### Theoretical Alternative: Multi-stage Workaround Without Patching

In theory, distributing clean DMK images without proprietary blobs could be achieved if the nRF update check in Bank 1 is bypassed. A complex 4-stage flashing sequence has been analyzed:

1. **Stage 1 (Temporary Firmware):** Flash a clean image with a dummy Bluetooth payload that satisfies nRF validation. The nRF accepts it (Bluetooth is temporarily rendered inoperable, but the update succeeds). Once booted into Bank 2, the temporary STM32 firmware writes to Bank 1 Flash to bypass the nRF success status requirement.
2. **Stage 2 (Restore Bluetooth):** Flash the official PFU update file through the vendor updater to restore genuine Bluetooth firmware on the Nordic SoC.
3. **Stage 3 (Install Clean DMK):** Flash clean DMK with an invalid nRF image. The nRF rejects the update and keeps its genuine Bluetooth firmware (from Stage 2), while the modified Bank 1 considers Bank 2 DMK valid and boots it.
4. **Stage 4 (Revert & Cleanup):** Toggling SW6 to return to stock restores the original validation logic in Bank 1.

> [!CAUTION]
> **Severe bricking risk:**
> This procedure involves writing to Bank 1, which is the **sole hardware recovery path via USB**. Any unexpected interruption, bus error, or brownout during Bank 1 modification would render the keyboard permanently unbootable over USB, requiring a full teardown and an external SWD programmer to recover.
>
> Because of this unacceptable risk to users, local patching remains the vastly preferred, safe, and reliable approach.


## Local browser patching and flashing

Download [hhkb.html](../../../../tools/hhkb/hhkb.html) and open it in Chrome or Edge. Patching works offline. The Flash keyboard tab uses WebHID with an explicit device permission prompt. Some browsers require serving the page on localhost. Select the PFU interface again after the recovery reboot. Keep SW6 OFF. Transfer errors appear in the log and stop the update. Browser flashing still requires hardware validation.


The **Hybrid patching** tab accepts only the official Hybrid ANSI A0.48 or JIS A2.49 HFB (both CRCs, version and SHA-256 checked). Select a current DMK `.bin` with embedded model/version metadata. Each file is labeled with its model and version, and incompatible models/layouts disable patching. Older DMK binaries without metadata must be rebuilt. Hybrid and Hybrid Type-S share a model-specific image; ANSI and JIS do not.

Classic does not require patching. Use its ready-made `.hfb` in **Flash keyboard**. The flasher checks the container model, version family and CRCs, and matches the exact PFU device model number before recovery and again before writing. Classic, Classic Type-S ANSI and Classic Type-S JIS have separate model IDs. When DMK identity metadata is present in an HFB, it must also match the container. File names are not used to establish compatibility.

For browser flashing, an HFB without embedded DMK identity is accepted only when its SHA-256 matches a supported official image: Classic A4.29, Classic Type-S ANSI A8.03 / JIS A9.04, Hybrid ANSI A0.48 / JIS A2.49. Legacy DMK HFB files without identity metadata must be rebuilt.
