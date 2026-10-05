# Export print-oriented binary STLs (flange's plate face on the bed) + watertight check.
#
# Fusion's own STL export keeps model orientation (bracket hanging upside-down), so this
# writes the STL itself: rotate 180 deg about X so the flange's plate face (Z=0) lands on the
# bed, then centre it and drop it to Z=0. In that orientation nothing needs supports; the only
# overhangs are the tops of the Ø3.4 holes and nut traps, which bridge.
exec(open(r'E:\Claude Stuff\CAD\scripts\helpers.py').read())
import struct, os, collections
print('ACTIVE:', app.activeDocument.name)
out = r'E:\Claude Stuff\CAD\out'
os.makedirs(out, exist_ok=True)


def write_stl(body, path):
    mc = body.meshManager.createMeshCalculator()
    mc.setQuality(adsk.fusion.TriangleMeshQualityOptions.VeryHighQualityTriangleMesh)
    m = mc.calculate()
    n = m.nodeCoordinatesAsDouble; idx = m.nodeIndices
    # print orientation: rotate 180 deg about X (plate-side face Z=0 -> bed), mm units
    P = [(n[i] * 10, -n[i + 1] * 10, -n[i + 2] * 10) for i in range(0, len(n), 3)]
    zmin = min(p[2] for p in P); cx = (min(p[0] for p in P) + max(p[0] for p in P)) / 2; cy = (min(p[1] for p in P) + max(p[1] for p in P)) / 2
    P = [(p[0] - cx, p[1] - cy, p[2] - zmin) for p in P]
    # weld coincident nodes (the mesher duplicates nodes along face boundaries)
    wid = {}; remap = []
    for p in P:
        key = (round(p[0], 4), round(p[1], 4), round(p[2], 4))
        remap.append(wid.setdefault(key, len(wid)))
    tris = [(remap[idx[i]], remap[idx[i + 1]], remap[idx[i + 2]]) for i in range(0, len(idx), 3)]
    tris = [t for t in tris if len(set(t)) == 3]
    P = [None] * len(wid)
    for key, i in wid.items(): P[i] = key
    # watertight: every undirected edge used exactly twice, and consistently oriented
    ec = collections.Counter(); dc = collections.Counter()
    for a, b, c in tris:
        for u, v in ((a, b), (b, c), (c, a)):
            ec[(min(u, v), max(u, v))] += 1; dc[(u, v)] += 1
    open_edges = sum(1 for k, v in ec.items() if v != 2)
    flipped = sum(1 for (u, v), k in dc.items() if k > 1 or dc.get((v, u), 0) != 1)
    with open(path, 'wb') as f:
        f.write(b'BLOC-BOT motor bracket'.ljust(80, b' ')); f.write(struct.pack('<I', len(tris)))
        for a, b, c in tris:
            A, B, Cc = P[a], P[b], P[c]
            ux, uy, uz = B[0] - A[0], B[1] - A[1], B[2] - A[2]; vx, vy, vz = Cc[0] - A[0], Cc[1] - A[1], Cc[2] - A[2]
            nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx; L = math.sqrt(nx * nx + ny * ny + nz * nz) or 1
            f.write(struct.pack('<12fH', nx / L, ny / L, nz / L, *A, *B, *Cc, 0))
    ext = [round(max(p[k] for p in P) - min(p[k] for p in P), 2) for k in range(3)]
    # bed contact: area of downward faces at z=0
    bed = 0.0
    for a, b, c in tris:
        A, B, Cc = P[a], P[b], P[c]
        if max(A[2], B[2], Cc[2]) < 1e-3:
            bed += abs((B[0] - A[0]) * (Cc[1] - A[1]) - (Cc[0] - A[0]) * (B[1] - A[1])) / 2
    print('%-16s tris %6d  size %s mm  open edges %d  bad orient %d  bed contact %.0f mm2  %d bytes' % (
        os.path.basename(path), len(tris), ext, open_edges, flipped, bed, os.path.getsize(path)))


for name, fn in (('Bracket_R', 'bracket_right.stl'), ('Bracket_L', 'bracket_left.stl'), ('test_tab', 'test_tab.stl')):
    write_stl(C.bRepBodies.itemByName(name), os.path.join(out, fn))
