**Language:** [English](install.md) | [Русский](../ru/install.md)

**Documentation:** [Install](install.md) • [Backup & restore](backup-restore.md) • [Build](build.md) • [Config](config.md) • [Models](models.md) • [VIA](via.md) • [Debug](debug.md)

# Install

> [!WARNING]
> You flash the keyboard and use DFU/SWD at your own risk. The author accepts no responsibility for keyboard damage, data loss, or consequences of selecting an incorrect firmware image, model, or recovery method.

## Before flashing

Build the matching firmware first. Classic uses an `AHHX01` container; Classic Type-S uses `AHHS01`. Keep a full backup before the first update.

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

You do not need the official Keymap Tool. `hhkb_flasher.py` sends the same `.hfb` through the PFU update interface and is a cross-platform alternative, including for Linux where the official Keymap Tool is not available. Stock firmware already exposes that interface; a running DMK image must first be returned to Bank 1 through SW6 as described above.

```bash
uv run --with hidapi python3 tools/hhkb/hhkb_flasher.py --info
uv run --with hidapi python3 tools/hhkb/hhkb_flasher.py build/hhkb-script/<firmware>.hfb
```

Confirm with `--info` that the PFU updater sees the correct model. The flasher checks the container CRC and model by default.

## DFU and SWD

ROM DFU and SWD are development and recovery paths. DFU can write a raw Bank 2 `.bin`, but it does not update PFU Bank 1 metadata. Use the PFU updater for normal installation. The five SWD pads beside connector CN3 are arranged in two rows:

```text
top:     GND | RST | VCC
bottom:    CLK | DIO
```

Read [Backup & restore](backup-restore.md) before using either path.
