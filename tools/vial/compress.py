#!/usr/bin/env python3
import sys
import json
import lzma
import hashlib
import os

def main():
    if len(sys.argv) < 3:
        print("Usage: compress.py <input_vial.json> <output_header.h>")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]

    if not os.path.exists(input_path):
        print(f"Error: Input file '{input_path}' not found.")
        sys.exit(1)

    with open(input_path, 'r', encoding='utf-8') as f:
        vial_data = json.load(f)

    # Convert to compact JSON representation
    compact_json = json.dumps(vial_data, separators=(',', ':'))
    json_bytes = compact_json.encode('utf-8')

    # Compress using raw LZMA (FORMAT_ALONE is required by Vial)
    compressed_bytes = lzma.compress(json_bytes, format=lzma.FORMAT_ALONE)

    # Generate a stable 8-byte keyboard ID from the MD5 hash of the JSON content
    m = hashlib.md5()
    m.update(json_bytes)
    keyboard_id = m.digest()[:8]

    # Write C header file
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write("#ifndef VIAL_GENERATED_H\n")
        f.write("#define VIAL_GENERATED_H\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stddef.h>\n\n")

        # Keyboard definition array
        f.write(f"// Compressed size: {len(compressed_bytes)} bytes, Uncompressed size: {len(json_bytes)} bytes\n")
        f.write("static const uint8_t vial_keyboard_def[] = {\n")
        for i, b in enumerate(compressed_bytes):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{b:02x}, ")
            if i % 12 == 11 or i == len(compressed_bytes) - 1:
                f.write("\n")
        f.write("};\n")
        f.write("static const size_t vial_keyboard_def_size = sizeof(vial_keyboard_def);\n\n")

        # Keyboard ID
        f.write("static const uint8_t vial_keyboard_id[] = {\n    ")
        f.write(", ".join(f"0x{b:02x}" for b in keyboard_id))
        f.write("\n};\n\n")
        f.write("#endif // VIAL_GENERATED_H\n")

    print(f"Successfully generated {output_path} ({len(compressed_bytes)} bytes compressed)")

if __name__ == '__main__':
    main()
