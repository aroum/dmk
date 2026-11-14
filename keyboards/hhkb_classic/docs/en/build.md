**Language:** [English](build.md) | [Русский](../ru/build.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Build

Run builds from the repository root. Use the standard DMK definition argument.

```bash
# Classic: current full feature configuration
./build_all.sh -b hhkb_classic -d HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING

# Classic Type-S: the same configuration plus its model definition
./build_all.sh -b hhkb_classic -d HHKB_TYPE_S,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
```

```powershell
.\build_all.ps1 -Keyboard hhkb_classic -Define HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
.\build_all.ps1 -Keyboard hhkb_classic -Define HHKB_TYPE_S,HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING
```

`DEFAULT_MCU` in the keyboard configuration selects STM32L072, so the build command does not need an MCU argument. `HHKB_TYPE_S` is only for Classic Type-S. The output directory contains an `.hfb` PFU container and a raw Bank 2 `.bin`; building does not flash the keyboard.

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

The [Build Firmware workflow](../../../../.github/workflows/build_test.yml) runs on pushes and pull requests targeting `main`/`master`. You can also start it through **Actions → Build Firmware → Run workflow**. After a successful run, download the appropriate archive under **Artifacts**:

- `hhkb-classic-classic-max-adc-hfb` — standard Classic, model `AHHX01`, HFB version A4.50.
- `hhkb-classic-type-s-max-adc-hfb` — Classic Type-S, model `AHHS01`, HFB version A8.50.

Each archive contains the model-specific `.hfb` and [via.json](../../via.json) for loading the custom menu through the [VIA web app](https://www.usevia.app/) Design tab. Select the archive that matches your keyboard. The Type-S build has been compiler-checked; physical board verification so far covers the standard Classic.

Both builds use `HHKB_RAPID_TRIGGER,HHKB_ADC_16MHZ,HHKB_ADC_OVERSAMPLING`; Type-S also uses `HHKB_TYPE_S`. The keyboard configuration enables [VIA](https://www.usevia.app/) v3 and Vial, custom calibration, scan rate, actuation, Rapid Trigger, LEDs, layers, and hold-tap settings. `HHKB_ADC_DMA` is not enabled in this workflow.

`HHKB_ADC_OVERSAMPLING` sets the initial [VIA](https://www.usevia.app/) toggle state: ×4 applies at 50–500 Hz, while 1000 Hz forces ×1. The saved preference applies again when the rate drops to 500 Hz or less. Downloading an artifact does not flash the keyboard; see [Install](install.md).
