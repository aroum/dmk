#!/usr/bin/env python3
"""
HHKB Classic Firmware Container (.hfb) Tool
===========================================

Wraps a raw binary into a standard 65,570-byte HHKB firmware update container (.hfb).
Computes the authentic CRC-16/MCRF4XX (poly 0x1021 reflected 0x8408, init 0xFFFF, no final XOR).

Format breakdown (65,570 bytes total):
- bytes [0:2]  : Outer CRC-16/MCRF4XX over bytes [2:65570], little-endian.
- bytes [2:4]  : Inner CRC-16/MCRF4XX over the 64 KB code payload [4:65540], little-endian.
- bytes [4:65540] : 64 KB Bank 2 firmware binary (padded with 0xFF).
- bytes [65540:65554] : 14 bytes 0xFF padding.
- bytes [65554:65560] : 6 bytes Model ID ASCII (e.g. 'AHHX01', 'AHHS01').
- bytes [65560:65564] : 4 bytes BCD version (e.g. 0A 04 02 09 -> A4.29).
- bytes [65564:65570] : 6 bytes 0xFF padding.

Usage:
    python3 make_hfb.py input.bin output.hfb [--model AHHX01] [--version A4.50]
"""

import sys
import argparse
import struct
import os

def compute_crc16_mcrf4xx(data: bytes) -> int:
    """
    Computes CRC-16/MCRF4XX (poly 0x1021 reflected -> 0x8408, init 0xFFFF, no final XOR).
    Returns 16-bit integer.
    """
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
    Parses a version string like 'A4.29', 'A4.50', or 'A8.50'
    Format: [Major_Hex, Minor_Hex1, Minor_Hex2, Minor_Hex3]
    E.g. 'A4.29' -> bytes([0x0A, 0x04, 0x02, 0x09])
         'A4.50' -> bytes([0x0A, 0x04, 0x05, 0x00])
         'A8.50' -> bytes([0x0A, 0x08, 0x05, 0x00])
    """
    ver = ver_str.strip()
    if ver.startswith('A') or ver.startswith('a'):
        major = 0x0A
        ver = ver[1:]
    else:
        major = 0x0A

    parts = ver.split('.')
    if len(parts) == 1:
        v1 = int(parts[0], 16)
        v2 = 0
        v3 = 0
    else:
        v1 = int(parts[0], 16)
        sub = parts[1]
        if len(sub) == 1:
            v2 = int(sub[0], 16)
            v3 = 0
        else:
            v2 = int(sub[0], 16)
            v3 = int(sub[1], 16)

    return bytes([major, v1, v2, v3])

def make_hfb(bin_path: str, out_path: str, model: str = "AHHX01", version_str: str = None):
    with open(bin_path, "rb") as f:
        data = f.read()

    # HHKB Classic firmware bank size is exactly 64KB (65536 bytes)
    target_size = 65536
    if len(data) > target_size:
        raise ValueError(f"Firmware binary size ({len(data)} bytes) exceeds 64KB bank limit!")

    # Pad firmware code with 0xFF up to 64KB
    padded_data = data + b"\xff" * (target_size - len(data))

    if version_str is None:
        version_str = "A8.50" if "S01" in model else "A4.50"

    # Inner tag is the CRC-16 of the 64KB payload (little-endian)
    inner_crc = compute_crc16_mcrf4xx(padded_data)
    inner_tag = inner_crc.to_bytes(2, "little")

    # Build 30-byte postfix:
    # 14 bytes 0xFF
    # 6 bytes Model ID: 'AHHX01' or 'AHHS01'
    # 4 bytes Version: e.g. [0x0A, 0x04, 0x03, 0x00]
    # 6 bytes 0xFF
    model_bytes = model.encode('ascii')[:6].ljust(6, b'\x00')
    version_bytes = parse_version_bytes(version_str)
    postfix = (b"\xff" * 14) + model_bytes + version_bytes + (b"\xff" * 6)
    assert len(postfix) == 30

    # Body to compute outer CRC over: 2 bytes inner tag + 64KB code + 30B postfix
    body = inner_tag + padded_data + postfix
    assert len(body) == 2 + 65536 + 30  # 65568 bytes

    # Outer CRC16 over the entire body (little-endian 2 bytes)
    outer_crc = compute_crc16_mcrf4xx(body)
    outer_crc_bytes = outer_crc.to_bytes(2, "little")

    hfb_data = outer_crc_bytes + body
    assert len(hfb_data) == 65570

    with open(out_path, "wb") as f:
        f.write(hfb_data)

    print(f"Generated {out_path}: {len(hfb_data)} bytes (Outer CRC: {outer_crc_bytes.hex()}, Inner Tag: {inner_tag.hex()}, Model: {model}, Version: {version_str})")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Wrap binary into HHKB .hfb firmware package")
    parser.add_argument("input", help="Input raw .bin or .hfb file")
    parser.add_argument("output", help="Output .hfb file")
    parser.add_argument("--model", default="AHHX01", help="Model code (default: AHHX01, e.g. AHHS01 for Classic Type-S)")
    parser.add_argument("--version", default=None, help="Firmware version (default: A4.50 for Classic, A8.50 for Type-S)")

    args = parser.parse_args()

    # If input is already an .hfb file (65570 bytes), inspect and extract:
    with open(args.input, "rb") as f_check:
        total_len = os.path.getsize(args.input)

    if total_len == 65570:
        with open(args.input, "rb") as f_in:
            hfb_raw = f_in.read()

        raw_payload = hfb_raw[4:4+65536]
        # Preserve model if not explicitly given
        in_model = hfb_raw[65554:65560].decode('ascii', errors='ignore').strip('\x00')
        if not args.model or args.model == "AHHX01":
            if in_model in ["AHHX01", "AHHS01", "AHKS01"]:
                args.model = in_model

        tmp_bin = args.input + ".extracted.bin"
        with open(tmp_bin, "wb") as f_tmp:
            f_tmp.write(raw_payload)
        make_hfb(tmp_bin, args.output, model=args.model, version_str=args.version)
        try: os.remove(tmp_bin)
        except OSError: pass
    else:
        make_hfb(args.input, args.output, model=args.model, version_str=args.version)
