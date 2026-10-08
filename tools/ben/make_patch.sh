#!/usr/bin/env bash
# BEN DROWNED: build the release patch (.bps) from a modified build.
#
#   tools/ben/make_patch.sh [path/to/retail-us-rom.z64]
#
# The patch is made against the retail US ROM (the compressed one, md5 2a0a8acb61538235bc1094d297fb6556),
# because that is what players own. It needs Floating IPS (flips) on PATH:
#   git clone https://github.com/Alcaro/Flips && cd Flips && ./make.sh   (then copy ./flips somewhere on PATH)
# flips emits copy instructions for data that moved, so the patch contains our changes, not Nintendo's data.
set -euo pipefail

cd "$(dirname "$0")/../.."

RETAIL_MD5="2a0a8acb61538235bc1094d297fb6556"
BASE="${1:-baseroms/n64-us/baserom.z64}"
OURS="build/n64-us/mm-n64-us-compressed.z64"
VERSION="$(git describe --tags --always --dirty 2>/dev/null || echo dev)"
OUT="dist/ben-drowned-${VERSION}.bps"

command -v flips >/dev/null || { echo "error: flips not found on PATH (see the header of this script)" >&2; exit 1; }
[ -f "$BASE" ] || { echo "error: base ROM not found: $BASE" >&2; exit 1; }
[ -f "$OURS" ] || { echo "error: build not found: $OURS - run: make NON_MATCHING=1 COMPARE=0" >&2; exit 1; }

base_md5="$(md5sum "$BASE" | cut -d' ' -f1)"
if [ "$base_md5" != "$RETAIL_MD5" ]; then
    echo "error: $BASE is not the retail US ROM (md5 $base_md5, expected $RETAIL_MD5)." >&2
    echo "       If your dump is .n64/.v64 byte order, convert it to .z64 first." >&2
    exit 1
fi

mkdir -p dist
flips --create --bps-delta "$BASE" "$OURS" "$OUT"

# Round-trip: applying the patch to the retail ROM must reproduce our build exactly.
tmp="$(mktemp)"
trap 'rm -f "$tmp"' EXIT
flips --apply "$OUT" "$BASE" "$tmp" >/dev/null
cmp -s "$tmp" "$OURS" || { echo "error: round-trip check failed" >&2; exit 1; }

echo "patch:  $OUT ($(du -h "$OUT" | cut -f1))"
echo "target: $(md5sum "$OURS" | cut -d' ' -f1)  (md5 of the patched ROM)"
