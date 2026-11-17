#!/usr/bin/env python3
"""
Generates the Expanded Horror visitors' models as N64 display lists (F3DEX2), each written to its own actor's folder
(src/overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor_models.inc.c for the Tall Man, ovl_En_Ben_Smile/
z_en_ben_smile_models.inc.c, ovl_En_Ben_Sleep/z_en_ben_sleep_models.inc.c), so only the one that's here is in memory.
Built on the model kit copied from the Majora's Mask: WTF project (tools/ben/modelkit.py).

Usage: tools/ben/gen_ben_visitors.py [Tall Man, Smiling Man and "Go to sleep" output files
                                      [SCP-173, SCP-096 and SCP-106 output files]]

- The Tall Man: twice Link's height, a black suit, a white shirt and a black tie, arms that hang past his knees with
  long pale fingers, and a pale head with no face at all. His tendrils are a display list of their own, drawn only
  when he's close to having you.
- The Smiling Man: far too thin and long in every limb, in a dark suit with a red tie, one knee up and his arms flung
  out as if he's dancing, the hands hanging limp from the wrists on fingers like a spider's legs. A gaunt head, the
  hair slicked flat, a long sharp nose and bloodshot eyes bulging out of sunken sockets with no lids to speak of. His
  mouth is a display list of its own: the grin (ear to ear, gums and two rows of small pointed teeth) or the frown he
  wears when he gives up on you (the corners dragged down to his chin, the same staring eyes).
- The one who says "Go to sleep": thin and hunched in a white hoodie soaked and sprayed with blood, black jeans, a
  kitchen knife point down in his fist, blood on the blade. His face is drawn unlit and unfogged (a display list of
  its own, vertex coloured, the shading painted on), so when the lights go out it's the only thing you can see:
  bleached leathery skin burned in patches, no eyelids (the rims charred black and raw red), bloodshot eyes staring,
  blood run down from them, a grin carved up into his cheeks with his teeth showing through it. His hair is lit,
  matted, hanging over his face. His head is cocked.

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

ACTORS = Path(__file__).resolve().parents[2] / "src/overlays/actors"
OUTS = [ACTORS / "ovl_En_Ben_Visitor/z_en_ben_visitor_models.inc.c", ACTORS / "ovl_En_Ben_Smile/z_en_ben_smile_models.inc.c",
        ACTORS / "ovl_En_Ben_Sleep/z_en_ben_sleep_models.inc.c"]
SCP_OUTS = [ACTORS / f"ovl_En_Ben_Scp{n}/z_en_ben_scp{n}_models.inc.c" for n in ("173", "096", "106")]

BLACK = (10, 10, 12)
SUIT = (26, 26, 30)
SHIRT = (228, 228, 224)
PALE = (232, 230, 228)
HAIR = (14, 12, 12)


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
# Pieces the Smiling Man and "Go to sleep" share


def patch(surface, x0, x1, y0, y1, lift, nx=4, ny=4):
    """A rectangle laid on the front of `surface` as a grid, so it follows the curve both ways."""
    faces = []
    for i in range(nx):
        a, b = x0 + (x1 - x0) * i / nx, x0 + (x1 - x0) * (i + 1) / nx
        for j in range(ny):
            c, d = y0 + (y1 - y0) * j / ny, y0 + (y1 - y0) * (j + 1) / ny
            faces.append(on_face(surface, [(a, c), (b, c), (b, d), (a, d)], lift))
    return faces


def smooth_patch(surface, x0, x1, y0, y1, lift, nx=4, ny=4):
    """A patch like `patch`, but one smooth piece: its points shared, so it's shaded as the surface under it is."""
    m = Smooth()
    idx = [[m.add((x0 + (x1 - x0) * i / nx, y0 + (y1 - y0) * j / ny,
                   surface.front_z(x0 + (x1 - x0) * i / nx, y0 + (y1 - y0) * j / ny, lift)))
            for j in range(ny + 1)] for i in range(nx + 1)]
    for i in range(nx):
        for j in range(ny):
            m.quad(idx[i][j], idx[i + 1][j], idx[i + 1][j + 1], idx[i][j + 1])
    if m.normals()[idx[0][0]][2] < 0:
        m.tris = [(a, c, b) for a, b, c in m.tris]
    return m


def vee(surface, y0, y1, w1, lift, bands=6):
    """A V down the front of `surface`: nothing at y0, w1 either side of the middle at y1."""
    faces = []
    for i in range(bands):
        ya, yb = y0 + (y1 - y0) * i / bands, y0 + (y1 - y0) * (i + 1) / bands
        wa, wb = w1 * i / bands, w1 * (i + 1) / bands
        for s in (-1, 1):
            faces.append(on_face(surface, [(0, ya), (s * wa, ya), (s * wb, yb), (0, yb)], lift))
    return faces


def ball(r, n=10, k=6):
    """A sphere as a RingSurface about (0, 0, 0), so things can be laid on its front (an eye's pupil)."""
    rows = []
    for i in range(k + 1):
        t = -math.pi / 2 + math.pi * i / k
        s = max(math.cos(t), 0.05)
        rows.append((r * math.sin(t), r * s, r * s, 0.0, 2.0))
    return RingSurface(rows, n)


def eye(r, pupil, iris, iris_colour, white, vein, seed, look=(0.0, 0.0), n=8, k=5):
    """A bulging eyeball at the origin, looking out along +Z: no lids, red veins running in from its edge to a small
    iris and a pinprick of a pupil (`look` moves them off centre)."""
    b = ball(r, n, k)
    parts = [S(b.build(cap_start=True, cap_end=True), white)]
    veins = []
    for v in range(4):
        a = math.tau * (v + 0.3 * noise(v * 31.0, seed, 0.0, seed)) / 4
        pts = []
        for j in range(3):
            f = 0.92 - 0.22 * j
            w = 0.18 * noise(j * 41.0, v * 13.0, seed, seed)
            pts.append((r * f * math.cos(a + w), r * f * math.sin(a + w)))
        veins += stroke(b, pts, r * 0.07, 1.0, taper=True, seg=r * 0.4)
    parts.append(F(veins, vein))
    if iris:
        parts.append(F(disc_on(b, look[0], look[1], iris, iris, 1.5, n=8, rings=1), iris_colour))
    parts.append(F(disc_on(b, look[0], look[1], pupil, pupil, 2.5, n=6, rings=1), BLACK))
    return parts


def surface_quads(surface):
    """Every quad of a RingSurface, facing out: for a vertex coloured display list."""
    quads = []
    for k in range(len(surface.rings) - 1):
        r0 = surface.ring_points(surface.rings[k])
        r1 = surface.ring_points(surface.rings[k + 1])
        for i in range(len(r0)):
            j = (i + 1) % len(r0)
            quads.append(facing_out([r0[i], r0[j], r1[j], r1[i]], surface))
    return quads


def moved_faces(faces, fn):
    return [[tuple(fn(p)) for p in f] for f in faces]


def spider_hand(side, wrist, down, curl, skin, nail, palm=300, lengths=(430, 500, 470, 380), width=30, spread=0.0):
    """A long hand hanging from `wrist` the way `down` points, fingers knuckled and too long, bending towards `curl`
    and fanning out by `spread`."""
    parts = []
    palm_end = tuple(wrist[i] + down[i] * palm for i in range(3))
    parts.append(S(bent_limb([wrist, tuple(wrist[i] + down[i] * palm * 0.5 for i in range(3)), palm_end],
                             [width * 1.6, width * 2.2, width * 1.8], n=6), skin))
    across = norm((-down[1], down[0], 0.0)) if abs(down[2]) < 0.7 else (1.0, 0.0, 0.0)
    for k in range(4):
        off = (k - 1.5) * width * 1.1
        fan = (k - 1.5) * spread
        base = tuple(palm_end[i] + across[i] * off for i in range(3))
        length = lengths[k]
        j1 = tuple(base[i] + down[i] * length * 0.42 + across[i] * fan * 0.4 + curl[i] * length * 0.06
                   for i in range(3))
        j2 = tuple(j1[i] + down[i] * length * 0.33 + across[i] * fan * 0.35 + curl[i] * length * 0.14
                   for i in range(3))
        tip = tuple(j2[i] + down[i] * length * 0.25 + across[i] * fan * 0.25 + curl[i] * length * 0.2
                    for i in range(3))
        parts.append(S(finger([base, j1, j2, tip], [width * 0.8, width * 0.7, width * 0.6, width * 0.4],
                              knuckle=True), skin))
        parts.append(S(smooth_spike(tuple(j2[i] + (tip[i] - j2[i]) * 0.55 for i in range(3)),
                                    tuple(tip[i] + down[i] * width * 1.3 + curl[i] * width for i in range(3)),
                                    width * 0.42, sides=4), nail))
    thumb_base = tuple(wrist[i] + down[i] * palm * 0.4 - across[i] * width * 2.2 for i in range(3))
    thumb_tip = tuple(thumb_base[i] + down[i] * lengths[0] * 0.6 - across[i] * width * 1.5 + curl[i] * width * 2
                      for i in range(3))
    parts.append(S(finger([thumb_base, tuple((thumb_base[i] + thumb_tip[i]) / 2 for i in range(3)), thumb_tip],
                          [width * 0.85, width * 0.7, width * 0.45], knuckle=True), skin))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# The Smiling Man

SMILE_SUIT = (40, 30, 44)
SMILE_LAPEL = (22, 16, 26)
SMILE_SHIRT = (196, 190, 176)  # gone grey
SMILE_TIE = (100, 12, 22)
SMILE_SKIN = (228, 214, 194)
SMILE_CREASE = (146, 118, 108)
SMILE_SOCKET = (104, 76, 80)
SMILE_LIP = (150, 66, 72)
SMILE_GUM = (172, 54, 68)
SMILE_MOUTH = (18, 4, 6)
SMILE_TEETH = (232, 222, 186)
SMILE_NAIL = (170, 150, 128)
EYE_WHITE = (240, 236, 226)
EYE_VEIN = (180, 30, 34)


def smile_body():
    """Too thin and too long in every limb, in a dark suit that fits him like a glove: mid-step in his dance, one knee
    up to his chest, arms flung out and up with the hands hanging limp from the wrists."""
    parts = []
    # The planted leg, a little bent, and the one kicked up, the knee up at his waist
    planted = [(-200, 3250, 0), (-212, 2450, 30), (-215, 1720, 90), (-205, 960, 30), (-190, 210, -10)]
    raised = [(230, 3250, 0), (240, 3080, 450), (250, 2880, 880), (255, 2280, 830), (262, 1700, 720)]
    for pts in (planted, raised):
        parts.append(S(bent_limb(pts, [150, 112, 92, 80, 66], n=8), SMILE_SUIT))
        parts.append(S(smooth_sphere(*pts[2], 98, 98, 98, n=6, stacks=4), SMILE_SUIT))  # a knee that pokes out
    # Long shoes, pointed, polished
    for shoe in ([(-190, 120, -150), (-192, 100, 160), (-188, 60, 560)],
                 [(262, 1660, 570), (264, 1560, 860), (268, 1380, 1180)]):
        parts.append(S(bent_limb(shoe, [84, 80, 22], n=6), BLACK))

    torso = RingSurface(resampled([
        (3150, 300, 190, 0, 2.2),
        (3600, 245, 160, 10, 2.2),  # the waist, pinched
        (4300, 330, 200, 20, 2.3),
        (4900, 430, 220, 20, 2.5),
        (5180, 520, 228, 10, 2.8),  # shoulders, square
        (5340, 330, 190, 0, 2.2),
        (5480, 110, 110, 0, 2.0),
    ], 320), 12, bump=bumpy(5, 4.0, 1.5))
    parts.append(S(torso.build(cap_start=True, cap_end=True), SMILE_SUIT))
    for side in (-1, 1):
        parts.append(S(smooth_sphere(side * 120, 3000, -150, 150, 420, 60, n=6, stacks=4), SMILE_SUIT))  # tails
        parts.append(S(smooth_sphere(side * 470, 5170, 0, 130, 90, 150, n=8, stacks=4), SMILE_SUIT))  # padding

    # The shirt in a V, the lapels either side, a thin red tie, the jacket's buttons and where it pulls
    parts.append(F(vee(torso, 4250, 5300, 175, 6, bands=7), SMILE_SHIRT))
    lapels = []
    for side in (-1, 1):
        lapels += stroke(torso, [(side * (175 * (y - 4250) / 1050 + 45), y) for y in (4250, 4600, 4950, 5300)], 90, 9)
    parts.append(F(lapels, SMILE_LAPEL))
    tie = stroke(torso, [(0, 4300), (0, 5150)], 56, 12, seg=150) + disc_on(torso, 0, 5200, 40, 42, 14, n=8, rings=1)
    parts.append(F(tie, SMILE_TIE))
    buttons = disc_on(torso, 0, 4100, 20, 20, 8, n=6, rings=1) + disc_on(torso, 0, 3820, 20, 20, 8, n=6, rings=1)
    folds = []
    for side in (-1, 1):
        folds += stroke(torso, [(side * 40, 3950), (side * 180, 3700), (side * 250, 3500)], 16, 5, taper=True)
        folds += stroke(torso, [(side * 60, 4400), (side * 240, 4150)], 14, 5, taper=True)
    parts.append(F(buttons + folds, SMILE_LAPEL))

    # Arms flung out and up, the elbows sharp, the hands hanging from the wrists on fingers like a spider's legs
    for side in (-1, 1):
        shoulder = (side * 520, 5150, 0)
        elbow = (side * 1250, 5350, 160)
        wrist = (side * 1560, 6250, 320)
        parts.append(S(bent_limb([shoulder, tuple((shoulder[i] + elbow[i]) / 2 for i in range(3)), elbow,
                                  tuple((elbow[i] + wrist[i]) / 2 for i in range(3)), wrist], [100, 80, 74, 62, 54],
                                 n=7), SMILE_SUIT))
        parts.append(S(smooth_sphere(*elbow, 80, 80, 80, n=6, stacks=4), SMILE_SUIT))
        cuff = tuple(wrist[i] + (wrist[i] - elbow[i]) * 0.06 for i in range(3))
        parts.append(S(bent_limb([wrist, cuff], [60, 58], n=6), SMILE_SHIRT))
        parts += spider_hand(side, cuff, norm((side * 0.25, -1.0, 0.2)), norm((side * -0.5, 0.0, 0.6)), SMILE_SKIN,
                             SMILE_NAIL, palm=260, lengths=(420, 500, 470, 370), width=26, spread=60)
    # A long neck, its Adam's apple and its cords standing out, in a starched collar
    parts.append(S(limb([(0, 5380, 0), (0, SMILE_NECK_Y + 120, 25)], [84, 70], cap_end=False), SMILE_SKIN))
    parts.append(S(smooth_sphere(0, 5560, 70, 30, 40, 30, n=6, stacks=4), SMILE_SKIN))
    for side in (-1, 1):
        parts.append(S(bent_limb([(side * 45, 5420, 40), (side * 30, 5620, 50)], [16, 12], n=4), SMILE_SKIN))
    parts.append(S(limb([(0, 5360, 0), (0, 5470, 5)], [108, 98], cap_start=True), SMILE_SHIRT))
    return parts


def smile_head_surface():
    return RingSurface(resampled([
        (40, 40, 40, 70, 2.0),  # a pointed chin
        (110, 115, 120, 75, 2.0),
        (220, 185, 200, 60, 2.0),
        (360, 228, 245, 40, 2.0),
        (480, 252, 262, 30, 2.0),
        (600, 252, 270, 20, 2.0),
        (720, 246, 268, 10, 2.0),
        (850, 218, 240, 0, 2.0),
        (960, 150, 170, -10, 2.0),
        (1030, 30, 40, -20, 2.0),
    ], 90), 12, bump=bumpy(4, 7.0, 2.0))


def smile_head():
    """His head, its pivot at his neck: long and gaunt, the hair slicked flat with a widow's peak, cheekbones like
    knuckles, a long sharp nose and eyes that bulge out of his skull with no lids to speak of. What his mouth does is
    smile_mouth's."""
    head = smile_head_surface()
    parts = [S(limb([(0, -80, -20), (0, 260, -30)], [82, 90], cap_start=True, cap_end=False), SMILE_SKIN),
             S(head.build(cap_start=True, cap_end=True), SMILE_SKIN)]
    parts.append(S(smooth_sphere(0, 730, -50, 246, 320, 290, n=12, stacks=6), HAIR))
    parts.append(F([on_face(head, [(-110, 900), (0, 780), (110, 900), (0, 940)], 4)], HAIR))  # a widow's peak
    for side in (-1, 1):
        parts.append(S(smooth_sphere(side * 258, 600, 10, 30, 82, 55, n=6, stacks=4), SMILE_SKIN))  # ears
    # Sunken sockets with the eyes standing out of them, dark under them, the cheeks hollow under the bone
    shadow = []
    for side in (-1, 1):
        shadow += ring_on(head, side * 105, 615, 58, 92, 3, n=10)
        shadow += stroke(head, [(side * 60, 530), (side * 110, 505), (side * 170, 530)], 22, 4)
    parts.append(F(shadow, SMILE_SOCKET))
    hollows = []
    for side in (-1, 1):
        hollows += stroke(head, [(side * 185, 450), (side * 180, 360), (side * 160, 270)], 40, 2, taper=True)
    parts.append(F(hollows, SMILE_CREASE))
    for side, look in ((-1, (-5.0, 2.0)), (1, (9.0, -3.0))):  # not quite looking the same way
        z = head.front_z(side * 105, 615) - 22
        parts += move(eye(68, 8, 17, (136, 130, 92), EYE_WHITE, EYE_VEIN, side + 2.0, look),
                      shift(side * 105, 615, z))
    # The nose: long, thin, sharp
    parts.append(S(smooth_spike((0, 600, head.front_z(0, 600) - 25), (0, 445, head.front_z(0, 445) + 115), 36,
                                sides=5, bend=(0, -10, 20)), SMILE_SKIN))
    parts.append(F(disc_on(head, -26, 452, 13, 8, 4, n=6, rings=1) + disc_on(head, 26, 452, 13, 8, 4, n=6, rings=1),
                   SMILE_MOUTH))
    return parts


def smile_mouth(grin):
    """His mouth and his brows, laid on his head: the grin (ear to ear, the corners up into his cheeks, the gums
    showing over two rows of small pointed teeth, brows high in delight), or with `grin` false the frown he gives you
    when you get away (the corners dragged down to his chin, the brows knotted, the same staring eyes)."""
    head = smile_head_surface()
    parts = []
    if grin:
        w = 240

        def gap(x):
            return 125 * max(0.0, 1 - (min(abs(x), w) / w) ** 2)

        def top(x):
            return 335 + 150 * (min(abs(x), w) / w) ** 2

        def bottom(x):
            return top(x) - 6 - gap(x)

        parts.append(F(band(head, -w - 10, w + 10, lambda x: top(x) + 10, lambda x: bottom(x) - 10, 3, steps=16),
                       SMILE_LIP))
        parts.append(F(band(head, -w, w, top, bottom, 5, steps=16), SMILE_MOUTH))
        g = 0.88 * w
        parts.append(F(band(head, -g, g, lambda x: top(x) - 2, lambda x: top(x) - 2 - 0.3 * gap(x), 7, steps=14)
                       + band(head, -g, g, lambda x: bottom(x) + 2 + 0.22 * gap(x), lambda x: bottom(x) + 2, 7,
                              steps=14), SMILE_GUM))
        teeth = Smooth()
        for row, count in ((1, 17), (-1, 15)):
            for k in range(count):
                x = -0.84 * w + 1.68 * w * k / (count - 1)
                y = top(x) - 0.28 * gap(x) if row > 0 else bottom(x) + 0.2 * gap(x)
                z = head.front_z(x, y, 4)
                length = (0.36 + 0.1 * abs(math.sin(k * 2.7 + row))) * gap(x) + 8
                teeth.extend(smooth_spike((x, y, z - 10), (x * 1.01, y - row * length, z + 6), 14, sides=3))
        parts.append(S(teeth, SMILE_TEETH))
        creases = []
        for s in (-1, 1):
            creases += stroke(head, [(s * 50, 500), (s * 140, 476), (s * 205, 505)], 16, 4)
            creases += stroke(head, [(s * (w - 12), top(w) + 10), (s * (w - 4), top(w) + 62)], 12, 4, taper=True)
            creases += stroke(head, [(s * 60, 236), (s * 140, 205), (s * 210, 250)], 12, 4, taper=True)
        for y in (830, 875):
            creases += stroke(head, [(-150, y), (-60, y + 14), (60, y + 6), (150, y + 18)], 9, 3, seg=60)
        parts.append(F(creases, SMILE_CREASE))
        brows = []
        for s in (-1, 1):
            brows += stroke(head, [(s * 40, 735), (s * 95, 790), (s * 160, 800), (s * 220, 760)], 22, 5, taper=True)
        parts.append(F(brows, HAIR))
    else:
        w = 170

        def top(x):
            return 320 - 115 * (min(abs(x), w) / w) ** 2

        def bottom(x):
            return top(x) - 16

        parts.append(F(band(head, -w - 10, w + 10, lambda x: top(x) + 12, lambda x: bottom(x) - 12, 3, steps=12),
                       SMILE_LIP))
        parts.append(F(band(head, -w, w, top, bottom, 5, steps=12), SMILE_MOUTH))
        creases = []
        for s in (-1, 1):
            creases += stroke(head, [(s * 55, 490), (s * 140, 420), (s * (w + 14), 230)], 16, 4)
            creases += stroke(head, [(s * w, 195), (s * w * 0.88, 125), (s * w * 0.66, 70)], 16, 4, taper=True)
            creases += stroke(head, [(s * 16, 700), (s * 10, 790)], 9, 4)
        creases += stroke(head, [(-40, 120), (0, 100), (40, 120)], 12, 4)  # the chin puckered
        parts.append(F(creases, SMILE_CREASE))
        brows = []
        for s in (-1, 1):
            brows += stroke(head, [(s * 40, 690), (s * 110, 735), (s * 215, 765)], 26, 5, taper=True)
        parts.append(F(brows, HAIR))
    return parts


# ---------------------------------------------------------------------------------------------------------------------
# Go to sleep

HOODIE = (214, 212, 206)
HOODIE_SHADE = (166, 162, 156)  # its folds, its pocket, its hem
BLOOD = (118, 6, 12)
BLOOD_DRY = (64, 8, 8)
JEANS = (24, 26, 34)
JEANS_WORN = (46, 52, 68)
SOLE = (196, 194, 186)
JEFF_SKIN = (232, 228, 222)  # his hands are as white as his face
KNUCKLE = (178, 112, 106)
BLADE = (176, 178, 186)
HANDLE = (30, 24, 22)
SLEEP_HEAD_TURN = (7.0, 9.0, 70.0)  # his head cocked this far to the side, forward, and pushed this far out


def blood_on(surface, spots, seed):
    """Splashes of blood on the front of `surface`: (x, y, size) each a ragged blot, the bigger ones running."""
    wet, dry = [], []
    for k, (x, y, size) in enumerate(spots):
        if size > 45:  # the big ones have dried dark round their edges, and run
            dry += disc_on(surface, x, y, size * 1.25, size * 1.1, 3, n=8, rings=1, wobble=0.5, seed=seed + k)
            wet += stroke(surface, drip(x, y, size * 4, size * 0.2, seed + k), size * 0.35, 5, taper=True, seg=160)
        wet += disc_on(surface, x, y, size, size * 0.85, 5, n=7 if size > 30 else 5, rings=1, wobble=0.45,
                       seed=seed + k + 0.5)
    return [F(dry, BLOOD_DRY), F(wet, BLOOD)]


def spray_on(surface, x0, y0, x1, y1, count, size, seed):
    """A spatter flung across `surface` from (x0, y0) to (x1, y1), drops getting smaller."""
    out = []
    for k in range(count):
        t = k / max(1, count - 1)
        x = x0 + (x1 - x0) * t + 40 * noise(k * 17.0, seed, 0.0, seed)
        y = y0 + (y1 - y0) * t + 40 * noise(seed, k * 23.0, 0.0, seed)
        out.append((x, y, size * (1 - 0.6 * t) * (0.7 + 0.3 * abs(noise(k * 7.0, 0.0, seed, seed)))))
    return out


def sleep_leg(side):
    """A leg in black jeans, worn pale at the knee, built about x = 0 and moved out to its side."""
    leg = RingSurface(resampled([
        (180, 105, 105, 0, 2.0),
        (700, 115, 112, 10, 2.0),
        (1250, 120, 118, 30, 2.0),
        (1350, 126, 128, 40, 2.0),  # the knee
        (1900, 140, 135, 15, 2.0),
        (2400, 160, 150, 0, 2.0),
        (2650, 178, 160, 0, 2.0),
    ], 340), 8, bump=bumpy(8, 3.0 + side, 2.0))
    parts = [S(leg.build(cap_start=True), JEANS)]
    parts.append(F(disc_on(leg, 0, 1330, 70, 95, 4, n=8, rings=1, wobble=0.3, seed=side + 5.0), JEANS_WORN))
    if side < 0:
        parts += blood_on(leg, [(30, 1900, 40), (-40, 1600, 22), (10, 1200, 16)], 31.0)
    # Black sneakers, their soles white
    parts.append(S(bent_limb([(0, 100, -110), (0, 105, 90), (0, 90, 270)], [100, 98, 78], n=8), BLACK))
    parts.append(S(smooth_sphere(0, 35, 70, 118, 36, 250, n=8, stacks=4), SOLE))
    return move(parts, shift(side * 180, 0, 0))


def sleep_arm(side):
    """A sleeve hanging straight, built about x = 0 and moved out: rows from the cuff up to the shoulder."""
    arm = RingSurface(resampled([
        (2280, 100, 100, 95, 2.0),
        (2300, 118, 118, 92, 2.0),  # the cuff
        (2420, 118, 118, 88, 2.0),
        (2440, 128, 128, 86, 2.0),
        (2900, 132, 132, 60, 2.0),
        (3400, 138, 138, 30, 2.0),
        (3800, 148, 148, 10, 2.0),
        (4050, 150, 150, 0, 2.0),
    ], 320), 8, bump=bumpy(12, 6.0 + side, 1.4))
    parts = [S(arm.build(cap_start=True, cap_end=True), HOODIE)]
    parts.append(F(patch(arm, -110, 110, 2300, 2420, 4, nx=4, ny=1), HOODIE_SHADE))
    if side < 0:  # the knife arm, soaked to the elbow
        parts += blood_on(arm, [(0, 2370, 70), (-30, 2650, 48), (40, 2900, 30), (-20, 3200, 18)], 51.0)
    else:
        parts += blood_on(arm, spray_on(arm, -40, 3300, 60, 2700, 4, 22, 61.0), 61.0)
    return move(parts, shift(side * 520, 0, 0))


def sleep_body():
    """Thin, a little hunched, in a white hoodie that isn't white any more: blood up his right sleeve and sprayed over
    his chest, its pocket and drawstrings, the hood bunched at his neck. Black jeans, black sneakers. In his right hand
    a kitchen knife, point down, blood on the blade and dripping off it; the left hand hangs open."""
    parts = sleep_leg(-1) + sleep_leg(1)
    torso = RingSurface(resampled([
        (2450, 440, 270, 0, 2.2),
        (2560, 470, 285, 0, 2.2),  # the hem
        (2800, 460, 275, 5, 2.2),
        (3300, 455, 265, 15, 2.3),
        (3700, 500, 270, 35, 2.4),
        (4000, 560, 280, 60, 2.6),  # shoulders
        (4200, 420, 250, 80, 2.2),
        (4320, 150, 150, 90, 2.0),
    ], 280), 14, bump=bumpy(13, 9.0, 1.2))
    parts.append(S(torso.build(cap_start=True, cap_end=True), HOODIE))
    hem = RingSurface([(2420, 458, 288, 0, 2.2), (2575, 488, 303, 0, 2.2)], 14)
    parts.append(S(hem.build(), HOODIE_SHADE))
    # The pocket, its openings, the drawstrings
    parts.append(S(smooth_patch(torso, -230, 230, 2680, 3150, 5, nx=6, ny=3), HOODIE_SHADE))
    seams = []
    for s in (-1, 1):
        seams += stroke(torso, [(s * 230, 3150), (s * 175, 2700)], 18, 8)
    parts.append(F(seams, (112, 108, 104)))
    for s in (-1, 1):
        top = (s * 60, 4180, torso.front_z(s * 60, 4180) + 10)
        end = (s * 75, 3700, torso.front_z(s * 75, 3700) + 18)
        parts.append(S(bent_limb([top, ((top[0] + end[0]) / 2, 3940, (top[2] + end[2]) / 2 + 6), end], [12, 11, 14],
                                 n=4), HOODIE_SHADE))
    # Blood: a splash across the chest, a spray up from the belly, drips
    parts += blood_on(torso, [(-170, 3700, 90), (-60, 3900, 50), (120, 3550, 38), (-230, 3250, 55),
                              (60, 3050, 28)], 11.0)
    parts += blood_on(torso, spray_on(torso, -120, 3350, 320, 3950, 7, 30, 21.0), 21.0)
    # The hood, bunched up behind his neck, and its opening round it
    parts.append(S(roughen(smooth_sphere(0, 4250, -200, 360, 210, 200, n=12, stacks=6), 30, 5.0), HOODIE))
    parts.append(S(smooth_sphere(0, 4300, 60, 230, 80, 200, n=12, stacks=4), HOODIE_SHADE))
    parts += sleep_arm(-1) + sleep_arm(1)

    # The knife hand, a fist round the handle, the knuckles raw
    fist = (-520, 2180, 115)
    parts.append(S(roughen(smooth_sphere(*fist, 82, 110, 92, n=8, stacks=5), 10, 3.0), JEFF_SKIN))
    knuckles = Smooth()
    for k in range(4):
        knuckles.extend(smooth_sphere(-520 + (k - 1.5) * 34, 2235 - abs(k - 1.5) * 10, 195, 22, 24, 22, n=5,
                                      stacks=3))
    parts.append(S(knuckles, KNUCKLE))
    parts.append(S(bent_limb([(-570, 2250, 150), (-560, 2160, 200), (-535, 2120, 205)], [26, 24, 20], n=5),
                   JEFF_SKIN))  # the thumb over the fingers
    parts.append(S(bent_limb([(-520, 2330, 115), (-520, 2050, 118)], [32, 34], n=6), HANDLE))
    parts.append(S(smooth_sphere(-520, 2030, 118, 52, 26, 30, n=6, stacks=3), BLADE))  # the bolster
    # The blade, point down, broad side out: its spine straight, its edge curving in to the point
    spine = [(-490, 2010), (-492, 1750), (-497, 1500), (-510, 1380), (-530, 1290)]
    edge = [(-640, 2010), (-648, 1750), (-635, 1550), (-600, 1400), (-530, 1290)]
    blade, bloody = [], []
    for k in range(len(spine) - 1):
        for zz, d in ((126, 1), (110, -1)):
            quad = [(spine[k][0], spine[k][1], zz), (spine[k + 1][0], spine[k + 1][1], zz),
                    (edge[k + 1][0], edge[k + 1][1], zz), (edge[k][0], edge[k][1], zz)]
            if k == len(spine) - 2:
                quad = quad[:3]
            blade.append(facing(quad, d))
            if k >= 1:
                lift = 3 * d
                f = 0.55 if k == 1 else 1.0  # the blood stops raggedly partway up
                top = [(spine[k][0], spine[k][1] - 60 * (1 - f)), (edge[k][0], edge[k][1] - 140 * (1 - f))]
                quad = [(top[0][0], top[0][1], zz + lift), (spine[k + 1][0], spine[k + 1][1], zz + lift),
                        (edge[k + 1][0], edge[k + 1][1], zz + lift), (top[1][0], top[1][1], zz + lift)]
                if k == len(spine) - 2:
                    quad = quad[:3]
                bloody.append(facing(quad, d))
        blade.append(facing([(spine[k][0], spine[k][1], 110), (spine[k + 1][0], spine[k + 1][1], 110),
                             (spine[k + 1][0], spine[k + 1][1], 126), (spine[k][0], spine[k][1], 126)], 1))
    parts.append(F(blade, BLADE))
    parts.append(F(bloody, BLOOD))
    parts.append(S(smooth_sphere(-531, 1255, 118, 12, 20, 12, n=5, stacks=3), BLOOD))  # a drop about to fall

    # The other hand hangs open, the fingers half curled
    parts += spider_hand(1, (520, 2290, 100), norm((0.03, -1.0, 0.12)), (-0.3, 0.0, 0.95), JEFF_SKIN, KNUCKLE,
                         palm=200, lengths=(240, 270, 255, 210), width=26, spread=10)
    parts.append(S(limb([(0, 4250, 60), (0, SLEEP_NECK_Y + 100, 70)], [112, 102], cap_end=False), JEFF_SKIN))
    return parts


def sleep_pose(p):
    """Where his head's pieces go: cocked to one side, tipped forward, pushed out over his hunch."""
    roll, pitch, out = SLEEP_HEAD_TURN
    return shift(0, 0, out)(turn_x(pitch, (0, 0, 0))(turn_z(roll, (0, 0, 0))(p)))


def sleep_head_surface():
    return RingSurface(resampled([
        (60, 60, 60, 40, 2.0),
        (130, 150, 160, 40, 2.0),
        (240, 215, 240, 30, 2.0),
        (380, 245, 265, 20, 2.0),
        (520, 252, 272, 15, 2.0),
        (650, 245, 265, 5, 2.0),
        (760, 215, 240, -5, 2.0),
        (850, 150, 175, -15, 2.0),
        (900, 40, 50, -20, 2.0),
    ], 70), 14, bump=bumpy(7, 11.0, 2.0))


def sleep_hair():
    """Black hair, lit (so it's lost in the dark): matted, hanging to his shoulders in ropes, strands of it fallen
    over his face."""
    parts = [S(roughen(smooth_sphere(0, 560, -60, 282, 362, 300, n=12, stacks=6), 26, 13.0), HAIR)]
    strands = Smooth()
    for i in range(16):
        a = math.pi * (-0.08 + 1.16 * i / 15)
        x, z = 280 * math.cos(a), -60 - 280 * math.sin(a) * 0.95
        drop = 560 + 260 * abs(noise(i * 29.0, 3.0, 1.0, 5.0)) + 120 * (i % 3)
        strands.extend(smooth_spike((x, 720, z), (x * 1.3, 720 - drop, z * 1.25 - 40), 62 - 3 * (i % 4), sides=5,
                                    bend=(x * 0.12, 0, -50)))
    for x, length, lean in ((-215, 330, -20), (-150, 250, 15), (-90, 300, 25), (-30, 420, 30), (30, 220, -10),
                            (90, 330, -15), (150, 380, 25), (215, 300, 10)):  # matted over his forehead, in his eyes
        strands.extend(smooth_spike((x * 0.9, 900, 80), (x + lean, 880 - length, 300), 62, sides=5,
                                    bend=(0, 20, 60)))
    parts.append(S(strands, HAIR))
    return move(parts, sleep_pose)


def sleep_face():
    """His face, unlit: (colour, faces) parts for a vertex coloured display list (a colour can be a function of the
    point, so the light that isn't there is painted on). Bleached white skin like leather, lumpy, burned in patches;
    no eyelids, the eyes ringed black and raw red where they were burned off, bloodshot and staring, blood run down
    from them like tears; the grin carved up into his cheeks, ragged, the teeth showing through it, dried blood at
    its corners."""
    head = sleep_head_surface()

    def skin(p):
        x, y, z = p
        _, rx, rz, cz, _ = head.params(y)
        side = min(1.0, abs(x) / max(rx, 1.0))
        n = noise(x * 3, y * 3, z * 3, 17.0)
        if y > 770 + 40 * n:  # his scalp, where his hair grows from
            return (14, 12, 12)
        v = 244 - 80 * side ** 2.5 - 40 * (y < 120) + 10 * n  # darker round its edges and under the chin
        return (int(max(0, v)), int(max(0, v - 4 + 5 * n)), int(max(0, v - 8)))

    # Only the front: his hair covers the rest
    out = [(skin, [q for q in surface_quads(head) if sum(p[2] for p in q) / 4 > head.params(q[0][1])[3] - 40])]
    scars = []
    scars += disc_on(head, -150, 700, 70, 50, 2, n=10, rings=1, wobble=0.5, seed=4.0)
    scars += disc_on(head, 175, 330, 60, 80, 2, n=10, rings=1, wobble=0.5, seed=8.0)
    scars += stroke(head, [(-200, 420), (-150, 350), (-160, 260)], 9, 3)
    scars += stroke(head, [(120, 760), (40, 720)], 8, 3)
    out.append(((206, 178, 174), scars))
    out.append(((96, 70, 70), disc_on(head, 205, 610, 32, 44, 3, n=8, rings=1, wobble=0.4, seed=9.0)))

    # The eyes: burned rims, raw inside them, eyeballs standing out, blood run from their corners
    rims, raw, tears = [], [], []
    for s in (-1, 1):
        rims += disc_on(head, s * 100, 520, 96, 86, 4, n=12, rings=1, wobble=0.35, seed=s + 1.0)
        raw += disc_on(head, s * 100, 520, 66, 60, 6, n=12, rings=1, wobble=0.2, seed=s + 2.0)
        tears += stroke(head, drip(s * 100 + s * 18, 460, 150 + 70 * (s > 0), 14, s + 3.0), 15, 7, taper=True,
                        seg=60)
    out += [((12, 4, 4), rims), ((112, 12, 16), raw), ((122, 0, 6), tears)]
    for s, look in ((-1, (4.0, -3.0)), (1, (-6.0, 2.0))):
        z = head.front_z(s * 100, 520) - 20
        for kind, colour, geo in eye(54, 9, 21, (64, 72, 66), (238, 234, 226), (186, 24, 30), s + 6.0, look, k=4):
            if kind == "smooth":  # the eyeball as quads, only the half of it that stands out of his head
                geo = [q for q in surface_quads(ball(54, 8, 4)) if sum(p[2] for p in q) > -40]
            out.append((colour, moved_faces(geo, shift(s * 100, 520, z))))

    # What's left of his nose
    out.append(((30, 10, 10), disc_on(head, -24, 400, 16, 12, 4, n=6, rings=1, wobble=0.3, seed=1.0)
                + disc_on(head, 24, 400, 16, 12, 4, n=6, rings=1, wobble=0.3, seed=2.0)))

    # The grin, carved: a dark slit up into the cheeks, its edges raw, his teeth behind it
    w = 225

    def top(x):
        return 300 + 160 * (min(abs(x), w) / w) ** 3

    def gap(x):
        return 10 + 34 * max(0.0, 1 - (min(abs(x), w) / w) ** 2)

    def bottom(x):
        return top(x) - gap(x)

    edges = band(head, -w - 10, w + 10, lambda x: top(x) + 13, lambda x: bottom(x) - 13, 4, steps=16)
    cuts = []
    for s in (-1, 1):
        x0, y0 = s * w, top(w) - 6
        edges += stroke(head, [(x0, y0), (x0 + s * 14, y0 + 44), (x0 + s * 4, y0 + 80), (x0 + s * 22, y0 + 120)],
                        30, 4, seg=40)
        cuts += stroke(head, [(x0, y0), (x0 + s * 14, y0 + 44), (x0 + s * 4, y0 + 80), (x0 + s * 22, y0 + 120)], 12,
                       7, taper=True, seg=40)
    out.append(((136, 10, 14), edges))
    out.append(((20, 0, 2), band(head, -w, w, top, bottom, 6, steps=16) + cuts))
    teeth = []
    for k in range(12):
        if k in (3, 8):  # gone
            continue
        x0 = -0.6 * w + 1.2 * w * k / 12 + 4
        x1 = -0.6 * w + 1.2 * w * (k + 1) / 12 - 4
        long = 0.5 + 0.35 * abs(noise(k * 19.0, 2.0, 0.0, 3.0))
        teeth.append(on_face(head, [(x0, top(x0) - long * gap(x0)), (x1, top(x1) - (long - 0.12) * gap(x1)),
                                    (x1, top(x1) - 3), (x0, top(x0) - 3)], 8))
    out.append(((200, 186, 140), teeth))
    crust = []
    for s in (-1, 1):
        crust += disc_on(head, s * (w - 10), top(w) - 30, 26, 22, 7, n=7, rings=1, wobble=0.5, seed=s + 12.0)
        crust += stroke(head, drip(s * (w - 30), top(w) - 40, 190 + 60 * (s < 0), 18, s + 13.0), 16, 7, taper=True,
                        seg=60)
    out.append(((84, 4, 6), crust))
    return [(colour, moved_faces(faces, sleep_pose)) for colour, faces in out]


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
    """A vertex coloured display list from (colour, faces) parts; a colour may be a function of the point."""
    colours = {}
    faces = []
    for colour, fs in parts:
        for f in fs:
            faces.append(f)
            for p in f:
                colours[p] = colour(p) if callable(colour) else colour
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
    args = sys.argv[1:]
    outs = [Path(a) for a in args[:3]] + OUTS[len(args[:3]):]
    setup = setup_dl("sVisitorSetupDL", LIT_OPA, "Its body: opaque, lit, untextured")

    tall = Mesh("sTallManVtx")
    dls = [gfx("sTallManDL", emit_parts(tall, tall_man())), gfx("sTallTendrilsDL", emit_parts(tall, tall_tendrils()))]
    write_model(outs[0], "The Tall Man", f"#define TALL_MAN_HEIGHT ({TALL_HEIGHT}.0f)\n\n", tall, [setup], dls)

    smile = Mesh("sSmileManVtx")
    dls = [gfx("sSmileBodyDL", emit_parts(smile, smile_body())),
           gfx("sSmileHeadDL", emit_parts(smile, smile_head())),
           gfx("sSmileGrinDL", emit_parts(smile, smile_mouth(True))),
           gfx("sSmileFrownDL", emit_parts(smile, smile_mouth(False)))]
    write_model(outs[1], "The Smiling Man", f"#define SMILE_MAN_NECK_Y ({SMILE_NECK_Y}.0f)\n\n", smile, [setup], dls)

    sleep = Mesh("sSleepManVtx")
    dls = [gfx("sSleepBodyDL", emit_parts(sleep, sleep_body())),
           gfx("sSleepHairDL", emit_parts(sleep, sleep_hair())),
           gfx("sSleepFaceDL", coloured_parts(sleep, sleep_face()))]
    face = setup_dl("sVisitorFaceSetupDL", GLOW_FRONT,
                    "A face that's there in the dark: unlit, no fog, colour = vertex colour x primitive colour")
    write_model(outs[2], "\"Go to sleep.\"", f"#define SLEEP_MAN_NECK_Y ({SLEEP_NECK_Y}.0f)\n\n", sleep, [setup, face],
                dls)

    # Each SCP has an actor (and so an overlay) of its own, so only the one that's here is in memory
    outs = [Path(a) for a in args[3:6]] + SCP_OUTS[len(args[3:6]):]
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


def write_model(out, name, defines, mesh, setups, dls):
    body = "\n".join(setups + dls)
    text = f"""// Generated by tools/ben/gen_ben_visitors.py; edit that script instead of this file.
// {name}, an Expanded Horror visitor. +Y up, +Z forward, +X to its left; 1/100 of a world unit.
// clang-format off

{defines}{mesh.vtx_array()}
{body}
// clang-format on
"""
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(text)
    print(f"wrote {out} ({len(mesh.vtx)} vertices)")


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
