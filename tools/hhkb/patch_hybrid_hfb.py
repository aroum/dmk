#!/usr/bin/env python3
"""Replace the STM32 application in a verified Hybrid ANSI or JIS update."""
import argparse
import hashlib
import struct
from pathlib import Path

from make_hfb import compute_crc16_mcrf4xx

STOCK_TEMPLATES = {
    "ansi": ("HHKB800_FW_A048.hfb", "635b995eb5a15aa50c2cedc60da7d8998fcca8285a09d41d984c07fffcaf9d43"),
    "jis": ("HHKB820_FW_JP_A249.hfb", "4978c4595f8588b682b2815bf088e99493cd1fe42fbf19909bfb387d3af9634e"),
}
BANK_SIZE = 0x10000


def patch(stock: bytes, application: bytes, *, layout: str = "ansi") -> bytes:
    filename, sha256 = STOCK_TEMPLATES[layout]
    if hashlib.sha256(stock).hexdigest() != sha256:
        raise ValueError(f"expected the official {filename} template for {layout.upper()}")
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
    parser.add_argument("--layout", choices=STOCK_TEMPLATES, default="ansi",
                        help="layout of the compiled DMK application (default: ansi)")
    args = parser.parse_args()
    if args.output.suffix.lower() != ".hfb":
        parser.error("output must have an .hfb extension")
    try:
        output = patch(args.stock.read_bytes(), args.application.read_bytes(), layout=args.layout)
        with args.output.open("xb") as stream:
            stream.write(output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
