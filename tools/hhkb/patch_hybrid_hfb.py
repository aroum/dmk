#!/usr/bin/env python3
"""Replace the STM32 application in a verified Hybrid A0.48 update."""
import argparse
import hashlib
import struct
from pathlib import Path

from make_hfb import compute_crc16_mcrf4xx

STOCK_SHA256 = "635b995eb5a15aa50c2cedc60da7d8998fcca8285a09d41d984c07fffcaf9d43"
BANK_SIZE = 0x10000


def patch(stock: bytes, application: bytes) -> bytes:
    if hashlib.sha256(stock).hexdigest() != STOCK_SHA256:
        raise ValueError("expected the official HHKB800_FW_A048.hfb template")
    if not 8 <= len(application) <= BANK_SIZE:
        raise ValueError("STM32 application must fit in 64 KiB")
    stack, reset = struct.unpack_from("<II", application)
    if not (0x20000000 < stack <= 0x20005000 and stack % 8 == 0
            and reset & 1 and 0x08010008 <= (reset & ~1) < 0x08010000 + len(application)):
        raise ValueError("invalid STM32 Bank 2 vector table")
    payload = application.ljust(BANK_SIZE, b"\xff")
    prefix_crc = struct.pack("<H", compute_crc16_mcrf4xx(payload))
    body = prefix_crc + payload + stock[4 + BANK_SIZE:]
    return struct.pack("<H", compute_crc16_mcrf4xx(body)) + body


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("application", type=Path)
    parser.add_argument("stock", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    if args.output.suffix.lower() != ".hfb":
        parser.error("output must have an .hfb extension")
    try:
        output = patch(args.stock.read_bytes(), args.application.read_bytes())
        with args.output.open("xb") as stream:
            stream.write(output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
