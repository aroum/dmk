#!/usr/bin/env python3
"""
DMK Source Lines of Code (SLOC) Counter

Accurately measures pure lines of code (excluding blank lines and comments)
for dmk_core and platforms, matching standard tools like cloc / scc / tokei.
Excludes third-party submodules, vendor SDKs, FreeRTOS, and build artifacts.
"""

import os
import sys

EXCLUDED_DIRS = {
    "pico-sdk",
    "dep",
    "sdk",
    "Middlewares",
    "Drivers",
    "u8g2",
    "freertos",
    "build",
    ".git",
    ".venv",
    ".cache",
}

SOURCE_EXTS = (".c", ".h", ".cpp")


def count_file_lines(file_path: str):
    code_lines = 0
    comment_lines = 0
    blank_lines = 0

    with open(file_path, "r", encoding="utf-8", errors="ignore") as fp:
        in_block_comment = False
        for line in fp:
            s = line.strip()
            if not s:
                blank_lines += 1
                continue

            if in_block_comment:
                comment_lines += 1
                if "*/" in s:
                    in_block_comment = False
                continue

            if s.startswith("/*"):
                comment_lines += 1
                if "*/" not in s:
                    in_block_comment = True
                continue

            if s.startswith("//"):
                comment_lines += 1
                continue

            code_lines += 1

    return code_lines, comment_lines, blank_lines


def analyze_directory(dir_path: str):
    total_code = 0
    total_comments = 0
    total_blanks = 0
    file_count = 0

    for root, dirs, files in os.walk(dir_path):
        # Exclude directories matching any excluded keyword
        dirs[:] = [d for d in dirs if d not in EXCLUDED_DIRS and not any(ex in d for ex in EXCLUDED_DIRS)]
        if any(ex in root.split(os.sep) for ex in EXCLUDED_DIRS):
            continue

        for f in sorted(files):
            if f.endswith(SOURCE_EXTS):
                p = os.path.join(root, f)
                c, com, b = count_file_lines(p)
                total_code += c
                total_comments += com
                total_blanks += b
                file_count += 1

    return {
        "files": file_count,
        "code": total_code,
        "comments": total_comments,
        "blanks": total_blanks,
        "total": total_code + total_comments + total_blanks,
    }


def fmt_k(val: int) -> str:
    return f"~{val / 1000:.1f}K"


def main():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    core_dir = os.path.join(repo_root, "dmk_core")
    plat_dir = os.path.join(repo_root, "platforms")

    if not os.path.isdir(core_dir) or not os.path.isdir(plat_dir):
        print("Error: must be run from within the DMK repository.", file=sys.stderr)
        sys.exit(1)

    core_stats = analyze_directory(core_dir)
    plat_stats = analyze_directory(plat_dir)

    total_code = core_stats["code"] + plat_stats["code"]
    total_comments = core_stats["comments"] + plat_stats["comments"]
    total_blanks = core_stats["blanks"] + plat_stats["blanks"]
    total_all = core_stats["total"] + plat_stats["total"]
    total_files = core_stats["files"] + plat_stats["files"]

    print("=" * 68)
    print("                 DMK Codebase Size Summary (SLOC)")
    print("=" * 68)
    print(f"{'Component':<15} | {'Files':<6} | {'Code (SLOC)':<11} | {'Comments':<9} | {'Blank':<6} | {'Total':<7}")
    print("-" * 68)
    print(
        f"{'dmk_core':<15} | {core_stats['files']:<6} | {core_stats['code']:<11} | {core_stats['comments']:<9} | {core_stats['blanks']:<6} | {core_stats['total']:<7}"
    )
    print(
        f"{'platforms':<15} | {plat_stats['files']:<6} | {plat_stats['code']:<11} | {plat_stats['comments']:<9} | {plat_stats['blanks']:<6} | {plat_stats['total']:<7}"
    )
    print("-" * 68)
    print(
        f"{'TOTAL':<15} | {total_files:<6} | {total_code:<11} | {total_comments:<9} | {total_blanks:<6} | {total_all:<7}"
    )
    print("=" * 68)

    print("\nFormatted for README.md:")
    print(f"**DMK** ({fmt_k(core_stats['code'])} core + {fmt_k(plat_stats['code'])} platforms) -> {fmt_k(total_code)}")
    print(
        f"| **DMK** ({fmt_k(core_stats['code'])} core + {fmt_k(plat_stats['code'])} platforms) | C | {fmt_k(total_code)} | 1× |"
    )


if __name__ == "__main__":
    main()
