#!/usr/bin/env python3
"""
Generates the Expanded Horror visitors' models as N64 display lists (F3DEX2), written to
src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_models.inc.c (the visitors' own actor, En_Ben_Visitor, includes it). Built on
the model kit copied from the Majora's Mask: WTF project (tools/ben/modelkit.py).

Usage: tools/ben/gen_ben_visitors.py [visitors output file] [SCP output file]

- The Tall Man: twice Link's height, a black suit, a white shirt and a black tie, arms that hang past his knees with
  long pale fingers, and a pale head with no face at all. His tendrils are a display list of their own, drawn only
  when he's close to having you.
- The Smiling Man: thin, in a dark suit, arms out and one knee up as if he's dancing. Two heads on the same neck: the
  grin, far too wide and full of teeth, and the frown he wears when he gives up on you.
- The one who says "Go to sleep": a white hoodie and black jeans, a kitchen knife. His face is drawn unlit and
  unfogged (a display list of its own, vertex coloured), so when the lights go out it's the only thing you can see:
  bone white, black rings around eyes that never close, a red cut grin up to the cheeks. Black hair, lit.

And the SCP Foundation's, written to src/overlays/actors/ovl_En_Ben_Scp/z_en_ben_scp_models.inc.c (En_Ben_Scp includes
it), each built from its SCP wiki article's words (CC BY-SA 3.0, credited in docs/ben/CREDITS.md), not from any picture:
- SCP-173: concrete and rebar, with traces of Krylon spray paint. A crude figure, thick limbs with rusted rebar
  sticking out of them, a big head, its face sprayed on.
- SCP-096: about 2.4 m, emaciated, no hair, skin almost without colour, arms far too long, pale eyes. Its body and
  its arms (hanging, or up over its face) are display lists of their own, and so are its head calm and screaming, the
  jaw hanging open far wider than a jaw should.
- SCP-106: an old man rotting, hunched over, black and wet. And the pool of corrosion he leaves, drawn on the ground
  (vertex alpha fades its edge).

Model space: +Y up, +Z forward (where the face looks), +X to its left; 1/100 of a world unit (drawn at scale 0.01).
Each head's pivot is its neck, at <NAME>_NECK_Y.
"""

import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from modelkit import (  # noqa: E402
    GLOW, GLOW_FRONT, LIT_OPA, Mesh, RingSurface, S, F, emit_parts, facing, gfx, ring, setup_dl, smooth_loft,
    smooth_sphere, smooth_spike,
)

OUT = Path(__file__).resolve().parents[2] / "src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_models.inc.c"
SCP_OUT = Path(__file__).resolve().parents[2] / "src/overlays/actors/ovl_En_Ben_Scp/z_en_ben_scp_models.inc.c"

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

CONCRETE = (148, 144, 136)
CONCRETE_DARK = (110, 106, 100)
REBAR = (96, 52, 30)
KRYLON_RED = (168, 32, 26)
KRYLON_GREEN = (44, 118, 62)
SHY_SKIN = (216, 212, 204)
SHY_SHADOW = (168, 162, 154)
SHY_SOCKET = (70, 64, 60)
OLD_SKIN = (44, 38, 32)
OLD_ROT = (80, 60, 40)
SLIME = (10, 8, 6)
OLD_EYE = (206, 190, 130)

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
# SCP-173


def sculpture():
    parts = []
    for side in (-1, 1):
        x = side * 300
        legs = smooth_loft([ring(y, r, r * 0.9, 8, cx=x, power=2.8) for y, r in ((0, 340), (1200, 300), (2200, 330))],
                           cap_start=True, cap_end=True)
        parts.append(S(legs, CONCRETE_DARK))
    torso = RingSurface([
        (2000, 680, 420, 0, 3.0),
        (3200, 640, 400, 0, 3.0),
        (4300, 740, 430, 0, 3.0),
        (4700, 620, 400, 0, 2.6),
        (4950, 260, 240, 0, 2.2),
    ], 12)
    parts.append(S(torso.build(cap_start=True, cap_end=True), CONCRETE))
    for side in (-1, 1):
        parts.append(S(limb([(side * 800, 4550, 0), (side * 830, 3500, 20), (side * 850, 2550, 40)], [230, 210, 190],
                            cap_start=True), CONCRETE))
        # Rusted rebar, out of the ends of its arms and its shoulders
        for root, tip in (((side * 850, 2500, 40), (side * 880, 2100, 120)),
                          ((side * 820, 2520, -40), (side * 760, 2150, -110)),
                          ((side * 760, 4700, -60), (side * 900, 5150, -160))):
            parts.append(S(smooth_spike(root, tip, 32, sides=4), REBAR))
    # Krylon on its chest, sprayed and run
    paint = [on_face(torso, [(-460, 4000), (-120, 4250), (-80, 4120), (-420, 3880)], 8),
             on_face(torso, [(150, 3500), (420, 3650), (380, 3550), (170, 3420)], 8)]
    parts.append(F(paint, KRYLON_GREEN))
    parts.append(F([on_face(torso, [(-200, 3000), (-150, 3000), (-140, 3500), (-210, 3500)], 8)], KRYLON_RED))
    parts.append(S(limb([(0, 4850, 0), (0, 5250, 0)], [260, 250], cap_end=False), CONCRETE))
    head = head_surface(5900, 560, 640, 520, power=2.6)
    parts.append(S(head.build(cap_start=True, cap_end=True), CONCRETE))
    # Its face: sprayed on, eyes too big and black, a red mouth, green running down from them
    eyes = [oval_on(head, side * 210, 6050, 140, 100, 8) for side in (-1, 1)]
    parts.append(F(eyes, BLACK))
    streaks = []
    for side in (-1, 1):
        streaks.append(on_face(head, [(side * 260, 5650), (side * 200, 5650), (side * 190, 5960), (side * 270, 5960)], 6))
        streaks.append(on_face(head, [(side * 420, 6300), (side * 120, 6420), (side * 110, 6340), (side * 410, 6230)], 6))
    parts.append(F(streaks, KRYLON_GREEN))
    w = 230
    mouth = band(head, -w, w, lambda x: 5600 + 40 * math.sin(x / 40), lambda x: 5540 + 40 * math.sin(x / 40), 7, steps=8)
    parts.append(F(mouth, KRYLON_RED))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# SCP-096


def shy_body():
    parts = []
    for side in (-1, 1):
        x = side * 200
        parts.append(S(limb([(x, 100, 0), (x * 1.05, 1900, 30), (x, 3700, 0)], [115, 100, 140], cap_start=True),
                       SHY_SKIN))
        parts.append(S(smooth_sphere(x, 60, 90, 110, 60, 230, n=8, stacks=4), SHY_SKIN))
    torso = RingSurface([
        (3550, 380, 220, 0, 2.2),
        (4200, 320, 180, 0, 2.2),  # starved thin at the waist
        (5000, 420, 225, 0, 2.2),
        (5800, 560, 250, 0, 2.4),
        (6200, 620, 240, 0, 2.6),
        (6450, 300, 200, 0, 2.2),
        (SHY_NECK_Y + 60, 120, 120, 0, 2.0),
    ], 12)
    parts.append(S(torso.build(cap_start=True, cap_end=True), SHY_SKIN))
    ribs = []
    for k in range(5):
        y = 4800 + 180 * k
        for side in (-1, 1):
            ribs.append(on_face(torso, [(side * 60, y), (side * 330, y - 70), (side * 330, y - 40), (side * 60, y + 30)],
                                6))
    parts.append(F(ribs, SHY_SHADOW))
    return parts


def shy_arms(up):
    parts = []
    for side in (-1, 1):
        if up:
            # Up over its face: elbows out in front, palms flat against where its face is
            pts = [(side * 620, 6200, 0), (side * 540, 6000, 650), (side * 170, 7100, 470)]
            fingers = [((side * (170 - 30 * k), 7150, 470), (side * (130 - 40 * k), 7700, 430)) for k in range(4)]
        else:
            pts = [(side * 640, 6200, 0), (side * 700, 4200, 40), (side * 720, 1800, 80)]
            fingers = [((side * (720 + 25 * (k - 1.5)), 1720, 80 + 20 * k), (side * (740 + 35 * (k - 1.5)), 1050, 110 + 30 * k))
                       for k in range(4)]
        parts.append(S(bent_limb(pts, [100, 80, 66]), SHY_SKIN))
        for root, tip in fingers:
            parts.append(S(smooth_spike(root, tip, 26, sides=4), SHY_SKIN))
    return parts


def shy_head(scream):
    """Its head, the pivot at the neck: calm, or screaming with the jaw hanging open to its chest."""
    parts = []
    head = head_surface(560 if scream else 520, 270, 380 if scream else 420, 300, cz=40)
    parts.append(S(head.build(cap_start=True, cap_end=True), SHY_SKIN))
    sockets, eyes = [], []
    for side in (-1, 1):
        sockets.append(oval_on(head, side * 105, 660, 75, 62 if scream else 52, 5))
        eyes.append(oval_on(head, side * 105, 660, 26, 26, 9, n=8))
    parts.append(F(sockets, SHY_SOCKET))
    parts.append(F(eyes, (236, 236, 230)))  # no colour in them at all
    if scream:
        jaw = head_surface(-120, 220, 560, 230, cz=70)
        parts.append(S(jaw.build(cap_start=True, cap_end=True), SHY_SKIN))
        parts.append(F([oval_on(jaw, 0, -60, 165, 470, 8, n=14)], BLACK))
    else:
        parts.append(F(band(head, -110, 110, lambda x: 290, lambda x: 262, 6, steps=4), SHY_SOCKET))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# SCP-106


def old_man():
    parts = []
    for side in (-1, 1):
        x = side * 220
        parts.append(S(bent_limb([(x, 90, 40), (x * 1.05, 1700, 220), (x * 0.95, 3200, 0)], [130, 115, 160]), OLD_SKIN))
        parts.append(S(smooth_sphere(x, 50, 130, 120, 60, 260, n=8, stacks=4), OLD_SKIN))
    # Bent over: each ring further forward than the one below
    torso = RingSurface([
        (3000, 400, 260, 0, 2.2),
        (3700, 380, 250, 80, 2.2),
        (4300, 460, 270, 220, 2.2),
        (4900, 560, 290, 400, 2.4),
        (5200, 500, 280, 480, 2.2),
        (5420, 190, 180, 560, 2.0),
    ], 12)
    parts.append(S(torso.build(cap_start=True, cap_end=True), OLD_SKIN))
    rot = [on_face(torso, [(-300, 3800), (-60, 4000), (-120, 4300), (-340, 4100)], 6),
           on_face(torso, [(80, 4500), (330, 4600), (280, 4850), (60, 4750)], 6)]
    parts.append(F(rot, OLD_ROT))
    drips = [on_face(torso, [(x - 25, 3300), (x + 25, 3300), (x + 30, y0), (x - 30, y0)], 9)
             for x, y0 in ((-180, 4600), (40, 4200), (220, 4900), (-40, 4900))]
    parts.append(F(drips, SLIME))
    for side in (-1, 1):
        parts.append(S(bent_limb([(side * 540, 5000, 430), (side * 640, 3800, 620), (side * 610, 2700, 640)],
                                 [115, 95, 80]), OLD_SKIN))
        for k in range(4):
            root = (side * (610 + 25 * (k - 1.5)), 2650, 640 + 25 * k)
            tip = (side * (630 + 45 * (k - 1.5)), 2050, 700 + 40 * k)
            parts.append(S(smooth_spike(root, tip, 24, sides=4, bend=(0, 0, 40)), OLD_SKIN))
    parts.append(S(limb([(0, 5300, 540), (0, 5550, 640)], [170, 160], cap_end=False), OLD_SKIN))
    head = head_surface(5850, 290, 360, 300, cz=700)
    parts.append(S(head.build(cap_start=True, cap_end=True), OLD_SKIN))
    sockets = [oval_on(head, side * 110, 5950, 80, 60, 5) for side in (-1, 1)]
    parts.append(F(sockets, SLIME))
    glints = [oval_on(head, side * 110, 5950, 18, 18, 9, n=6) for side in (-1, 1)]
    parts.append(F(glints, OLD_EYE))
    mouth = band(head, -170, 170, lambda x: 5700 - 40 * (x / 170) ** 2, lambda x: 5600 + 30 * (x / 170) ** 2, 6, steps=8)
    parts.append(F(mouth, SLIME))
    wrinkles = [on_face(head, [(-200, y), (200, y + 10), (200, y + 30), (-200, y + 20)], 4) for y in (6080, 6150)]
    parts.append(F(wrinkles, (24, 20, 16)))
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

    scp_out = Path(sys.argv[2]) if len(sys.argv) > 2 else SCP_OUT
    s173 = Mesh("sScp173Vtx")
    s173_dl = gfx("sScp173DL", emit_parts(s173, sculpture()))
    s096 = Mesh("sScp096Vtx")
    s096_body_dl = gfx("sScp096BodyDL", emit_parts(s096, shy_body()))
    s096_down_dl = gfx("sScp096ArmsDownDL", emit_parts(s096, shy_arms(False)))
    s096_up_dl = gfx("sScp096ArmsUpDL", emit_parts(s096, shy_arms(True)))
    s096_head_dl = gfx("sScp096HeadDL", emit_parts(s096, shy_head(False)))
    s096_scream_dl = gfx("sScp096ScreamDL", emit_parts(s096, shy_head(True)))
    s106 = Mesh("sScp106Vtx")
    s106_dl = gfx("sScp106DL", emit_parts(s106, old_man()))
    pool_faces, pool_colour = corrosion()
    pool_dl = gfx("sScp106PoolDL", s106.coloured_dl(pool_faces, pool_colour))
    setup = "\n".join([
        setup_dl("sScpSetupDL", LIT_OPA, "Their bodies: opaque, lit, untextured"),
        setup_dl("sScpGroundSetupDL", GLOW,
                 "Corrosion on the ground: unlit, colour and alpha = vertex colour x primitive colour, double sided"),
    ])
    text = f"""// Generated by tools/ben/gen_ben_visitors.py; edit that script instead of this file.
// SCP-173, SCP-096 and SCP-106, after the SCP wiki (CC BY-SA 3.0, docs/ben/CREDITS.md). +Y up, +Z forward, +X to
// its left; 1/100 of a world unit.
// clang-format off

#define SCP_096_NECK_Y ({SHY_NECK_Y}.0f)

{s173.vtx_array()}
{s096.vtx_array()}
{s106.vtx_array()}
{setup}
{s173_dl}
{s096_body_dl}
{s096_down_dl}
{s096_up_dl}
{s096_head_dl}
{s096_scream_dl}
{s106_dl}
{pool_dl}
// clang-format on
"""
    scp_out.parent.mkdir(parents=True, exist_ok=True)
    scp_out.write_text(text)
    print(f"wrote {scp_out} ({len(s173.vtx)} 173, {len(s096.vtx)} 096, {len(s106.vtx)} 106 vertices)")


if __name__ == "__main__":
    main()
