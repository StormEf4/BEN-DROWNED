#!/usr/bin/env python3
"""
BEN DROWNED: find, identify and import the base ROM.

Standard library only (it runs before the project's Python venv exists).

    rom_tool.py identify FILE         one line: STATUS<TAB>human-readable explanation
    rom_tool.py import FILE           normalize to .z64 byte order, verify, copy to baseroms/<version>/baserom.z64
    rom_tool.py find DIR [DIR ...]    list N64 ROM files under DIR (a few levels deep), one identify line each

STATUS is one of: retail, decompressed, wrong-version, not-mm, not-rom, unreadable.
Only `retail` and `decompressed` can be built from.
"""

import argparse
import hashlib
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# First word of an N64 ROM in each byte order.
MAGIC_Z64 = bytes([0x80, 0x37, 0x12, 0x40])  # big-endian (native)
MAGIC_V64 = bytes([0x37, 0x80, 0x40, 0x12])  # byte-swapped
MAGIC_N64 = bytes([0x40, 0x12, 0x37, 0x80])  # little-endian (word-swapped)

ROM_EXTENSIONS = {".z64", ".n64", ".v64", ".rom", ".bin"}
MAX_ROM_SIZE = 64 * 1024 * 1024
REGIONS = {"E": "USA", "J": "Japan", "P": "Europe", "D": "Germany", "F": "France", "S": "Spain", "I": "Italy"}


def to_z64(data: bytes) -> bytes:
    """Return the ROM in big-endian (.z64) byte order, or raise ValueError if it isn't an N64 ROM."""
    magic = data[:4]
    if magic == MAGIC_Z64:
        return data
    if len(data) % 4 != 0:
        raise ValueError("size is not a multiple of 4")
    if magic == MAGIC_V64:
        out = bytearray(data)
        out[0::2], out[1::2] = data[1::2], data[0::2]
        return bytes(out)
    if magic == MAGIC_N64:
        out = bytearray(data)
        out[0::4], out[1::4], out[2::4], out[3::4] = data[3::4], data[2::4], data[1::4], data[0::4]
        return bytes(out)
    raise ValueError("not an N64 ROM header")


def expected_md5s(version: str) -> dict:
    """md5 -> kind, read from the decomp's own checksum files so this stays correct for every version it supports."""
    out = {}
    base = ROOT / "baseroms" / version
    for name, kind in (("checksum-compressed.md5", "retail"), ("checksum.md5", "decompressed")):
        path = base / name
        if path.exists():
            out[path.read_text().split()[0].lower()] = kind
    return out


def identify(path: Path, version: str):
    """Returns (status, message, z64_bytes_or_None)."""
    try:
        size = path.stat().st_size
        if size > MAX_ROM_SIZE or size < 0x1000:
            return "not-rom", f"{path.name}: {size} bytes is not the size of an N64 ROM", None
        raw = path.read_bytes()
    except OSError as e:
        return "unreadable", f"{path}: {e.strerror}", None

    try:
        data = to_z64(raw)
    except ValueError as e:
        return "not-rom", f"{path.name}: {e}", None

    order = {MAGIC_Z64: ".z64", MAGIC_V64: ".v64 (byte-swapped)", MAGIC_N64: ".n64 (little-endian)"}[raw[:4]]
    title = data[0x20:0x34].decode("ascii", "replace").strip("\x00 ")
    region = REGIONS.get(chr(data[0x3E]), f"region '{chr(data[0x3E])}'")
    md5 = hashlib.md5(data).hexdigest()
    kind = expected_md5s(version).get(md5)

    if kind == "retail":
        return "retail", f"{path.name}: Majora's Mask {region}, retail cartridge dump, {order} - OK", data
    if kind == "decompressed":
        return "decompressed", f"{path.name}: Majora's Mask {region}, already decompressed, {order} - OK", data
    if "MAJORA" in title.upper():
        hint = "a different region or revision, the GameCube version, or an already-patched/hacked ROM"
        return "wrong-version", f"{path.name}: '{title}' ({region}) but not the N64 USA ROM this hack needs - probably {hint} (md5 {md5})", data
    return "not-mm", f"{path.name}: N64 ROM '{title}' ({region}), not Majora's Mask", data


def cmd_identify(args) -> int:
    status, message, _ = identify(Path(args.file), args.version)
    print(f"{status}\t{message}")
    return 0 if status in ("retail", "decompressed") else 1


def cmd_import(args) -> int:
    src = Path(args.file)
    status, message, data = identify(src, args.version)
    print(message)
    if status not in ("retail", "decompressed"):
        return 1

    dest_dir = ROOT / "baseroms" / args.version
    dest = dest_dir / "baserom.z64"
    # Remove other byte-order copies so the decomp can't pick up a stale one.
    for ext in ("z64", "n64", "v64", "Z64", "N64", "V64"):
        other = dest_dir / f"baserom.{ext}"
        if other.exists() and other.resolve() != src.resolve():
            other.unlink()

    if not (dest.exists() and dest.resolve() == src.resolve()):
        tmp = dest.with_suffix(".tmp")
        tmp.write_bytes(data)
        os.replace(tmp, dest)
    print(f"imported -> {dest.relative_to(ROOT)}")
    return 0


def cmd_find(args) -> int:
    found = 0
    for top in args.dirs:
        top = Path(top)
        if not top.is_dir():
            continue
        base_depth = len(top.parts)
        for dirpath, dirnames, filenames in os.walk(top):
            if len(Path(dirpath).parts) - base_depth >= args.depth:
                dirnames[:] = []
            # Skip huge or irrelevant trees.
            dirnames[:] = [d for d in dirnames if not d.startswith(".") and d not in ("AppData", "node_modules")]
            for name in filenames:
                p = Path(dirpath) / name
                if p.suffix.lower() not in ROM_EXTENSIONS:
                    continue
                status, message, _ = identify(p, args.version)
                if status == "not-rom":
                    continue
                print(f"{status}\t{p}\t{message}")
                found += 1
    return 0 if found else 1


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--version", default="n64-us")
    sub = parser.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("identify")
    p.add_argument("file")
    p.set_defaults(func=cmd_identify)
    p = sub.add_parser("import")
    p.add_argument("file")
    p.set_defaults(func=cmd_import)
    p = sub.add_parser("find")
    p.add_argument("dirs", nargs="+")
    p.add_argument("--depth", type=int, default=3)
    p.set_defaults(func=cmd_find)
    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
