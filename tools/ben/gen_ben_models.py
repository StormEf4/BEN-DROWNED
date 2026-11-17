#!/usr/bin/env python3
"""
Generates the models of the Song of Unhealing's quest as N64 display lists (F3DEX2), written to
src/overlays/actors/ovl_En_Ben/z_en_ben_models.inc.c (En_Ben includes it). Built on the model kit copied from the
Majora's Mask: WTF project (tools/ben/modelkit.py).

Usage: tools/ben/gen_ben_models.py [output file]

- The Moon Child: a small figure in a dirty white robe torn at the hem, a cowl drooping to a point behind, sleeves too
  long and empty, a long blank mask with two narrow eye holes and a crescent painted on its brow. Two rigid parts, so its head can turn on its own: the body (feet at
  the origin) and the head (its pivot at the neck). Lit, opaque.
- The moon sigil: a ring with a crescent inside it, glowing on the ground where a sigil was stamped. Unlit,
  translucent, its colour and alpha the primitive colour's (so it can pulse).
- A wet footprint: a bare child's foot, dark and wet on the ground. Unlit, translucent; mirrored for the other foot.

Model space: +Y up, +Z forward (where the mask faces), +X to its left; 1/100 of a world unit (drawn at scale 0.01).
"""

import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from modelkit import (  # noqa: E402
    GLOW, LIT_OPA, Mesh, S, F, emit_parts, facing, gfx, ring, setup_dl, smooth_loft, smooth_sphere,
)

OUT = Path(__file__).resolve().parents[2] / "src/overlays/actors/ovl_En_Ben/z_en_ben_models.inc.c"

ROBE = (190, 184, 170)
ROBE_DARK = (120, 114, 104)
INSIDE = (22, 18, 24)
MASK = (238, 234, 222)
HOLE = (8, 6, 10)
MOON = (120, 22, 18)  # painted on, the colour of old blood

NECK_Y = 3200  # the head's pivot
HEIGHT = 4400  # a little shorter than Link


def ragged(points, cx=0.0, cz=0.0, seed=7):
    """The same ring with its edge torn: each point pulled in or out a little (the same tears every time)."""
    out = []
    for i, (x, y, z) in enumerate(points):
        k = 1.0 + 0.09 * math.sin(i * 2.3 + seed) - 0.05 * math.cos(i * 5.1 + seed)
        out.append((cx + (x - cx) * k, y, cz + (z - cz) * k))
    return out


def robe():
    """The robe, from the torn hem up to the neck: wide at the bottom, narrow shoulders, leaning a little forward."""
    rows = [
        (0, 1200, 1060, 60),
        (300, 1120, 1000, 50),
        (1200, 900, 760, 20),
        (2200, 720, 560, 0),
        (2750, 760, 540, 0),  # the shoulders
        (3000, 500, 400, 30),
        (NECK_Y + 60, 260, 250, 40),
    ]
    rings = [ring(y, rx, rz, 16, cz=cz) for y, rx, rz, cz in rows]
    rings[0] = ragged(rings[0], cz=60)
    body = smooth_loft(rings, cap_end=True)
    # Under the hem it's dark: you can't see its feet
    hem = smooth_loft([ragged(ring(2, 1200, 1060, 16, cz=60), cz=60), ring(2, 10, 10, 16, cz=60)])
    return [S(body, ROBE), S(hem, INSIDE)]


def sleeve(side):
    """An empty sleeve, too long, hanging from the shoulder nearly to the ground, its open end dark inside."""
    x = side * 740
    rows = [
        (2850, 230, 230, 0, 0),
        (2300, 240, 240, side * 90, 60),
        (1500, 260, 250, side * 130, 120),
        (900, 300, 280, side * 140, 150),
    ]
    rings = [ring(y, rx, rz, 10, cx=x + dx, cz=cz) for y, rx, rz, dx, cz in rows]
    rings[-1] = ragged(rings[-1], cx=x + side * 140, cz=150, seed=3 + side)
    tube = smooth_loft(rings, cap_start=True)
    opening = smooth_loft([[(px, py - 1, pz) for px, py, pz in rings[-1]],
                           ring(980, 40, 40, 10, cx=x + side * 140, cz=150)])
    return [S(tube, ROBE_DARK), S(opening, INSIDE)]


def head():
    """The hooded head, its pivot (the neck) at the origin: a cowl drooping to a point behind, a long blank mask, two
    narrow eye holes (one a little lower than the other) and a crescent painted on its brow."""
    rows = [
        (-40, 300, 300, 0),
        (250, 640, 620, -40),
        (700, 720, 700, -60),
        (1100, 600, 600, -140),
        (1400, 330, 340, -300),
        (1560, 60, 60, -470),  # the cowl's point, drooping back
    ]
    hood = smooth_loft([ring(y, rx, rz, 14, cz=cz) for y, rx, rz, cz in rows], cap_end=True)
    mask = smooth_sphere(0, 560, 520, 470, 780, 210, n=14, stacks=8)
    eyes = [smooth_sphere(side * 190, 760 - (40 if side > 0 else 0), 715, 55, 170, 30, n=8, stacks=4)
            for side in (-1, 1)]
    # A crescent on the brow, flat on the mask's front, facing forward
    moon = []
    outer, inner, steps = 230, 180, 10
    cx, cy, z = 0, 1000, 690
    for i in range(steps):
        a0 = math.pi * (0.1 + 0.8 * i / steps)
        a1 = math.pi * (0.1 + 0.8 * (i + 1) / steps)
        # The inner edge is a smaller circle pushed down, so the moon is thick in the middle and thin at its tips
        p = [
            (cx + outer * math.cos(a0), cy + outer * math.sin(a0) - 200, z),
            (cx + outer * math.cos(a1), cy + outer * math.sin(a1) - 200, z),
            (cx + inner * math.cos(a1), cy + inner * math.sin(a1) * 0.55 - 200, z),
            (cx + inner * math.cos(a0), cy + inner * math.sin(a0) * 0.55 - 200, z),
        ]
        moon.append(facing(p, 1))
    return [S(hood, ROBE), S(mask, MASK)] + [S(e, HOLE) for e in eyes] + [F(moon, MOON)]


def face_up(points, y):
    """A flat polygon on the ground (x, z points, counter-clockwise seen from above), facing up."""
    return [(x, y, z) for x, z in reversed(points)]


def sigil():
    """A ring and a crescent, glowing on the ground: vertex alpha fades the ring's edges."""
    faces = []
    colours = {}
    n = 24
    r_in, r_mid, r_out = 2400, 2600, 2800

    def put(p, a):
        colours[p] = (255, 255, 255, a)
        return p

    for i in range(n):
        a0 = math.tau * i / n
        a1 = math.tau * (i + 1) / n
        for (ra, aa), (rb, ab) in (((r_in, 0), (r_mid, 255)), ((r_mid, 255), (r_out, 0))):
            q = [
                put((ra * math.cos(a0), 4.0, ra * math.sin(a0)), aa),
                put((ra * math.cos(a1), 4.0, ra * math.sin(a1)), aa),
                put((rb * math.cos(a1), 4.0, rb * math.sin(a1)), ab),
                put((rb * math.cos(a0), 4.0, rb * math.sin(a0)), ab),
            ]
            faces.append(q)
    # The crescent: the space between two circles, the inner one pushed aside
    steps = 14
    big, small, shift = 1700, 1350, 520
    for i in range(steps):
        a0 = math.pi * (-0.8 + 1.6 * i / steps)
        a1 = math.pi * (-0.8 + 1.6 * (i + 1) / steps)
        q = [
            put((small * math.cos(a0) - shift, 6.0, small * math.sin(a0)), 120),
            put((small * math.cos(a1) - shift, 6.0, small * math.sin(a1)), 120),
            put((big * math.cos(a1), 6.0, big * math.sin(a1)), 230),
            put((big * math.cos(a0), 6.0, big * math.sin(a0)), 230),
        ]
        faces.append(q)
    return faces, lambda p: colours[p]


def footprint():
    """A small bare foot, its heel at the origin, toes towards +Z: a sole and five toes, all flat on the ground."""
    faces = []
    sole = []
    for i in range(12):
        t = math.tau * i / 12
        # Narrow at the heel, wider at the ball
        w = 170 + 70 * max(0.0, math.sin(t))
        sole.append((w * math.cos(t), 330 + 330 * math.sin(t)))
    faces.append(face_up(sole, 3.0))
    for x, z, r in ((-120, 760, 70), (-30, 800, 60), (50, 790, 55), (115, 760, 50), (165, 710, 45)):
        toe = [(x + r * math.cos(math.tau * i / 6), z + r * math.sin(math.tau * i / 6)) for i in range(6)]
        faces.append(face_up(toe, 3.0))
    return faces, lambda p: (255, 255, 255, 255)


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else OUT

    child = Mesh("sMoonChildVtx")
    body_dl = gfx("sMoonChildBodyDL", emit_parts(child, robe() + sleeve(1) + sleeve(-1)))
    head_dl = gfx("sMoonChildHeadDL", emit_parts(child, head()))

    fx = Mesh("sBenGroundVtx")
    sigil_faces, sigil_colour = sigil()
    foot_faces, foot_colour = footprint()
    sigil_dl = gfx("sMoonSigilDL", fx.coloured_dl(sigil_faces, sigil_colour))
    foot_dl = gfx("sWetFootDL", fx.coloured_dl(foot_faces, foot_colour))

    setup = "\n".join([
        setup_dl("sMoonChildSetupDL", LIT_OPA, "The Moon Child: opaque, lit, untextured"),
        setup_dl("sBenGroundSetupDL", GLOW,
                 "Marks on the ground: unlit, colour and alpha = vertex colour x primitive colour, double sided"),
    ])
    text = f"""// Generated by tools/ben/gen_ben_models.py; edit that script instead of this file.
// The Moon Child, the moon sigil and a wet footprint. +Y up, +Z forward, +X to its left; 1/100 of a world unit.
// clang-format off

#define MOON_CHILD_NECK_Y ({NECK_Y}.0f)
#define MOON_CHILD_HEIGHT ({HEIGHT}.0f)

{child.vtx_array()}
{fx.vtx_array()}
{setup}
{body_dl}
{head_dl}
{sigil_dl}
{foot_dl}
// clang-format on
"""
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(text)
    print(f"wrote {out} ({len(child.vtx)} child, {len(fx.vtx)} ground vertices)")


if __name__ == "__main__":
    main()
