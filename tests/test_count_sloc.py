import os
import sys
import tempfile
import unittest
from pathlib import Path

# Add project root to sys.path
ROOT_DIR = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT_DIR / "tools"))

from count_sloc import count_file_lines, analyze_directory, fmt_k


class TestCountSloc(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_count_file_lines_c_file(self):
        content = """// Single line comment
/*
 * Multi-line block comment
 */
#include <stdio.h>

int main(void) {
    // Another comment
    printf("Hello\\n");

    return 0;
}
"""
        test_file = Path(self.temp_dir.name) / "test.c"
        test_file.write_text(content, encoding="utf-8")

        code, comments, blanks = count_file_lines(str(test_file))

        # Blank lines: 2 (between #include and main, and before return)
        self.assertEqual(blanks, 2)
        # Comments: // line (1) + /* block (3 lines) + // another (1) = 5
        self.assertEqual(comments, 5)
        # Code lines: #include, int main(void) {, printf, return 0, } = 5
        self.assertEqual(code, 5)

    def test_count_file_lines_all_comments_or_blanks(self):
        content = """
        // Comment 1
        // Comment 2
        /* Multi-line
           comment */
        
        """
        test_file = Path(self.temp_dir.name) / "empty.c"
        test_file.write_text(content, encoding="utf-8")

        code, comments, blanks = count_file_lines(str(test_file))
        self.assertEqual(code, 0)
        self.assertGreater(comments, 0)
        self.assertGreater(blanks, 0)

    def test_analyze_directory_with_exclusions(self):
        base = Path(self.temp_dir.name)

        # Valid source file
        src_file = base / "main.c"
        src_file.write_text("int main(void) { return 0; }\n", encoding="utf-8")

        # Excluded directory (e.g. pico-sdk)
        sdk_dir = base / "pico-sdk"
        sdk_dir.mkdir()
        sdk_file = sdk_dir / "sdk.c"
        sdk_file.write_text("int sdk_fn() { return 1; }\n" * 100, encoding="utf-8")

        # Excluded build directory
        build_dir = base / "build"
        build_dir.mkdir()
        build_file = build_dir / "generated.c"
        build_file.write_text("int gen() { return 2; }\n" * 50, encoding="utf-8")

        res = analyze_directory(str(base))
        self.assertEqual(res["files"], 1)
        self.assertEqual(res["code"], 1)

    def test_fmt_k(self):
        self.assertEqual(fmt_k(1000), "~1.0K")
        self.assertEqual(fmt_k(7700), "~7.7K")
        self.assertEqual(fmt_k(15400), "~15.4K")


if __name__ == "__main__":
    unittest.main()
