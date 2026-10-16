# Export one caster support as a print-oriented binary STL, with a watertight check.
#
# Set PART before exec'ing ('A' or 'B'); one body per call (meshing several in one script
# froze Fusion before). Rotated 180 deg about X so the top face (Z = 0, the face against the
# plate) lands on the bed; B's boss then points up. Every hole runs upright, the nut traps
# open upward and the countersinks narrow upward at 45 deg, so nothing needs supports.
exec(open(r'E:\Claude Stuff\CAD\scripts\cst_helpers.py').read())
import struct, os, collections
out = r'E:\Claude Stuff\CAD\designs\caster_support\stl'
os.makedirs(out, exist_ok=True)
body = occ('Support_' + PART).bRepBodies.item(0)
path = os.path.join(out, 'caster_support_%s.stl' % PART)

mc = body.meshManager.createMeshCalculator()
mc.surfaceTolerance = 0.001  # cm = 0.01 mm (design rules)
m = mc.calculate()
n = m.nodeCoordinatesAsDouble; idx = m.nodeIndices
P = [(n[i] * 10, -n[i + 1] * 10, -n[i + 2] * 10) for i in range(0, len(n), 3)]
zmin = min(p[2] for p in P)
cxm = (min(p[0] for p in P) + max(p[0] for p in P)) / 2; cym = (min(p[1] for p in P) + max(p[1] for p in P)) / 2
P = [(p[0] - cxm, p[1] - cym, p[2] - zmin) for p in P]
wid = {}; remap = []
for p in P:
    remap.append(wid.setdefault((round(p[0], 4), round(p[1], 4), round(p[2], 4)), len(wid)))
tris = [(remap[idx[i]], remap[idx[i + 1]], remap[idx[i + 2]]) for i in range(0, len(idx), 3)]
tris = [t for t in tris if len(set(t)) == 3]
P = [None] * len(wid)
for key, i in wid.items(): P[i] = key
ec = collections.Counter(); dc = collections.Counter()
for a, b, c in tris:
    for u, v in ((a, b), (b, c), (c, a)):
        ec[(min(u, v), max(u, v))] += 1; dc[(u, v)] += 1
open_edges = sum(1 for k, v in ec.items() if v != 2)
flipped = sum(1 for (u, v), k in dc.items() if k > 1 or dc.get((v, u), 0) != 1)
with open(path, 'wb') as f:
    f.write(('BLOC-BOT caster support ' + PART).encode().ljust(80, b' ')); f.write(struct.pack('<I', len(tris)))
    for a, b, c in tris:
        A, B, Cc = P[a], P[b], P[c]
        ux, uy, uz = B[0] - A[0], B[1] - A[1], B[2] - A[2]; vx, vy, vz = Cc[0] - A[0], Cc[1] - A[1], Cc[2] - A[2]
        nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx; L = math.sqrt(nx * nx + ny * ny + nz * nz) or 1
        f.write(struct.pack('<12fH', nx / L, ny / L, nz / L, *A, *B, *Cc, 0))
ext = [round(max(p[k] for p in P) - min(p[k] for p in P), 2) for k in range(3)]
print('%s tris %d size %s mm open edges %d bad orient %d %d bytes' % (
    os.path.basename(path), len(tris), ext, open_edges, flipped, os.path.getsize(path)))
