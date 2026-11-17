#!/usr/bin/env python3
"""
BEN DROWNED: turn windows/builder/ben.png into the 64x64 RGBA16 texture En_Ben flashes on screen
(src/overlays/actors/ovl_En_Ben/z_en_ben_face.h). Needs Pillow.

    python3 tools/ben/make_face.py
"""

from pathlib import Path

from PIL import Image, ImageEnhance

SRC = Path("windows/builder/ben.png")
DST = Path("src/overlays/actors/ovl_En_Ben/z_en_ben_face.h")


def main() -> None:
    im = Image.open(SRC).convert("RGBA").crop((20, 10, 160, 150)).resize((64, 64), Image.LANCZOS)
    im = ImageEnhance.Brightness(im.convert("RGB")).enhance(0.9)
    words = [((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | 1 for (r, g, b) in im.getdata()]
    quads = []
    for i in range(0, len(words), 4):
        q = 0
        for w in words[i : i + 4]:
            q = (q << 16) | w
        quads.append(q)
    rows = ["    " + ", ".join("0x%016X" % v for v in quads[i : i + 5]) + "," for i in range(0, len(quads), 5)]
    DST.write_text(
        "#ifndef Z_EN_BEN_FACE_H\n#define Z_EN_BEN_FACE_H\n\n/*\n"
        " * BEN's face, 64x64 RGBA16, from windows/builder/ben.png (tools/ben/make_face.py regenerates it). Flashed on screen\n"
        " * for a frame or two at the worst moments. Two 64x32 halves, each a single TMEM load.\n */\n"
        "#define EN_BEN_FACE_WIDTH 64\n#define EN_BEN_FACE_HEIGHT 64\n\nstatic u64 sBenFaceTex[] = {\n"
        + "\n".join(rows)
        + "\n};\n\n#endif\n"
    )


if __name__ == "__main__":
    main()
