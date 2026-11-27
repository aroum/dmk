#!/usr/bin/env python3
"""Derive HHKB's compact embedded Vial definition from its curated VIA layout."""

import argparse
import json
from pathlib import Path


def make_vial_definition(via):
    # Keep the physical layout verbatim: config.h has matrix coordinates but
    # cannot describe key widths, row offsets or the stepped JIS Enter key.
    return {
        "name": via["name"],
        "vendorId": via["vendorId"],
        "productId": via["productId"],
        "customKeycodes": [],
        "matrix": via["matrix"],
        "layouts": via["layouts"],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_file", type=Path, help="Curated HHKB via.json")
    parser.add_argument("output_file", type=Path, help="Compact vial.json to generate")
    args = parser.parse_args()
    if args.input_file.resolve() == args.output_file.resolve():
        parser.error("Input and output must differ; preserve the full VIA definition")

    via = json.loads(args.input_file.read_text(encoding="utf-8"))
    vial = make_vial_definition(via)
    args.output_file.parent.mkdir(parents=True, exist_ok=True)
    args.output_file.write_text(
        json.dumps(vial, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )


if __name__ == "__main__":
    main()
