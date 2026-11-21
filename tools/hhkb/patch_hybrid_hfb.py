#!/usr/bin/env python3
"""Replace the STM32 application in a verified Hybrid ANSI or JIS update."""
import argparse
import hashlib
import struct
from pathlib import Path

from make_hfb import compute_crc16_mcrf4xx, parse_version_bytes

STOCK_TEMPLATES = {
    "ansi": ("HHKB800_FW_A048.hfb", "635b995eb5a15aa50c2cedc60da7d8998fcca8285a09d41d984c07fffcaf9d43"),
    "jis": ("HHKB820_FW_JP_A249.hfb", "4978c4595f8588b682b2815bf088e99493cd1fe42fbf19909bfb387d3af9634e"),
}
BANK_SIZE = 0x10000
NRF_IMAGE_SIZE = 0x35000
HYBRID_MODELS = {"ansi": (b"AHUX01", "A0.50"), "jis": (b"AHJX01", "A2.50")}


def bank_payload(application: bytes) -> bytes:
    if not 8 <= len(application) <= BANK_SIZE:
        raise ValueError("STM32 application must fit in 64 KiB")
    stack, reset = struct.unpack_from("<II", application)
    if not (0x20000000 < stack <= 0x20005000 and stack % 8 == 0
            and reset & 1 and 0x08010008 <= (reset & ~1) < 0x08010000 + len(application)):
        raise ValueError("invalid STM32 Bank 2 vector table")
    return application.ljust(BANK_SIZE, b"\xff")


def assemble(payload: bytes, tail: bytes) -> bytes:
    prefix_crc = struct.pack("<H", compute_crc16_mcrf4xx(payload))
    body = prefix_crc + payload + tail
    return struct.pack("<H", compute_crc16_mcrf4xx(body)) + body


def patch(stock: bytes, application: bytes, *, layout: str = "ansi") -> bytes:
    filename, sha256 = STOCK_TEMPLATES[layout]
    if hashlib.sha256(stock).hexdigest() != sha256:
        raise ValueError(f"expected the official {filename} template for {layout.upper()}")
    tail = bytearray(stock[4 + BANK_SIZE:])
    tail[-10:-6] = parse_version_bytes(HYBRID_MODELS[layout][1])
    return assemble(bank_payload(application), bytes(tail))


def make_stm32_only(application: bytes, *, layout: str) -> bytes:
    """Build an experimental Hybrid container with an nRF image meant to be rejected."""
    model, version = HYBRID_MODELS[layout]
    blank_radio = b"\xff" * NRF_IMAGE_SIZE
    invalid_digest = bytes(16)
    if hashlib.md5(blank_radio).digest() == invalid_digest:
        raise ValueError("blank radio image must fail the Nordic integrity check")
    # The last 14 bytes of the nRF image also begin the HFB footer.
    tail = b"\xff" * 12 + invalid_digest + blank_radio
    tail += model + parse_version_bytes(version) + b"\xff" * 6
    return assemble(bank_payload(application), tail)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("application", type=Path)
    parser.add_argument("stock", type=Path, nargs="?")
    parser.add_argument("output", type=Path)
    parser.add_argument("--layout", choices=STOCK_TEMPLATES, default="ansi",
                        help="layout of the compiled DMK application (default: ansi)")
    parser.add_argument("--stm32-only", action="store_true",
                        help="generate a Hybrid HFB with an intentionally invalid blank nRF image")
    args = parser.parse_args()
    if args.output.suffix.lower() != ".hfb":
        parser.error("output must have an .hfb extension")
    if args.stm32_only == (args.stock is not None):
        parser.error("provide a factory template or use --stm32-only")
    try:
        application = args.application.read_bytes()
        if args.stm32_only:
            output = make_stm32_only(application, layout=args.layout)
        else:
            output = patch(args.stock.read_bytes(), application, layout=args.layout)
        with args.output.open("xb") as stream:
            stream.write(output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
