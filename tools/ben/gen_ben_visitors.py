#!/usr/bin/env python3
"""
Generates the Expanded Horror visitors' models as N64 display lists (F3DEX2), written to
src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_models.inc.c (the visitors' own actor, En_Ben_Visitor, includes it). Built on
the model kit copied from the Majora's Mask: WTF project (tools/ben/modelkit.py).

Usage: tools/ben/gen_ben_visitors.py [visitors output file] [SCP-173, SCP-096 and SCP-106 output files]

- The Tall Man: twice Link's height, a black suit, a white shirt and a black tie, arms that hang past his knees with
  long pale fingers, and a pale head with no face at all. His tendrils are a display list of their own, drawn only
  when he's close to having you.
- The Smiling Man: thin, in a dark suit, arms out and one knee up as if he's dancing. Two heads on the same neck: the
  grin, far too wide and full of teeth, and the frown he wears when he gives up on you.
- The one who says "Go to sleep": a white hoodie and black jeans, a kitchen knife. His face is drawn unlit and
  unfogged (a display list of its own, vertex coloured), so when the lights go out it's the only thing you can see:
  bone white, black rings around eyes that never close, a red cut grin up to the cheeks. Black hair, lit.

And the SCP Foundation's, each written to its own actor's folder (src/overlays/actors/ovl_En_Ben_Scp173/
z_en_ben_scp173_models.inc.c, and 096's and 106's the same way), each built from its SCP wiki article's words
(CC BY-SA 3.0, credited in docs/ben/CREDITS.md), not from any picture:
- SCP-173: concrete and rebar, with traces of Krylon spray paint. A crude, lopsided figure, lumpy and cracked, rebar
  where chunks have broken away and for fingers, its face sprayed on and stitched shut, the reddish-brown stain down
  its front and over its hands.
- SCP-096: about 2.4 m, emaciated, no hair, skin almost without colour, arms far too long, pale eyes. Every rib,
  knuckle and knob of its spine shows. Its body and its arms (hanging, or up over its face, the fingers spread over
  its skull) are display lists of their own, and so are its head calm and screaming, the jaw hanging open to its
  chest, lined with teeth.
- SCP-106: an old man rotting, hunched over, black and wet, sloughing off in patches and dripping from everywhere,
  grinning with lipless teeth. And the pool of corrosion he leaves, drawn on the ground (vertex alpha fades its
  edge).

Model space: +Y up, +Z forward (where the face looks), +X to its left; 1/100 of a world unit (drawn at scale 0.01).
Each head's pivot is its neck, at <NAME>_NECK_Y.
"""

import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from modelkit import (  # noqa: E402
    GLOW, GLOW_FRONT, LIT_OPA, Mesh, RingSurface, S, F, Smooth, emit_parts, facing, gfx, ring, setup_dl, smooth_loft,
    smooth_sphere, smooth_spike,
)

OUT = Path(__file__).resolve().parents[2] / "src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_models.inc.c"
SCP_OUTS = [Path(__file__).resolve().parents[2] / f"src/overlays/actors/ovl_En_Ben_Scp{n}/z_en_ben_scp{n}_models.inc.c"
            for n in ("173", "096", "106")]

BLACK = (10, 10, 12)
SUIT = (26, 26, 30)
SHIRT = (228, 228, 224)
PALE = (232, 230, 228)
SMILE_SUIT = (44, 34, 48)
SMILE_SKIN = (236, 226, 210)
HAIR = (14, 12, 12)
HOODIE = (222, 222, 218)
JEANS = (24, 26, 34)
BLADE = (176, 178, 186)
HANDLE = (60, 40, 28)
MOUTH = (16, 4, 6)
TEETH = (238, 232, 206)
EYE_WHITE = (240, 238, 230)


TALL_HEIGHT = 8900
SHY_NECK_Y = 6600
SMILE_NECK_Y = 5600
SLEEP_NECK_Y = 4300


# ---------------------------------------------------------------------------------------------------------------------
# Pieces


def limb(points, radii, n=8, cap_start=False, cap_end=True):
    """A tube through `points` (x, y, z), mostly vertical, with a radius at each."""
    rings = [ring(y, r, r, n, cx=x, cz=z) for (x, y, z), r in zip(points, radii)]
    return smooth_loft(rings, cap_start=cap_start, cap_end=cap_end)


def bent_limb(points, radii, n=8):
    """A tube through `points` that can go any way (an arm out to the side): each ring square to its segment."""
    rings = []
    for k, (p, r) in enumerate(zip(points, radii)):
        a = points[max(0, k - 1)]
        b = points[min(len(points) - 1, k + 1)]
        d = [b[i] - a[i] for i in range(3)]
        ln = math.sqrt(sum(x * x for x in d)) or 1.0
        d = [x / ln for x in d]
        ref = (0, 0, 1) if abs(d[2]) < 0.9 else (1, 0, 0)
        u = [d[1] * ref[2] - d[2] * ref[1], d[2] * ref[0] - d[0] * ref[2], d[0] * ref[1] - d[1] * ref[0]]
        ul = math.sqrt(sum(x * x for x in u))
        u = [x / ul for x in u]
        v = [d[1] * u[2] - d[2] * u[1], d[2] * u[0] - d[0] * u[2], d[0] * u[1] - d[1] * u[0]]
        rings.append([tuple(p[i] + r * (math.cos(math.tau * j / n) * u[i] + math.sin(math.tau * j / n) * v[i])
                            for i in range(3)) for j in range(n)])
    return smooth_loft(rings, cap_start=True, cap_end=True)


def head_surface(cy, rx, ry, rz, cz=0.0, k=8, power=2.0):
    """An egg of a head as a RingSurface (so features can be laid on its front), its rings from chin to crown."""
    rows = []
    for i in range(k + 1):
        t = -math.pi / 2 + math.pi * i / k
        s = max(math.cos(t), 0.06)
        rows.append((cy + ry * math.sin(t), rx * s, rz * s, cz, power))
    return RingSurface(rows, 12)


def on_face(surface, pts, lift):
    """A flat polygon (x, y points) laid on the front of `surface`, facing out."""
    return facing([(x, y, surface.front_z(x, y, lift)) for x, y in pts], 1)


def band(surface, x0, x1, top, bottom, lift, steps=10):
    """Quads between two edges across the face, top(x) and bottom(x), following its curve."""
    faces = []
    for i in range(steps):
        a = x0 + (x1 - x0) * i / steps
        b = x0 + (x1 - x0) * (i + 1) / steps
        faces.append(on_face(surface, [(a, bottom(a)), (b, bottom(b)), (b, top(b)), (a, top(a))], lift))
    return faces


def oval_on(surface, cx, cy, rx, ry, lift, n=10):
    return on_face(surface, [(cx + rx * math.cos(math.tau * i / n), cy + ry * math.sin(math.tau * i / n))
                             for i in range(n)], lift)


def ring_on(surface, cx, cy, r0, r1, lift, n=12, ry_scale=1.0):
    """An outline ring on the face, between radii r0 and r1."""
    faces = []
    for i in range(n):
        a0, a1 = math.tau * i / n, math.tau * (i + 1) / n
        faces.append(on_face(surface, [
            (cx + r0 * math.cos(a0), cy + r0 * math.sin(a0) * ry_scale),
            (cx + r1 * math.cos(a0), cy + r1 * math.sin(a0) * ry_scale),
            (cx + r1 * math.cos(a1), cy + r1 * math.sin(a1) * ry_scale),
            (cx + r0 * math.cos(a1), cy + r0 * math.sin(a1) * ry_scale),
        ], lift))
    return faces


# ---------------------------------------------------------------------------------------------------------------------
# The Tall Man


def tall_man():
    parts = []
    for side in (-1, 1):
        x = side * 210
        parts.append(S(limb([(x, 120, 20), (x, 2000, 0), (x * 1.05, 4100, 0), (x * 0.9, 4500, 0)],
                            [150, 150, 190, 220], cap_start=True), SUIT))
        parts.append(S(smooth_sphere(x, 70, 110, 140, 85, 290, n=10, stacks=5), BLACK))
    torso = RingSurface([
        (4300, 470, 260, 0, 2.2),
        (4700, 420, 245, 0, 2.2),  # the waist
        (5500, 500, 285, 0, 2.2),
        (6200, 640, 315, 0, 2.4),
        (6700, 760, 330, 0, 2.6),  # the shoulders
        (6950, 560, 300, 0, 2.2),
        (7120, 170, 170, 0, 2.0),
    ], 16)
    parts.append(S(torso.build(cap_start=True, cap_end=True), SUIT))

    # The shirt, a white V down the front, and the tie down the middle of it
    shirt = []
    bands = 6
    for i in range(bands):
        y0 = 5500 + 1300 * i / bands
        y1 = 5500 + 1300 * (i + 1) / bands
        w0, w1 = 250 * (y0 - 5500) / 1300, 250 * (y1 - 5500) / 1300
        for a, b in ((-1, 0), (0, 1)):
            shirt.append(on_face(torso, [(a * w0 if a else 0, y0), (b * w0 if b else 0, y0),
                                         (b * w1 if b else 0, y1), (a * w1 if a else 0, y1)], 8))
    parts.append(F(shirt, SHIRT))
    tie = []
    for i in range(6):
        y0 = 5450 + 1200 * i / 6
        y1 = 5450 + 1200 * (i + 1) / 6
        w0 = 70 if i else 40
        tie.append(on_face(torso, [(-w0, y0), (w0, y0), (60, y1), (-60, y1)], 16))
    tie.append(on_face(torso, [(-75, 6650), (75, 6650), (90, 6790), (-90, 6790)], 18))  # the knot
    parts.append(F(tie, BLACK))

    # Arms that hang far too low, and long pale fingers
    for side in (-1, 1):
        parts.append(S(limb([(side * 730, 6700, 0), (side * 800, 5600, 30), (side * 850, 4300, 70),
                             (side * 880, 2950, 110)], [175, 160, 150, 125], cap_start=True), SUIT))
        parts.append(S(smooth_sphere(side * 885, 2790, 120, 70, 170, 95, n=8, stacks=5), PALE))
        for k, dz in enumerate((-60, -10, 40, 90)):
            fx = side * (885 + (k - 1.5) * 18)
            parts.append(S(smooth_spike((fx, 2660, 120 + dz), (fx + side * 40, 2050 - 60 * abs(k - 1.5), 150 + dz),
                                        24, sides=5, bend=(side * 25, 0, 20)), PALE))

    # The neck, and a head with nothing on it
    parts.append(S(limb([(0, 7000, 10), (0, 7350, 20)], [135, 130], cap_end=False), PALE))
    parts.append(S(smooth_sphere(0, 7900, 30, 330, 560, 370, n=12, stacks=7), PALE))
    return parts


def tall_tendrils():
    parts = []
    for side in (-1, 1):
        for (rx, ry), (tx, ty, tz), bend in (
            ((220, 6450), (2700, 8900, -1500), (300, 900, -200)),
            ((340, 6150), (3300, 6300, -1700), (500, 500, -300)),
            ((180, 5800), (2200, 3800, -1600), (400, -200, -400)),
            ((90, 6650), (900, 9800, -1300), (100, 700, -100)),
        ):
            parts.append(S(smooth_spike((side * rx, ry, -250), (side * tx, ty, tz), 70, sides=6,
                                        bend=(side * bend[0], bend[1], bend[2])), BLACK))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# The Smiling Man


def smile_body():
    parts = []
    # One leg planted, the other knee up: a dancer mid-step
    parts.append(S(bent_limb([(-200, 80, 0), (-220, 1500, 40), (-200, 2900, 0), (-170, 3300, 0)],
                             [110, 110, 140, 160]), SMILE_SUIT))
    parts.append(S(bent_limb([(260, 900, 650), (240, 1700, 520), (220, 2500, 120), (180, 3300, 0)],
                             [110, 115, 140, 160]), SMILE_SUIT))
    parts.append(S(bent_limb([(260, 900, 650), (260, 200, 560)], [105, 100]), SMILE_SUIT))
    parts.append(S(smooth_sphere(-200, 60, 100, 110, 70, 260, n=8, stacks=5), BLACK))
    parts.append(S(smooth_sphere(260, 150, 680, 110, 70, 250, n=8, stacks=5), BLACK))
    torso = RingSurface([
        (3200, 360, 210, 0, 2.2),
        (3700, 330, 200, 0, 2.2),
        (4500, 420, 230, 0, 2.3),
        (5150, 560, 250, 0, 2.6),
        (5350, 420, 230, 0, 2.2),
        (5520, 120, 120, 0, 2.0),
    ], 14)
    parts.append(S(torso.build(cap_start=True, cap_end=True), SMILE_SUIT))
    collar = [on_face(torso, [(-120, 5000), (0, 4700), (0, 5000), (-80, 5250)], 8),
              on_face(torso, [(0, 4700), (120, 5000), (80, 5250), (0, 5000)], 8)]
    parts.append(F(collar, SHIRT))
    # Arms out wide, elbows up, long hands hanging loose
    for side in (-1, 1):
        parts.append(S(bent_limb([(side * 520, 5150, 0), (side * 1300, 5000, 120), (side * 1700, 5800, 250)],
                                 [110, 100, 90]), SMILE_SUIT))
        parts.append(S(bent_limb([(side * 1700, 5800, 250), (side * 1780, 6350, 300)], [70, 40]), SMILE_SKIN))
    parts.append(S(limb([(0, 5450, 0), (0, SMILE_NECK_Y + 80, 10)], [110, 100], cap_end=False), SMILE_SKIN))
    return parts


def smile_head(grin):
    """The head with its pivot at the neck: the grin (or, `grin` false, the frown)."""
    head = head_surface(450, 270, 400, 300, cz=20)
    parts = [S(head.build(cap_start=True, cap_end=True), SMILE_SKIN)]
    hair = smooth_sphere(0, 610, -30, 290, 300, 300, n=12, stacks=6)
    parts.append(S(hair, HAIR))
    # Wide eyes: whites and small black pupils
    eyes, pupils = [], []
    for side in (-1, 1):
        eyes.append(oval_on(head, side * 105, 560, 70 if grin else 66, 52 if grin else 22, 6))
        pupils.append(oval_on(head, side * 105, 560, 18, 18, 10, n=8))
    parts.append(F(eyes, EYE_WHITE))
    parts.append(F(pupils, BLACK))
    if grin:
        w = 245

        def top(x):
            return 330 + 150 * (x / w) ** 2

        def bottom(x):
            return 330 - 150 * (1 - (x / w) ** 2) + 150 * (x / w) ** 2

        parts.append(F(band(head, -w, w, top, bottom, 6, steps=14), MOUTH))
        # A row of teeth along the top and one along the bottom, with gaps between
        teeth = []
        n = 12
        for i in range(n):
            a = -w * 0.86 + 2 * w * 0.86 * i / n + 6
            b = -w * 0.86 + 2 * w * 0.86 * (i + 1) / n - 6
            ta, tb = top(a) - 4, top(b) - 4
            teeth.append(on_face(head, [(a, ta - 55), (b, tb - 55), (b, tb), (a, ta)], 10))
            ba, bb = bottom(a) + 4, bottom(b) + 4
            if bottom((a + b) / 2) < 300:
                teeth.append(on_face(head, [(a, ba), (b, bb), (b, bb + 45), (a, ba + 45)], 10))
        parts.append(F(teeth, TEETH))
    else:
        w = 140

        def top(x):
            return 300 + 50 * (1 - (x / w) ** 2)

        def bottom(x):
            return top(x) - 26

        parts.append(F(band(head, -w, w, top, bottom, 6, steps=8), MOUTH))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# Go to sleep


def sleep_body():
    parts = []
    for side in (-1, 1):
        x = side * 180
        parts.append(S(limb([(x, 120, 0), (x, 1300, 10), (x * 1.05, 2400, 0), (x * 0.95, 2650, 0)],
                            [120, 120, 150, 175], cap_start=True), JEANS))
        parts.append(S(smooth_sphere(x, 70, 90, 125, 75, 240, n=8, stacks=5), BLACK))
    torso = RingSurface([
        (2500, 430, 260, 0, 2.2),
        (2800, 470, 280, 0, 2.2),  # the hoodie's baggy hem
        (3600, 470, 270, 0, 2.3),
        (4000, 560, 290, 0, 2.6),
        (4200, 420, 260, 0, 2.2),
        (4320, 140, 140, 0, 2.0),
    ], 14)
    parts.append(S(torso.build(cap_start=True, cap_end=True), HOODIE))
    # The hood, bunched behind the neck
    parts.append(S(smooth_sphere(0, 4250, -230, 330, 200, 180, n=10, stacks=5), HOODIE))
    # A dark smear down the front
    stain = [on_face(torso, [(60, 2900), (230, 3100), (170, 3500), (40, 3300)], 6),
             on_face(torso, [(-200, 3200), (-80, 3350), (-130, 3550), (-230, 3400)], 6)]
    parts.append(F(stain, (96, 8, 10)))
    # Arms down, the right hand holding a knife point down
    for side in (-1, 1):
        parts.append(S(limb([(side * 500, 4000, 0), (side * 560, 3200, 40), (side * 590, 2350, 90)],
                            [135, 125, 115], cap_start=True), HOODIE))
        parts.append(S(smooth_sphere(side * 595, 2200, 100, 75, 120, 85, n=8, stacks=5), PALE))
    parts.append(S(smooth_sphere(-595, 2200, 100, 30, 70, 30, n=6, stacks=4), HANDLE))
    blade = [(-555, 2120, 150), (-640, 2120, 150), (-610, 1350, 165)]
    parts.append(F([facing(blade, 1), facing([(x, y, z - 8) for x, y, z in blade], -1)], BLADE))
    parts.append(S(limb([(0, 4250, 0), (0, SLEEP_NECK_Y + 100, 10)], [120, 110], cap_end=False), PALE))
    return parts


def sleep_hair():
    """Black hair, lit (so it's lost in the dark): a cap over the back of the head and strands hanging down."""
    parts = [S(smooth_sphere(0, 560, -50, 290, 330, 290, n=12, stacks=6), HAIR)]
    for i in range(9):
        a = math.pi * (0.15 + 0.7 * i / 8)
        x, z = 270 * math.cos(a), -40 - 250 * math.sin(a) * 0.9
        parts.append(S(smooth_spike((x, 650, z), (x * 1.25, 60 - 40 * (i % 3), z * 1.2 - 40), 70, sides=5,
                                    bend=(x * 0.1, 0, -40)), HAIR))
    for x in (-180, -60, 70, 190):  # a fringe over the brow, ragged
        parts.append(S(smooth_spike((x, 800, 150), (x * 1.1, 600, 290), 60, sides=5, bend=(0, 0, 30)), HAIR))
    return parts


def sleep_face():
    """His face, unlit: (colour, faces) parts for a vertex coloured display list."""
    head = head_surface(430, 250, 360, 270, cz=20)
    skin = []
    for k in range(len(head.rings) - 1):
        r0 = head.ring_points(head.rings[k])
        r1 = head.ring_points(head.rings[k + 1])
        for i in range(len(r0)):
            j = (i + 1) % len(r0)
            skin.append([r0[i], r0[j], r1[j], r1[i]])
    out = [((246, 246, 244), [facing_out(q, head) for q in skin])]
    eyes, whites, irises = [], [], []
    for side in (-1, 1):
        eyes += ring_on(head, side * 100, 500, 40, 95, 5, ry_scale=1.1)  # burned black around them
        whites.append(oval_on(head, side * 100, 500, 42, 46, 7))
        irises.append(oval_on(head, side * 100, 500, 15, 15, 10, n=8))
    out += [((6, 4, 6), eyes), ((250, 250, 250), whites), ((40, 40, 40), irises)]
    # The grin, carved up to the cheeks: a thin dark mouth, and red cuts up from its corners
    w = 210

    def top(x):
        return 290 + 110 * (x / w) ** 4

    def bottom(x):
        return top(x) - 20 - 20 * (1 - (x / w) ** 2)

    out.append(((110, 0, 6), band(head, -w, w, top, bottom, 6, steps=12)))
    cuts = []
    for side in (-1, 1):
        x0 = side * w
        cuts.append(on_face(head, [(x0 - side * 10, top(w) - 30), (x0 + side * 30, top(w) - 10),
                                   (x0 + side * 25, top(w) + 70), (x0 - side * 5, top(w) + 55)], 7))
    out.append(((150, 0, 8), cuts))
    return out


def facing_out(quad, surface):
    """Winds a quad on `surface` so it faces away from the surface's axis."""
    a, b, c = quad[0], quad[1], quad[2]
    n = [(b[1] - a[1]) * (c[2] - a[2]) - (b[2] - a[2]) * (c[1] - a[1]),
         (b[2] - a[2]) * (c[0] - a[0]) - (b[0] - a[0]) * (c[2] - a[2]),
         (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])]
    mid = [sum(p[i] for p in quad) / len(quad) for i in range(3)]
    cz = surface.rings[0][3]
    out = [mid[0], 0, mid[2] - cz]
    return quad if n[0] * out[0] + n[2] * out[2] >= 0 else list(reversed(quad))


def coloured_parts(mesh, parts):
    colours = {}
    faces = []
    for colour, fs in parts:
        for f in fs:
            faces.append(f)
            for p in f:
                colours[p] = colour
    return mesh.coloured_dl(faces, lambda p: colours[p])


# ---------------------------------------------------------------------------------------------------------------------
# The SCPs' surfaces: rough, cracked, rotten. Everything here is deterministic (no random module): the same script
# always writes the same file.


def noise(x, y, z, seed=0.0):
    """A smooth, repeatable lumpiness in about -1..1."""
    return (math.sin(x * 0.011 + seed) * math.sin(y * 0.009 + seed * 1.7)
            + 0.6 * math.sin(x * 0.023 + z * 0.017 + seed * 2.3) * math.cos(y * 0.021 + seed)
            + 0.4 * math.sin((x + y + z) * 0.041 + seed * 3.1)) / 2.0


def bumpy(amp, seed, scale=1.0):
    """A RingSurface bump: lumps `amp` deep."""
    return lambda x, y, t: amp * noise(x * scale, y * scale, t * 500 * scale, seed)


def roughen(m, amp, seed, scale=1.0):
    """A Smooth mesh with its surface pushed in and out along its normals."""
    normals = m.normals()
    out = m.copy()
    out.verts = [tuple(p[i] + n[i] / 127.0 * amp * noise(p[0] * scale, p[1] * scale, p[2] * scale, seed)
                       for i in range(3)) for p, n in zip(m.verts, normals)]
    return out


def resampled(rows, step):
    """RingSurface rows with more rings between them, `step` apart, so a bump or a ridge has rings to show on."""
    tmp = RingSurface(rows, 4)
    ys = [rows[0][0]]
    for a, b in zip(rows, rows[1:]):
        k = max(1, int(math.ceil((b[0] - a[0]) / step)))
        ys += [a[0] + (b[0] - a[0]) * i / k for i in range(1, k + 1)]
    return [tmp.params(y) for y in ys]


def move(parts, fn):
    """A part list with every point moved by fn (a rotation or a shift: nothing that turns faces inside out)."""
    out = []
    for kind, colour, geo in parts:
        if kind == "smooth":
            out.append((kind, colour, geo.transformed(fn)))
        else:
            out.append((kind, colour, [[tuple(fn(p)) for p in f] for f in geo]))
    return out


def shift(dx=0.0, dy=0.0, dz=0.0):
    return lambda p: (p[0] + dx, p[1] + dy, p[2] + dz)


def turn_z(deg, origin):
    a = math.radians(deg)
    c, s = math.cos(a), math.sin(a)
    return lambda p: (origin[0] + (p[0] - origin[0]) * c - (p[1] - origin[1]) * s,
                      origin[1] + (p[0] - origin[0]) * s + (p[1] - origin[1]) * c, p[2])


def turn_x(deg, origin):
    """Tips forward (+Z) for a positive angle."""
    a = math.radians(deg)
    c, s = math.cos(a), math.sin(a)
    return lambda p: (p[0], origin[1] + (p[1] - origin[1]) * c - (p[2] - origin[2]) * s,
                      origin[2] + (p[1] - origin[1]) * s + (p[2] - origin[2]) * c)


def disc_on(surface, cx, cy, rx, ry, lift, n=12, rings=2, wobble=0.0, seed=0.0):
    """A filled blob on the front of `surface`, in rings so it follows the curve; `wobble` makes its edge ragged."""
    def edge(i):
        t = math.tau * i / n
        w = 1.0 + wobble * noise(i * 97.0, seed * 31.0, 0.0, seed)
        return math.cos(t) * w, math.sin(t) * w

    faces = []
    for k in range(rings):
        f0, f1 = k / rings, (k + 1) / rings
        for i in range(n):
            (ax, ay), (bx, by) = edge(i), edge(i + 1)
            outer = [(cx + rx * ax * f1, cy + ry * ay * f1), (cx + rx * bx * f1, cy + ry * by * f1)]
            if k == 0:
                faces.append(on_face(surface, [(cx, cy)] + outer, lift))
            else:
                inner = [(cx + rx * bx * f0, cy + ry * by * f0), (cx + rx * ax * f0, cy + ry * ay * f0)]
                faces.append(on_face(surface, outer + inner, lift))
    return faces


def stroke(surface, pts, width, lift, taper=False, seg=190.0):
    """A line of paint (or a crack, or a drip) along (x, y) points on the front of `surface`."""
    dense = [pts[0]]
    for a, b in zip(pts, pts[1:]):
        k = max(1, int(math.ceil(math.hypot(b[0] - a[0], b[1] - a[1]) / seg)))
        dense += [(a[0] + (b[0] - a[0]) * i / k, a[1] + (b[1] - a[1]) * i / k) for i in range(1, k + 1)]
    faces = []
    count = len(dense) - 1
    for i, (a, b) in enumerate(zip(dense, dense[1:])):
        dx, dy = b[0] - a[0], b[1] - a[1]
        ln = math.hypot(dx, dy) or 1.0
        w0 = width * (1 - i / count * 0.8) if taper else width
        w1 = width * (1 - (i + 1) / count * 0.8) if taper else width
        n0 = (-dy / ln * w0 / 2, dx / ln * w0 / 2)
        n1 = (-dy / ln * w1 / 2, dx / ln * w1 / 2)
        faces.append(on_face(surface, [(a[0] - n0[0], a[1] - n0[1]), (b[0] - n1[0], b[1] - n1[1]),
                                       (b[0] + n1[0], b[1] + n1[1]), (a[0] + n0[0], a[1] + n0[1])], lift))
    return faces


def drip(x, y, length, wobble, seed):
    """Points of a run down from (x, y), wandering a little."""
    k = max(2, int(length / 120))
    return [(x + wobble * noise(seed * 13.0, i * 71.0, 0.0, seed), y - length * i / k) for i in range(k + 1)]


def strand(root, length, radius, seed, blob=True):
    """Something thick and wet hanging from `root`: a thin cone down, a drop at its end."""
    tip = (root[0] + 25 * noise(seed, 1.0, 2.0, seed), root[1] - length, root[2] + 40 * noise(2.0, seed, 1.0, seed))
    m = smooth_spike(root, tip, radius, sides=4, bend=(0, -length * 0.1, 15))
    if blob:
        m.extend(smooth_sphere(tip[0], tip[1] + radius * 0.5, tip[2], radius * 0.9, radius * 1.3, radius * 0.9,
                               n=4, stacks=3))
    return m


def finger(points, radii, knuckle=None):
    """A long finger through its joints, with a knot at each knuckle."""
    m = bent_limb(points, radii, n=4)
    if knuckle:
        p, r = points[1], radii[1]
        m.extend(smooth_sphere(p[0], p[1], p[2], r * 1.4, r * 1.4, r * 1.4, n=4, stacks=3))
    return m


# ---------------------------------------------------------------------------------------------------------------------
# SCP-173

CONCRETE = (138, 134, 126)
CONCRETE_DARK = (98, 94, 88)
CONCRETE_GRIME = (74, 70, 64)
CRACK = (34, 32, 30)
HOLE = (22, 20, 18)
REBAR = (104, 48, 26)
STAIN = (82, 30, 18)  # the reddish-brown mix on its floor
KRYLON_RED = (176, 26, 22)
KRYLON_GREEN = (36, 122, 56)
KRYLON_BLACK = (12, 12, 14)


def sculpture():
    """Poured concrete, roughly made and lopsided: thick legs, long hanging arms with rebar for fingers, a big head
    tipped to one side. Its face is spray paint: eyes too large and ringed in red, red and green run down from them,
    and a red slash of a mouth stitched shut with black. Cracks all over, rebar where chunks have broken off, and the
    reddish-brown stain down its front and over its hands."""
    parts = []
    # Legs: short, thick, not quite a pair
    for side, x, w in ((-1, -310, 1.0), (1, 330, 0.92)):
        rows = [(0, 360 * w, 430, 70, 2.8), (260, 330 * w, 360, 30, 2.6), (1300, 290 * w, 300, 0, 2.4),
                (2350, 350 * w, 340, 0, 2.6)]
        leg = smooth_loft([ring(y, rx, rz, 9, cx=x, cz=cz, power=p) for y, rx, rz, cz, p in resampled(rows, 520)],
                          cap_start=True, cap_end=True)
        parts.append(S(roughen(leg, 28, 3.0 + side), CONCRETE_DARK))
    # Grime splashed up its legs from the floor
    for side, x in ((-1, -310), (1, 330)):
        grime = smooth_loft([ring(y, r, r + 30, 9, cx=x, cz=40, power=2.6) for y, r in ((-10, 372), (300, 342))],
                            cap_start=False)
        parts.append(S(roughen(grime, 24, 3.0 + side), STAIN))

    torso = RingSurface(resampled([
        (2150, 650, 410, 0, 2.6),
        (2800, 610, 390, 10, 2.6),
        (3500, 590, 380, 20, 2.6),
        (4150, 700, 410, 20, 2.6),
        (4620, 770, 420, 0, 2.8),  # the shoulders, square
        (4900, 560, 380, -10, 2.4),
        (5080, 300, 260, 0, 2.2),
    ], 380), 12, bump=bumpy(34, 1.3))
    body = [S(torso.build(cap_start=True, cap_end=True), CONCRETE)]
    # Cracks
    cracks = []
    for pts in (((-380, 4400), (-300, 4220), (-350, 4050), (-250, 3880), (-290, 3700)),
                ((210, 3300), (300, 3150), (250, 2960), (340, 2800)),
                ((60, 4560), (120, 4420), (40, 4300)),
                ((-120, 2700), (-60, 2550), (-150, 2420), (-90, 2260))):
        cracks += stroke(torso, pts, 26, 6)
    body.append(F(cracks, CRACK))
    # Chunks broken off, rebar showing
    holes = [disc_on(torso, -420, 3250, 110, 90, 4, n=9, rings=1, wobble=0.25, seed=2),
             disc_on(torso, 380, 4150, 90, 120, 4, n=9, rings=1, wobble=0.25, seed=5)]
    body.append(F([f for h in holes for f in h], HOLE))
    for (x, y), (dx, dy, dz) in (((-420, 3250), (-70, -260, 200)), ((-400, 3230), (40, -320, 150)),
                                 ((380, 4150), (90, 230, 210))):
        z = torso.front_z(x, y) - 60
        body.append(S(smooth_spike((x, y, z), (x + dx, y + dy, z + dz), 26, sides=4, bend=(10, 0, 20)), REBAR))
    # The stain: down its front from the chest, dried in runs
    stain = disc_on(torso, -60, 3950, 230, 160, 7, n=10, rings=1, wobble=0.35, seed=7)
    for k, (x, length) in enumerate(((-200, 900), (-80, 1300), (70, 700), (150, 1050), (-20, 500))):
        stain += stroke(torso, drip(x, 3900, length, 40, k + 1), 70 - 6 * k, 8, taper=True)
    body.append(F(stain, STAIN))
    # Krylon: a green scrawl over one side, red sprayed across the belly
    body.append(F(stroke(torso, ((220, 4450), (480, 4250), (300, 4050), (520, 3850), (360, 3650)), 60, 9)
                  + stroke(torso, ((-480, 2600), (-260, 2900), (-420, 3050)), 55, 9), KRYLON_GREEN))
    body.append(F(stroke(torso, ((-380, 3450), (-100, 3380), (180, 3500), (420, 3420)), 70, 10)
                  + stroke(torso, drip(-100, 3380, 600, 30, 9), 38, 10, taper=True)
                  + stroke(torso, drip(260, 3470, 450, 30, 11), 34, 10, taper=True), KRYLON_RED))
    # One shoulder higher than the other
    parts += move(body, turn_z(3.5, (0, 2150, 0)))

    # Arms: hanging, too long, crude; the left reaching a little lower, both a little forward
    for side, low, seed in ((-1, 2300, 1.0), (1, 2480, 2.0)):
        sx = side * 790
        top_y = 4620 + side * 40
        pts = [(sx, top_y, 0), (side * 830, top_y - 700, 40), (side * 860, 3300, 90), (side * 870, low + 300, 150)]
        arm = bent_limb(pts, [250, 225, 215, 205], n=8)
        parts.append(S(roughen(arm, 22, seed), CONCRETE))
        hx, hy, hz = side * 880, low + 80, 170
        hand = smooth_sphere(hx, hy, hz, 230, 290, 250, n=8, stacks=4)
        parts.append(S(roughen(hand, 30, seed + 4), CONCRETE_DARK))
        # The stain all over its hands, and running off them
        parts.append(S(roughen(smooth_sphere(hx, hy - 60, hz + 30, 236, 220, 248, n=8, stacks=3), 30, seed + 4),
                       STAIN))
        # Rebar for fingers, bent and of different lengths
        for k, (dx, length, dz) in enumerate(((-90, 420, 60), (0, 560, 110), (90, 380, 40))):
            root = (hx + side * dx, hy - 180, hz + 40)
            tip = (hx + side * (dx * 1.6), hy - 180 - length, hz + dz + 120)
            parts.append(S(smooth_spike(root, tip, 28, sides=4, bend=(side * 30, 0, 60 - 30 * k)), REBAR))
        # Rebar out of the shoulder
        parts.append(S(smooth_spike((side * 700, top_y + 60, -80), (side * 900, top_y + 480, -230), 30, sides=4,
                                    bend=(side * 40, 0, -40)), REBAR))

    # Neck and head: big, lumpy, tipped over to one side and a little forward
    head = RingSurface(resampled([
        (5000, 90, 90, 20, 2.0),
        (5150, 420, 390, 30, 2.4),
        (5500, 590, 520, 40, 2.6),
        (5900, 620, 540, 40, 2.6),
        (6300, 560, 500, 30, 2.4),
        (6600, 380, 350, 20, 2.2),
        (6760, 60, 60, 20, 2.0),
    ], 280), 12, bump=bumpy(34, 4.4, 1.3))
    face = [S(head.build(cap_start=True, cap_end=True), CONCRETE)]
    face.append(S(roughen(limb([(0, 4900, 10), (0, 5250, 20)], [300, 290], n=8), 20, 6.0), CONCRETE_DARK))
    # The eyes: big, not a pair, ringed in red, the black filled in heavily
    for cx, cy, rx, ry, seed in ((-230, 6020, 175, 135, 1), (240, 5990, 150, 170, 2)):
        face.append(F(disc_on(head, cx, cy, rx * 1.38, ry * 1.38, 6, n=12, rings=1, wobble=0.12, seed=seed),
                      KRYLON_RED))
        face.append(F(disc_on(head, cx, cy, rx, ry, 10, n=12, rings=1, wobble=0.1, seed=seed + 3), KRYLON_BLACK))
        runs = []
        for k, (dx, length) in enumerate(((-60, 520), (40, 760), (110, 380))):
            runs += stroke(head, drip(cx + dx, cy - ry, length, 25, seed * 5 + k), 42, 8, taper=True)
        face.append(F(runs, KRYLON_RED if seed == 1 else KRYLON_GREEN))
    # Green sprayed across the brow
    face.append(F(stroke(head, ((-420, 6350), (-150, 6420), (80, 6330), (380, 6440)), 60, 9), KRYLON_GREEN))
    # The mouth: a long red slash, stitched shut in black, running
    mouth = [(-330, 5530), (-180, 5480), (0, 5500), (170, 5460), (320, 5520)]
    face.append(F(stroke(head, mouth, 80, 8)
                  + stroke(head, drip(-120, 5470, 420, 20, 21), 34, 8, taper=True)
                  + stroke(head, drip(200, 5450, 300, 20, 22), 30, 8, taper=True), KRYLON_RED))
    stitches = []
    for i in range(9):
        x = -300 + 75 * i
        y = 5500 + 20 * math.sin(i * 1.7)
        stitches += stroke(head, ((x - 10, y + 80), (x + 10, y - 80)), 18, 12)
    face.append(F(stitches, KRYLON_BLACK))
    face.append(F(stroke(head, ((300, 6600), (380, 6420), (330, 6250)), 22, 6)
                  + stroke(head, ((-470, 5600), (-400, 5800), (-460, 5900)), 22, 6), CRACK))
    parts += move(move(face, turn_z(-9, (0, 5000, 0))), turn_x(5, (0, 5000, 0)))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# SCP-096

SHY_SKIN = (214, 210, 202)
SHY_SHADOW = (150, 144, 136)
SHY_BONE = (232, 228, 220)
SHY_SOCKET = (96, 88, 84)
SHY_EYE = (244, 244, 240)
SHY_NAIL = (120, 104, 92)
SHY_GUM = (96, 14, 18)
SHY_THROAT = (16, 4, 6)
SHY_TEETH = (226, 218, 186)
SHY_HEAD_Z = 300  # its head hangs this far forward of its neck


def shy_body():
    """Starved: legs like sticks with knots for knees, a belly sunk in under a ribcage you can count, collarbones and
    shoulder blades standing out, every knob of its spine, all of it stooped forward."""
    parts = []
    for side in (-1, 1):
        x = side * 210
        # Knobbed knees: the leg swells there and thins again either side
        parts.append(S(bent_limb([(x, 230, -20), (x * 1.02, 1100, 40), (x * 1.04, 1720, 120), (x * 1.04, 1900, 150),
                                  (x * 1.04, 2080, 130), (x, 2800, 70), (x * 0.95, 3650, 0)],
                                 [62, 80, 74, 104, 82, 100, 140], n=8), SHY_SKIN))
        # A long thin foot, its toes too long
        parts.append(S(bent_limb([(x, 120, -110), (x, 110, 120), (x * 1.06, 60, 380)], [80, 72, 46], n=6),
                       SHY_SKIN))
        for k in range(4):
            tx = x * 1.06 + side * (k - 1.5) * 40
            parts.append(S(smooth_spike((tx, 50, 380), (tx + side * (k - 1.5) * 20, 20, 600 - 40 * abs(k - 1.5)),
                                        20, sides=3), SHY_SKIN))

    torso = RingSurface(resampled([
        (3500, 330, 200, 0, 2.2),
        (3900, 290, 150, 30, 2.2),  # the belly, sunk in
        (4400, 280, 140, 50, 2.2),
        (4900, 380, 230, 70, 2.2),  # the bottom of its ribs
        (5400, 450, 255, 110, 2.2),
        (5900, 520, 255, 170, 2.3),
        (6200, 600, 230, 220, 2.5),  # shoulders
        (6420, 340, 190, 260, 2.2),
        (6560, 120, 120, 280, 2.0),
    ], 300), 12)
    parts.append(S(torso.build(cap_start=True, cap_end=True), SHY_SKIN))
    # The shadows between its ribs, and under them
    hollows = []
    for k in range(6):
        y = 5045 + 190 * k
        for side in (-1, 1):
            hollows += stroke(torso, ((side * 70, y + 30), (side * 250, y - 20), (side * 400, y - 70)), 34, 5, seg=400)
    for side in (-1, 1):
        hollows += stroke(torso, ((side * 60, 4650), (side * 190, 4300), (side * 170, 3950)), 70, 4, taper=True)
    parts.append(F(hollows, SHY_SHADOW))
    parts.append(F(stroke(torso, ((0, 5000), (0, 6100)), 50, 9), SHY_BONE))  # breastbone
    for side in (-1, 1):
        # Collarbones and shoulder blades
        a = (side * 60, 6230, torso.front_z(60, 6230) + 10)
        b = (side * 540, 6300, 230)
        parts.append(S(bent_limb([a, ((a[0] + b[0]) / 2, 6290, (a[2] + b[2]) / 2 + 40), b], [40, 36, 46], n=4),
                       SHY_BONE))
        parts.append(S(smooth_sphere(side * 250, 5850, torso.back_z(250, 5850) + 30, 170, 260, 70, n=6, stacks=3),
                       SHY_SKIN))
        parts.append(S(smooth_sphere(side * 600, 6200, 210, 120, 110, 120, n=6, stacks=3), SHY_SKIN))  # shoulder
    # Its spine, each knob of it
    for k in range(12):
        y = 3800 + 210 * k
        parts.append(S(smooth_sphere(0, y, torso.back_z(0, y) + 10, 44, 60, 42, n=4, stacks=3), SHY_BONE))
    return parts


def shy_hand(side, wrist, down, curl):
    """Long hands: fingers half as long again as they should be, two knuckles each, dark nails. `down` is the way
    the hand points (unit x, y, z); `curl` is which way the fingers bend."""
    parts = []
    palm_end = tuple(wrist[i] + down[i] * 420 for i in range(3))
    parts.append(S(bent_limb([wrist, tuple(wrist[i] + down[i] * 200 for i in range(3)), palm_end], [52, 70, 56],
                             n=6), SHY_SKIN))
    across = (1.0, 0.0, 0.0) if abs(down[0]) < 0.7 else (0.0, 0.0, 1.0)
    for k in range(4):
        off = (k - 1.5) * 34
        base = tuple(palm_end[i] + across[i] * off for i in range(3))
        length = (520, 600, 570, 460)[k]
        j1 = tuple(base[i] + down[i] * length * 0.42 + curl[i] * 30 for i in range(3))
        j2 = tuple(j1[i] + down[i] * length * 0.33 + curl[i] * 80 for i in range(3))
        tip = tuple(j2[i] + down[i] * length * 0.25 + curl[i] * 110 for i in range(3))
        parts.append(S(finger([base, j1, j2, tip], [24, 21, 18, 12], knuckle=True), SHY_SKIN))
        parts.append(S(smooth_spike(tuple(j2[i] + (tip[i] - j2[i]) * 0.6 for i in range(3)),
                                    tuple(tip[i] + down[i] * 40 + curl[i] * 40 for i in range(3)), 13, sides=4),
                       SHY_NAIL))
    thumb_base = tuple(wrist[i] + down[i] * 160 + curl[i] * 50 for i in range(3))
    thumb_tip = tuple(thumb_base[i] + down[i] * 320 + curl[i] * 120 for i in range(3))
    parts.append(S(finger([thumb_base, tuple((thumb_base[i] + thumb_tip[i]) / 2 for i in range(3)), thumb_tip],
                          [26, 22, 14], knuckle=True), SHY_SKIN))
    return parts


def norm(v):
    ln = math.sqrt(sum(x * x for x in v)) or 1.0
    return tuple(x / ln for x in v)


def shy_arms(up):
    parts = []
    for side in (-1, 1):
        shoulder = (side * 610, 6230, 210)
        if up:
            # Hands over its face: elbows out in front of its chest, long fingers spread over its head
            elbow = (side * 640, 4700, 900)
            wrist = (side * 230, 6560, 760)
            down = norm((side * -0.15, 1.0, -0.05))
            curl = (0.0, 0.0, -1.0)  # over the top of its head
        else:
            elbow = (side * 690, 4500, 230)
            wrist = (side * 720, 2600, 330)
            down = norm((side * 0.02, -1.0, 0.12))
            curl = (side * -0.4, 0.0, 0.9)
        mid1 = tuple((shoulder[i] + elbow[i]) / 2 for i in range(3))
        mid2 = tuple((elbow[i] + wrist[i]) / 2 for i in range(3))
        parts.append(S(bent_limb([shoulder, mid1, elbow, mid2, wrist], [100, 66, 82, 56, 48], n=7), SHY_SKIN))
        parts += shy_hand(side, wrist, down, curl)
    return parts


def shy_head(scream):
    """Its head, the pivot at its neck, hanging forward: long and gaunt, cheeks sunk in, small eyes with no colour in
    them deep in dark sockets, no nose to speak of. Screaming, its jaw drops to its chest, the mouth a hole lined with
    teeth."""
    parts = []
    z = SHY_HEAD_Z
    parts.append(S(bent_limb([(0, -120, 230), (0, 120, 270), (0, 260, z)], [120, 112, 105], n=8), SHY_SKIN))
    skull = [
        (520, 250, 245, z, 2.0),  # cheekbones
        (700, 270, 270, z - 10, 2.0),
        (860, 266, 276, z - 20, 2.0),
        (990, 222, 240, z - 30, 2.0),
        (1090, 120, 140, z - 40, 2.0),
        (1125, 20, 20, z - 40, 2.0),
    ]
    if scream:
        jaw = [(-760, 50, 50, z + 70, 2.0), (-680, 170, 150, z + 70, 2.0), (-380, 225, 215, z + 60, 2.0),
               (0, 235, 225, z + 40, 2.0), (300, 232, 225, z + 20, 2.0)]
    else:
        jaw = [(70, 50, 50, z + 40, 2.0), (150, 150, 140, z + 40, 2.0), (290, 195, 200, z + 30, 2.0),
               (420, 188, 210, z + 20, 2.0)]  # the cheeks, sunk in
    head = RingSurface(resampled(jaw + skull, 170), 12)
    parts.append(S(head.build(cap_start=True, cap_end=True), SHY_SKIN))
    # Sockets, deep, and the eyes in them: no colour at all, too small
    sockets, eyes, hollows = [], [], []
    for side in (-1, 1):
        sockets += disc_on(head, side * 108, 700, 96, 84 if scream else 72, 5, n=10, rings=1, wobble=0.18,
                           seed=side + 3)
        eyes += disc_on(head, side * 106, 690, 44 if scream else 38, 34 if scream else 26, 9, n=8, rings=1)
        hollows += stroke(head, ((side * 120, 520), (side * 128, 420), (side * 110, 320)), 40, 3, taper=True)
    parts.append(F(sockets, SHY_SOCKET))
    parts.append(F(eyes, SHY_EYE))
    parts.append(F(hollows, SHY_SHADOW))
    parts.append(F(disc_on(head, -26, 545, 14, 24, 6, n=6, rings=1) + disc_on(head, 26, 545, 14, 24, 6, n=6, rings=1),
                   SHY_SOCKET))  # where a nose would be
    if scream:
        parts.append(F(disc_on(head, 0, -150, 200, 600, 6, n=14, rings=2), SHY_GUM))
        parts.append(F(disc_on(head, 0, -170, 150, 500, 10, n=12, rings=2), SHY_THROAT))
        teeth = Smooth()
        for k in range(9):
            u = -1 + 2 * k / 8
            for top, (cy, ry) in ((True, (-150, 600)), (False, (-150, 600))):
                x = u * 150
                y = cy + (1 if top else -1) * ry * math.sqrt(max(0.0, 1 - u * u)) * 0.86
                zz = head.front_z(x, y, 4)
                length = 70 + 40 * abs(math.sin(k * 2.3 + (0 if top else 1)))
                tip = (x * 0.92, y + (-length if top else length), zz + 10)
                teeth.extend(smooth_spike((x, y, zz - 15), tip, 18, sides=3))
        parts.append(S(teeth, SHY_TEETH))
    else:
        # Its mouth a little open, and nothing behind it
        parts.append(F(disc_on(head, 0, 262, 82, 18, 6, n=8, rings=1), SHY_THROAT))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# SCP-106

OLD_SKIN = (40, 34, 30)
OLD_WET = (16, 12, 10)
OLD_ROT = (92, 64, 36)
OLD_MOULD = (58, 66, 36)
OLD_PEEL = (118, 92, 66)
OLD_BONE = (150, 132, 100)
OLD_EYE = (232, 204, 110)
OLD_TEETH = (176, 154, 104)
OLD_GUM = (60, 20, 16)


def old_man():
    """An old man, rotting: hunched, bald, his skin black and wet, sloughing off in patches, rot and mould on it.
    Everything about him drips: his chin, his elbows, his fingers, his belly. His eyes are pinpricks deep in his skull,
    and his lips have gone, so he grins with all his teeth."""
    parts = []
    drips = Smooth()
    for side in (-1, 1):
        x = side * 220
        leg = bent_limb([(x, 80, 40), (x * 1.04, 900, 160), (x * 1.06, 1700, 250), (x * 1.02, 2500, 140),
                         (x * 0.95, 3200, 0)], [120, 100, 118, 130, 165], n=8)
        parts.append(S(roughen(leg, 26, 1.5 + side), OLD_SKIN))
        parts.append(S(roughen(smooth_sphere(x, 50, 150, 130, 70, 270, n=6, stacks=3), 20, 2.0 + side), OLD_WET))
        drips.extend(strand((x * 1.06, 1650, 330), 520, 26, side * 3.0))
    torso = RingSurface(resampled([
        (3000, 410, 270, 0, 2.2),
        (3700, 390, 255, 80, 2.2),
        (4300, 470, 275, 220, 2.2),
        (4900, 570, 295, 400, 2.4),
        (5200, 510, 285, 480, 2.2),
        (5420, 190, 180, 560, 2.0),
    ], 330), 12, bump=bumpy(44, 2.7, 1.4))
    parts.append(S(torso.build(cap_start=True, cap_end=True), OLD_SKIN))
    parts.append(F(disc_on(torso, -230, 4000, 200, 260, 6, n=10, rings=1, wobble=0.4, seed=1)
                   + disc_on(torso, 250, 4650, 170, 210, 6, n=10, rings=1, wobble=0.4, seed=2), OLD_ROT))
    parts.append(F(disc_on(torso, 40, 3500, 160, 130, 7, n=10, rings=1, wobble=0.45, seed=3)
                   + disc_on(torso, -330, 4750, 110, 150, 7, n=10, rings=1, wobble=0.4, seed=4), OLD_MOULD))
    parts.append(F(disc_on(torso, 300, 3900, 140, 180, 9, n=10, rings=1, wobble=0.5, seed=5), OLD_PEEL))
    # His ribs through the rot on one side
    ribs = []
    for k in range(4):
        y = 4250 + 160 * k
        ribs += stroke(torso, ((-120, y), (-260, y - 40), (-420, y - 110)), 34, 10)
    parts.append(F(ribs, OLD_BONE))
    wet = []
    for k, (x, y) in enumerate(((-100, 4500), (180, 4200), (-260, 3600))):
        wet += disc_on(torso, x, y, 70, 60, 10, n=8, rings=1, wobble=0.3, seed=k + 6)
        wet += stroke(torso, drip(x, y - 40, 700 + 200 * k, 30, k + 6), 44, 10, taper=True)
    parts.append(F(wet, OLD_WET))
    for x in (-250, -80, 120, 300):
        drips.extend(strand((x, 3020, torso.front_z(x, 3020) - 20), 420 + abs(x), 30, x / 100.0))

    for side in (-1, 1):
        shoulder = (side * 540, 5000, 430)
        elbow = (side * 660, 3850, 640)
        wrist = (side * 640, 2650, 700)
        arm = bent_limb([shoulder, (side * 610, 4400, 560), elbow, (side * 660, 3250, 690), wrist],
                        [130, 105, 100, 92, 78], n=8)
        parts.append(S(roughen(arm, 24, 3.3 + side), OLD_SKIN))
        parts.append(S(roughen(smooth_sphere(elbow[0], elbow[1], elbow[2] - 50, 110, 120, 100, n=6, stacks=3), 15,
                               side), OLD_WET))
        drips.extend(strand((elbow[0], elbow[1] - 80, elbow[2] - 60), 600, 30, side * 5.0))
        palm = (side * 650, 2380, 740)
        parts.append(S(bent_limb([wrist, palm], [74, 84], n=6), OLD_SKIN))
        # Long fingers, crooked, the nails gone to claws
        for k in range(5):
            if k == 4:
                base = (side * 590, 2480, 820)
                d = norm((side * -0.4, -0.8, 0.5))
                length = 360
            else:
                base = (palm[0] + side * (k - 1.5) * 38, palm[1] - 60, palm[2] + 30)
                d = norm((side * (k - 1.5) * 0.12, -1.0, 0.25))
                length = (560, 640, 610, 500)[k]
            j1 = tuple(base[i] + d[i] * length * 0.45 for i in range(3))
            j2 = (j1[0] + d[0] * length * 0.3, j1[1] + d[1] * length * 0.3, j1[2] + d[2] * length * 0.3 + 70)
            tip = (j2[0] + d[0] * length * 0.25, j2[1] + d[1] * length * 0.25 + 40, j2[2] + 120)
            parts.append(S(finger([base, j1, j2, tip], [26, 23, 19, 14], knuckle=True), OLD_SKIN))
            parts.append(S(smooth_spike(j2, (tip[0], tip[1] - 30, tip[2] + 60), 15, sides=3), OLD_BONE))
            if k % 2 == 0:
                drips.extend(strand(tip, 260 + 80 * k, 14, side * 7.0 + k, blob=k != 2))

    parts.append(S(roughen(limb([(0, 5300, 540), (0, 5550, 650)], [180, 165], n=8, cap_end=False), 18, 7.0),
                   OLD_SKIN))
    head = RingSurface(resampled([
        (5420, 60, 60, 760, 2.0),
        (5500, 200, 190, 740, 2.0),  # the chin, sagging
        (5650, 270, 270, 720, 2.0),
        (5850, 300, 300, 700, 2.0),
        (6050, 310, 300, 690, 2.0),
        (6250, 270, 270, 680, 2.0),
        (6420, 160, 170, 670, 2.0),
        (6480, 30, 30, 670, 2.0),
    ], 150), 12, bump=bumpy(30, 5.1, 1.8))
    parts.append(S(head.build(cap_start=True, cap_end=True), OLD_SKIN))
    parts.append(F(disc_on(head, -150, 6250, 110, 90, 6, n=10, rings=1, wobble=0.5, seed=8)
                   + disc_on(head, 170, 6150, 80, 110, 6, n=10, rings=1, wobble=0.5, seed=9), OLD_PEEL))
    parts.append(F(disc_on(head, 60, 6330, 90, 60, 6, n=10, rings=1, wobble=0.5, seed=10), OLD_ROT))
    # Sockets sunk deep, a pinprick of light in each
    parts.append(F(disc_on(head, -112, 5960, 95, 78, 5, n=10, rings=1, wobble=0.15, seed=11)
                   + disc_on(head, 112, 5975, 88, 82, 5, n=10, rings=1, wobble=0.15, seed=12), OLD_WET))
    parts.append(F(disc_on(head, -112, 5955, 20, 20, 10, n=6, rings=1) + disc_on(head, 112, 5970, 20, 20, 10, n=6,
                                                                                    rings=1), OLD_EYE))
    parts.append(F(stroke(head, ((-20, 5880), (-30, 5800)), 22, 6) + stroke(head, ((20, 5880), (30, 5800)), 22, 6),
                   OLD_WET))
    # The grin: no lips, gums, every tooth crooked
    grin = disc_on(head, 0, 5680, 210, 70, 6, n=12, rings=1)
    parts.append(F(grin, OLD_GUM))
    teeth = Smooth()
    for k in range(10):
        u = -0.9 + 1.8 * k / 9
        if k in (2, 7):
            continue  # gone
        x = u * 200
        for top in (True, False):
            y = 5680 + (24 if top else -24)
            zz = head.front_z(x, y, 2)
            length = 45 + 25 * abs(math.sin(k * 1.9 + top))
            teeth.extend(smooth_spike((x, y + (20 if top else -20), zz - 10),
                                      (x + 10 * math.sin(k * 3.1), y + (-length if top else length), zz + 14), 16,
                                      sides=3))
    parts.append(S(teeth, OLD_TEETH))
    for k, x in enumerate((-160, -40, 90, 190)):
        drips.extend(strand((x, 5560 - abs(x) * 0.3, head.front_z(x, 5560) - 30), 300 + 120 * (k % 2), 22, k + 20.0))
    parts.append(S(drips, OLD_WET))
    return parts


def corrosion():
    """A pool of black corrosion on the ground, its edge ragged and fading out."""
    faces = []
    colours = {}
    n = 18
    rings = []
    for k, (r, a) in enumerate(((0, 235), (900, 225), (1700, 170), (2300, 0))):
        pts = []
        for i in range(n):
            t = math.tau * i / n
            rr = r * (1.0 + 0.22 * math.sin(i * 2.7 + k) + 0.1 * math.cos(i * 5.3))
            p = (rr * math.cos(t), 3.0 + k, rr * math.sin(t))
            colours[p] = (255, 255, 255, a)
            pts.append(p)
        rings.append(pts)
    for k in range(len(rings) - 1):
        for i in range(n):
            j = (i + 1) % n
            q = [rings[k][i], rings[k][j], rings[k + 1][j], rings[k + 1][i]]
            faces.append(q if k else q[:1] + q[2:])  # the middle ring is one point: triangles
    return faces, lambda p: colours[p]


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else OUT

    tall = Mesh("sTallManVtx")
    tall_dl = gfx("sTallManDL", emit_parts(tall, tall_man()))
    tendrils_dl = gfx("sTallTendrilsDL", emit_parts(tall, tall_tendrils()))

    smile = Mesh("sSmileManVtx")
    smile_body_dl = gfx("sSmileBodyDL", emit_parts(smile, smile_body()))
    grin_dl = gfx("sSmileGrinDL", emit_parts(smile, smile_head(True)))
    frown_dl = gfx("sSmileFrownDL", emit_parts(smile, smile_head(False)))

    sleep = Mesh("sSleepManVtx")
    sleep_body_dl = gfx("sSleepBodyDL", emit_parts(sleep, sleep_body()))
    hair_dl = gfx("sSleepHairDL", emit_parts(sleep, sleep_hair()))
    face_dl = gfx("sSleepFaceDL", coloured_parts(sleep, sleep_face()))

    setup = "\n".join([
        setup_dl("sVisitorSetupDL", LIT_OPA, "The visitors' bodies: opaque, lit, untextured"),
        setup_dl("sVisitorFaceSetupDL", GLOW_FRONT,
                 "A face that's there in the dark: unlit, no fog, colour = vertex colour x primitive colour"),
    ])
    text = f"""// Generated by tools/ben/gen_ben_visitors.py; edit that script instead of this file.
// The Expanded Horror visitors. +Y up, +Z forward, +X to its left; 1/100 of a world unit.
// clang-format off

#define TALL_MAN_HEIGHT ({TALL_HEIGHT}.0f)
#define SMILE_MAN_NECK_Y ({SMILE_NECK_Y}.0f)
#define SLEEP_MAN_NECK_Y ({SLEEP_NECK_Y}.0f)

{tall.vtx_array()}
{smile.vtx_array()}
{sleep.vtx_array()}
{setup}
{tall_dl}
{tendrils_dl}
{smile_body_dl}
{grin_dl}
{frown_dl}
{sleep_body_dl}
{hair_dl}
{face_dl}
// clang-format on
"""
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(text)
    print(f"wrote {out} ({len(tall.vtx)} tall, {len(smile.vtx)} smiling, {len(sleep.vtx)} sleep vertices)")

    # Each SCP has an actor (and so an overlay) of its own, so only the one that's here is in memory
    outs = [Path(a) for a in sys.argv[2:5]] + [p for p in SCP_OUTS[len(sys.argv[2:5]):]]
    setup = setup_dl("sScpSetupDL", LIT_OPA, "Its body: opaque, lit, untextured")
    ground = setup_dl("sScpGroundSetupDL", GLOW,
                      "What it leaves on the ground: unlit, colour and alpha = vertex colour x primitive colour, "
                      "double sided")

    s173 = Mesh("sScp173Vtx")
    s173_dl = gfx("sScp173DL", emit_parts(s173, sculpture()))
    pool_faces, pool_colour = corrosion()
    stain_dl = gfx("sScpPoolDL", s173.coloured_dl(pool_faces, pool_colour))
    write_scp(outs[0], "SCP-173", "", s173, [setup, ground], [s173_dl, stain_dl])

    s096 = Mesh("sScp096Vtx")
    dls = [gfx("sScp096BodyDL", emit_parts(s096, shy_body())),
           gfx("sScp096ArmsDownDL", emit_parts(s096, shy_arms(False))),
           gfx("sScp096ArmsUpDL", emit_parts(s096, shy_arms(True))),
           gfx("sScp096HeadDL", emit_parts(s096, shy_head(False))),
           gfx("sScp096ScreamDL", emit_parts(s096, shy_head(True)))]
    write_scp(outs[1], "SCP-096", f"#define SCP_096_NECK_Y ({SHY_NECK_Y}.0f)\n\n", s096, [setup], dls)

    s106 = Mesh("sScp106Vtx")
    s106_dl = gfx("sScp106DL", emit_parts(s106, old_man()))
    pool_dl = gfx("sScpPoolDL", s106.coloured_dl(pool_faces, pool_colour))
    write_scp(outs[2], "SCP-106", "", s106, [setup, ground], [s106_dl, pool_dl])


def write_scp(out, name, defines, mesh, setups, dls):
    body = "\n".join(setups + dls)
    text = f"""// Generated by tools/ben/gen_ben_visitors.py; edit that script instead of this file.
// {name}, after the SCP wiki (CC BY-SA 3.0, docs/ben/CREDITS.md). +Y up, +Z forward, +X to its left; 1/100 of a world
// unit.
// clang-format off

{defines}{mesh.vtx_array()}
{body}
// clang-format on
"""
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(text)
    print(f"wrote {out} ({len(mesh.vtx)} vertices)")

if __name__ == "__main__":
    main()
