import os
import json
import tempfile
import unittest
from pathlib import Path

# Add project root to sys.path if needed
import sys
ROOT_DIR = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT_DIR / "tools" / "vial"))

from generate_json import main, parse_config, extract_layout, extract_array_count


class TestGenerateJson(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.output_json = Path(self.temp_dir.name) / "vial.json"

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_basic_keyboard_generation(self):
        config_content = """
        #define VIAL_KEYBOARD_NAME "Test Keyboard"
        #define VIAL_VENDOR_ID 0x1234
        #define VIAL_PRODUCT_ID 0x5678
        #define NUM_ROWS 2
        #define NUM_COLS 3
        #define LAYOUT { \\
            {0, 0}, {0, 1}, {0, 2}, \\
            {1, 0}, {1, 1}, {1, 2}  \\
        }
        """
        config_file = Path(self.temp_dir.name) / "config.h"
        config_file.write_text(config_content)

        sys.argv = ["generate_json.py", str(config_file), str(self.output_json)]
        main()

        self.assertTrue(self.output_json.exists())
        data = json.loads(self.output_json.read_text())

        self.assertEqual(data["name"], "Test Keyboard")
        self.assertEqual(data["vendorId"], "0x1234")
        self.assertEqual(data["productId"], "0x5678")
        self.assertEqual(data["matrix"], {"rows": 2, "cols": 3})
        self.assertEqual(data["layouts"]["keymap"], [
            ["0,0", "0,1", "0,2"],
            ["1,0", "1,1", "1,2"]
        ])
        self.assertNotIn("lighting", data)
        self.assertNotIn("vial", data)
        self.assertNotIn("features", data)

    def test_rgb_lighting_flag(self):
        config_content = """
        #define RGB_NUM 10
        #define NUM_ROWS 1
        #define NUM_COLS 1
        #define LAYOUT { {0, 0} }
        """
        config_file = Path(self.temp_dir.name) / "config.h"
        config_file.write_text(config_content)

        sys.argv = ["generate_json.py", str(config_file), str(self.output_json)]
        main()

        data = json.loads(self.output_json.read_text())
        self.assertEqual(data.get("lighting"), "qmk_rgblight")

    def test_midi_custom_keycodes(self):
        config_content = """
        #define MIDI_USB
        #define NUM_ROWS 1
        #define NUM_COLS 1
        #define LAYOUT { {0, 0} }
        """
        config_file = Path(self.temp_dir.name) / "config.h"
        config_file.write_text(config_content)

        sys.argv = ["generate_json.py", str(config_file), str(self.output_json)]
        main()

        data = json.loads(self.output_json.read_text())
        self.assertEqual(data.get("vial"), {"midi": "advanced"})
        custom_keycodes = data.get("customKeycodes", [])
        self.assertEqual(len(custom_keycodes), 64)

        # Check INC, DEC, VAL_127, TOGGLE sample values
        self.assertEqual(custom_keycodes[0]["name"], "MIDI_CC_0_INC")
        self.assertEqual(custom_keycodes[0]["value"], 0x7E00)
        self.assertEqual(custom_keycodes[16]["name"], "MIDI_CC_0_DEC")
        self.assertEqual(custom_keycodes[16]["value"], 0x7E10)
        self.assertEqual(custom_keycodes[32]["name"], "MIDI_CC_0_VAL_127")
        self.assertEqual(custom_keycodes[32]["value"], 0x7E20)
        self.assertEqual(custom_keycodes[48]["name"], "MIDI_CC_0_TOGGLE")
        self.assertEqual(custom_keycodes[48]["value"], 0x7E30)

    def test_encoder_parsing(self):
        config_content = """
        #define ENCODER_PINS_A {GPIO2, GPIO4}
        #define ENCODER_PINS_B {GPIO3, GPIO5}
        #define NUM_ROWS 1
        #define NUM_COLS 2
        #define LAYOUT { {0, 0}, {0, 1} }
        """
        config_file = Path(self.temp_dir.name) / "config.h"
        config_file.write_text(config_content)

        sys.argv = ["generate_json.py", str(config_file), str(self.output_json)]
        main()

        data = json.loads(self.output_json.read_text())
        self.assertEqual(data.get("features"), {"encoder": True})
        keymap = data["layouts"]["keymap"]
        # First row must be encoder definitions
        self.assertEqual(keymap[0], [
            "0,0\n\n\n\n\ne",
            "0,1\n\n\n\n\ne",
            "1,0\n\n\n\n\ne",
            "1,1\n\n\n\n\ne"
        ])
        # Second row is matrix row 0
        self.assertEqual(keymap[1], ["0,0", "0,1"])

    def test_comments_ignoring(self):
        config_content = """
        /* Multi-line comment with
           #define VIAL_KEYBOARD_NAME "Ignored"
           #define NUM_ROWS 99
        */
        // Single line: #define RGB_NUM 50
        #define VIAL_KEYBOARD_NAME "Real Name"
        #define NUM_ROWS 1
        #define NUM_COLS 1
        #define LAYOUT { {0, 0} }
        """
        config_file = Path(self.temp_dir.name) / "config.h"
        config_file.write_text(config_content)

        sys.argv = ["generate_json.py", str(config_file), str(self.output_json)]
        main()

        data = json.loads(self.output_json.read_text())
        self.assertEqual(data["name"], "Real Name")
        self.assertEqual(data["matrix"]["rows"], 1)
        self.assertNotIn("lighting", data)

    def test_extra_define_flag(self):
        config_content = """
        #define NUM_ROWS 1
        #define NUM_COLS 1
        #define LAYOUT { {0, 0} }
        """
        config_file = Path(self.temp_dir.name) / "config.h"
        config_file.write_text(config_content)

        sys.argv = [
            "generate_json.py",
            str(config_file),
            str(self.output_json),
            "-D", "RGB_NUM=10",
            "-D", "MIDI_USB"
        ]
        main()

        data = json.loads(self.output_json.read_text())
        self.assertEqual(data.get("lighting"), "qmk_rgblight")
        self.assertEqual(data.get("vial"), {"midi": "advanced"})

    def test_repo_keyboards(self):
        keyboards_dir = ROOT_DIR / "keyboards"
        for kb_dir in keyboards_dir.iterdir():
            if not kb_dir.is_dir():
                continue
            config_h = kb_dir / "config.h"
            if not config_h.exists():
                continue

            out_json = Path(self.temp_dir.name) / f"{kb_dir.name}.json"
            sys.argv = ["generate_json.py", str(config_h), str(out_json)]
            main()

            self.assertTrue(out_json.exists(), f"Failed to generate json for {kb_dir.name}")
            data = json.loads(out_json.read_text())
            self.assertIn("name", data)
            self.assertIn("vendorId", data)
            self.assertIn("productId", data)
            self.assertIn("matrix", data)
            self.assertIn("layouts", data)


if __name__ == "__main__":
    unittest.main()
