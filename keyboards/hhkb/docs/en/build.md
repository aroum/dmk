**Language:** [English](build.md) | [Русский](../ru/build.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Build

Run builds from the repository root using standard DMK definition flags. DMK supports building for all HHKB models: Classic, Classic Type-S, Hybrid, and Hybrid Type-S (in both 60-key ANSI and 69-key Japanese JIS layouts).

## Build Commands

### 1. HHKB Classic and Classic Type-S (Wired)

```bash
# Classic (ANSI 60-key):
./build_all.sh -b hhkb -d HHKB_CLASSIC,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING

# Classic Type-S (ANSI 60-key):
./build_all.sh -b hhkb -d HHKB_CLASSIC_TYPE_S,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING

# Classic Type-S JIS (69-key):
./build_all.sh -b hhkb -d HHKB_CLASSIC_TYPE_S,HHKB_JIS,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
```

PowerShell (Windows):
```powershell
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC_TYPE_S,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC_TYPE_S,HHKB_JIS,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
```

### 2. HHKB Hybrid and Hybrid Type-S (Bluetooth + USB)

For Hybrid keyboards, DMK builds a raw STM32 binary image with SPI communication enabled for the Nordic nRF52832 SoC:

```bash
# Hybrid / Hybrid Type-S (ANSI 60-key):
./build_all.sh -b hhkb -d HHKB_HYBRID,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING -c

# Hybrid / Hybrid Type-S JIS (69-key):
./build_all.sh -b hhkb -d HHKB_HYBRID,HHKB_JIS,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING -c
```

After building, patch the raw STM32 image into the official vendor firmware container using `tools/hhkb/patch_hybrid_hfb.py` (see [Patching Official Hybrid Firmware](#hybrid-only-local-build-and-factory-image-patch)).

`DEFAULT_MCU` in the keyboard configuration selects STM32L072, so the build command does not require an MCU argument. The output directory contains an `.hfb` PFU container (for Classic models) and a raw Bank 2 `.bin`; compiling does not flash the keyboard.

## Model Identifiers and HFB Container Formats

| Keyboard Line | Layout | DMK Build Defines | PFU Model ID | Version Line | Resulting Container |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Classic** | ANSI (60-key) | `HHKB_CLASSIC` | `AHHX01` | `A4.xx` (A4.50) | Ready `.hfb` (65,570 bytes) |
| **Classic Type-S** | ANSI (60-key) | `HHKB_CLASSIC_TYPE_S` | `AHHS01` | `A8.xx` (A8.50) | Ready `.hfb` (65,570 bytes) |
| **Classic Type-S JIS** | JIS (69-key) | `HHKB_CLASSIC_TYPE_S,HHKB_JIS` | `AHKS01` | `A9.xx` (A9.50) | Ready `.hfb` (65,570 bytes) |
| **Hybrid / Type-S** | ANSI (60-key) | `HHKB_HYBRID` | `AHUX01` | `A0.xx` (A0.48) | Patched `HHKB800_FW_A048.hfb` (282,672 bytes) |
| **Hybrid / Type-S JIS** | JIS (69-key) | `HHKB_HYBRID,HHKB_JIS` | `AHJX01` | `A2.xx` (A2.49) | Patched `HHKB820_FW_JP_A249.hfb` (282,672 bytes) |

### HFB Container Structure for Wired Models (Classic)

An HHKB `.hfb` container for wired Classic models is exactly 65,570 bytes:

```text
+---------------------------------------------------------------+
| Outer CRC-16/MCRF4XX, little-endian (2 bytes)                 |
+---------------------------------------------------------------+
| Inner CRC-16/MCRF4XX of the Bank 2 payload (2 bytes)          |
+---------------------------------------------------------------+
| Bank 2 firmware, padded with 0xFF (65,536 bytes)              |
+---------------------------------------------------------------+
| Trailer: 14 × 0xFF, model ID (6), BCD version (4), 6 × 0xFF   |
+---------------------------------------------------------------+
```

Both CRCs use reflected polynomial `0x8408`, initial value `0xFFFF`, and no final XOR. The inner CRC is payload-specific; it is not a fixed model tag. The image is linked for Bank 2 at `0x08010000`.

`tools/hhkb/make_hfb.py` builds Classic variants and recalculates CRCs automatically.

## 3. Flash and RAM usage

Bank 2 provides 65,536 bytes of Flash. The latest local full-feature build (`HHKB_RAPID_TRIGGER`, `HHKB_FACTORY_THRESHOLDS`, `HHKB_ADC_16MHZ`, `HHKB_ADC_OVERSAMPLING`) produces a 36,116-byte raw Bank 2 image: 55.1% of the bank, leaving 29,420 bytes before padding. The 65,570-byte `.hfb` includes the padded Bank 2 payload and 34 bytes of container metadata.

The current linked STM32L072 image reserves 20,480 bytes of RAM: 288 bytes `.data`, 16,888 bytes `.bss`, and 2,048 bytes for `._user_heap_stack`, totalling 19,224 bytes (93.9%) and leaving 1,256 bytes in the link-time allocation. This is a build-time layout, not a measurement of worst-case runtime stack depth.

## GitHub Actions

The `dev` workflow builds Classic ANSI, Classic Type-S ANSI and Classic Type-S JIS, as well as Hybrid ANSI and JIS (the same targets cover Hybrid Type-S). Experimental Hybrid CI artifacts are built without proprietary vendor blobs (with an empty nRF section). Because the nRF bootloader rejects this image, and subsequently STM32 Bank 1 rejects the overall update, clean CI-built Hybrid artifacts cannot be flashed using the stock updater. For Hybrid keyboards, use the local patching script over an official vendor HFB.

The [Build Firmware workflow](../../../../.github/workflows/build_test.yml) runs on pushes and pull requests targeting `main`/`master`. You can also start it through **Actions → Build Firmware → Run workflow**. After a successful run, download the appropriate archive under **Artifacts**:

- `dmk-hhkb-classic-classic-via.hfb` — standard Classic, model `AHHX01`, HFB version A4.50.
- `dmk-hhkb-classic-type-s-via.hfb` — Classic Type-S, model `AHHS01`, HFB version A8.50.

Each archive contains the model-specific `.hfb` and [via.json](../../via.json) for loading the custom menu through the [VIA web app](https://www.usevia.app/) Design tab. Select the archive that matches your keyboard. The Type-S build has been compiler-checked; physical board verification so far covers the standard Classic.

Both builds use `HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING`; Type-S also uses `HHKB_CLASSIC_TYPE_S`. The keyboard configuration enables [VIA](https://www.usevia.app/) v3 and Vial, custom calibration, scan rate, actuation, Rapid Trigger, LEDs, layers, and hold-tap settings. `HHKB_ADC_DMA` is not enabled in this workflow.

`HHKB_ADC_OVERSAMPLING` sets the initial [VIA](https://www.usevia.app/) toggle state: ×4 applies at 50–500 Hz, while 1000 Hz forces ×1. The saved preference applies again when the rate drops to 500 Hz or less. Downloading an artifact does not flash the keyboard; see [Install](install.md).

## Hybrid only: local build and factory-image patch

Obtain the exact official `HHKB800_FW_A048.hfb` separately from the [HHKB download center](https://happyhackingkb.com/download/) or your saved copy. The patcher expects SHA-256 `635b995eb5a15aa50c2cedc60da7d8998fcca8285a09d41d984c07fffcaf9d43`. Keep original and generated containers in the Git-ignored `keyboards/hhkb/local/` directory.

For **Hybrid JIS**, add `HHKB_JIS` to the build definitions and pass `--layout jis` to the patcher with the original `HHKB820_FW_JP_A249.hfb`. The patcher preserves `AHJX01 / A2.49`. The ANSI template uses `AHUX01 / A0.48`.

From the repository root:

```bash
mkdir -p keyboards/hhkb/local
# Place the unmodified HHKB800_FW_A048.hfb in keyboards/hhkb/local/ first.
./build_all.sh -b hhkb -d HHKB_HYBRID,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING -c
python3 tools/hhkb/patch_hybrid_hfb.py \
  build/hhkb-script/dmk_hhkb_stm32l072.bin \
  keyboards/hhkb/local/HHKB800_FW_A048.hfb \
  keyboards/hhkb/local/dmk_hybrid.hfb
```

`-c` requests a clean build. `HHKB_HYBRID` selects Hybrid and its SPI transport. The other definitions enable Rapid Trigger, a 16 MHz ADC clock, and the initial ×4 oversampling preference. Omit `HHKB_RAPID_TRIGGER` for a build without it; the VIA Rapid Trigger menu will be hidden. The shared [build guide](build.md) explains the definition argument.

[patch_hybrid_hfb.py](../../../../tools/hhkb/patch_hybrid_hfb.py) verifies the stock hash and Bank 2 vector table, replaces only the 64 KiB STM32 application, pads it with `FF`, and recalculates both CRCs. It preserves the factory nRF image, footer, and version. The output name must be new because the script refuses to overwrite files. **Firmware images generated with this patching method have been tested and verified to work on physical HHKB Hybrid hardware.**
