"""Keep HHKB's embedded geometry in sync with its curated VIA definition."""

import json
import lzma
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
GENERATOR = ROOT / "tools/hhkb/generate_vial.py"
VIA = ROOT / "keyboards/hhkb/via.json"


class HhkbGenerateVialTest(unittest.TestCase):
    def generate(self, source, output):
        subprocess.run([sys.executable, str(GENERATOR), str(source), str(output)], check=True)
        return json.loads(output.read_text(encoding="utf-8"))

    def test_curated_layout_survives_generation_and_compression(self):
        source_bytes = VIA.read_bytes()
        via = json.loads(source_bytes)
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "vial.json"
            vial = self.generate(VIA, output)
            self.assertEqual(vial["layouts"], via["layouts"])
            self.assertEqual(vial["matrix"], {"rows": 5, "cols": 15})
            self.assertEqual(vial["customKeycodes"], [])
            self.assertNotIn("menus", vial)
            self.assertEqual(vial, json.loads((ROOT / "keyboards/hhkb/vial.json").read_bytes()))

            header = Path(tmp) / "vial_generated.h"
            subprocess.run([sys.executable, str(ROOT / "tools/vial/compress.py"),
                            str(output), str(header)], check=True, capture_output=True)
            array = header.read_text().split("vial_keyboard_def[] = {", 1)[1].split("};", 1)[0]
            compressed = bytes(int(b, 16) for b in re.findall(r"0x([0-9a-f]{2})", array))
            self.assertEqual(json.loads(lzma.decompress(compressed)), vial)
        self.assertEqual(VIA.read_bytes(), source_bytes)

    def test_geometry_edits_propagate_without_changing_matrix_labels(self):
        via = json.loads(VIA.read_bytes())
        rows = via["layouts"]["keymap"]
        keys = [k for row in rows for k in row if isinstance(k, str)]
        jis = [k for k in keys if k.split("\n")[3] == "0,1"]
        self.assertEqual(len(jis), 69)
        self.assertEqual(len({k.split("\n")[0] for k in jis}), 69)
        # A future geometry edit must propagate without changing generator code.
        rows[-1][rows[-1].index("1,6\nSpace\n\n0,1") - 1]["w"] = 2.75
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "via.json"
            source.write_text(json.dumps(via), encoding="utf-8")
            vial = self.generate(source, Path(tmp) / "vial.json")
        self.assertEqual(vial["layouts"], via["layouts"])
        self.assertEqual([k for row in vial["layouts"]["keymap"] for k in row
                          if isinstance(k, str)], keys)

    def test_rejects_overwriting_via_source(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "via.json"
            original = VIA.read_bytes()
            source.write_bytes(original)
            result = subprocess.run([sys.executable, str(GENERATOR), str(source), str(source)],
                                    capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Input and output must differ", result.stderr)
            self.assertEqual(source.read_bytes(), original)


if __name__ == "__main__":
    unittest.main()
