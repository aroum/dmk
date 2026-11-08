#!/usr/bin/env python3
"""
HHKB Classic Firmware Container (.hfb) Tool

Wraps a raw binary into a standard 65,570-byte HHKB firmware update container (.hfb).
Computes the authentic CRC16-X25 (reflected polynomial 0x8408) required by the PFU updater.

Usage:
    python3 make_hfb.py input.bin output.hfb [--version A4.30]
"""

import sys
import argparse
import struct

def compute_crc16_x25(data: bytes) -> int:
    """Computes CRC-16-CCITT/X.25 (reflected, init 0xFFFF, poly 0x8408)."""
    crc = 0xFFFF
    for b in data:
        cur = (crc ^ b) & 0xFF
        for _ in range(8):
            if cur & 1:
                cur = (cur >> 1) ^ 0x8408
            else:
                cur >>= 1
        crc = (crc >> 8) ^ cur
    return crc

def parse_version_bytes(ver_str: str) -> bytes:
    """
    Parses a version string like 'A4.29' or 'A4.30' or 'A9.99'
    Format: [Major_Hex, Minor_Hex1, Minor_Hex2, Minor_Hex3]
    E.g. 'A4.29' -> bytes([0x0A, 0x04, 0x02, 0x09])
         'A4.30' -> bytes([0x0A, 0x04, 0x03, 0x00])
    """
    ver = ver_str.strip()
    if ver.startswith('A') or ver.startswith('a'):
        major = 0x0A
        ver = ver[1:]
    else:
        major = 0x0A

    parts = ver.split('.')
    if len(parts) == 1:
        v1 = int(parts[0])
        v2 = 0
        v3 = 0
    else:
        v1 = int(parts[0])
        sub = parts[1]
        if len(sub) == 1:
            v2 = int(sub[0])
            v3 = 0
        else:
            v2 = int(sub[0])
            v3 = int(sub[1])

    return bytes([major, v1, v2, v3])

MODEL_TAGS = {
    "AHHX01": bytes.fromhex("07de"),  # HHKB Classic (PD-KB401W, PD-KB401B, etc.)
    "AHHS01": bytes.fromhex("72e1"),  # HHKB Classic Type-S ANSI (PD-KB401WSC, PD-KB401BSC)
    "AHKS01": bytes.fromhex("72e1"),  # HHKB Classic Type-S JIS (PD-KB421WSC, PD-KB421BSC)
}

def make_hfb(bin_path, out_path, model="AHHX01", version_str=None):
    with open(bin_path, "rb") as f:
        data = f.read()

    # HHKB Classic firmware bank size is exactly 64KB (65536 bytes)
    target_size = 65536
    if len(data) > target_size:
        raise ValueError(f"Firmware binary size ({len(data)} bytes) exceeds 64KB bank limit!")

    # Pad firmware code with 0xFF up to 64KB
    padded_data = data + b"\xff" * (target_size - len(data))

    if version_str is None:
        version_str = "A8.05" if "S01" in model else "A4.30"

    # Build 30-byte postfix:
    # 14 bytes 0xFF
    # 6 bytes Model ID: 'AHHX01' or 'AHHS01'
    # 4 bytes Version: e.g. [0x0A, 0x04, 0x03, 0x00]
    # 6 bytes 0xFF
    model_bytes = model.encode('ascii')[:6].ljust(6, b'\x00')
    version_bytes = parse_version_bytes(version_str)
    postfix = (b"\xff" * 14) + model_bytes + version_bytes + (b"\xff" * 6)
    assert len(postfix) == 30

    # Body to compute CRC over: 2 bytes header tag + 64KB code + 30B postfix
    tag = MODEL_TAGS.get(model, bytes.fromhex("07de"))
    body = tag + padded_data + postfix
    assert len(body) == 2 + 65536 + 30  # 65568 bytes

    # Compute CRC16 (little-endian 2 bytes)
    crc = compute_crc16_x25(body)
    crc_bytes = crc.to_bytes(2, "little")

    hfb_data = crc_bytes + body
    assert len(hfb_data) == 65570

    with open(out_path, "wb") as f:
        f.write(hfb_data)

    print(f"Generated {out_path}: {len(hfb_data)} bytes (CRC: {crc_bytes.hex()}, Model: {model}, Version: {version_str})")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Wrap binary into HHKB .hfb firmware package")
    parser.add_argument("input", help="Input raw .bin or .hfb file")
    parser.add_argument("output", help="Output .hfb file")
    parser.add_argument("--model", default="AHHX01", help="Model code (default: AHHX01, e.g. AHHS01 for Classic Type-S)")
    parser.add_argument("--version", default=None, help="Firmware version (default: A4.30 for Classic, A8.05 for Type-S)")

    args = parser.parse_args()

    # If input is already an .hfb file (65570 bytes), automatically extract raw 64KB Bank 2 payload:
    with open(args.input, "rb") as f_check:
        header = f_check.read(4)
        f_check.seek(0, 2)
        total_len = f_check.tell()

    if total_len == 65570:
        # Extract 64KB raw code payload (skip 4 bytes header, drop last 30 bytes metadata)
        with open(args.input, "rb") as f_in:
            f_in.seek(4)
            raw_payload = f_in.read(65536)
        tmp_bin = args.input + ".extracted.bin"
        with open(tmp_bin, "wb") as f_tmp:
            f_tmp.write(raw_payload)
        make_hfb(tmp_bin, args.output, model=args.model, version_str=args.version)
        import os
        try: os.remove(tmp_bin)
        except OSError: pass
    else:
        make_hfb(args.input, args.output, model=args.model, version_str=args.version)

