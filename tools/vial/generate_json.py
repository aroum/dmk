#!/usr/bin/env python3
"""Generate vial.json from keyboard config.h definitions."""

import os
import re
import sys
import json
import argparse


def parse_config(config_path, extra_defs=None):
    if not os.path.exists(config_path):
        return {}

    with open(config_path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    # Prepend extra defines
    if extra_defs:
        content = "\n".join(f"#define {d}" for d in extra_defs) + "\n" + content

    # Strip multi-line comments
    content = re.sub(r"/\*.*?\*/", "", content, flags=re.DOTALL)
    # Strip single-line comments
    lines = []
    for line in content.splitlines():
        line = re.sub(r"//.*$", "", line)
        lines.append(line)
    text = "\n".join(lines)

    # Simple macro extractor
    defs = {}
    for m in re.finditer(r"^[ \t]*#define[ \t]+([A-Za-z0-9_]+)(?:[ \t]+([^\r\n]*))?", text, flags=re.MULTILINE):
        name = m.group(1)
        val = m.group(2).strip() if m.group(2) else "1"
        # Handle line continuations
        defs[name] = val

    return defs, text


def extract_layout(text):
    # Extract LAYOUT macro content which might be multiline
    m = re.search(r"#define[ \t]+LAYOUT[ \t]+\{([^#]+)\}", text)
    if not m:
        return []
    layout_str = m.group(1)
    keys = []
    # Match {row, col}
    for km in re.finditer(r"\{\s*([0-9]+)\s*,\s*([0-9]+)\s*\}", layout_str):
        keys.append((int(km.group(1)), int(km.group(2))))
    return keys


def extract_array_count(text, name):
    m = re.search(r"#define[ \t]+" + name + r"[ \t]+\{([^}]+)\}", text)
    if not m:
        return 0
    items = [x.strip() for x in m.group(1).split(",") if x.strip()]
    return len(items)


def main():
    parser = argparse.ArgumentParser(description="Generate vial.json from config.h")
    parser.add_argument("config_file", help="Path to config.h")
    parser.add_argument("output_file", help="Path to output vial.json")
    parser.add_argument("-D", "--define", action="append", default=[], help="Extra definitions")
    args = parser.parse_args()

    defs, raw_text = parse_config(args.config_file, args.define)

    # Keyboard name
    name = defs.get("VIAL_KEYBOARD_NAME", '"DMK Keyboard"').strip('"')

    # Vendor & Product IDs
    def parse_hex_or_int(val, default):
        if not val:
            return default
        try:
            if val.lower().startswith("0x"):
                return int(val, 16)
            return int(val)
        except ValueError:
            return default

    vid = parse_hex_or_int(defs.get("VIAL_VENDOR_ID"), 0xCafe)
    pid = parse_hex_or_int(defs.get("VIAL_PRODUCT_ID"), 0x0001)

    num_rows = int(defs.get("NUM_ROWS", 1))
    num_cols = int(defs.get("NUM_COLS", 1))

    root = {
        "name": name,
        "vendorId": f"0x{vid:04X}",
        "productId": f"0x{pid:04X}",
    }

    if "RGB_NUM" in defs or any(d == "RGB_NUM" or d.startswith("RGB_NUM=") for d in args.define):
        root["lighting"] = "qmk_rgblight"

    if "MIDI_USB" in defs or any(d == "MIDI_USB" or d.startswith("MIDI_USB=") for d in args.define):
        root["vial"] = {"midi": "advanced"}
        custom_keycodes = []
        # INC keycodes (0x7E00 .. 0x7E0F)
        for i in range(16):
            custom_keycodes.append({
                "name": f"MIDI_CC_{i}_INC",
                "title": f"MIDI CC {i} Inc",
                "shortName": f"CC{i}+",
                "value": 0x7E00 + i,
            })
        # DEC keycodes (0x7E10 .. 0x7E1F)
        for i in range(16):
            custom_keycodes.append({
                "name": f"MIDI_CC_{i}_DEC",
                "title": f"MIDI CC {i} Dec",
                "shortName": f"CC{i}-",
                "value": 0x7E10 + i,
            })
        # VAL_127 keycodes (0x7E20 .. 0x7E2F)
        for i in range(16):
            custom_keycodes.append({
                "name": f"MIDI_CC_{i}_VAL_127",
                "title": f"MIDI CC {i} Val 127",
                "shortName": f"CC{i}!",
                "value": 0x7E20 + i,
            })
        # TOGGLE keycodes (0x7E30 .. 0x7E3F)
        for i in range(16):
            custom_keycodes.append({
                "name": f"MIDI_CC_{i}_TOGGLE",
                "title": f"MIDI CC {i} Toggle",
                "shortName": f"CC{i}T",
                "value": 0x7E30 + i,
            })
        root["customKeycodes"] = custom_keycodes

    root["matrix"] = {
        "rows": num_rows,
        "cols": num_cols,
    }

    keymap_arr = []
    # Check encoders
    enc_a_count = extract_array_count(raw_text, "ENCODER_PINS_A")
    enc_b_count = extract_array_count(raw_text, "ENCODER_PINS_B")
    num_encoders = min(enc_a_count, enc_b_count) if (enc_a_count and enc_b_count) else 0

    if num_encoders > 0:
        root["features"] = {"encoder": True}
        enc_row = []
        for e in range(num_encoders):
            enc_row.append(f"{e},0\n\n\n\n\ne")
            enc_row.append(f"{e},1\n\n\n\n\ne")
        keymap_arr.append(enc_row)

    layout_keys = extract_layout(raw_text)
    for r in range(num_rows):
        row_keys = [f"{k[0]},{k[1]}" for k in layout_keys if k[0] == r]
        keymap_arr.append(row_keys)

    root["layouts"] = {"keymap": keymap_arr}

    os.makedirs(os.path.dirname(os.path.abspath(args.output_file)), exist_ok=True)
    with open(args.output_file, "w", encoding="utf-8") as f:
        json.dump(root, f, indent=2)
        f.write("\n")


if __name__ == "__main__":
    main()
