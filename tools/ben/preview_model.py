# Copied unchanged from the Majora's Mask: WTF project's model kit (tools/preview_model.py at f4f3135).
#!/usr/bin/env python3
"""
Renders display lists from a generated .inc.c model file to a PNG, for checking models without the game.

It understands what the generators in tools/ write: VTX arrays, gsSPVertex, gsSP1Triangle/gsSP2Triangles and
gsDPSetPrimColor. Lit parts are flat shaded with the primitive colour; vertex-coloured parts use their colours.

Usage: tools/preview_model.py <file.inc.c> <out.png> <DL>[@x,y,z,rx,ry,rz,s] [<DL>...] [--yaw D] [--pitch D]
                             [--size N] [--zoom F]
Each DL can be placed with a translation, a rotation in degrees (applied Z, then Y, then X... see `place`) and a scale.
"""

import argparse
import math
import re
import struct
import zlib


def parse(path):
    text = open(path).read()
    vtx = {}
    for m in re.finditer(r"(?:static )?Vtx (\w+)\[\] = \{(.*?)\};", text, re.S):
        arr = []
        for v in re.finditer(r"VTX\(([-\d]+), ([-\d]+), ([-\d]+), [-\d]+, [-\d]+, (\d+), (\d+), (\d+), (\d+)\)", m.group(2)):
            arr.append(tuple(int(x) for x in v.groups()))
        vtx[m.group(1)] = arr
    dls = {}
    for m in re.finditer(r"(?:static )?Gfx (\w+)\[\] = \{(.*?)\};", text, re.S):
        dls[m.group(1)] = m.group(2)
    return vtx, dls


def tris_of(vtx, dl_text):
    """Yields (colour or None, [v0, v1, v2]) with raw VTX tuples."""
    loaded = [None] * 32
    prim = (255, 255, 255, 255)
    for line in dl_text.splitlines():
        line = line.strip()
        m = re.match(r"gsDPSetPrimColor\(0, 0, (\d+), (\d+), (\d+), (\d+)\)", line)
        if m:
            prim = tuple(int(x) for x in m.groups())
            continue
        m = re.match(r"gsSPVertex\(&(\w+)\[(\d+)\], (\d+), (\d+)\)", line)
        if m:
            arr, start, n, dst = vtx[m.group(1)], int(m.group(2)), int(m.group(3)), int(m.group(4))
            for i in range(n):
                loaded[dst + i] = arr[start + i]
            continue
        m = re.match(r"gsSP2Triangles\((\d+), (\d+), (\d+), 0, (\d+), (\d+), (\d+), 0\)", line)
        if m:
            g = [int(x) for x in m.groups()]
            yield prim, [loaded[g[0]], loaded[g[1]], loaded[g[2]]]
            yield prim, [loaded[g[3]], loaded[g[4]], loaded[g[5]]]
            continue
        m = re.match(r"gsSP1Triangle\((\d+), (\d+), (\d+), 0\)", line)
        if m:
            g = [int(x) for x in m.groups()]
            yield prim, [loaded[g[0]], loaded[g[1]], loaded[g[2]]]


def mat_mul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)] for i in range(4)]


def mat_apply(m, p):
    x, y, z = p
    return tuple(m[i][0] * x + m[i][1] * y + m[i][2] * z + m[i][3] for i in range(3))


def rot(axis, deg):
    c, s = math.cos(math.radians(deg)), math.sin(math.radians(deg))
    if axis == "x":
        return [[1, 0, 0, 0], [0, c, -s, 0], [0, s, c, 0], [0, 0, 0, 1]]
    if axis == "y":
        return [[c, 0, s, 0], [0, 1, 0, 0], [-s, 0, c, 0], [0, 0, 0, 1]]
    return [[c, -s, 0, 0], [s, c, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]


def place(tx=0, ty=0, tz=0, rx=0, ry=0, rz=0, s=1):
    """Same order as Matrix_Translate then Matrix_RotateZYX (x applied last... i.e. M = T * Rz * Ry * Rx * S)."""
    m = [[s, 0, 0, tx], [0, s, 0, ty], [0, 0, s, tz], [0, 0, 0, 1]]
    m[0][3], m[1][3], m[2][3] = tx, ty, tz
    r = mat_mul(mat_mul(rot("z", rz), rot("y", ry)), rot("x", rx))
    sm = [[s, 0, 0, 0], [0, s, 0, 0], [0, 0, s, 0], [0, 0, 0, 1]]
    t = [[1, 0, 0, tx], [0, 1, 0, ty], [0, 0, 1, tz], [0, 0, 0, 1]]
    return mat_mul(mat_mul(t, r), sm)


def view_matrix(yaw, pitch):
    """The camera of render(): view space x is right, y up and z towards the viewer."""
    return mat_mul(rot("x", pitch), rot("y", -yaw))


def to_screen(p, frame, size):
    """A view space point to render()'s pixel coordinates (x, y, depth), for a frame (cx, cy, span)."""
    cx, cy, span = frame
    return ((p[0] - cx) / span * size + size / 2, size / 2 - (p[1] - cy) / span * size, p[2])


def render(items, size=512, yaw=30, pitch=15, zoom=1.0, bg=(70, 80, 100), frame=None):
    """items: list of (matrix, triangles). Orthographic view. The picture is fitted to what's drawn, unless `frame`
    gives its centre and width in view space, (cx, cy, span), to keep a fixed scale from one picture to the next."""
    world = []
    for m, tris in items:
        for colour, tri in tris:
            pts = [mat_apply(m, v[:3]) for v in tri]
            world.append((colour, tri, pts, m))
    view = view_matrix(yaw, pitch)
    if frame is None:
        allp = [mat_apply(view, p) for _, _, pts, _ in world for p in pts]
        if not allp:
            raise SystemExit("nothing to draw")
        minx, maxx = min(p[0] for p in allp), max(p[0] for p in allp)
        miny, maxy = min(p[1] for p in allp), max(p[1] for p in allp)
        span = max(maxx - minx, maxy - miny) * 1.08 / zoom
        cx, cy = (minx + maxx) / 2, (miny + maxy) / 2
    else:
        cx, cy, span = frame
    W = H = size
    zbuf = [[-1e30] * W for _ in range(H)]
    img = [[list(bg) for _ in range(W)] for _ in range(H)]
    light = (0.4, 0.75, 0.5)
    ll = math.sqrt(sum(x * x for x in light))
    light = [x / ll for x in light]
    for colour, tri, pts, m_rot in world:
        vp = [mat_apply(view, p) for p in pts]
        sp = [((p[0] - cx) / span * W + W / 2, H / 2 - (p[1] - cy) / span * H, p[2]) for p in vp]
        # Back-face cull (counter-clockwise on screen after the y flip = clockwise in raw)
        area = (sp[1][0] - sp[0][0]) * (sp[2][1] - sp[0][1]) - (sp[2][0] - sp[0][0]) * (sp[1][1] - sp[0][1])
        if area >= 0:
            continue
        lit = tri[0][3] is not None and colour is not None and _is_lit(tri)
        if lit:
            # Gouraud: shade each vertex from its own normal (as the N64 does with smooth shading)
            vcols = []
            for v in tri:
                n = [((c + 128) & 0xFF) - 128 for c in v[3:6]]
                n = [sum(m_rot[i][k] * n[k] for k in range(3)) for i in range(3)]
                nl = math.sqrt(sum(x * x for x in n)) or 1
                d = max(0.0, sum(n[i] / nl * light[i] for i in range(3)))
                shade = 0.35 + 0.65 * d
                vcols.append((colour[0] * shade, colour[1] * shade, colour[2] * shade, 255))
            _raster(img, zbuf, sp, None, W, H, vcols, opaque=True)
        else:
            c = tri[0]
            base = [c[3 + 0], c[3 + 1], c[3 + 2], c[6]]
            _raster(img, zbuf, sp, base, W, H, _vertex_colours(tri))
    return img


_LIT = {}


def _is_lit(tri):
    return _LIT.get("mode", True)


def _vertex_colours(tri):
    return [(v[3], v[4], v[5], v[6]) for v in tri]


def _raster(img, zbuf, sp, base, W, H, vcols, opaque=False):
    xs = [p[0] for p in sp]
    ys = [p[1] for p in sp]
    x0, x1 = max(0, int(min(xs))), min(W - 1, int(max(xs)) + 1)
    y0, y1 = max(0, int(min(ys))), min(H - 1, int(max(ys)) + 1)
    (ax, ay, az), (bx, by, bz), (cx, cy, cz) = sp
    den = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy)
    if den == 0:
        return
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            px, py = x + 0.5, y + 0.5
            w0 = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / den
            w1 = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / den
            w2 = 1 - w0 - w1
            if w0 < 0 or w1 < 0 or w2 < 0:
                continue
            z = w0 * az + w1 * bz + w2 * cz
            if vcols is None:
                if z > zbuf[y][x]:
                    zbuf[y][x] = z
                    img[y][x] = [int(base[0]), int(base[1]), int(base[2])]
            elif opaque:
                if z > zbuf[y][x]:
                    zbuf[y][x] = z
                    img[y][x] = [int(w0 * vcols[0][i] + w1 * vcols[1][i] + w2 * vcols[2][i]) for i in range(3)]
            else:
                col = [w0 * vcols[0][i] + w1 * vcols[1][i] + w2 * vcols[2][i] for i in range(4)]
                if z > zbuf[y][x]:
                    a = col[3] / 255.0
                    old = img[y][x]
                    img[y][x] = [int(old[i] * (1 - a) + col[i] * a) for i in range(3)]


def write_png(path, img):
    H, W = len(img), len(img[0])
    raw = b"".join(b"\x00" + bytes(max(0, min(255, c)) for px in row for c in px) for row in img)

    def chunk(t, d):
        return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    open(path, "wb").write(png)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("model")
    ap.add_argument("out")
    ap.add_argument("dls", nargs="+")
    ap.add_argument("--yaw", type=float, default=30)
    ap.add_argument("--pitch", type=float, default=15)
    ap.add_argument("--size", type=int, default=400)
    ap.add_argument("--zoom", type=float, default=1.0)
    ap.add_argument("--unlit", action="store_true", help="DLs use vertex colours")
    args = ap.parse_args()
    vtx, dls = parse(args.model)
    _LIT["mode"] = not args.unlit
    items = []
    for spec in args.dls:
        name, _, params = spec.partition("@")
        vals = [float(x) for x in params.split(",")] if params else []
        m = place(*vals)
        items.append((m, list(tris_of(vtx, dls[name]))))
    write_png(args.out, render(items, args.size, args.yaw, args.pitch, args.zoom))


if __name__ == "__main__":
    main()
