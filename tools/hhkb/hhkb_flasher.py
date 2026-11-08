#!/usr/bin/env python3
"""
HHKB Flasher CLI (Open-Source Firmware Updater for Happy Hacking Keyboard)
========================================================================

Communicates directly with HHKB Classic / Classic Type-S via USB Raw HID
(Vendor ID: 0x04FE, UsagePage: 0xFF00, Usage: 0x01) to read device info
and flash firmware into STM32L072 Bank 2 (0x08010000).

Features:
- Bypasses official PFU Keymap Tool version downgrade restrictions.
- Supports both raw .bin files (64 KB) and .hfb firmware containers.
- Can flash custom firmware (DMK) or restore stock firmware.
- Interactive progress bar and safe verification.
- Works across macOS, Linux, and Windows via 'hidapi'.
"""

import argparse
import os
import sys
import time

try:
    import hid
except ImportError:
    # Attempt to advise user or run via uv / pip
    print("Error: 'hidapi' package is required.\nRun: pip install hidapi   (or: uv run --with hidapi python3 ...)", file=sys.stderr)
    sys.exit(1)

# USB Identification
PFU_VID = 0x04FE
PFU_USAGE_PAGE = 0xFF00
PFU_USAGE = 0x01

# Report protocol constants
REPORT_SIZE = 64
CHUNK_SIZE = 57  # Maximum data payload per E2 packet

CMD_GET_INFO = 0x02
CMD_FIRMUP_MODE = 0xE0
CMD_FIRMUP_START = 0xE1
CMD_FIRMUP_SEND = 0xE2
CMD_FIRMUP_END = 0xE3

REQ_HEADER = bytes([0xAA, 0xAA])
RESP_HEADER = bytes([0x55, 0x55])


def find_hhkb_device(product_id=None):
    """Locate matching HHKB device with UsagePage 0xFF00 / Usage 0x01."""
    devices = hid.enumerate(PFU_VID)
    matches = []
    for d in devices:
        if product_id is not None and d["product_id"] != product_id:
            continue
        # Check usage page & usage if reported by OS
        up = d.get("usage_page", 0)
        u = d.get("usage", 0)
        if up == PFU_USAGE_PAGE or up == 0:  # on some OS usage_page is 0 for generic hid
            matches.append(d)

    if not matches:
        return None

    # Prefer exact usage_page matching if available
    for d in matches:
        if d.get("usage_page") == PFU_USAGE_PAGE and d.get("usage") == PFU_USAGE:
            return d
    return matches[0]


class HHKBDevice:
    def __init__(self, dev_info):
        self.dev_info = dev_info
        self.handle = hid.device()
        self.handle.open_path(dev_info["path"])
        self.handle.set_nonblocking(False)

    def close(self):
        try:
            self.handle.close()
        except Exception:
            pass

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def send_report(self, data: bytes):
        """Send a 64-byte report to the keyboard."""
        if len(data) > REPORT_SIZE:
            raise ValueError(f"Packet too large ({len(data)} > {REPORT_SIZE})")
        # Pad with zeros to 64 bytes
        packet = data + b"\x00" * (REPORT_SIZE - len(data))
        # On macOS/Windows/Linux, prepend Report ID 0x00 for HID write
        res = self.handle.write(b"\x00" + packet)
        if res < 0:
            raise IOError("HID write failed")
        return res

    def read_report(self, timeout_ms=5000) -> bytes:
        """Read a 64-byte response report from the keyboard."""
        start = time.time()
        timeout_sec = timeout_ms / 1000.0
        while time.time() - start < timeout_sec:
            res = self.handle.read(REPORT_SIZE, timeout_ms=int((timeout_sec - (time.time() - start)) * 1000))
            if res:
                return bytes(res)
        raise TimeoutError("Device did not respond within timeout")

    def get_info(self):
        """Query keyboard model and current firmware version (Cmd 0x02)."""
        req = REQ_HEADER + bytes([CMD_GET_INFO, 0x00, 0x00, 0x00])
        self.send_report(req)
        resp = self.read_report(timeout_ms=3000)

        if not resp.startswith(RESP_HEADER + bytes([CMD_GET_INFO])):
            raise RuntimeError(f"Unexpected response to GetInfo: {resp[:8].hex()}")

        # Response format:
        # bytes [6:26] (20 bytes): Type Number ASCII string (e.g. "PD-KB401W")
        # bytes [26:30] (4 bytes): Firmware Version (e.g. "A426" or raw bytes)
        type_str = resp[6:26].split(b"\x00")[0].decode("ascii", errors="ignore").strip()
        ver_raw = resp[26:30]
        # Format version string (e.g. A4.29)
        if len(ver_raw) == 4 and ver_raw[0] == 0x0A:
            ver_str = f"A{ver_raw[1]:X}.{ver_raw[2]:02X}"
        else:
            ver_str = ver_raw.decode("ascii", errors="ignore").strip()

        return {
            "type_number": type_str or "Unknown HHKB",
            "version": ver_str or "Unknown",
            "raw_version": ver_raw.hex(),
        }

    def flash_firmware(self, fw_data: bytes, progress_cb=None):
        """
        Execute full firmware update sequence (0xE0 -> 0xE1 -> 0xE2 loop -> 0xE3).
        fw_data must be exactly 65,536 bytes (64 KB).
        """
        if len(fw_data) != 65536:
            raise ValueError(f"Firmware payload must be exactly 65,536 bytes (got {len(fw_data)})")

        # 1. Cmd E0: Enter Firmup Mode
        req_e0 = REQ_HEADER + bytes([CMD_FIRMUP_MODE, 0x00, 0x00, 0x00])
        self.send_report(req_e0)
        resp = self.read_report(timeout_ms=3000)
        if not resp.startswith(RESP_HEADER + bytes([CMD_FIRMUP_MODE])):
            raise RuntimeError(f"Firmup mode entry rejected: {resp[:6].hex()}")

        time.sleep(0.2)

        # 2. Cmd E1: Firmup Start (erase Bank 2)
        # Payload size = 65536 (0x00010000 in little-endian: 00 00 01 00)
        size_bytes = len(fw_data).to_bytes(4, "little")
        req_e1 = REQ_HEADER + bytes([CMD_FIRMUP_START, 0x00, 0x08, 0x00]) + size_bytes
        self.send_report(req_e1)

        # Flash erase can take up to several seconds
        resp = self.read_report(timeout_ms=15000)
        if not resp.startswith(RESP_HEADER + bytes([CMD_FIRMUP_START])):
            raise RuntimeError(f"Firmup start failed (flash erase error): {resp[:6].hex()}")

        time.sleep(0.2)

        # 3. Cmd E2: Send chunks (57 bytes each)
        total_bytes = len(fw_data)
        offset = 0
        seq = 0

        while offset < total_bytes:
            chunk = fw_data[offset : offset + CHUNK_SIZE]
            chunk_len = len(chunk)
            length_byte = chunk_len + 2
            seq_bytes = seq.to_bytes(2, "little")

            packet = REQ_HEADER + bytes([CMD_FIRMUP_SEND, length_byte]) + seq_bytes + chunk
            self.send_report(packet)

            resp = self.read_report(timeout_ms=3000)
            if not resp.startswith(RESP_HEADER + bytes([CMD_FIRMUP_SEND])):
                raise RuntimeError(f"Firmup send failed at offset {offset} (seq {seq}): {resp[:6].hex()}")

            offset += chunk_len
            seq += 1

            if progress_cb:
                progress_cb(offset, total_bytes)

        time.sleep(0.2)

        # 4. Cmd E3: Firmup End (verify checksum & reboot to Bank 2)
        req_e3 = REQ_HEADER + bytes([CMD_FIRMUP_END, 0x00, 0x00, 0x00])
        self.send_report(req_e3)
        try:
            resp = self.read_report(timeout_ms=5000)
            if not resp.startswith(RESP_HEADER + bytes([CMD_FIRMUP_END])):
                raise RuntimeError(f"Firmup end verification failed: {resp[:6].hex()}")
        except TimeoutError:
            # Device may immediately reset upon receiving E3
            pass


def load_firmware(path: str) -> bytes:
    """Load firmware from .bin or .hfb file and extract clean 64 KB payload."""
    with open(path, "rb") as f:
        data = f.read()

    # Check if .hfb container (65,570 bytes)
    if len(data) == 65570:
        print("Detected official .hfb container (65,570 bytes). Extracting 64 KB payload...")
        return data[2:65538]
    elif len(data) == 65536:
        print("Detected raw binary image (65,536 bytes).")
        return data
    elif len(data) < 65536:
        print(f"Firmware size is {len(data)} bytes. Padding with 0xFF to 64 KB...")
        return data + b"\xFF" * (65536 - len(data))
    else:
        raise ValueError(f"Invalid file size ({len(data)} bytes). Expected 65536 (.bin) or 65570 (.hfb).")


def print_progress(current, total):
    percent = (current / total) * 100
    bar_len = 40
    filled = int(bar_len * current // total)
    bar = "=" * filled + "-" * (bar_len - filled)
    sys.stdout.write(f"\rFlashing: [{bar}] {percent:5.1f}% ({current}/{total} bytes)")
    sys.stdout.flush()


def main():
    parser = argparse.ArgumentParser(
        description="Open-Source Flasher for Happy Hacking Keyboard (HHKB Classic / Type-S)"
    )
    parser.add_argument("firmware", nargs="?", help="Path to .bin or .hfb firmware file to flash")
    parser.add_argument("--info", action="store_true", help="Read device information only (no flashing)")
    parser.add_argument("--force", action="store_true", help="Bypass model mismatch checks")
    args = parser.parse_args()

    print("=== HHKB Open Flasher CLI ===")
    print("Searching for connected HHKB keyboards (VID 0x04FE)...")
    dev_info = find_hhkb_device()

    if not dev_info:
        print("Error: No compatible HHKB device found via USB HID.", file=sys.stderr)
        print("Make sure your HHKB Classic is plugged in and recognized by the OS.", file=sys.stderr)
        sys.exit(1)

    print(f"Found HHKB: {dev_info.get('product_string', 'Keyboard')} (PID: 0x{dev_info['product_id']:04X})")

    with HHKBDevice(dev_info) as kbd:
        info = kbd.get_info()
        print(f"  Model       : {info['type_number']}")
        print(f"  Current FW  : {info['version']} (raw: {info['raw_version']})")

        if args.info or not args.firmware:
            print("\nDevice query completed successfully.")
            return

        fw_path = args.firmware
        if not os.path.isfile(fw_path):
            print(f"Error: File not found: {fw_path}", file=sys.stderr)
            sys.exit(1)

        payload = load_firmware(fw_path)

        # Sanity check vector table (Cortex-M0+ reset vector at offset 4)
        initial_sp = int.from_bytes(payload[0:4], "little")
        reset_vector = int.from_bytes(payload[4:8], "little")
        print(f"Payload Initial SP: 0x{initial_sp:08X}, Reset Vector: 0x{reset_vector:08X}")

        # Check Bank 2 address range (0x08010000 - 0x08020000)
        if not (0x08010000 <= reset_vector <= 0x08020000) and not args.force:
            print("Warning: Reset vector does not point into Bank 2 (0x08010000-0x08020000).")
            print("Use --force if you intentionally want to flash this image.")
            confirm = input("Continue anyway? (y/N): ").strip().lower()
            if confirm != "y":
                print("Aborted.")
                sys.exit(1)

        print("\nStarting firmware update process...")
        start_time = time.time()
        kbd.flash_firmware(payload, progress_cb=print_progress)
        print("\n\nFlashing successfully completed in {:.2f}s!".format(time.time() - start_time))
        print("Keyboard is rebooting into the new firmware...")


if __name__ == "__main__":
    main()
