**Language:** [English](install.md) | [Русский](../ru/install.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA & Debug](via-debug.md)

# Install

> [!WARNING]
> Any firmware flashing by any method is performed entirely at your own risk. The author accepts no responsibility whatsoever for any keyboard damage, bricked devices, data loss, or other consequences.

> [!WARNING]
> Prebuilt Hybrid HFB images cannot be distributed directly because they contain proprietary Nordic Bluetooth firmware. Download the official PFU update and patch it locally with the open STM32 binary using the [HTML patcher](build.md#hybrid-local-patching) or Python patcher. See [Why Firmware Patching Is Required for Hybrid](build.md#why-hybrid-patching-is-required).

> [!TIP]
> **Recommended initial key calibration after first flashing:**
> While the firmware includes robust default capacitive baseline thresholds, it is strongly recommended to calibrate all keys (60 keys for ANSI or 69 for JIS) after initial flashing to match your specific PCB and conical spring tolerances. Step-by-step instructions and visual verification via the _Key Tester_ (_Test Matrix_) tab are provided in the [VIA guide](via-debug.md#key-calibration-process).

## Before flashing

Download the prebuilt files from [GitHub Releases](https://github.com/aroum/dmk/releases) or build the firmware for your model:

- **Classic (ANSI):** ready `.hfb` container `AHHX01` (A4.50)
- **Classic Type-S (ANSI):** ready `.hfb` container `AHHS01` (A8.50)
- **Classic Type-S (JIS):** ready `.hfb` container `AHKS01` (A9.50)
- **Hybrid (ANSI / JIS):** locally patched container `AHUX01` (A0.50) or `AHJX01` (A2.50), generated via the [HTML patcher](../../../../tools/hhkb/hhkb.html) or `patch_hybrid_hfb.py` from the STM32 binary and the official PFU update.

### Required: Download `via.json`

Be sure to download [via.json](../../via.json) alongside your firmware. It is required to configure keymaps, Rapid Trigger thresholds, capacitive calibration, and debugging options through the [VIA web app](https://www.usevia.app/) (under the _Settings_ $\to$ _Design_ tab).

The installation and flashing procedure itself is identical for both Classic and Hybrid models: you flash the prepared `.hfb` container using any of the supported methods:

1. **[HHKB Keymap Tool](#install-through-hhkb-keymap-tool)** (Official PFU updater)
2. **[DMK Flasher (`hhkb_flasher.py`)](#install-with-the-dmk-flasher)** (Cross-platform Python CLI)
3. **[Browser Flashing (HTML / WebHID)](#install-via-browser-webhid)** (Directly in Chrome/Edge using `hhkb.html`)
4. **[DFU and SWD](#dfu-and-swd)** (Hardware recovery / programming)

Keep a [full backup](backup-restore.md) before the first update (Flash + EEPROM for STM32, and Flash + UICR for Nordic nRF52832 on Hybrid).

DMK occupies Flash Bank 2 (`0x08010000–0x0801FFFF`). The original Bank 1 updater remains in place and provides the PFU update interface.

DMK uses a different USB identity, so Keymap Tool cannot flash it directly. If DMK is already installed, first return to the stock Bank 1 updater.

With the keyboard powered off, set SW6 to ON, connect power, and return SW6 to OFF after Bank 1 starts. This recovery action erases the first 128-byte page of Bank 2. See [SW6 recovery](backup-restore.md#sw6-recovery) for the detailed description of this irreversible transition to Bank 1. Then flash the generated `.hfb` with the official updater and reconnect the keyboard after it completes.

> **Dev: keep SW5 OFF when using SW6 for a normal update.** SW5+SW6 ON also [erases recognized DMK EEPROM settings](backup-restore.md#sw5--sw6-erase-dmk-settings-and-enter-bank-1-dev). Factory calibration and PFU Bluetooth records are preserved. For either recovery mode, remove all power first (including Hybrid batteries) and wait for Bank 1 USB detection before switching SW6 back OFF.

If the keyboard is currently running the original PFU firmware, its PFU update interface is already available: do **not** use SW6.

> [!WARNING]
> Make sure SW6 is OFF before flashing.

## Install through HHKB Keymap Tool

Download [HHKB Keymap Tool from the official website](https://happyhackingkb.com/download/).

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

Confirm with `--info` that the [HHKB Keymap Tool](https://happyhackingkb.com/download/) sees the expected keyboard model.

The browser flasher and Python flasher recognize the hardware family rather than an exhaustive list of retail part numbers. Colour/legend variants and anniversary suffixes such as `PD-KB800YSC` / `PD-KB800YSC1` map to Hybrid ANSI (`AHUX01`); the corresponding `PD-KB820…` variants map to Hybrid JIS (`AHJX01`). Classic and Classic Type-S still require their own matching HFB model. CRC, embedded DMK model and ANSI/JIS checks remain in place; these suffixes do not require bypass flags. The browser also checks the target again after recovery and tolerates the numeric revision suffix being omitted, while rejecting a different family or colour/legend variant.

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

## Install via Browser (WebHID)

You can flash prepared `.hfb` containers (both Classic and Hybrid) directly from your browser without installing any tools or Python drivers.

1. Download or open [hhkb.html](../../../../tools/hhkb/hhkb.html) in Chrome or Edge (WebHID is required).
2. Connect your keyboard over USB. If DMK is already installed, first [return to Bank 1 using SW6](backup-restore.md#sw6-recovery). Ensure SW6 is returned to OFF before writing.
3. Open the **Flash keyboard** tab and select your `.hfb` container (prebuilt Classic `.hfb` or locally patched Hybrid `.hfb`).
4. Click **Connect and flash** and grant permission to access the PFU USB interface in the browser prompt.
5. The page switches the device into recovery mode, pauses ~10s for re-enumeration, prompts to confirm the recovery interface, and writes the image. Keep the cable securely connected until complete.

## DFU and SWD

ROM DFU and SWD are development and recovery paths. DFU can write a raw Bank 2 `.bin`, but it does not update PFU Bank 1 metadata. Use the [HHKB Keymap Tool](https://happyhackingkb.com/download/), DMK flasher, or browser flasher for normal installation. The five SWD pads beside connector CN3 are arranged in two rows:

```text
top:     GND | RST | VCC
bottom:    CLK | DIO
```

Read [Backup & restore](backup-restore.md) before using either path.

## Check USB detection after flashing

If the keyboard does not appear in VIA or the HHKB Keymap Tool no longer recognizes it, follow [Check USB detection in Chrome](via-debug.md#check-usb-detection-in-chrome). Running DMK uses `CAFE:4014` / **HHKB Professional**, so the factory tool no longer recognizing it is expected.
