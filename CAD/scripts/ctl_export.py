# Export print-oriented binary STLs + watertight check (same mesher/welder as export_stl.py).
#   controller_top.stl    : top face down (rotate 180 deg about X); bosses and posts grow upward
#   controller_bottom.stl : floor flat on the bed, as modelled
#   controller_caps.stl   : 6 caps, flange down, in a row 16 mm apart
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
import struct, os, collections
print('ACTIVE:', app.activeDocument.name)
out = r'E:\Claude Stuff\CAD\out'
os.makedirs(out, exist_ok=True)


def mesh(body, flip, dx=0.0):
    mc = body.meshManager.createMeshCalculator()
    mc.setQuality(adsk.fusion.TriangleMeshQualityOptions.VeryHighQualityTriangleMesh)
    m = mc.calculate(); n = m.nodeCoordinatesAsDouble; idx = m.nodeIndices
    if flip: P = [(n[i] * 10, -n[i + 1] * 10, -n[i + 2] * 10) for i in range(0, len(n), 3)]
    else: P = [(n[i] * 10, n[i + 1] * 10, n[i + 2] * 10) for i in range(0, len(n), 3)]
    zmin = min(p[2] for p in P); cx = (min(p[0] for p in P) + max(p[0] for p in P)) / 2; cy = (min(p[1] for p in P) + max(p[1] for p in P)) / 2
    P = [(p[0] - cx + dx, p[1] - cy, p[2] - zmin) for p in P]
    return P, [(idx[i], idx[i + 1], idx[i + 2]) for i in range(0, len(idx), 3)]


def write_stl(parts, path, title):
    """parts: list of (points, tris); welds each part, checks it is closed, writes one STL."""
    allP, allT, open_e, bad, bed = [], [], 0, 0, 0.0
    for P, T0 in parts:
        wid = {}; remap = []
        for p in P:
            remap.append(wid.setdefault((round(p[0], 4), round(p[1], 4), round(p[2], 4)), len(wid)))
        tris = [(remap[a], remap[b], remap[c_]) for a, b, c_ in T0]
        tris = [t for t in tris if len(set(t)) == 3]
        Q = [None] * len(wid)
        for key, i in wid.items(): Q[i] = key
        ec = collections.Counter(); dc = collections.Counter()
        for a, b, c_ in tris:
            for u, v in ((a, b), (b, c_), (c_, a)):
                ec[(min(u, v), max(u, v))] += 1; dc[(u, v)] += 1
        open_e += sum(1 for v in ec.values() if v != 2)
        bad += sum(1 for (u, v), k in dc.items() if k > 1 or dc.get((v, u), 0) != 1)
        off = len(allP); allP += Q; allT += [(a + off, b + off, c_ + off) for a, b, c_ in tris]
    with open(path, 'wb') as f:
        f.write(title.encode().ljust(80, b' ')); f.write(struct.pack('<I', len(allT)))
        for a, b, c_ in allT:
            A, B, Cc = allP[a], allP[b], allP[c_]
            ux, uy, uz = B[0] - A[0], B[1] - A[1], B[2] - A[2]; vx, vy, vz = Cc[0] - A[0], Cc[1] - A[1], Cc[2] - A[2]
            nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx; L = math.sqrt(nx * nx + ny * ny + nz * nz) or 1
            f.write(struct.pack('<12fH', nx / L, ny / L, nz / L, *A, *B, *Cc, 0))
            if max(A[2], B[2], Cc[2]) < 1e-3:
                bed += abs((B[0] - A[0]) * (Cc[1] - A[1]) - (Cc[0] - A[0]) * (B[1] - A[1])) / 2
    ext = [round(max(p[k] for p in allP) - min(p[k] for p in allP), 2) for k in range(3)]
    print('%-22s tris %6d  size %s mm  open edges %d  bad orient %d  bed contact %.0f mm2  %d bytes' % (
        os.path.basename(path), len(allT), ext, open_e, bad, bed, os.path.getsize(path)))


# ONLY = 'top' / 'bottom' / 'caps' exports one file per call (big exports have hung Fusion)
ONLY = globals().get('ONLY')
Cc = occ('Controller_Case').component
if ONLY in (None, 'top'):
    write_stl([mesh(Cc.bRepBodies.itemByName('controller_top'), True)], os.path.join(out, 'controller_top.stl'), 'BLOC-BOT controller top')
if ONLY in (None, 'bottom'):
    write_stl([mesh(Cc.bRepBodies.itemByName('controller_bottom'), False)], os.path.join(out, 'controller_bottom.stl'), 'BLOC-BOT controller bottom')
if ONLY in (None, 'caps'):
    K = occ('Controller_Caps').component
    write_stl([mesh(b, False, i * 16.0) for i, b in enumerate(K.bRepBodies)], os.path.join(out, 'controller_caps.stl'), 'BLOC-BOT controller caps')
