#!/usr/bin/env python3
"""
Flash nRF52840 firmware via USB using adafruit-nrfutil (Adafruit DFU bootloader).

Usage:
    uv run flash_nrf.py --hex <path/to/firmware.hex> [--port <PORT>] [--baud <BAUD>]

If --port is not specified, the script will attempt to auto-detect the device.
"""
import os
import shutil
import subprocess
import sys
import argparse
import tempfile

import serial.tools.list_ports


def find_serial_port():
    """Find an available serial port using pyserial.
    If multiple ports are detected, present a numbered list and prompt the user to select one.
    Returns the selected port device string or None if no ports are found.
    """
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        return None
    if len(ports) == 1:
        return ports[0].device
    # Multiple ports: show list and ask user to choose
    print("Available serial ports:")
    for idx, p in enumerate(ports, start=1):
        desc = p.description if p.description else ""
        print(f"  {idx}) {p.device} - {desc}")
    while True:
        choice = input("Select a port by number (or press Enter to cancel): ")
        if not choice:
            return None
        if not choice.isdigit():
            print("Please enter a numeric choice.")
            continue
        idx = int(choice)
        if 1 <= idx <= len(ports):
            return ports[idx - 1].device
        else:
            print("Choice out of range. Try again.")


def _resolve_nrfutil_cmd():
    """Resolve the adafruit-nrfutil command.

    Prefers the CLI wrapper on PATH; falls back to Python module invocation.
    Avoids using sys.executable directly in frozen/Nuitka builds.
    """
    for name in ("adafruit-nrfutil", "nrfutil"):
        path = shutil.which(name)
        if path:
            return [path]

    if os.name == "nt" and shutil.which("py"):
        return ["py", "-m", "adafruit_nrfutil"]

    for py in ("python", "python3", "python3.13", "python3.12", "python3.11"):
        if shutil.which(py):
            return [py, "-m", "adafruit_nrfutil"]

    if not getattr(sys, "_MEIPASS", False) and not getattr(sys, "frozen", False):
        return [sys.executable, "-m", "adafruit_nrfutil"]

    raise RuntimeError(
        "Cannot locate 'adafruit-nrfutil'. Install with: uv run --project tools/nrfutil pip install adafruit-nrfutil"
    )


def flash_firmware(hex_file, port=None, baudrate=115200):
    hex_file = os.path.abspath(hex_file)
    if not os.path.isfile(hex_file):
        print(f"Error: hex file not found: {hex_file}", file=sys.stderr)
        sys.exit(1)

    base_cmd = _resolve_nrfutil_cmd()

    if port is None:
        # No port provided; prompt user to select one
        port = find_serial_port()
    elif port == "auto":
        # Attempt to auto-select a port containing 'nice!nano' in description or device name
        ports = list(serial.tools.list_ports.comports())
        matching = [p for p in ports if "nice!nano" in (p.description or "").lower() or "nice!nano" in p.device.lower()]
        if matching:
            port = matching[0].device
            print(f"Auto-selected nice!nano port: {port}")
        else:
            print("nice!nano port not found, falling back to interactive selection.", file=sys.stderr)
            port = find_serial_port()
    # Validate that we have a port
    if port is None:
        print(
            "Error: No serial device detected.\n"
            "  Double-tap the Reset button on your board to enter DFU mode,\n"
            "  or specify --port manually.",
            file=sys.stderr,
        )
        sys.exit(1)
    else:
        print(f"Using port: {port}")

    with tempfile.TemporaryDirectory() as tmpdir:
        dfu_zip = os.path.join(tmpdir, "dfu_package.zip")

        # Generate DFU package — first attempt without --sd-req
        genpkg_cmd = base_cmd + [
            "dfu", "genpkg",
            "--dev-type", "0x0052",
            "--application", hex_file,
            dfu_zip,
        ]
        print(f"$ {' '.join(genpkg_cmd)}", flush=True)
        res = subprocess.run(genpkg_cmd, capture_output=True, text=True)
        print(res.stdout, end="")
        if res.returncode != 0:
            # Fallback: some bootloader versions require --sd-req 0x00
            print(f"First attempt failed, retrying with --sd-req 0x00 ...", flush=True)
            genpkg_cmd2 = base_cmd + [
                "dfu", "genpkg",
                "--dev-type", "0x0052",
                "--application", hex_file,
                "--sd-req", "0x00",
                dfu_zip,
            ]
            print(f"$ {' '.join(genpkg_cmd2)}", flush=True)
            res2 = subprocess.run(genpkg_cmd2, capture_output=True, text=True)
            print(res2.stdout, end="")
            if res2.returncode != 0:
                print(f"Error: DFU package creation failed:\n{res.stderr}\n{res2.stderr}", file=sys.stderr)
                sys.exit(1)

        # Flash DFU package over serial
        flash_cmd = base_cmd + [
            "dfu", "serial",
            "--package", dfu_zip,
            "-p", port,
            "-b", str(baudrate),
            "--singlebank",
        ]
        print(f"\nFlashing to {port} at {baudrate} baud...", flush=True)
        print(f"$ {' '.join(flash_cmd)}", flush=True)
        res_flash = subprocess.run(flash_cmd)
        if res_flash.returncode != 0:
            print(
                "\nFlash failed. Common causes:\n"
                "  - Board is not in DFU mode: double-tap the Reset button.\n"
                "  - Bootloader version mismatch.\n"
                "  - Wrong port selected (use --port to override).",
                file=sys.stderr,
            )
            sys.exit(res_flash.returncode)
        else:
            print("\nFlash successful!")


def main():
    parser = argparse.ArgumentParser(description="Flash nRF52840 via Adafruit DFU bootloader")
    parser.add_argument("--hex", required=True, help="Path to .hex firmware file")
    parser.add_argument("--port", default=None, help="Serial port (auto-detected if not specified)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    args = parser.parse_args()

    flash_firmware(args.hex, args.port, args.baud)
    # Success message is handled inside flash_firmware after a successful flash.


if __name__ == "__main__":
    main()
