# Copied unchanged from the Majora's Mask: WTF project's model kit (tools/modelkit.py at f4f3135), so BEN DROWNED can
# build its own models the same way. Read only over there: improve it in this copy.
"""
Small toolkit for building untextured N64 models out of simple solids and writing them as F3DEX2 display lists.

A model is a list of parts. Each part is (colour, faces), where a face is a convex polygon given as a list of
(x, y, z) points, counter-clockwise seen from outside. Units are model units.

Two kinds of mesh are written:
- lit: flat-shaded faces whose vertex "colours" are normals; each part sets the primitive colour, so the display list
  needs a material set up beforehand (setup_dl and the materials below: LIT_OPA and the like);
- vertex coloured: every point gets a colour (and alpha) from a function, for unlit, usually translucent effects
  (GLOW and GLOW_FRONT).

Smooth meshes (Smooth) share vertices and average their normals. A part list of both kinds is a list of S(...) and
F(...) parts, written in order by emit_parts. RingSurface describes a body by rings, so details can be laid on it.

collision() writes the game's own collision (a CollisionHeader) from faces, for an actor that Link stands on or walks
into (a DynaPolyActor: the mod's solid pieces, tools/gen_solid_pieces.py), each part with a surface preset (SURFACES),
or for a scene of the mod's own (its static collision, with its exits and camera: tools/scenekit.py).

A scene's rooms are drawn as the game's own mostly are, unlit with their light baked into the vertices' colours
(Mesh.shaded_dl, the SHADED_OPA material).

Used by every model generator in tools/.
"""

import math


# ---------------------------------------------------------------------------------------------------------------------
# Transforms


def rot_x(p, deg, origin=(0, 0, 0)):
    a = math.radians(deg)
    x, y, z = p[0] - origin[0], p[1] - origin[1], p[2] - origin[2]
    return (x + origin[0], y * math.cos(a) - z * math.sin(a) + origin[1], y * math.sin(a) + z * math.cos(a) + origin[2])


def rot_y(p, deg, origin=(0, 0, 0)):
    a = math.radians(deg)
    x, y, z = p[0] - origin[0], p[1] - origin[1], p[2] - origin[2]
    return (x * math.cos(a) + z * math.sin(a) + origin[0], y + origin[1], -x * math.sin(a) + z * math.cos(a) + origin[2])


def rot_z(p, deg, origin=(0, 0, 0)):
    a = math.radians(deg)
    x, y, z = p[0] - origin[0], p[1] - origin[1], p[2] - origin[2]
    return (x * math.cos(a) - y * math.sin(a) + origin[0], x * math.sin(a) + y * math.cos(a) + origin[1], z + origin[2])


def translate(faces, dx, dy, dz):
    return [[(p[0] + dx, p[1] + dy, p[2] + dz) for p in f] for f in faces]


def scale(faces, sx, sy, sz):
    out = [[(p[0] * sx, p[1] * sy, p[2] * sz) for p in f] for f in faces]
    if (sx * sy * sz) < 0:
        # A mirror flips the winding
        out = [list(reversed(f)) for f in out]
    return out


def transform(faces, fn):
    return [[fn(p) for p in f] for f in faces]


def mirror_x(faces):
    return scale(faces, -1, 1, 1)


# ---------------------------------------------------------------------------------------------------------------------
# Solids


def box(x0, x1, y0, y1, z0, z1):
    return frustum_y(y0, y1, (x0, x1, z0, z1), (x0, x1, z0, z1))


def frustum_y(y0, y1, bottom, top, caps=True):
    """A box whose bottom (y0) and top (y1) rectangles differ. Rectangles are (x0, x1, z0, z1)."""
    bx0, bx1, bz0, bz1 = bottom
    tx0, tx1, tz0, tz1 = top
    p = [
        (bx0, y0, bz0), (bx1, y0, bz0), (tx1, y1, tz0), (tx0, y1, tz0),
        (bx0, y0, bz1), (bx1, y0, bz1), (tx1, y1, tz1), (tx0, y1, tz1),
    ]
    faces = [(4, 5, 6, 7), (1, 0, 3, 2), (0, 4, 7, 3), (5, 1, 2, 6)]
    if caps:
        faces += [(3, 7, 6, 2), (0, 1, 5, 4)]
    out = [[p[i] for i in f] for f in faces]
    if y1 < y0:
        out = [list(reversed(f)) for f in out]
    return out


def frustum_z(z0, z1, back, front, caps=True):
    """Like frustum_y but along Z. Rectangles are (x0, x1, y0, y1)."""
    faces = frustum_y(z0, z1, (back[0], back[1], back[2], back[3]), (front[0], front[1], front[2], front[3]), caps)
    # Swap y and z, which mirrors: fix the winding
    return [list(reversed([(q[0], q[2], q[1]) for q in f])) for f in faces]


def prism(axis, a0, a1, c0, c1, radius, sides, radius1=None, phase=0.0, caps=True):
    """A prism (or truncated cone with radius1) along `axis` ('x', 'y' or 'z') from a0 to a1, centred on (c0, c1)
    in the other two axes (in xyz order)."""
    if radius1 is None:
        radius1 = radius

    def ring(r, a):
        pts = []
        for i in range(sides):
            t = 2 * math.pi * i / sides + phase
            u, v = c0 + r * math.cos(t), c1 + r * math.sin(t)
            pts.append(_axis_point(axis, a, u, v))
        return pts

    r0, r1 = ring(radius, a0), ring(radius1, a1)
    faces = []
    for i in range(sides):
        j = (i + 1) % sides
        faces.append([r0[i], r0[j], r1[j], r1[i]])
    if caps:
        faces.append(list(reversed(r0)))
        faces.append(list(r1))
    return _fix_winding_outward(faces, axis, c0, c1, a0, a1)


def _axis_point(axis, a, u, v):
    if axis == "x":
        return (a, u, v)
    if axis == "y":
        return (v, a, u)  # keeps (u, v, axis) right handed: z, x, y
    return (u, v, a)


def _fix_winding_outward(faces, axis, c0, c1, a0, a1):
    """Makes every face wind counter-clockwise seen from outside, by checking the normal against the centre."""
    mid = (a0 + a1) / 2.0
    centre = _axis_point(axis, mid, c0, c1)
    out = []
    for f in faces:
        n = face_normal_f(f)
        fc = [sum(p[i] for p in f) / len(f) for i in range(3)]
        d = [fc[i] - centre[i] for i in range(3)]
        if sum(n[i] * d[i] for i in range(3)) < 0:
            f = list(reversed(f))
        out.append(f)
    return out


def ellipsoid(cx, cy, cz, rx, ry, rz, slices=8, stacks=5):
    """A low-poly ellipsoid."""
    faces = []
    for j in range(stacks):
        t0 = math.pi * j / stacks - math.pi / 2
        t1 = math.pi * (j + 1) / stacks - math.pi / 2
        for i in range(slices):
            p0 = 2 * math.pi * i / slices
            p1 = 2 * math.pi * (i + 1) / slices

            def pt(t, p):
                return (cx + rx * math.cos(t) * math.cos(p), cy + ry * math.sin(t), cz + rz * math.cos(t) * math.sin(p))

            quad = [pt(t0, p0), pt(t0, p1), pt(t1, p1), pt(t1, p0)]
            # Drop the degenerate point at the poles
            if j == 0:
                quad = [quad[0], quad[2], quad[3]]
            elif j == stacks - 1:
                quad = [quad[0], quad[1], quad[2]]
            faces.append(list(reversed(quad)))
    return fix_winding_centre(faces, (cx, cy, cz))


def fix_winding_centre(faces, centre):
    out = []
    for f in faces:
        n = face_normal_f(f)
        fc = [sum(p[i] for p in f) / len(f) for i in range(3)]
        d = [fc[i] - centre[i] for i in range(3)]
        if sum(n[i] * d[i] for i in range(3)) < 0:
            f = list(reversed(f))
        out.append(f)
    return out


def rect(axis, at, a, b, facing):
    """A rectangle across `axis` ('x', 'y' or 'z') at `at`, spanning `a` and `b` in the other two axes (in xyz order),
    wound to face `facing` (+1 or -1 along the axis)"""
    (a0, a1), (b0, b1) = a, b
    if axis == "x":
        pts = [(at, a0, b0), (at, a1, b0), (at, a1, b1), (at, a0, b1)]
    elif axis == "y":
        pts = [(a0, at, b0), (a1, at, b0), (a1, at, b1), (a0, at, b1)]
    else:
        pts = [(a0, b0, at), (a1, b0, at), (a1, b1, at), (a0, b1, at)]
    k = "xyz".index(axis)
    centre = [sum(p[i] for p in pts) / 4 for i in range(3)]
    centre[k] -= facing
    return fix_winding_centre([pts], centre)[0]


def pyramid(base, apex, cap=True):
    """A pyramid from a convex base polygon (counter-clockwise seen from the apex) to `apex`, with a base cap."""
    faces = []
    n = len(base)
    for i in range(n):
        faces.append([base[i], base[(i + 1) % n], apex])
    if cap:
        faces.append(list(reversed(base)))
    centre = [sum(p[i] for p in base + [apex]) / (n + 1) for i in range(3)]
    return fix_winding_centre(faces, centre)


def spike(root, tip, radius, sides=4, twist=0.0):
    """A pointed spike from `root` to `tip` with a base of `radius`. No base cap: spikes grow out of something."""
    d = [tip[i] - root[i] for i in range(3)]
    length = math.sqrt(sum(x * x for x in d)) or 1.0
    d = [x / length for x in d]
    # Any vector not parallel to d
    ref = (0, 1, 0) if abs(d[1]) < 0.9 else (1, 0, 0)
    u = cross(d, ref)
    ul = math.sqrt(sum(x * x for x in u))
    u = [x / ul for x in u]
    v = cross(d, u)
    base = []
    for i in range(sides):
        t = 2 * math.pi * i / sides + twist
        base.append(tuple(root[k] + radius * (math.cos(t) * u[k] + math.sin(t) * v[k]) for k in range(3)))
    return pyramid(base, tip, cap=False)


def loft(sections, caps=True):
    """Connects rings of points (same count, consistent order) into a closed tube. The winding is fixed against
    each pair of rings' centre, so the rings may go either way round."""
    faces = []
    for s in range(len(sections) - 1):
        a, b = sections[s], sections[s + 1]
        n = len(a)
        centre = [sum(p[i] for p in a + b) / (2 * n) for i in range(3)]
        ring_faces = []
        for i in range(n):
            j = (i + 1) % n
            ring_faces.append([a[i], a[j], b[j], b[i]])
        faces += fix_winding_centre(ring_faces, centre)
    if caps:
        for ring in (sections[0], sections[-1]):
            centre_all = [sum(p[i] for sec in sections for p in sec) / (len(sections) * len(ring)) for i in range(3)]
            faces += fix_winding_centre([list(ring)], centre_all)
    return faces


def quad_both(points):
    """A double-sided polygon (for flags, sails and other thin sheets)."""
    return [list(points), list(reversed(points))]


# ---------------------------------------------------------------------------------------------------------------------
# Maths


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def face_normal_f(face):
    # Newell's method: robust for slightly non-planar polygons
    n = [0.0, 0.0, 0.0]
    for i in range(len(face)):
        a, b = face[i], face[(i + 1) % len(face)]
        n[0] += (a[1] - b[1]) * (a[2] + b[2])
        n[1] += (a[2] - b[2]) * (a[0] + b[0])
        n[2] += (a[0] - b[0]) * (a[1] + b[1])
    length = math.sqrt(sum(x * x for x in n)) or 1.0
    return [x / length for x in n]


def face_normal(face):
    return [max(-127, min(127, round(127 * x))) for x in face_normal_f(face)]


# ---------------------------------------------------------------------------------------------------------------------
# Output


class Mesh:
    """Collects vertices for one Vtx array shared by several display lists."""

    def __init__(self, name):
        self.name = name
        self.vtx = []

    def _clamp(self, v):
        v = int(round(v))
        if not -32768 <= v <= 32767:
            raise ValueError(f"{self.name}: coordinate {v} out of range")
        return v

    def _emit_batches(self, batches, lit, colour_fn, shade=None, uv=None):
        """batches: list of (prim colour or None, faces). Returns display list lines. With `shade`, each face's
        vertices are coloured shade(colour, its unit normal) and no primitive colour is set. With `uv`, each face's
        points get the texture coordinates uv(face) gives, [(s, t)] in texels"""
        dl = []
        for colour, faces in batches:
            face_fn = None
            if shade is not None:
                face_fn = (lambda c: lambda face: shade(c, face_normal_f(face)))(colour)
            elif colour is not None:
                alpha = colour[3] if len(colour) > 3 else 255
                dl.append(f"    gsDPSetPrimColor(0, 0, {colour[0]}, {colour[1]}, {colour[2]}, {alpha}),")
            chunk = []
            used = 0
            for face in faces:
                if len(face) > 32:
                    raise ValueError("face too large")
                if used + len(face) > 32:
                    dl += self._flush(chunk, lit, colour_fn, face_fn, uv)
                    chunk, used = [], 0
                chunk.append(face)
                used += len(face)
            dl += self._flush(chunk, lit, colour_fn, face_fn, uv)
        return dl

    def _flush(self, chunk, lit, colour_fn, face_fn=None, uv=None):
        if not chunk:
            return []
        start = len(self.vtx)
        tris = []
        for face in chunk:
            base = len(self.vtx) - start
            n = face_normal(face)
            baked = [max(0, min(255, int(round(v)))) for v in face_fn(face)] + [255] if face_fn else None
            st = [(int(round(s * 32)), int(round(t * 32))) for s, t in uv(face)] if uv else [(0, 0)] * len(face)
            for p, (s, t) in zip(face, st):
                if baked:
                    c = baked
                elif lit:
                    c = [v & 0xFF for v in n] + [255]
                else:
                    c = list(colour_fn(p))
                    if len(c) == 3:
                        c.append(255)
                x, y, z = (self._clamp(v) for v in p)
                if not (-32768 <= s <= 32767 and -32768 <= t <= 32767):
                    raise ValueError(f"{self.name}: texture coordinate ({s}, {t}) out of range")
                self.vtx.append(f"    VTX({x}, {y}, {z}, {s}, {t}, {c[0]}, {c[1]}, {c[2]}, {c[3]}),")
            for i in range(1, len(face) - 1):
                tris.append((base, base + i, base + i + 1))
        lines = [f"    gsSPVertex(&{self.name}[{start}], {len(self.vtx) - start}, 0),"]
        for i in range(0, len(tris) - 1, 2):
            a, b = tris[i], tris[i + 1]
            lines.append(f"    gsSP2Triangles({a[0]}, {a[1]}, {a[2]}, 0, {b[0]}, {b[1]}, {b[2]}, 0),")
        if len(tris) % 2:
            a = tris[-1]
            lines.append(f"    gsSP1Triangle({a[0]}, {a[1]}, {a[2]}, 0),")
        return lines

    def lit_dl(self, parts):
        """parts: list of (colour, faces). Each part sets the primitive colour."""
        return self._emit_batches(parts, True, None)

    def coloured_dl(self, faces, colour_fn):
        """Unlit faces coloured per point by colour_fn(p) -> (r, g, b[, a])."""
        return self._emit_batches([(None, faces)], False, colour_fn)

    def shaded_dl(self, parts, shade):
        """parts: list of (colour, faces), unlit, each face's light baked into its vertices' colours: shade(colour,
        unit normal) -> (r, g, b) (the SHADED_OPA material draws them)"""
        return self._emit_batches(parts, False, None, shade)

    def textured_dl(self, faces, uv, lit=False, shade=None):
        """Faces drawn with a texture (the material loads it and sets its colour): uv(face) -> [(s, t)] in texels for
        each of its points; lit (the vertices' normals, for the game's lights) or, with `shade`, unlit with each
        face's light baked in (shade((255, 255, 255), unit normal)). No primitive colour is set"""
        if shade is not None:
            return self._emit_batches([((255, 255, 255), faces)], False, None, shade, uv)
        return self._emit_batches([(None, faces)], lit, None, None, uv)

    def vtx_array(self):
        return "static Vtx {}[] = {{\n{}\n}};\n".format(self.name, "\n".join(self.vtx))


def planar_uv(period, texels=32):
    """A texture laid flat on each face from the side it faces most (a floor from above, a wall from the front): one
    texture of `texels` across every `period` world units, the same on every face (each face's coordinates moved by
    whole textures towards 0, so they stay small and the faces meet). For Mesh.textured_dl"""

    def uv(face):
        n = face_normal_f(face)
        axis = max(range(3), key=lambda k: abs(n[k]))
        u_axis, v_axis = {0: (2, 1), 1: (0, 2), 2: (0, 1)}[axis]
        st = [(p[u_axis] * texels / period, -p[v_axis] * texels / period if axis != 1 else p[v_axis] * texels / period)
              for p in face]
        du = math.floor(min(s for s, _ in st) / texels) * texels
        dv = math.floor(min(t for _, t in st) / texels) * texels
        return [(s - du, t - dv) for s, t in st]

    return uv


def gfx(name, lines, setup=(), end=True):
    body = list(setup) + list(lines)
    if end:
        body.append("    gsSPEndDisplayList(),")
    return "static Gfx {}[] = {{\n{}\n}};\n".format(name, "\n".join(body))


# ---------------------------------------------------------------------------------------------------------------------
# Smooth meshes: shared vertices with averaged normals, for rounded shapes (Gouraud shaded on the N64)


class Smooth:
    """An indexed triangle mesh. Vertices shared by several triangles get the average of their normals, so the
    surface is shaded smoothly; anything that should have a hard edge must use separate vertices."""

    def __init__(self):
        self.verts = []
        self.tris = []

    def add(self, p):
        self.verts.append(tuple(float(v) for v in p))
        return len(self.verts) - 1

    def tri(self, a, b, c):
        self.tris.append((a, b, c))

    def quad(self, a, b, c, d):
        self.tris.append((a, b, c))
        self.tris.append((a, c, d))

    def extend(self, other):
        base = len(self.verts)
        self.verts += other.verts
        self.tris += [(a + base, b + base, c + base) for a, b, c in other.tris]
        return self

    def copy(self):
        m = Smooth()
        m.verts = list(self.verts)
        m.tris = list(self.tris)
        return m

    def transformed(self, fn):
        m = self.copy()
        m.verts = [tuple(fn(p)) for p in m.verts]
        return m

    def mirrored_x(self):
        m = self.transformed(lambda p: (-p[0], p[1], p[2]))
        m.tris = [(a, c, b) for a, b, c in m.tris]
        return m

    def flipped(self):
        m = self.copy()
        m.tris = [(a, c, b) for a, b, c in m.tris]
        return m

    def normals(self):
        acc = [[0.0, 0.0, 0.0] for _ in self.verts]
        for a, b, c in self.tris:
            pa, pb, pc = self.verts[a], self.verts[b], self.verts[c]
            u = [pb[i] - pa[i] for i in range(3)]
            v = [pc[i] - pa[i] for i in range(3)]
            n = cross(u, v)  # area weighted
            for idx in (a, b, c):
                for i in range(3):
                    acc[idx][i] += n[i]
        out = []
        for n in acc:
            length = math.sqrt(sum(x * x for x in n)) or 1.0
            out.append([max(-127, min(127, round(127 * x / length))) for x in n])
        return out


def ring(cy, rx, rz, n, cx=0.0, cz=0.0, power=2.0, start=0.0, end=None, axis="y"):
    """Points of a superellipse (power 2 = ellipse, higher = boxier) around `axis`. Angle 0 is +Z (the front) and
    angles increase towards +X. With `end`, an open arc from `start` to `end` (radians), endpoints included."""
    pts = []
    count = n if end is None else n + 1
    span = 2 * math.pi if end is None else end - start
    for i in range(count):
        t = start + span * i / n
        c, s = math.cos(t), math.sin(t)
        e = 2.0 / power
        x = cx + rx * math.copysign(abs(s) ** e, s)
        z = cz + rz * math.copysign(abs(c) ** e, c)
        if axis == "y":
            pts.append((x, cy, z))
        elif axis == "z":  # rings across the Z axis: "x" stays x, "z" becomes y, cy is the z position
            pts.append((x, z, cy))
        else:  # axis x: (cy, x, z)
            pts.append((cy, x, z))
    return pts


def smooth_loft(rings, closed=True, cap_start=False, cap_end=False, outward_hint=None):
    """Connects rings of points (all the same count) into a smooth tube. Faces are wound outwards, judged against
    the axis through the rings' centres (or `outward_hint(p)` -> a point inside). Caps close an end with a centre
    vertex."""
    m = Smooth()
    n = len(rings[0])
    idx = [[m.add(p) for p in r] for r in rings]
    seg = n if closed else n - 1

    def centre_of(r):
        return [sum(p[i] for p in r) / len(r) for i in range(3)]

    # Column by column (down the tube, then round it): the N64 loads 32 vertices at a time, and columns share
    # most of their vertices with the next one, where rings would each have to be loaded twice
    for i in range(seg):
        j = (i + 1) % n
        for k in range(len(rings) - 1):
            ca, cb = centre_of(rings[k]), centre_of(rings[k + 1])
            a, b, c, d = idx[k][i], idx[k][j], idx[k + 1][j], idx[k + 1][i]
            pa, pb, pc = m.verts[a], m.verts[b], m.verts[c]
            nrm = cross([pb[t] - pa[t] for t in range(3)], [pc[t] - pa[t] for t in range(3)])
            mid = [(pa[t] + pc[t]) / 2 for t in range(3)]
            inside = outward_hint(mid) if outward_hint else [(ca[t] + cb[t]) / 2 for t in range(3)]
            if sum(nrm[t] * (mid[t] - inside[t]) for t in range(3)) >= 0:
                m.quad(a, b, c, d)
            else:
                m.quad(a, d, c, b)
    for flag, r, k in ((cap_start, rings[0], 0), (cap_end, rings[-1], len(rings) - 1)):
        if not flag:
            continue
        c = centre_of(r)
        other = centre_of(rings[1] if k == 0 else rings[-2])
        ci = m.add(c)
        for i in range(n):
            j = (i + 1) % n
            a, b = idx[k][i], idx[k][j]
            pa, pb = m.verts[a], m.verts[b]
            nrm = cross([pa[t] - c[t] for t in range(3)], [pb[t] - c[t] for t in range(3)])
            away = [c[t] - other[t] for t in range(3)]
            if sum(nrm[t] * away[t] for t in range(3)) >= 0:
                m.tri(ci, a, b)
            else:
                m.tri(ci, b, a)
    return m


def smooth_sphere(cx, cy, cz, rx, ry, rz, n=10, stacks=6, power=2.0):
    """An ellipsoid with poles, smooth."""
    rings = []
    for k in range(1, stacks):
        t = -math.pi / 2 + math.pi * k / stacks
        rings.append(ring(cy + ry * math.sin(t), rx * math.cos(t), rz * math.cos(t), n, cx, cz, power))
    m = smooth_loft(rings)
    bottom = m.add((cx, cy - ry, cz))
    top = m.add((cx, cy + ry, cz))
    first, last = 0, (stacks - 2) * n
    for i in range(n):
        j = (i + 1) % n
        for pole, base, sign in ((bottom, first, -1), (top, last, 1)):
            a, b = base + i, base + j
            pa, pb, pp = m.verts[a], m.verts[b], m.verts[pole]
            nrm = cross([pa[t] - pp[t] for t in range(3)], [pb[t] - pp[t] for t in range(3)])
            if nrm[1] * sign >= 0:
                m.tri(pole, a, b)
            else:
                m.tri(pole, b, a)
    return m


def smooth_spike(root, tip, radius, sides=5, bend=(0, 0, 0)):
    """A soft cone (a tuft of hair): a ring at the root, one at a third of the way bent by `bend`, and the tip."""
    d = [tip[i] - root[i] for i in range(3)]
    length = math.sqrt(sum(x * x for x in d)) or 1.0
    d = [x / length for x in d]
    ref = (0, 1, 0) if abs(d[1]) < 0.9 else (1, 0, 0)
    u = cross(d, ref)
    ul = math.sqrt(sum(x * x for x in u))
    u = [x / ul for x in u]
    v = cross(d, u)

    def circle(centre, r):
        return [tuple(centre[k] + r * (math.cos(2 * math.pi * i / sides) * u[k] + math.sin(2 * math.pi * i / sides) * v[k])
                      for k in range(3)) for i in range(sides)]

    mid = [root[k] + (tip[k] - root[k]) * 0.4 + bend[k] for k in range(3)]
    m = smooth_loft([circle(root, radius), circle(mid, radius * 0.6)])
    t = m.add(tip)
    base = sides
    for i in range(sides):
        j = (i + 1) % sides
        a, b = base + i, base + j
        pa, pb = m.verts[a], m.verts[b]
        nrm = cross([pa[k] - tip[k] for k in range(3)], [pb[k] - tip[k] for k in range(3)])
        out = [pa[k] - mid[k] for k in range(3)]
        if sum(nrm[k] * out[k] for k in range(3)) <= 0:
            m.tri(t, a, b)
        else:
            m.tri(t, b, a)
    return m


def _smooth_emit(mesh_obj, parts):
    """parts: list of (colour, Smooth). Emits vertices with smooth normals into `mesh_obj` (a Mesh), in batches of
    at most 32 vertices. Returns display list lines."""
    dl = []
    for colour, sm in parts:
        if not sm.tris:
            continue
        alpha = colour[3] if len(colour) > 3 else 255
        dl.append(f"    gsDPSetPrimColor(0, 0, {colour[0]}, {colour[1]}, {colour[2]}, {alpha}),")
        normals = sm.normals()
        local = {}
        order = []
        tris = []

        def flush():
            if not tris:
                return
            start = len(mesh_obj.vtx)
            for g in order:
                x, y, z = (mesh_obj._clamp(v) for v in sm.verts[g])
                n = normals[g]
                mesh_obj.vtx.append(f"    VTX({x}, {y}, {z}, 0, 0, {n[0] & 0xFF}, {n[1] & 0xFF}, {n[2] & 0xFF}, 255),")
            dl.append(f"    gsSPVertex(&{mesh_obj.name}[{start}], {len(order)}, 0),")
            for i in range(0, len(tris) - 1, 2):
                a, b = tris[i], tris[i + 1]
                dl.append(f"    gsSP2Triangles({a[0]}, {a[1]}, {a[2]}, 0, {b[0]}, {b[1]}, {b[2]}, 0),")
            if len(tris) % 2:
                a = tris[-1]
                dl.append(f"    gsSP1Triangle({a[0]}, {a[1]}, {a[2]}, 0),")

        for t in sm.tris:
            new = [g for g in dict.fromkeys(t) if g not in local]
            if len(order) + len(new) > 32:
                flush()
                local.clear()
                order.clear()
                tris.clear()
                new = list(dict.fromkeys(t))
            for g in new:
                local[g] = len(order)
                order.append(g)
            tris.append(tuple(local[g] for g in t))
        flush()
    return dl


def _smooth_dl(self, parts):
    return _smooth_emit(self, parts)


Mesh.smooth_dl = _smooth_dl


def smooth_grid(rows):
    """A smooth sheet from a grid of points (rows of equal length). Faces are wound as the grid is laid out:
    counter-clockwise when rows go up and columns go right, seen from the front. Ordered column by column."""
    m = Smooth()
    idx = [[m.add(p) for p in r] for r in rows]
    for i in range(len(rows[0]) - 1):
        for k in range(len(rows) - 1):
            m.quad(idx[k][i], idx[k][i + 1], idx[k + 1][i + 1], idx[k + 1][i])
    return m


def double_sided(m):
    """The sheet plus its back face (separate vertices, so each side has its own normals)."""
    return m.copy().extend(m.flipped())


# ---------------------------------------------------------------------------------------------------------------------
# Part lists of smooth and flat parts


def S(m, colour):
    """A smooth part: a Smooth mesh in one colour."""
    return ("smooth", colour, m)


def F(faces, colour):
    """A flat part: faces in one colour."""
    return ("flat", colour, faces)


def emit_parts(mesh, part_list):
    """Display list lines for a part list, smooth and flat parts in order."""
    lines = []
    for kind, colour, geo in part_list:
        if kind == "smooth":
            lines += mesh.smooth_dl([(colour, geo)])
        else:
            lines += mesh.lit_dl([(colour, geo)])
    return lines


# ---------------------------------------------------------------------------------------------------------------------
# Surfaces described by rings, for placing details on them


class RingSurface:
    """A body described by rings (y, rx, rz, cz, power) around the Y axis. Can build itself and tell where its
    surface is, so details (scars, eyes) can be laid on it."""

    def __init__(self, rings, n, bump=None):
        self.rings = rings
        self.n = n
        self.bump = bump

    def params(self, y):
        rs = self.rings
        if y <= rs[0][0]:
            return rs[0]
        for a, b in zip(rs, rs[1:]):
            if a[0] <= y <= b[0]:
                t = (y - a[0]) / (b[0] - a[0])
                return tuple(a[i] + (b[i] - a[i]) * t for i in range(5))
        return rs[-1]

    def front_z(self, x, y, offset=0.0):
        _, rx, rz, cz, p = self.params(y)
        u = min(abs(x) / (rx + offset), 0.999)
        z = cz + (rz + offset) * (1 - u ** p) ** (1 / p)
        if self.bump:
            z += self.bump(x, y, math.atan2(x, max(z - cz, 1)))
        return z

    def back_z(self, x, y, offset=0.0):
        _, rx, rz, cz, p = self.params(y)
        u = min(abs(x) / (rx + offset), 0.999)
        return cz - (rz + offset) * (1 - u ** p) ** (1 / p)

    def ring_points(self, entry, offset=0.0, start=0.0, end=None, n=None):
        y, rx, rz, cz, p = entry
        pts = ring(y, rx + offset, rz + offset, n or self.n, 0, cz, p, start, end)
        if self.bump:
            out = []
            for x, yy, z in pts:
                t = math.atan2(x, z - cz)
                out.append((x, yy, z + self.bump(x, yy, t)))
            pts = out
        return pts

    def build(self, offset=0.0, cap_start=False, cap_end=False):
        return smooth_loft([self.ring_points(r, offset) for r in self.rings], cap_start=cap_start, cap_end=cap_end)

    def shell(self, y0, y1, open_fn, offset, n):
        """An open shell (the coat): rings between y0 and y1, leaving out the front within open_fn(y) radians of
        straight ahead."""
        ys = [r[0] for r in self.rings if y0 <= r[0] <= y1]
        ys = sorted(set([y0] + ys + [y1]))
        rings = []
        for y in ys:
            a = open_fn(y)
            entry = self.params(y)
            rings.append(self.ring_points(entry, offset, a, 2 * math.pi - a, n))
        return rings


def facing(face, direction):
    """Winds a face so that it faces +Z (direction 1) or -Z (-1)."""
    a, b, c = face[0], face[1], face[2]
    n = cross([b[i] - a[i] for i in range(3)], [c[i] - a[i] for i in range(3)])
    return face if n[2] * direction >= 0 else list(reversed(face))


def strip_on(surface, pts, width, lift, back=False):
    """A flat ribbon following `pts` (x, y) over the front (or back) of `surface` (a scar, a brow, an eye line)."""
    faces = []
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        dx, dy = x1 - x0, y1 - y0
        length = math.hypot(dx, dy) or 1.0
        nx, ny = -dy / length * width / 2, dx / length * width / 2
        quad = []
        for (x, y) in ((x0 - nx, y0 - ny), (x1 - nx, y1 - ny), (x1 + nx, y1 + ny), (x0 + nx, y0 + ny)):
            z = surface.back_z(x, y, lift) if back else surface.front_z(x, y, lift)
            quad.append((x, y, z))
        faces.append(facing(quad, -1 if back else 1))
    return faces


def oval(cx, cy, rx, ry, n=8, squash=0.0):
    """An oval outline (x, y), its bottom half squashed by `squash`."""
    pts = []
    for i in range(n):
        t = 2 * math.pi * i / n
        y = ry * math.sin(t)
        if y < 0:
            y *= 1 - squash
        pts.append((cx + rx * math.cos(t), cy + y))
    return pts


# ---------------------------------------------------------------------------------------------------------------------
# Glows (vertex coloured, drawn turned to the camera)


def lerp(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(len(a)))


def glow_disc(radius, inner, outer, rings=4, n=16):
    """A flat disc facing +Z, bright in the middle and fading out to its edge (drawn turned to the camera)"""
    faces = []
    for k in range(rings):
        r0, r1 = radius * k / rings, radius * (k + 1) / rings
        for i in range(n):
            a0, a1 = 2 * math.pi * i / n, 2 * math.pi * (i + 1) / n
            if k == 0:
                faces.append([(0, 0, 0), (r1 * math.cos(a0), r1 * math.sin(a0), 0),
                              (r1 * math.cos(a1), r1 * math.sin(a1), 0)])
            else:
                faces.append([(r0 * math.cos(a0), r0 * math.sin(a0), 0), (r1 * math.cos(a0), r1 * math.sin(a0), 0),
                              (r1 * math.cos(a1), r1 * math.sin(a1), 0), (r0 * math.cos(a1), r0 * math.sin(a1), 0)])

    def colour(p):
        t = min(1.0, math.sqrt(p[0] ** 2 + p[1] ** 2) / radius)
        c = lerp(inner, outer, t)
        return (int(c[0]), int(c[1]), int(c[2]), int(c[3]))

    return faces, colour


# ---------------------------------------------------------------------------------------------------------------------
# Materials: the setup display lists drawn before a model's parts (after the game's Gfx_SetupDL25_Opa or _Xlu)

# (render mode, combiner, geometry mode)
_LIT = "G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH"
# Opaque, lit, untextured: colour = material (primitive) colour x lighting
LIT_OPA = ("G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2",
           "PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED", _LIT)
# Translucent, lit, untextured: colour = material x lighting, alpha from the environment colour
LIT_XLU = ("G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2",
           "PRIMITIVE, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED", _LIT)
# A translucent silhouette: lit, colour and alpha from the environment colour
LIT_SILHOUETTE = ("G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2",
                  "ENVIRONMENT, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED", _LIT)
# Opaque, unlit, untextured: colour = the vertices' own (a room's baked light: Mesh.shaded_dl), fogged
SHADED_OPA = ("G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2",
              "0, 0, 0, SHADE, 0, 0, 0, 1, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED",
              "G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_SHADING_SMOOTH")
# Glows and effects: unlit, colour and alpha = vertex colour x primitive colour, translucent, double sided
GLOW = ("G_RM_PASS, G_RM_AA_ZB_XLU_SURF2",
        "SHADE, 0, PRIMITIVE, 0, SHADE, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED",
        "G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH")
# The same, front faces only
GLOW_FRONT = GLOW[:2] + ("G_ZBUFFER | G_SHADE | G_CULL_BACK | G_SHADING_SMOOTH",)


def material(mat):
    """The display list lines that set up a material (one of the above), to start a display list with."""
    render_mode, combiner, geometry = mat
    return [
        "    gsDPPipeSync(),",
        "    gsSPTexture(0, 0, 0, G_TX_RENDERTILE, G_OFF),",
        "    gsDPSetCycleType(G_CYC_2CYCLE),",
        f"    gsDPSetRenderMode({render_mode}),",
        f"    gsDPSetCombineLERP({combiner}),",
        f"    gsSPLoadGeometryMode({geometry}),",
    ]


def setup_dl(name, mat, comment):
    """A material's own setup display list, `comment` above it, to draw before a model's parts."""
    return f"// {comment}\n" + gfx(name, material(mat))


# ---------------------------------------------------------------------------------------------------------------------
# Collision: a solid actor's own (a DynaPolyActor's mesh, include/z64bgcheck.h)

# A piece's polygons at most (MODSOLID_POLYS, src/mod_solid.h): the scene's solid actors share 544 polygons and 512
# vertices (z_bgcheck.c, BgCheck_Allocate), the game's own platforms, doors and blocks with the mod's
MAX_POLYS = 64
# CollisionPoly's vertex indices are 13 bits, the top 3 its exclusion flags (COLPOLY_VTX, z64bgcheck.h)
MAX_VERTICES = 0x1FFF

# Surface presets: what the game reads off a polygon's SurfaceType (z64bgcheck.h's SURFACETYPE0 and SURFACETYPE1), each
# as the game's own code reads it. Join two with "+" ("stone+hookable"). Anything not given is the game's 0: no camera
# of its own, no exit, a plain floor and wall, footsteps by material
SURFACES = {
    # Footsteps and dust (SurfaceType_GetMaterial: sSurfaceSfxOffsets, z_bgcheck.c)
    "stone": {"material": "SURFACE_MATERIAL_STONE"},
    "wood": {"material": "SURFACE_MATERIAL_WOOD"},
    "plank": {"material": "SURFACE_MATERIAL_BRIDGE"},  # A bridge's planks
    "grass": {"material": "SURFACE_MATERIAL_GRASS"},
    "dirt": {"material": "SURFACE_MATERIAL_DIRT"},
    # Link slides on it (FLOOR_TYPE_5: z_player.c's ice, unless he wears the Zora's underwater boots)
    "ice": {"material": "SURFACE_MATERIAL_ICE", "floor": "FLOOR_TYPE_5"},
    # The hookshot catches on it (SurfaceType_IsHookshotSurface)
    "hookable": {"material": "SURFACE_MATERIAL_WOOD", "hook": "true"},
    # Link climbs it, as a ladder or a grate (WALL_TYPE_4: sWallFlags' WALL_FLAG_3, z_player.c's climb)
    "climbable": {"material": "SURFACE_MATERIAL_WOOD", "wall": "WALL_TYPE_4"},
    # Link falling onto it voids out, as into the game's pits (FLOOR_PROPERTY_12, z_player.c)
    "void": {"material": "SURFACE_MATERIAL_STONE", "property": "FLOOR_PROPERTY_12"},
    # It burns Link who stands on it, but Goron Link (FLOOR_TYPE_2: z_player.c's hurting floors, as lava)
    "hot": {"material": "SURFACE_MATERIAL_LAVA", "floor": "FLOOR_TYPE_2"},
    # Epona won't go onto it (isHorseBlocked: SurfaceType_IsHorseBlocked)
    "no_horse": {"material": "SURFACE_MATERIAL_STONE", "horse": "true"},
}


# What a scene's own collision gives a polygon besides its preset (a part's third element, a dict): the scene's exit
# it takes Link through, 1-based ("exit": n is the scene's exit list's n-th; SurfaceType_GetSceneExitIndex, 5 bits),
# its camera (an index into the scene's camera list) and its light setting (into the scene's light settings)
SURFACE_EXTRAS = {"exit": 0x1F, "camera": 0xFF, "light": 0x1F}


def surface(spec, extra=None):
    """A surface preset (or several joined with "+") as its SurfaceType's two words, written with the game's macros;
    `extra` (SURFACE_EXTRAS) a scene's exit, camera and light for it"""
    fields = {}
    for name in spec.split("+"):
        if name not in SURFACES:
            raise ValueError(f"no surface preset {name!r}: the presets are {', '.join(SURFACES)} (modelkit.SURFACES)")
        fields.update(SURFACES[name])
    extra = extra or {}
    for key, value in extra.items():
        if key not in SURFACE_EXTRAS or not 0 <= value <= SURFACE_EXTRAS[key]:
            raise ValueError(f"a surface's {key!r} {value!r}: the extras are {', '.join(SURFACE_EXTRAS)}, each from 0 "
                             "to what its bits hold (modelkit.SURFACE_EXTRAS)")
    g = fields.get
    word0 = (f"SURFACETYPE0({extra.get('camera', 0)}, {extra.get('exit', 0)}, {g('floor', 'FLOOR_TYPE_0')}, 0, "
             f"{g('wall', 'WALL_TYPE_0')}, {g('property', 'FLOOR_PROPERTY_0')}, false, {g('horse', 'false')})")
    word1 = (f"SURFACETYPE1({g('material', 'SURFACE_MATERIAL_STONE')}, FLOOR_EFFECT_0, {extra.get('light', 0)}, 0, "
             f"{g('hook', 'false')}, CONVEYOR_SPEED_DISABLED, 0, false)")
    return word0, word1


class Collision:
    """A CollisionHeader's C, with its counts: polys, vertices (each against the scene's shared budget)"""

    def __init__(self, text, polys, vertices, bounds, triangles=None):
        self.text = text
        self.polys = polys
        self.vertices = vertices
        self.bounds = bounds  # ((min x, y, z), (max x, y, z)), model units
        # [(its three points, its unit normal, its surface's extras)] of every polygon, for a scene's checks
        self.triangles = triangles or []


def no_camera(name):
    """The camera list a piece's collision names: one entry, no camera of its own (CAM_SET_NONE). Not left NULL: the
    camera reads a list by the floor's index for whatever Link stands on (Camera_GetBgCamIndex ->
    BgCheck_GetBgCamSettingImpl, which doesn't check for NULL), so a solid actor's floor needs one to read"""
    return f"static BgCamInfo {name}[] = {{\n    {{ CAM_SET_NONE, 0, NULL }},\n}};\n"


def collision(name, parts, camera, max_polys=MAX_POLYS, storage="static ", water=()):
    """The game's collision for `parts`, a list of (surface preset, faces) (the model's own faces, or a simpler hull;
    a scene's parts may have a third element, the SURFACE_EXTRAS of their surface), as C: its vertices ({name}Vtx),
    polygons ({name}Polys), surface types ({name}Surfaces) and the CollisionHeader `name`, naming `camera`
    (no_camera()) as its camera list, each `storage` (a scene's are global: ""), with `water` its water boxes
    ({name}Water, each (x, y, z, x length, z length, properties): y the surface, x and z the lowest corner). Faces are
    split into triangles from their first point; points are rounded to whole model units and welded. Each polygon's
    normal and distance are worked out as the game does for a solid actor's (DynaPoly_AddBgActorToLookup,
    z_bgcheck.c: the unit normal of (B - A) x (C - A) in s16, COLPOLY_SNORMAL, and the plane's distance,
    nearbyint(-n . A)); a floor is a polygon whose normal points up past 0.5, a ceiling down past 0.8, the rest walls.
    Refuses a piece past `max_polys` polygons, a vertex past 13 bits, a point out of s16 range, a triangle with no area
    (a face with three points in a line) and a water box with none."""
    index, verts, polys, surfaces, triangles = {}, [], [], [], []

    def vertex(p):
        key = tuple(int(round(v)) for v in p)
        if any(not -32768 <= v <= 32767 for v in key):
            raise ValueError(f"{name}: point {key} out of s16 range")
        if key not in index:
            index[key] = len(verts)
            verts.append(key)
        return index[key]

    for part in parts:
        spec, faces = part[0], part[1]
        extra = part[2] if len(part) > 2 else None
        words = surface(spec, extra)
        if words not in surfaces:
            surfaces.append(words)
        kind = surfaces.index(words)
        for face in faces:
            ids = [vertex(p) for p in face]
            for i in range(1, len(ids) - 1):
                tri = (ids[0], ids[i], ids[i + 1])
                a, b, c = (verts[j] for j in tri)
                n = cross([b[k] - a[k] for k in range(3)], [c[k] - a[k] for k in range(3)])
                length = math.sqrt(sum(x * x for x in n))
                if length < 1e-6 or len(set(tri)) < 3:
                    raise ValueError(f"{name}: a triangle with no area ({a}, {b}, {c}): a face has three points in a "
                                     "line, or two the same once rounded")
                unit = [x / length for x in n]
                normal = [int(x * 32767) for x in unit]  # (s16)(x * SHRT_MAX) truncates, as C does
                dist = round(-sum(unit[k] * a[k] for k in range(3)))  # nearbyint: to the nearest, halves to even
                if not -32768 <= dist <= 32767:
                    raise ValueError(f"{name}: a plane {dist} units from the origin, past s16")
                polys.append((kind, tri, normal, dist))
                triangles.append(((a, b, c), unit, dict(extra or {})))
    if len(polys) > max_polys:
        raise ValueError(f"{name}: {len(polys)} polygons, past a piece's {max_polys} (the scene's solid actors share "
                         "544): simplify its collision (a hull of fewer faces)")
    if len(verts) > MAX_VERTICES:
        raise ValueError(f"{name}: {len(verts)} vertices, past 13 bits")
    lo = tuple(min(v[k] for v in verts) for k in range(3))
    hi = tuple(max(v[k] for v in verts) for k in range(3))
    floors = sum(1 for p in polys if p[2][1] > 0.5 * 32767)
    ceilings = sum(1 for p in polys if p[2][1] < -0.8 * 32767)
    out = [f"// Collision: {len(polys)} polygons ({floors} floor, {ceilings} ceiling, {len(polys) - floors - ceilings} "
           f"wall), {len(verts)} vertices",
           f"{storage}Vec3s {name}Vtx[] = {{"]
    out += [f"    {{ {x}, {y}, {z} }}," for x, y, z in verts]
    out += ["};", f"{storage}CollisionPoly {name}Polys[] = {{"]
    out += [f"    {{ {kind}, {{ {{ {a}, {b}, {c} }} }}, {{ {n[0]}, {n[1]}, {n[2]} }}, {d} }},"
            for kind, (a, b, c), n, d in polys]
    out += ["};", f"{storage}SurfaceType {name}Surfaces[] = {{"]
    out += [f"    {{ {{ {w0},\n        {w1} }} }}," for w0, w1 in surfaces]
    out += ["};"]
    for box in water:
        if any(not -32768 <= v <= 32767 for v in box[:5]) or box[3] <= 0 or box[4] <= 0:
            raise ValueError(f"{name}: a water box {box[:5]} out of s16 range or with no area")
    if water:
        out += [f"{storage}WaterBox {name}Water[] = {{"]
        out += [f"    {{ {{ {x}, {y}, {z} }}, {xl}, {zl}, {props} }}," for x, y, z, xl, zl, props in water]
        out += ["};"]
    out += [f"{storage}CollisionHeader {name} = {{",
            f"    {{ {lo[0]}, {lo[1]}, {lo[2]} }}, {{ {hi[0]}, {hi[1]}, {hi[2]} }},",
            f"    {len(verts)}, {name}Vtx,",
            f"    {len(polys)}, {name}Polys,",
            f"    {name}Surfaces,",
            f"    {camera},",
            f"    {len(water)}, {name}Water," if water else "    0, NULL,",
            "};", ""]
    return Collision("\n".join(out), len(polys), len(verts), (lo, hi), triangles)


# ---------------------------------------------------------------------------------------------------------------------
# Imported models: a model made in Blender (or anything that writes OBJ or glTF 2.0), read by tools/modelio.py, as the
# game draws it. Its description is tools/models/<name>.py (Model, ModelMaterial), and what reads it: the importer
# (tools/import_model.py: an actor's .inc.c, an object's file), a solid piece's shape (tools/gen_solid_pieces.py,
# imported_shape) and a scene's room (tools/scenekit.py, a part that is an Imported)

# Model units a world unit, by where the model goes: an actor's and a piece's are drawn at 0.1 (finer than a world
# unit: a vertex is whole numbers), a room's at 1
MODEL_UNITS = {"overlay": 10, "object": 10, "solid": 10, "room": 1}
OUTPUTS = tuple(MODEL_UNITS)
AXES = ("y_up", "z_up")
ALPHA = {"opaque": "OPAQUE", "cutout": "MASK", "blend": "BLEND"}
WRAP = {"wrap": "G_TX_WRAP", "mirror": "G_TX_MIRROR", "clamp": "G_TX_CLAMP"}
# A model's vertices are s16; texture coordinates S10.5 (texels x 32)
S16 = (-32768, 32767)


class ModelMaterial:
    """What a model's material becomes, where the file doesn't say or says otherwise (each None: the file's):
    - format: its texture's N64 format (texkit.FORMATS: RGBA16, RGBA32, CI4, CI8, I4, I8, IA4, IA8, IA16; None: the
      most faithful that fits in one load, texkit.choose_format);
    - lit: drawn lit by the game's lights (the vertices' normals) or not (their colours: the file's vertex colours,
      or white). The file's unlit materials (KHR_materials_unlit, OBJ's illum 0) and faces with vertex colours are
      unlit unless this says lit;
    - alpha: "opaque", "cutout" (the texture's alpha cuts holes, drawn with the solid faces) or "blend" (see-through,
      drawn after the solid faces);
    - double_sided: both faces drawn (no culling);
    - colour: (r, g, b[, a]) of 0-255 instead of the file's base colour (a texture is drawn times it);
    - filter: "bilerp" or "point"; wrap: (s, t), each "wrap", "mirror" or "clamp";
    - surface: its faces' surface preset (SURFACES, "stone+hookable") when they're collision (a face of the
      collision object whose material has no preset's name, or every face with Model's collision="model"); extra: a
      scene's SURFACE_EXTRAS for them ({"exit": 1})"""

    def __init__(self, format=None, lit=None, alpha=None, double_sided=None, colour=None, filter=None, wrap=None,
                 surface=None, extra=None):
        self.format, self.lit, self.alpha, self.double_sided = format, lit, alpha, double_sided
        self.colour, self.filter, self.wrap, self.surface, self.extra = colour, filter, wrap, surface, extra


class Model:
    """A model's description (tools/models/<name>.py's model()): the file it's made from (in assets/models/<name>/:
    the user's own, never the ROM's), its scale (world units a unit of the file: 10, fast64's, by default, so Link is
    about 4.4 units tall in Blender), its axes ("y_up": glTF's, and Blender's OBJ export's; "z_up" turns a Z-up file's
    Z into Y), where it goes (output: "overlay", an actor's .inc.c, for `actor`; "object", an object of the mod's own,
    src/objects/object_mod_<name>/; "solid", a solid piece's shape, tools/gen_solid_pieces.py; "room", a scene's room,
    tools/scenes/), its materials (name: ModelMaterial) and its collision ("collision": the faces of its objects named
    collision..., never drawn, each material's name a surface preset or its ModelMaterial's surface; "model": its own
    faces; None: none). With a skeleton (tools/animkit.py's Skin, its file's armature, or Rig, built in Python with no
    file: source None), it's a skeleton and its animations in the game's own formats (6.7), an actor's or an object's"""

    def __init__(self, source, scale=10.0, output="overlay", actor=None, axes="y_up", materials=None,
                 collision="collision", about="", skeleton=None):
        self.source, self.scale, self.output, self.actor, self.axes = source, float(scale), output, actor, axes
        self.materials, self.collision, self.about = dict(materials or {}), collision, about
        self.skeleton = skeleton
        self.name = None

    def errors(self):
        out = []
        if self.output not in OUTPUTS:
            out.append(f"its output is {self.output!r}: one of {', '.join(OUTPUTS)}")
        if self.axes not in AXES:
            out.append(f"its axes are {self.axes!r}: one of {', '.join(AXES)}")
        if not self.scale > 0:
            out.append(f"its scale is {self.scale}: more than 0")
        if self.collision not in ("collision", "model", None):
            out.append(f"its collision is {self.collision!r}: \"collision\", \"model\" or None")
        for name, m in self.materials.items():
            if not isinstance(m, ModelMaterial):
                out.append(f"its material {name!r} isn't a ModelMaterial")
                continue
            from texkit import FORMATS  # noqa: E402

            if m.format is not None and m.format not in FORMATS:
                out.append(f"its material {name!r}'s format {m.format!r}: one of {', '.join(FORMATS)}")
            if m.alpha is not None and m.alpha not in ALPHA:
                out.append(f"its material {name!r}'s alpha {m.alpha!r}: one of {', '.join(ALPHA)}")
            if m.filter not in (None, "bilerp", "point"):
                out.append(f"its material {name!r}'s filter {m.filter!r}: \"bilerp\" or \"point\"")
            if m.wrap is not None and (len(m.wrap) != 2 or any(w not in WRAP for w in m.wrap)):
                out.append(f"its material {name!r}'s wrap {m.wrap!r}: (s, t), each one of {', '.join(WRAP)}")
            if m.colour is not None and (not isinstance(m.colour, (tuple, list)) or len(m.colour) not in (3, 4) or
                                         any(type(c) is not int or not 0 <= c <= 255 for c in m.colour)):
                out.append(f"its material {name!r}'s colour {m.colour!r}: (r, g, b[, a]), whole numbers of 0-255 "
                           "(not Blender's 0-1)")
            for field in ("lit", "double_sided"):
                if not any(getattr(m, field) is v for v in (None, True, False)):
                    out.append(f"its material {name!r}'s {field} {getattr(m, field)!r}: True, False or None")
        if self.skeleton is not None:
            import animkit  # noqa: E402

            out += animkit.description_errors(self)
        elif not self.source:
            out.append("it has no file (source=): only a skeleton built in Python (animkit.Rig) has none")
        return out


def model_path(name, mod=None):
    from pathlib import Path  # noqa: E402

    root = Path(mod) if mod else Path(__file__).resolve().parent.parent
    return root / "tools/models" / f"{name}.py"


def load_model(name, mod=None):
    """tools/models/<name>.py's Model (its name set), checked; ValueError when it doesn't load or is wrong"""
    import importlib.util  # noqa: E402

    path = model_path(name, mod)
    if not path.exists():
        raise ValueError(f"no model {name}: {path} isn't there (tools/kit.py new-model writes one)")
    spec = importlib.util.spec_from_file_location(f"model_{name}", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    if not hasattr(module, "model"):
        raise ValueError(f"{path}: no model() (it returns the model's Model)")
    model = module.model()
    model.name = name
    model.root = path.parent.parent.parent
    errors = model.errors()
    if errors:
        raise ValueError(f"{path}: " + "; ".join(errors))
    return model


class ImportedTexture:
    """A texture of an imported model, encoded: its format, size, texels and palette (bytes)"""

    def __init__(self, key, image, fmt, data, palette):
        self.key, self.image, self.format, self.data, self.palette = key, image, fmt, data, palette


class Section:
    """One material's faces of an imported model: how it's drawn and its triangles, each three corners of (position in
    model units, (s, t) in texels or None, colour (r, g, b, a) of 0-255 or None, unit normal)"""

    def __init__(self, name, texture, colour, lit, alpha, double_sided, filter, wrap, triangles):
        self.name, self.texture, self.colour, self.lit, self.alpha = name, texture, colour, lit, alpha
        self.double_sided, self.filter, self.wrap, self.triangles = double_sided, filter, wrap, triangles


class Imported:
    """A model read and made into what the game draws: its sections (opaque and cutout first, then see-through), its
    textures, its collision parts [(surface preset, faces[, extra])] in model units, and its bounds"""

    def __init__(self, model, units, sections, textures, collision, bounds):
        self.model, self.units, self.sections, self.textures = model, units, sections, textures
        self.collision, self.bounds = collision, bounds

    def triangle_count(self):
        return sum(len(s.triangles) for s in self.sections)


def _is_collision(name):
    return name.lower().startswith("collision")


def imported(model, units=None, asset=None):
    """Model (a description) -> Imported, refusing what the game can't draw: a point past a vertex's s16, a texture past
    one load of TMEM or not a power of two, texture coordinates past S10.5 on a face, a collision face with no surface
    preset; each refusal names the face's object, the material or the texture. `asset` is its file already read (a
    skeleton's: tools/animkit.py gives its triangles in the bind pose)"""
    import math  # noqa: E402

    import modelio  # noqa: E402
    import texkit  # noqa: E402

    units = units or MODEL_UNITS[model.output]
    where = f"tools/models/{model.name}.py ({model.source})"
    if asset is None:
        try:
            asset = modelio.load(model.root / model.source)
        except (OSError, ValueError) as e:
            raise ValueError(f"{where}: {e}") from None
    k = model.scale * units

    def place(p):
        x, y, z = p if model.axes == "y_up" else (p[0], p[2], -p[1])
        return (x * k, y * k, z * k)

    def turn(n):
        return n if model.axes == "y_up" else (n[0], n[2], -n[1])

    def vertex(p, obj, i):
        out = tuple(int(round(v)) for v in place(p))
        if any(not S16[0] <= v <= S16[1] for v in out):
            raise ValueError(f"{where}: the object {obj}'s face {i} has a corner at {tuple(round(v, 3) for v in p)} "
                             f"(the file's units), {out} in model units, past what a vertex holds (-32768 to 32767): a "
                             "smaller scale, or the model nearer its origin")
        return out

    names = [m.name for m in asset.materials]
    # A description's material the file doesn't have (a typo, or Blender's rename to Planks.001) would be ignored
    unknown = sorted(set(model.materials) - set(names) - {"default"})
    if unknown:
        raise ValueError(f"{where}: its description names material(s) the file doesn't have "
                         f"({', '.join(map(repr, unknown))}): the file's are {', '.join(map(repr, names)) or 'none'} "
                         "(\"default\" for faces with none)")
    textures, sections, collision = {}, [], {}
    by_material = {}
    for i, tri in enumerate(asset.triangles()):
        if _is_collision(tri.object):
            if model.collision == "collision":
                collision.setdefault(tri.material, []).append((tri.object, i, tri))
            continue
        by_material.setdefault(tri.material, []).append((tri.object, i, tri))
        if model.collision == "model":
            collision.setdefault(tri.material, []).append((tri.object, i, tri))
    if not by_material:
        raise ValueError(f"{where}: nothing to draw (every object is collision, or the file has no faces)")
    order = [m for m in range(len(names)) if m in by_material] + ([None] if None in by_material else [])
    for mat in order:
        src = asset.materials[mat] if mat is not None else modelio.Material("default")
        desc = model.materials.get(src.name, ModelMaterial())
        tris = by_material[mat]
        has_colours = all(c.colour is not None for _, _, t in tris for c in t.corners)
        lit = desc.lit if desc.lit is not None else not (src.unlit or has_colours)
        alpha = ALPHA[desc.alpha] if desc.alpha else src.alpha_mode
        double = desc.double_sided if desc.double_sided is not None else src.double_sided
        colour = tuple(desc.colour) if desc.colour else tuple(int(round(max(0.0, min(1.0, c)) * 255)) for c in src.colour)
        colour = colour + (255,) * (4 - len(colour))
        filt = desc.filter or src.filter
        wrap = tuple(desc.wrap) if desc.wrap else src.wrap
        tex = None
        if src.texture is not None:
            img = src.texture
            what = f"{where}: the material {src.name}'s texture {img.name} ({img.width}x{img.height})"
            fmt = desc.format or texkit.choose_format(img.pixels, img.width, img.height, alpha in ("MASK", "BLEND"))
            try:
                texkit.check_texture(fmt, img.width, img.height, what)
            except ValueError as e:
                raise ValueError(str(e)) from None
            if fmt in ("I4", "I8") and alpha in ("MASK", "BLEND"):
                raise ValueError(f"{what}: {fmt} on a {'cut-out' if alpha == 'MASK' else 'see-through'} material "
                                 "takes its alpha from its brightness (the RDP reads an I texel as I, I, I, I), so its "
                                 "dark texels would be cut away or faded: IA8 or IA16 (their alpha is the file's)")
            key = (id(img), fmt)
            if key not in textures:
                data, palette = texkit.encode_texture(fmt, img.pixels, img.width, img.height)
                textures[key] = ImportedTexture(key, img, fmt, data, palette)
            tex = textures[key]
            if not any(t.corners[0].uv is not None for _, _, t in tris):
                raise ValueError(f"{where}: the material {src.name} has a texture but its faces have no texture "
                                 "coordinates (unwrap them in Blender)")
        if alpha == "MASK" and tex is None:
            alpha = "OPAQUE"
        out = []
        for obj, i, t in tris:
            corners = []
            st = None
            if tex is not None:
                w, h = tex.image.width, tex.image.height
                half = 0.5 if filt == "bilerp" else 0.0  # The RDP's bilinear filter samples texel n at n, GL's at n + 0.5
                st = [((c.uv or (0.0, 0.0))[0] * w - half, (c.uv or (0.0, 0.0))[1] * h - half) for c in t.corners]
                # Moved by whole textures (two when mirrored) towards 0 on each face, so they stay small
                for axis, size, mode in ((0, w, wrap[0]), (1, h, wrap[1])):
                    if mode == "clamp":
                        continue
                    period = size * (2 if mode == "mirror" else 1)
                    shift = math.floor(min(p[axis] for p in st) / period + 0.5) * period
                    st = [(p[0] - shift, p[1]) if axis == 0 else (p[0], p[1] - shift) for p in st]
                st = [(int(round(s * 32)), int(round(tt * 32))) for s, tt in st]
                if any(not S16[0] <= v <= S16[1] for p in st for v in p):
                    raise ValueError(f"{where}: the object {obj}'s face {i}, of the material {src.name}, spans more "
                                     f"than the 1024 texels a face can (its texture coordinates {st} in 1/32 texels): "
                                     "split the face, or scale its texture coordinates down")
            for j, c in enumerate(t.corners):
                colour_c = None
                if c.colour is not None and not lit:
                    colour_c = tuple(int(round(max(0.0, min(1.0, v)) * 255)) for v in c.colour[:4])
                    colour_c = colour_c + (255,) * (4 - len(colour_c))
                corners.append((vertex(c.pos, obj, i), st[j] if st else (0, 0), colour_c, turn(c.normal)))
            out.append(tuple(corners))
        sections.append(Section(src.name, tex, colour, lit, alpha, double, filt, wrap, out))
    sections.sort(key=lambda s: s.alpha == "BLEND")
    parts = []
    for mat, tris in collision.items():
        name = names[mat] if mat is not None else None
        desc = model.materials.get(name, ModelMaterial()) if name else ModelMaterial()
        preset = desc.surface or (name if name and all(p in SURFACES for p in name.split("+")) else None)
        if preset is None:
            preset = "stone" if model.collision == "model" or name is None else None
        if preset is None:
            raise ValueError(f"{where}: the collision's material {name!r} isn't a surface preset ({', '.join(SURFACES)}"
                             ", joined with +): name it after one, or give it ModelMaterial(surface=...)")
        faces = [[vertex(c.pos, obj, i) for c in t.corners] for obj, i, t in tris]
        parts.append((preset, faces, dict(desc.extra)) if desc.extra else (preset, faces))
    points = [c[0] for s in sections for t in s.triangles for c in t]
    bounds = (tuple(min(p[k] for p in points) for k in range(3)), tuple(max(p[k] for p in points) for k in range(3)))
    return Imported(model, units, sections, list(textures.values()), parts, bounds)


def _texture_lines(tex, name, palette_name, wrap, filt):
    """A section's texture load: its look-up table (a CI's palette), its filter, and its texels into TMEM"""
    import texkit  # noqa: E402

    fmt, siz, bits = texkit.FORMATS[tex.format]
    w, h = tex.image.width, tex.image.height
    masks, maskt = w.bit_length() - 1, h.bit_length() - 1
    cms, cmt = (f"G_TX_NOMIRROR | {WRAP[wrap[0]]}" if wrap[0] != "mirror" else "G_TX_MIRROR | G_TX_WRAP",
                f"G_TX_NOMIRROR | {WRAP[wrap[1]]}" if wrap[1] != "mirror" else "G_TX_MIRROR | G_TX_WRAP")
    lines = ["    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),",
             f"    gsDPSetTextureLUT({'G_TT_RGBA16' if tex.palette else 'G_TT_NONE'}),",
             f"    gsDPSetTextureFilter({'G_TF_POINT' if filt == 'point' else 'G_TF_BILERP'}),"]
    if tex.format == "CI4":
        lines.append(f"    gsDPLoadTLUT_pal16(0, {palette_name}),")
    elif tex.format == "CI8":
        lines.append(f"    gsDPLoadTLUT_pal256({palette_name}),")
    if bits == 4:
        lines.append(f"    gsDPLoadTextureBlock_4b({name}, {fmt}, {w}, {h}, 0, {cms}, {cmt}, {masks}, {maskt}, "
                     "G_TX_NOLOD, G_TX_NOLOD),")
    else:
        lines.append(f"    gsDPLoadTextureBlock({name}, {fmt}, {siz}, {w}, {h}, 0, {cms}, {cmt}, {masks}, {maskt}, "
                     "G_TX_NOLOD, G_TX_NOLOD),")
    return lines


def _section_setup(section, lit, tex_lines):
    """A section's material: the pipeline's modes (after the game's setup display list 25: two cycles, fog), its
    combiner (texture x shade x colour, alpha texture x colour), its geometry mode and its colour"""
    second = {"OPAQUE": "G_RM_AA_ZB_OPA_SURF2", "MASK": "G_RM_AA_ZB_TEX_EDGE2", "BLEND": "G_RM_AA_ZB_XLU_SURF2"}
    geometry = "G_ZBUFFER | G_SHADE | " + ("" if section.double_sided else "G_CULL_BACK | ") + "G_FOG | " + \
        ("G_LIGHTING | " if lit else "") + "G_SHADING_SMOOTH"
    if tex_lines:
        combiner = "TEXEL0, 0, SHADE, 0, TEXEL0, 0, PRIMITIVE, 0, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED"
    else:
        combiner = "PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED"
    c = section.colour
    lines = ["    gsDPPipeSync(),",
             "    gsDPSetCycleType(G_CYC_2CYCLE),",
             f"    gsDPSetRenderMode(G_RM_FOG_SHADE_A, {second[section.alpha]}),",
             f"    gsDPSetCombineLERP({combiner}),",
             f"    gsSPLoadGeometryMode({geometry}),"]
    lines += tex_lines or ["    gsSPTexture(0, 0, 0, G_TX_RENDERTILE, G_OFF),"]
    lines.append(f"    gsDPSetPrimColor(0, 0, {c[0]}, {c[1]}, {c[2]}, {c[3]}),")
    return lines


def _mesh_indexed(mesh, triangles):
    """Display list lines for triangles of whole vertices (x, y, z, s, t, r, g, b, a), each vertex loaded once a
    batch, batches of at most 32 (the RSP's vertex buffer)"""
    dl, order, local, tris = [], [], {}, []

    def flush():
        if not tris:
            return
        start = len(mesh.vtx)
        for v in order:
            mesh.vtx.append("    VTX({}, {}, {}, {}, {}, {}, {}, {}, {}),".format(*v))
        dl.append(f"    gsSPVertex(&{mesh.name}[{start}], {len(order)}, 0),")
        for i in range(0, len(tris) - 1, 2):
            a, b = tris[i], tris[i + 1]
            dl.append(f"    gsSP2Triangles({a[0]}, {a[1]}, {a[2]}, 0, {b[0]}, {b[1]}, {b[2]}, 0),")
        if len(tris) % 2:
            a = tris[-1]
            dl.append(f"    gsSP1Triangle({a[0]}, {a[1]}, {a[2]}, 0),")

    for tri in triangles:
        new = [v for v in dict.fromkeys(tri) if v not in local]
        if len(order) + len(new) > 32:
            flush()
            order, local, tris = [], {}, []
            new = list(dict.fromkeys(tri))
        for v in new:
            local[v] = len(order)
            order.append(v)
        tris.append(tuple(local[v] for v in tri))
    flush()
    return dl


def imported_dl(imp, mesh, prefix, shade=None):
    """An Imported's display list lines into `mesh` (a Mesh: its vertices go into its array): (the opaque and cutout
    sections' lines, the see-through ones', the textures' [(C name, bytes, what it is)]). Each texture is
    <prefix>Tex<n> (its palette <prefix>Pal<n>). With `shade` (a scene's baked light, shade(colour, normal)), its lit
    sections are drawn unlit with their light baked into their vertices"""
    opa, xlu, data = [], [], []
    names = {}
    for n, tex in enumerate(imp.textures):
        names[tex.key] = (f"{prefix}Tex{n}", f"{prefix}Pal{n}")
        img = tex.image
        data.append((names[tex.key][0], tex.data, f"{img.name}: {img.width}x{img.height}, {tex.format}"))
        if tex.palette:
            data.append((names[tex.key][1], tex.palette, f"{img.name}'s palette: {len(tex.palette) // 2} colours, "
                         "RGBA16"))
    for s in imp.sections:
        lit = s.lit and shade is None
        tex_lines = []
        if s.texture is not None:
            tex_lines = _texture_lines(s.texture, *names[s.texture.key], s.wrap, s.filter)
        lines = [f"    // {s.name}: {len(s.triangles)} triangles, "
                 + ("lit" if lit else "unlit") + {"OPAQUE": "", "MASK": ", cut out", "BLEND": ", see-through"}[s.alpha]
                 + (", both sides" if s.double_sided else "")
                 + (f", {s.texture.format} {s.texture.image.width}x{s.texture.image.height}" if s.texture else "")]
        lines += _section_setup(s, lit, tex_lines)
        verts = []
        for tri in s.triangles:
            out = []
            for pos, st, colour, normal in tri:
                if lit:
                    c = tuple(max(-127, min(127, int(round(127 * v)))) & 0xFF for v in normal) + (255,)
                elif colour is not None:
                    c = colour
                elif shade is not None:
                    c = tuple(max(0, min(255, int(round(v)))) for v in shade((255, 255, 255), normal)) + (255,)
                else:
                    c = (255, 255, 255, 255)
                out.append(pos + st + c)
            verts.append(tuple(out))
        lines += _mesh_indexed(mesh, verts)
        if s.texture is not None and s.texture.palette:
            # The palette's look-up off again: what's drawn next (a room's scrolling texture) may not set it
            lines += ["    gsDPPipeSync(),", "    gsDPSetTextureLUT(G_TT_NONE),"]
        (xlu if s.alpha == "BLEND" else opa).extend(lines)
    return opa, xlu, data


def imported_c(imp, prefix, storage="static ", max_polys=MAX_POLYS):
    """An Imported as C (an actor's .inc.c with `storage` "static ", an object's file with ""): its textures and
    palettes, its vertices (<prefix>Vtx), its display list (<prefix>DL: opaque and cutout) and, when it has
    see-through faces, <prefix>XluDL; its collision (<prefix>Col, its camera list <prefix>ColCameras) when it has one.
    Its collision is a solid actor's (at most `max_polys` polygons: modelkit.collision). Returns (the C, [(C type,
    name) of each array and header it defines])"""
    mesh = Mesh(f"{prefix}Vtx")
    opa, xlu, data = imported_dl(imp, mesh, prefix)
    out, names = [], []
    import texkit  # noqa: E402

    for name, raw, what in data:
        out.append(texkit.emit_bytes(name, raw, storage, what))
        names.append(("u64", name))
    out.append(f"{storage}Vtx {mesh.name}[] = {{\n" + "\n".join(mesh.vtx) + "\n};\n")
    names.append(("Vtx", mesh.name))
    out.append(f"{storage}Gfx {prefix}DL[] = {{\n" + "\n".join(opa + ["    gsSPEndDisplayList(),"]) + "\n};\n")
    names.append(("Gfx", f"{prefix}DL"))
    if xlu:
        out.append(f"// Its see-through faces: drawn after the scene's solid ones (POLY_XLU_DISP)\n"
                   f"{storage}Gfx {prefix}XluDL[] = {{\n" + "\n".join(xlu + ["    gsSPEndDisplayList(),"]) + "\n};\n")
        names.append(("Gfx", f"{prefix}XluDL"))
    if imp.collision:
        out.append("// No camera of its own (modelkit.no_camera)\n" + no_camera(f"{prefix}ColCameras")
                   .replace("static ", storage, 1))
        col = collision(f"{prefix}Col", imp.collision, f"{prefix}ColCameras", max_polys=max_polys, storage=storage)
        out.append(col.text)
        names += [("BgCamInfo", f"{prefix}ColCameras"), ("CollisionHeader", f"{prefix}Col")]
    return "\n".join(out), names


def imported_shape(name, mod=None):
    """A solid piece's shape from tools/models/<name>.py (its output "solid"), for tools/gen_solid_pieces.py's SHAPES:
    a function returning its Imported (its model and collision)"""

    def shape():
        model = load_model(name, mod)
        if model.output != "solid":
            raise ValueError(f"tools/models/{name}.py's output is {model.output!r}, not \"solid\"")
        imp = imported(model)
        if not imp.collision:
            raise ValueError(f"tools/models/{name}.py: a solid piece needs collision (an object named collision, or "
                             "collision=\"model\")")
        return imp

    shape.__doc__ = f"The imported model {name} (tools/models/{name}.py)"
    return shape


def imported_room(name, mod=None):
    """A scene's room from tools/models/<name>.py (its output "room"): (its model parts, its collision parts), for a
    Room: Room(model=parts, collision=collision). Its parts are one, the Imported itself, which the room's file draws
    with its materials and textures (lit by the scene's light through the day, or with its baked light)"""
    model = load_model(name, mod)
    if model.output != "room":
        raise ValueError(f"tools/models/{name}.py's output is {model.output!r}, not \"room\"")
    imp = imported(model)
    return [(imp, [])], list(imp.collision)
