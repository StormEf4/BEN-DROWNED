#!/usr/bin/env python3
"""
BEN DROWNED: make sure our custom message IDs don't collide with vanilla ones.

The game looks messages up with a linear search, so a vanilla message with the same ID would silently
win over ours. Run after `make init` (it needs the extracted vanilla text):

    python3 tools/ben/check_text_ids.py
"""

import argparse
import re
import sys
from pathlib import Path

DEFINE_RE = re.compile(r"^\s*DEFINE_MESSAGE\(\s*(0x[0-9A-Fa-f]+)", re.MULTILINE)
RESERVED = {0xFFFC, 0xFFFD}
CREDITS_START = 0x4E20


def read_ids(path: Path) -> list:
    return [int(m, 16) for m in DEFINE_RE.findall(path.read_text(encoding="utf-8", errors="replace"))]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--version", default="n64-us")
    args = parser.parse_args()

    vanilla_path = Path("extracted") / args.version / "text" / "message_data.h"
    ours_path = Path("assets/text/message_data.h")

    if not vanilla_path.exists():
        print(f"error: {vanilla_path} not found - run `make init` first", file=sys.stderr)
        return 2

    vanilla = set(read_ids(vanilla_path))
    ours = [i for i in read_ids(ours_path) if i not in RESERVED]

    problems = []
    seen = set()
    for text_id in ours:
        if text_id in seen:
            problems.append(f"0x{text_id:04X} is defined twice in {ours_path}")
        seen.add(text_id)
        if text_id in vanilla:
            problems.append(f"0x{text_id:04X} collides with a vanilla message")
        if text_id >= CREDITS_START:
            problems.append(f"0x{text_id:04X} is in the credits range (>= 0x{CREDITS_START:04X}) and would render as credits")

    print(f"vanilla messages: {len(vanilla)} (highest 0x{max(vanilla):04X}), custom messages: {len(ours)}")
    for p in problems:
        print("FAIL:", p)
    if not problems:
        print("OK: no text ID collisions")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
