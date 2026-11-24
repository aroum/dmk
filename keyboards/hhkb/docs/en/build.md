**Language:** [English](build.md) | [Русский](../ru/build.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA & Debug](via-debug.md)

# Build

Run builds from the repository root using standard DMK definition flags. DMK supports building for all HHKB models: Classic, Classic Type-S, Hybrid, and Hybrid Type-S (in both 60-key ANSI and 69-key Japanese JIS layouts).

## Compiler version

HHKB CI builds use **Arm GNU Toolchain 15.3.Rel2 (GCC 15.3.1)**, downloaded from [Arm's official release](https://gitlab.arm.com/tooling/gnu-toolchains-for-arm/-/blob/releases/15.3.rel2/README.md) with a pinned SHA256. Use the same version locally and put its `bin` directory first in `PATH`. Check `arm-none-eabi-gcc --version`, then rebuild with `-c` after changing the compiler. HHKB keeps the normal `-Os` build without LTO. Older compiler versions can exceed Hybrid's 64 KiB Bank 2 limit even when this pinned version fits.

## Build Commands

### 1. HHKB Classic and Classic Type-S (Wired)

```bash
# Classic (ANSI 60-key):
./build_all.sh -b hhkb -d HHKB_CLASSIC

# Classic Type-S (ANSI 60-key):
./build_all.sh -b hhkb -d HHKB_CLASSIC_TYPE_S

# Classic Type-S JIS (69-key):
./build_all.sh -b hhkb -d HHKB_CLASSIC_TYPE_S,HHKB_JIS
```

PowerShell (Windows):

```powershell
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC_TYPE_S
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC_TYPE_S,HHKB_JIS
```

### 2. HHKB Hybrid and Hybrid Type-S (Bluetooth + USB)

For Hybrid keyboards, DMK builds a raw STM32 binary image with SPI communication enabled for the Nordic nRF52832 SoC:

```bash
# Hybrid / Hybrid Type-S (ANSI 60-key):
./build_all.sh -b hhkb -d HHKB_HYBRID -c

# Hybrid / Hybrid Type-S JIS (69-key):
./build_all.sh -b hhkb -d HHKB_HYBRID,HHKB_JIS -c
```

After building, patch the raw STM32 image into the official vendor firmware container using `tools/hhkb/patch_hybrid_hfb.py` (see [Patching Official Hybrid Firmware](#hybrid-only-local-build-and-factory-image-patch)).

`DEFAULT_MCU` in the keyboard configuration selects STM32L072, so the build command does not require an MCU argument. The output directory contains an `.hfb` PFU container (for Classic models) and a raw Bank 2 `.bin`; compiling does not flash the keyboard.

### Hybrid build output and next step

The commands above compile only the open STM32 image: `build/hhkb-script/dmk_hhkb_stm32l072.bin`. Use `HHKB_HYBRID` for ANSI/Type-S, or `HHKB_HYBRID,HHKB_JIS` for JIS/Type-S JIS. Hybrid Type-S needs no separate flag. Build variants sequentially because they share an output directory.

Compiling yourself is optional: [GitHub Actions](#github-actions) publishes `dmk-hhkb-hybrid-ansi-stm32` and `dmk-hhkb-hybrid-jis-stm32` with matching `.bin` files, the HTML patcher and `via.json`. Patched Hybrid HFB images are not distributed because they contain proprietary Nordic code.

After obtaining the BIN, download the matching original PFU firmware yourself and create an HFB using the [HTML patcher and installation instructions](install.md#hybrid-local-patching). Local packaging is required even for a CI binary. See below for the Python alternative. Building or packaging alone does not flash the keyboard.

## Model Identifiers and HFB Container Formats

The table lists the PFU-compatible version written to the final DMK HFB, not the DMK CalVer/git version shown in VIA. All final HFB versions end in `.50`: A4.50, A8.50, A9.50, A0.50 and A2.50. Hybrid input templates remain the original A0.48 (ANSI) and A2.49 (JIS); their SHA-256 checks are unchanged. The Python and HTML patchers update the output version and CRC while preserving the Nordic payload.

| Keyboard Line           | Layout        | DMK Build Defines              | PFU Model ID | DMK HFB Version | Resulting Container                              |
| :---------------------- | :------------ | :----------------------------- | :----------- | :-------------- | :----------------------------------------------- |
| **Classic**             | ANSI (60-key) | `HHKB_CLASSIC`                 | `AHHX01`     | `A4.50`         | Ready `.hfb` (65,570 bytes)                      |
| **Classic Type-S**      | ANSI (60-key) | `HHKB_CLASSIC_TYPE_S`          | `AHHS01`     | `A8.50`         | Ready `.hfb` (65,570 bytes)                      |
| **Classic Type-S JIS**  | JIS (69-key)  | `HHKB_CLASSIC_TYPE_S,HHKB_JIS` | `AHKS01`     | `A9.50`         | Ready `.hfb` (65,570 bytes)                      |
| **Hybrid / Type-S**     | ANSI (60-key) | `HHKB_HYBRID`                  | `AHUX01`     | `A0.50`         | Patched `HHKB800_FW_A048.hfb` (282,672 bytes)    |
| **Hybrid / Type-S JIS** | JIS (69-key)  | `HHKB_HYBRID,HHKB_JIS`         | `AHJX01`     | `A2.50`         | Patched `HHKB820_FW_JP_A249.hfb` (282,672 bytes) |

### HFB Container Structure, Layout, and Versioning

Official and DMK PFU firmware update files use the proprietary `.hfb` container format.

#### Classic / Classic Type-S (ANSI/JIS)

**Container size:** 65,570 bytes.

**Structure:**

```text
Outer CRC-16/MCRF4XX (2 bytes)
Inner STM32 CRC-16/MCRF4XX (2 bytes)
STM32 Bank 2, padded with 0xFF (65,536 bytes)
Trailer: 14 x 0xFF, model (6), version (4), 6 x 0xFF
```

**Byte layout:**

- `0x0000–0x0001`: outer CRC over `[0x0002:]`.
- `0x0002–0x0003`: inner CRC over STM32 `[0x0004:0x10004]`.
- `0x0004–0x10003`: 64 KiB STM32 image, linked at `0x08010000`, padded with `0xFF`.
- `0x10004–0x10011`: 14 bytes of `0xFF` padding.
- `0x10012–0x10017`: ASCII model ID: `AHHX01`, `AHHS01` or `AHKS01`.
- `0x10018–0x1001B`: version bytes.
- `0x1001C–0x10021`: six trailing `0xFF` bytes.

**Checksums:** both CRCs use reflected polynomial `0x8408`, initial value `0xFFFF`, and no final XOR. The inner CRC depends on the STM32 payload; it is not a fixed model tag.

**Versioning:** DMK outputs `A4.50` (`AHHX01`), `A8.50` (`AHHS01`) or `A9.50` (`AHKS01`). For example, `A4.50` is encoded as `0A 04 05 00`.

**Packaging:** `make_hfb.py` creates the complete container directly from the DMK STM32 binary and calculates both CRCs. No original firmware or patching is needed.

#### Hybrid / Hybrid Type-S (ANSI/JIS)

**Container size:** 282,672 bytes.

**Structure:**

```text
Outer CRC-16/MCRF4XX (2 bytes)
Inner STM32 CRC-16/MCRF4XX (2 bytes)
STM32 Bank 2, padded with 0xFF (65,536 bytes)
Nordic update data and firmware (preserved)
Model ID (6), version (4), trailing 0xFF (6)
```

**Byte layout:**

- `0x0000–0x0001`: outer CRC over `[0x0002:]`.
- `0x0002–0x0003`: inner CRC over STM32 `[0x0004:0x10004]`.
- `0x0004–0x10003`: 64 KiB STM32 image, linked at `0x08010000`, padded with `0xFF`.
- `0x10004–0x4501F`: Nordic update data and firmware, preserved unchanged.
- `0x45020–0x45025`: ASCII model ID: `AHUX01` (ANSI) or `AHJX01` (JIS); `[-16:-10]`.
- `0x45026–0x45029`: version bytes; `[-10:-6]`.
- `0x4502A–0x4502F`: six trailing `0xFF` bytes.

**Checksums:** both CRCs use reflected polynomial `0x8408`, initial value `0xFFFF`, and no final XOR. The inner CRC depends on the STM32 payload; it is not a fixed model tag.

**Versioning:** DMK outputs `A0.50` (ANSI: `0A 00 05 00`) or `A2.50` (JIS: `0A 02 05 00`). The original input templates remain A0.48 and A2.49.

**Packaging:** `hhkb.html` and `patch_hybrid_hfb.py` replace the STM32 image locally, update the version and both CRCs, and preserve all Nordic update data. Patched containers are not distributed because they retain proprietary firmware.

> [!NOTE]
> The PFU-compatible version in the HFB container (A4.50, A8.50, A9.50, A0.50, A2.50) is used exclusively by the PFU bootloader to validate the update file. The DMK CalVer/git version embedded in the STM32 application and displayed in VIA is independent of the container's version field.

## 3. Flash and RAM usage

The [HHKB linker layout](../../../../platforms/stm32l072/stm32l072_bank2.ld) provides **65,536 bytes of Bank 2 Flash** at `0x08010000–0x0801FFFF` and **20,480 bytes of SRAM**. These measurements use code commit `3e48c218`, built on 2026-11-24 with **Arm GNU Toolchain 15.3.Rel2 / GCC 15.3.1**, `-Os`, no LTO, and the default configuration including compressed EEPROM storage and SW5+SW6 recovery. All sizes are bytes.

### HHKB Classic

| Variant | Flash used / 65,536 B | Flash free | RAM allocated / 20,480 B | RAM unallocated |
| --- | ---: | ---: | ---: | ---: |
| ANSI: Classic / Classic Type-S | 51,320 (78.31%) | 14,216 | 18,080 (88.28%) | 2,400 |
| JIS: Classic Type-S | 51,088 (77.95%) | 14,448 | 18,632 (90.98%) | 1,848 |

RAM section breakdown:

- **ANSI:** `.data` 2,536 + `.bss` 13,492 + `._user_heap_stack` 2,052 = **18,080**.
- **JIS:** `.data` 2,504 + `.bss` 14,080 + `._user_heap_stack` 2,048 = **18,632**.

Classic and Classic Type-S ANSI were built separately and have identical memory usage.

### HHKB Hybrid

Includes the Nordic SPI transport, battery telemetry, and sleep/OFF state machines. Type-S uses the same build.

| Variant | Flash used / 65,536 B | Flash free | RAM allocated / 20,480 B | RAM unallocated |
| --- | ---: | ---: | ---: | ---: |
| ANSI: Hybrid / Type-S | 62,760 (95.76%) | 2,776 | 18,328 (89.49%) | 2,152 |
| JIS: Hybrid / Type-S | 62,560 (95.46%) | 2,976 | 18,888 (92.23%) | 1,592 |

RAM section breakdown:

- **ANSI:** `.data` 2,592 + `.bss` 13,688 + `._user_heap_stack` 2,048 = **18,328**.
- **JIS:** `.data` 2,560 + `.bss` 14,276 + `._user_heap_stack` 2,052 = **18,888**.

Hybrid ANSI now has **2,776 bytes** of Bank 2 Flash remaining; JIS has **2,976 bytes**. Compiler, feature flags and firmware-version strings can change these values.

### Memory and idle-work optimizations

The EC curve LUT is built with Q20 integer arithmetic at startup or after a settings change. Its 252-byte RAM table and integer scan-path interpolation remain unchanged. Rebuilding occurs after the calibration loader returns, to avoid nesting their large stack frames on the matrix task.

HHKB uses its statically created matrix queue. Removing the unreachable dynamic-queue fallback lets the linker discard the 2 KiB FreeRTOS heap in these default builds. Vial also omits unused encoder storage and Flash-page padding from HHKB RAM; the compact EEPROM serialization format remains unchanged.

When USB power saving suspends matrix scanning, scan timer interrupts are disabled and rearmed on resume. An idle HHKB USB task waits for USB/producer notifications, with a one-second background heartbeat; pending EEPROM operations retain frequent service. Keyboard reports still wake the task immediately. Other platforms retain their existing USB fallback cadence.

The scanner additionally caches exact raw press/release thresholds and the last raw sample: 376 bytes for ANSI or 468 bytes for JIS, plus a 4-byte range revision. Fixed-mode scanning needs no LUT/normalization unless EC streaming is enabled. RT normalization runs once per key and its result is reused for logging. Normalization overlaps the inter-key settling interval; state updates overlap the next ADC conversion. This trades additional cache and RAM-resident code for less per-key CPU work. The tables above include that memory cost.

With `HHKB_SCAN_DIAGNOSTICS=1`, the reference builds use Flash/RAM respectively: Classic ANSI **51,784 / 18,120 B**, Classic JIS **51,560 / 18,672 B**, Hybrid ANSI **63,256 / 18,368 B**, Hybrid JIS **63,056 / 18,928 B**. Hybrid JIS retains **1,552 B** of unallocated RAM in the diagnostic build. Runtime stack headroom and scan deadlines require hardware measurements; see [detailed scan timing](via-debug.md#detailed-scan-timing-ansi-and-jis).

### How to interpret the numbers

Flash usage is the raw `.bin` length, including the Flash initialization image for `.data` and RAM functions. The fixed-size, padded HFB container is not a measure of occupied Flash. RAM allocation is `.data + .bss + ._user_heap_stack`; `.data` includes RAM functions, `.bss` includes static task stacks, and `._user_heap_stack` reserves the main/interrupt stack with alignment. The FreeRTOS dynamic heap is absent from these default HHKB builds; optional modules using dynamic allocation can bring it back. Unallocated RAM is not a runtime stack high-water mark or free-heap measurement.

Inspect the sections after any build:

```bash
arm-none-eabi-size -A -d build/hhkb-script/dmk_hhkb_stm32l072
arm-none-eabi-objdump -h build/hhkb-script/dmk_hhkb_stm32l072
```

### EEPROM

[Compressed DMK storage](via-debug.md#compressed-eeprom-storage) reserves **1,772 bytes** of the 6,144-byte EEPROM for every supported Classic/Hybrid ANSI/JIS variant: two 880-byte snapshots plus a 12-byte ownership/reset header. This is separate from Flash and SRAM usage. Each snapshot allows 860 compressed payload bytes; actual payload length depends on the saved configuration. The rest of EEPROM includes PFU/factory data and must not be treated as free DMK space.

## GitHub Actions and Prebuilt Releases

Ready-to-flash containers and binaries are available in [GitHub Releases](https://github.com/aroum/dmk/releases) and CI build artifacts:

- **Classic models:** ready-made `.hfb` containers (`AHHX01`, `AHHS01`, `AHKS01`) that can be flashed directly.
- **Hybrid models:** open-source STM32 binaries (`dmk_hhkb_stm32l072.bin` for ANSI and JIS) alongside the [HTML patcher](../../../../tools/hhkb/hhkb.html) and `via.json`.

Hybrid CI publishes STM32 binaries for ANSI and JIS. Download the matching binary and the standalone [HTML patcher](../../../../tools/hhkb/hhkb.html). Open the page locally, choose your official HFB and DMK binary, then click Patch and download. The result retains the factory radio firmware. The former empty-radio HFB step is commented out.

The [Build Firmware workflow](../../../../.github/workflows/build_test.yml) runs on pushes and pull requests targeting `main`/`master`. You can also start it through **Actions → Build Firmware → Run workflow**. After a successful run, download the appropriate archive under **Artifacts**:

- `dmk-hhkb-classic-classic-via.hfb` — standard Classic, model `AHHX01`, HFB version A4.50.
- `dmk-hhkb-classic-type-s-via.hfb` — Classic Type-S, model `AHHS01`, HFB version A8.50.

Each archive contains the model-specific `.hfb` and [via.json](../../via.json) for loading the custom menu through the [VIA web app](https://www.usevia.app/) Design tab. Select the archive that matches your keyboard. The Type-S build has been compiler-checked; physical board verification so far covers the standard Classic.

<a id="why-hybrid-patching-is-required"></a>
<a id="hybrid-local-build"></a>

## Why Firmware Patching Is Required for Hybrid

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

To satisfy the Bank 1 update validator without redistributing vendor binaries, DMK provides the local patching utilities (the standalone [HTML patcher](../../../../tools/hhkb/hhkb.html) or `tools/hhkb/patch_hybrid_hfb.py`):

1. The user downloads the official update file directly from [PFU's official download center](https://happyhackingkb.com/download/) (`HHKB800_FW_A048.hfb` for ANSI or `HHKB820_FW_JP_A249.hfb` for JIS).
2. The patcher replaces **only the first 64 KB STM32 payload** with the freshly compiled or downloaded DMK binary, preserves the authentic factory nRF payload untouched, and recalculates the outer container CRCs.
3. Because the nRF section remains authentic and valid, both microcontrollers validate successfully, and Bank 1 boots DMK on the STM32 while keeping full Bluetooth functionality.

<a id="hybrid-local-patching"></a>

### Hybrid: local HTML patching and browser flashing

Download [hhkb.html](../../../../tools/hhkb/hhkb.html) and open it in Chrome or Edge. Patching works offline. The Flash keyboard tab uses WebHID with an explicit device permission prompt. Some browsers require serving the page on localhost. Select the PFU interface again after the recovery reboot. Keep SW6 OFF. Transfer errors appear in the log and stop the update.

For **Hybrid and Hybrid Type-S**, this single HTML file lets you prepare the update without installing Python:

1. Open **Hybrid patching**. Follow the links to [PFU downloads for ANSI](https://happyhackingkb.com/download/) or [PFU downloads for JIS](https://happyhackingkb.com/jp/download/) and manually download `HHKB800_FW_A048.hfb` (ANSI) or `HHKB820_FW_JP_A249.hfb` (JIS).
2. Select that file in **Original PFU update (.hfb)** and the matching DMK `.bin` from the build / release in **DMK STM32 firmware (.bin)**. Check the model and version shown for both files, then click **Patch and download**. The page replaces only the STM32 image, preserves the original Nordic firmware and downloads the resulting `.hfb`. Processing is local; the page does not upload the firmware files.
3. To flash the result, open **Flash keyboard**, select the downloaded `.hfb` and click **Connect and flash**, or use the [HHKB Keymap Tool](https://happyhackingkb.com/download/) / DMK flasher described in [Install](install.md). The browser tab shows update status, progress and a log. Follow the Bank 1 recovery and SW6 instructions when DMK is already installed.

The **Hybrid patching** tab accepts only the official Hybrid ANSI A0.48 or JIS A2.49 HFB (both CRCs, version and SHA-256 checked). Select a current DMK `.bin` with embedded model/version metadata. Each file is labeled with its model and version, and incompatible models/layouts disable patching. Older DMK binaries without metadata must be rebuilt. Hybrid and Hybrid Type-S share a model-specific image; ANSI and JIS do not.

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

## Patch Hybrid Firmware (Python CLI)

Obtain the exact official `HHKB800_FW_A048.hfb` separately from the [HHKB download center](https://happyhackingkb.com/download/) or your saved copy. The patcher expects SHA-256 `635b995eb5a15aa50c2cedc60da7d8998fcca8285a09d41d984c07fffcaf9d43`. Keep original and generated containers in the Git-ignored `keyboards/hhkb/local/` directory.

For **Hybrid JIS**, add `HHKB_JIS` to the build definitions and pass `--layout jis` to the patcher with the original `HHKB820_FW_JP_A249.hfb`. The patcher keeps model `AHJX01` and emits `A2.50`. ANSI output uses `AHUX01 / A0.50`.

From the repository root:

```bash
mkdir -p keyboards/hhkb/local
# Place the unmodified HHKB800_FW_A048.hfb in keyboards/hhkb/local/ first.
./build_all.sh -b hhkb -d HHKB_HYBRID -c
python3 tools/hhkb/patch_hybrid_hfb.py \
  build/hhkb-script/dmk_hhkb_stm32l072.bin \
  keyboards/hhkb/local/HHKB800_FW_A048.hfb \
  keyboards/hhkb/local/dmk_hybrid.hfb
```

[patch_hybrid_hfb.py](../../../../tools/hhkb/patch_hybrid_hfb.py) verifies the stock hash and Bank 2 vector table, replaces only the 64 KiB STM32 application, pads it with `FF`, and recalculates both CRCs. It preserves the factory nRF image and model; the output HFB version is A0.50 (ANSI) or A2.50 (JIS). The output name must be new because the script refuses to overwrite files. **Firmware images generated with this patching method have been tested and verified to work on physical HHKB Hybrid hardware.**

Rapid Trigger and 16 MHz ADC are included in every HHKB build. VIA controls Rapid Trigger and ADC oversampling at runtime.
