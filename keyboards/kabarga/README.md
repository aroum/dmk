# Kabarga Keyboard

The **Kabarga** is an ergonomic split/monoblock mechanical keyboard powered by DMK Firmware. It supports multiple microcontroller architectures in the Pro Micro footprint, including **Baikal BE-U1000** (RISC-V), **nRF52840**, and **Milandr K1986BE92FI**.

---

## Hardware Revisions

There are two hardware revisions of the Kabarga PCB with differing row pinouts and backlight capabilities:

| Feature | Standard (Classic) | Kabarga CE (Community Edition) |
| :--- | :--- | :--- |
| **Row Pins (7 rows)** | `{PM_21, PM_20, PM_19, PM_1, PM_0, PM_2, PM_3}` | `{PM_0, PM_3, PM_2, PM_21, PM_20, PM_19, PM_18}` |
| **Col Pins (6 cols)** | `{PM_9, PM_8, PM_7, PM_6, PM_5, PM_4}` | `{PM_9, PM_8, PM_7, PM_6, PM_5, PM_4}` |
| **Pin `PM_1` (Pro Micro D1 / TX)** | Used for Matrix Row 3 | Freed from matrix, used for **RGB Backlight** (`RGB_PIN`) |
| **RGB Underglow/Backlight** | Not available (no dedicated pin) | **42 WS2812 LEDs** on `PM_1` |
| **Build Flag Required** | None (default) | `-d CE` (`--define CE`) |
| **Binary Suffix** | `dmk_kabarga_<mcu>` | `dmk_kabarga_<mcu>_ce` |

> [!WARNING]
> If you flash a Kabarga CE board with firmware built **without** `-d CE`:
> 1. RGB lighting will be completely compiled out (`NO_RGB`), appearing as disabled ("All Off", brightness 0) in Vial.
> 2. `PM_1` will be driven as a matrix row line instead of WS2812 data, preventing the LEDs from turning on.

---

## Building Firmware

### 1. Kabarga CE (with RGB Backlight)
To compile for the CE revision with RGB enabled, you **must pass the `-d CE` flag**:

```bash
# Build for Baikal BE-U1000:
./build_all.sh -c -b kabarga --mcu baikal -d CE

# Build for nRF52840:
./build_all.sh -c -b kabarga --mcu nrf52840 -d CE

# Build for RP2040:
./build_all.sh -c -b kabarga --mcu rp2040 -d CE
```

The resulting binary will be placed at:
- `build/dmk_kabarga_baikal_ce.uf2` (or `.bin` / `.hex`)

### 2. Standard (Classic) Kabarga
To compile for the classic PCB revision without RGB:

```bash
./build_all.sh -c -b kabarga --mcu baikal
```

The resulting binary will be placed at:
- `build/dmk_kabarga_baikal.uf2`

---

## Flashing Instructions (Baikal BE-U1000)

1. Put the board into bootloader mode (double-click Reset or hold Boot and press Reset).
2. The mass storage drive `BAIKAL_LI` will mount on your system.
3. Copy the compiled `.uf2` file:
   ```bash
   cp build/dmk_kabarga_baikal_ce.uf2 /Volumes/BAIKAL_LI/
   ```
4. The board will automatically flash, unmount the volume, and reboot into DMK firmware.

---

## Configuration Options

In [`keyboards/kabarga/config.h`](config.h):
* `RGB_LIMIT_VAL`: Clamps peak brightness (0–255) for all 42 WS2812 LEDs to keep thermal dissipation low on onboard regulators. Default is `60`.
* `TAPPING_TERM_DEFAULT`: Configures hold-tap timing (default `200` ms).
* `vial.json`: Layout definitions for real-time keymap and underglow editing via [Vial GUI](https://get.vial.today/).

---

## Layer Indication LEDs

The Kabarga board features 4 onboard status LEDs (`PM_15`, `PM_14`, `PM_16`, `PM_10`) handled automatically by the [`modules/layer_leds`](modules/layer_leds/) module:
* **Layer 0**: LED 1 (`PM_15`) ON.
* **Layer 1**: LED 2 (`PM_14`) ON.
* **Layer 2**: LED 3 (`PM_16`) ON.
* **Layer 3**: LED 4 (`PM_10`) ON.

Only one LED is active at a time to clearly indicate the currently active keyboard layer.
