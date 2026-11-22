#!/usr/bin/env python3
"""
Voxel3D-PS3 :: OBJ -> VX3 (Big-Endian)
Formato de vertice (12 bytes): >3h3b3B
  int16  x, y, z     posicion Q8.8  (coord * 256)
  int8   nx,ny,nz    normal empaquetada (-128..127)
  uint8  flags       indice de material (0..2)
  uint8  u, v        UV cuantizado 0..255
Indices: uint16 big-endian.
"""
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OBJ  = r"D:\Descargas de OperaGX\M4A1.obj"
MTL  = r"D:\Descargas de OperaGX\M4A1.mtl"
OUT  = os.path.join(ROOT, "build_ps3", "assets", "m4a1.vx3")

MAX_VERTS, MAX_INDICES, Q = 2500, 3750, 256


def clamp(v, lo, hi):
    return lo if v < lo else hi if v > hi else v


def parse_mtl(path):
    mats, name, kd = [], None, (0.5, 0.5, 0.5)
    if os.path.exists(path):
        for line in open(path, "r", errors="replace"):
            s = line.strip()
            if s.startswith("newmtl "):
                if name: mats.append(name)
                name, kd = s.split(None, 1)[1], (0.5, 0.5, 0.5)
            elif s.startswith("Kd "):
                try: kd = tuple(float(x) for x in s.split()[1:4])
                except ValueError: pass
        if name: mats.append(name)
    return mats or ["default"]


def resolve(i, n):
    return None if not i else (int(i) - 1 if int(i) > 0 else n + int(i))


def main():
    mats = parse_mtl(MTL)
    midx = {m: i for i, m in enumerate(mats)}
    pos, uv, nrm, faces = [], [], [], []
    cur = 0
    for line in open(OBJ, "r", errors="replace"):
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        p = s.split()
        t = p[0]
        try:
            if t == "v":   pos.append(tuple(float(x) for x in p[1:4]))
            elif t == "vt": uv.append(tuple(float(x) for x in p[1:3]))
            elif t == "vn": nrm.append(tuple(float(x) for x in p[1:4]))
            elif t == "usemtl": cur = midx.get(p[1], 0)
            elif t == "f":
                c = []
                for tok in p[1:]:
                    b = tok.split("/")
                    c.append((resolve(b[0], len(pos)),
                              resolve(b[1], len(uv)) if len(b) > 1 else None,
                              resolve(b[2], len(nrm)) if len(b) > 2 else None))
                faces.append((c, cur))
        except (ValueError, IndexError):
            pass

    tris = []
    for c, m in faces:
        for i in range(1, len(c) - 1):
            tris.append(((c[0], c[i], c[i + 1]), m))

    def face_normal(tri):
        a, b, c = [pos[x[0]] for x in tri]
        ux, uy, uz = b[0]-a[0], b[1]-a[1], b[2]-a[2]
        vx, vy, vz = c[0]-a[0], c[1]-a[1], c[2]-a[2]
        nx, ny, nz = uy*vz-uz*vy, uz*vx-ux*vz, ux*vy-uy*vx
        l = (nx*nx+ny*ny+nz*nz) ** 0.5
        return (nx/l, ny/l, nz/l) if l > 1e-9 else (0.0, 1.0, 0.0)

    verts, seen, idx = [], {}, []

    for tri, m in tris:
        fn = face_normal(tri)
        for pi, ui, ni in tri:
            if pi is None:
                continue
            x, y, z = pos[pi]
            n = nrm[ni] if ni is not None else fn
            uu, vv = uv[ui] if ui is not None else (0.0, 0.0)
            vb = struct.pack(">3h3b3B",
                clamp(int(round(x*Q)), -32768, 32767),
                clamp(int(round(y*Q)), -32768, 32767),
                clamp(int(round(z*Q)), -32768, 32767),
                clamp(int(round(n[0]*127)), -128, 127),
                clamp(int(round(n[1]*127)), -128, 127),
                clamp(int(round(n[2]*127)), -128, 127),
                m & 0xFF,
                clamp(int(round(uu*255)), 0, 255),
                clamp(int(round(vv*255)), 0, 255))
            j = seen.get(vb)
            if j is None:
                j = len(verts); seen[vb] = j; verts.append(vb)
            idx.append(j)

    if len(verts) > MAX_VERTS or len(idx) > MAX_INDICES:
        print("ERROR: excede presupuesto (%d/%d, %d/%d)" %
              (len(verts), MAX_VERTS, len(idx), MAX_INDICES), file=sys.stderr)
        return 1

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    hdr = struct.pack(">4sHHII", b"VX3D", 1, 0, len(verts), len(idx))
    with open(OUT, "wb") as f:
        f.write(hdr)
        f.writelines(verts)
        for i in idx:
            f.write(struct.pack(">H", i))

    print("OBJ: %d caras -> %d triangulos" % (len(faces), len(tris)))
    print("VX3: %d vertices (max %d), %d indices (max %d)" %
          (len(verts), MAX_VERTS, len(idx), MAX_INDICES))
    print("formato vertice: >3h3b3B = 12 bytes, Big-Endian")
    print("ESCRITO: %s (%d bytes / %.1f KB)" %
          (OUT, os.path.getsize(OUT), os.path.getsize(OUT)/1024.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
