**Language:** [English](build.md) | [Русский](../ru/build.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Build

Add `HHKB_JIS` alongside `HHKB_CLASSIC_TYPE_S` or `HHKB_HYBRID` for the 69-key layout. Use the shared `via.json` definition. Classic Type-S JIS generates an AHKS01 container. Hybrid JIS generates a raw STM32 image. Verify the matrix and attachment to the correct factory container on hardware before flashing.

Run builds from the repository root. Use the standard DMK definition argument.

```bash
# Classic: current full feature configuration
./build_all.sh -b hhkb -d HHKB_CLASSIC,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING

# Classic Type-S: the same configuration plus its model definition
./build_all.sh -b hhkb -d HHKB_CLASSIC_TYPE_S,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
```

```powershell
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC_TYPE_S,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
```

`DEFAULT_MCU` in the keyboard configuration selects STM32L072, so the build command does not need an MCU argument. `HHKB_CLASSIC_TYPE_S` is only for Classic Type-S. The output directory contains an `.hfb` PFU container and a raw Bank 2 `.bin`; building does not flash the keyboard.

The generated HFB versions are A4.50 for Classic and A8.50 for Type-S. The container builder calculates both required CRC values and sets the matching PFU model identifier.

## HFB container and model variants

An HHKB `.hfb` is exactly 65,570 bytes:

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

Classic and Classic Type-S use the same STM32L072RB, 4 × 15 matrix, 4051 multiplexers, and 64 KiB Bank 2 layout for this port. The firmware-facing differences are the PFU identifiers and version series:

| Parameter               | Classic      | Classic Type-S |
| ----------------------- | ------------ | -------------- |
| Model ID                | `AHHX01`     | `AHHS01`       |
| Version series          | `A4.xx`      | `A8.xx`        |
| Current DMK HFB version | A4.50        | A8.50          |
| Container size          | 65,570 bytes | 65,570 bytes   |

`tools/hhkb/make_hfb.py` builds both variants and calculates their CRCs automatically. For a manual invocation, use `--model AHHX01 --version A4.50` or `--model AHHS01 --version A8.50`.

## 3. Flash and RAM usage

Bank 2 provides 65,536 bytes of Flash. The latest local full-feature build (`HHKB_RAPID_TRIGGER`, `HHKB_FACTORY_THRESHOLDS`, `HHKB_ADC_16MHZ`, `HHKB_ADC_OVERSAMPLING`) produces a 36,116-byte raw Bank 2 image: 55.1% of the bank, leaving 29,420 bytes before padding. The 65,570-byte `.hfb` includes the padded Bank 2 payload and 34 bytes of container metadata.

The current linked STM32L072 image reserves 20,480 bytes of RAM: 288 bytes `.data`, 16,888 bytes `.bss`, and 2,048 bytes for `._user_heap_stack`, totalling 19,224 bytes (93.9%) and leaving 1,256 bytes in the link-time allocation. This is a build-time layout, not a measurement of worst-case runtime stack depth.

## GitHub Actions

The `dev` workflow builds Classic ANSI, Classic Type-S ANSI and Classic Type-S JIS, as well as Hybrid ANSI and JIS (the same targets cover Hybrid Type-S). Hybrid artifacts marked `experimental` contain DMK for STM32 and an erased-byte nRF section with an intentionally mismatched MD5. CI generates these without factory binaries. Whether the updater commits the STM32 update after the radio rejects its section still needs hardware verification.

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

[patch_hybrid_hfb.py](../../../../tools/hhkb/patch_hybrid_hfb.py) verifies the stock hash and Bank 2 vector table, replaces only the 64 KiB STM32 application, pads it with `FF`, and recalculates both CRCs. It preserves the factory nRF image, footer, and version. The output name must be new because the script refuses to overwrite files. Back up the keyboard before testing: installation and rollback of this patched Hybrid container have not been validated on hardware.
