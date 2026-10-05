# Clearance / interference checks against the reference bodies (read-only).
#
# Re-run after changing any parameter. The reference parts (motor, disc, LM393, wheel) are
# placed at fixed positions and do NOT follow parameters; only the bracket does. Pass marks:
# disc-tab >= 2, wheel-bracket >= 3, wheel-plate >= 5, nut keep-out from disc/fork >= 3,
# and no overlap with the pillar hex tabs in plan view (screws are reached from below).
# Nuts and the M3x30 motor bolts are modelled as temporary bodies at every real plate hole
# that falls inside a slot.
exec(open(r'E:\Claude Stuff\CAD\scripts\helpers.py').read())
print('ACTIVE:', app.activeDocument.name)
mm = app.measureManager
T = adsk.fusion.TemporaryBRepManager.get()


def occ(name):
    return [o for o in root.occurrences if o.component.name == name][0]


def bodies_of(o):
    out = list(o.bRepBodies)
    for c in o.childOccurrences: out += bodies_of(c)
    return out


def tcopy(b):
    return T.copy(b)


def dist(a, b):
    return mm.measureMinimumDistance(a, b).value * 10


def tdist(ta, tb):
    # min distance between temporary bodies, via vertex/mesh-free approach: overlap volume first
    inter = T.copy(ta); T.booleanOperation(inter, tb, adsk.fusion.BooleanTypes.IntersectionBooleanType)
    return inter.volume * 1000


def inter_vol(a, b):
    ta, tb = tcopy(a), tcopy(b)
    T.booleanOperation(ta, tb, adsk.fusion.BooleanTypes.IntersectionBooleanType)
    return ta.volume * 1000  # mm3


br = occ('Bracket').bRepBodies.itemByName('Bracket_R')  # proxy in assembly context
plate = list(occ('REF_Plate').bRepBodies)[0]
wheel = list(occ('REF_Wheel').bRepBodies)[0]
motor = bodies_of(occ('REF_TT_Motor'))
disc = bodies_of(occ('REF_Disc'))[0]
lm = bodies_of(occ('REF_LM393'))
fork = [b for b in lm if 'Corps' in b.assemblyContext.component.name][0]
pcb = [b for b in lm if 'PCB' in b.assemblyContext.component.name][0]
R = {}
R['plate underside above floor'] = -(pv('shaft_z') - pv('wheel_d') / 2)
floor_z = pv('shaft_z') - pv('wheel_d') / 2
R['bracket lowest Z'] = bb(br)[2]
R['bracket lowest above floor'] = bb(br)[2] - floor_z
lm_low = min(bb(b)[2] for b in lm)
R['LM393 lowest above floor'] = lm_low - floor_z
R['disc-bracket min dist'] = dist(disc, br)
R['disc-fork min dist'] = dist(disc, fork)
# disc faces vs slot walls (slot centred on disc plane): gap each side
R['disc face to fork arm (each side)'] = (5.0 - pv('disc_t')) / 2
R['disc-fork overlap vol'] = inter_vol(disc, fork)
R['wheel-bracket min dist'] = dist(wheel, br)
R['wheel-plate min dist'] = dist(wheel, plate)
R['bracket-motor overlap vol'] = sum(inter_vol(br, m) for m in motor)
R['bracket-motor min dist'] = min(dist(br, m) for m in motor)
R['bracket-plate overlap vol'] = inter_vol(br, plate)
R['bracket-LM393 overlap vol'] = sum(inter_vol(br, b) for b in lm)
R['bracket-PCB min dist'] = dist(br, pcb)
R['fork-motor min dist'] = min(dist(fork, m) for m in motor)
R['LM393-motor overlap'] = sum(inter_vol(b, m) for b in lm for m in motor)
R['disc-motor overlap'] = sum(inter_vol(disc, m) for m in motor)
R['disc-PCB min dist'] = dist(disc, pcb)

# light path: aperture of the opto, in robot frame
ap_z0 = pv('slot_floor_z'); ap_z1 = ap_z0 + 6.0
win_r0, win_r1 = 6.61, 11.5
w0 = pv('shaft_z') - win_r1; w1 = pv('shaft_z') - win_r0
R['beam aperture Z'] = (round(ap_z0, 2), round(ap_z1, 2))
R['disc windows Z (below shaft)'] = (round(w0, 2), round(w1, 2))
R['window length inside beam'] = max(0, min(ap_z1, w1) - max(ap_z0, w0))

# plate holes under slots + nuts / bolts
holes = []
for f in plate.faces:
    g = f.geometry
    if isinstance(g, adsk.core.Cylinder) and abs(g.axis.z) > 0.99 and f.boundingBox.maxPoint.z - f.boundingBox.minPoint.z > 0.8:
        holes.append((round(g.origin.x * 10, 2), round(g.origin.y * 10, 2), round(g.radius * 20, 2)))
holes = sorted(set(holes))
hl = (pv('slot_len') - pv('slot_w')) / 2
slots = {'A': ('Y', pv('slotA_x'), pv('slotA_y')), 'B': ('Y', pv('slotB_x'), pv('slotB_y')), 'C': ('X', pv('slotC_x'), pv('slotC_y'))}


def in_slot(x, y, s):
    ax, cx, cy = s
    if ax == 'Y':
        dy = max(0, abs(y - cy) - hl); return math.hypot(x - cx, dy) <= pv('slot_w') / 2 - 0.3
    dx = max(0, abs(x - cx) - hl); return math.hypot(dx, y - cy) <= pv('slot_w') / 2 - 0.3


def cyl(p0, p1, r):
    return T.createCylinderOrCone(P3(*[v / 10 for v in p0]), r / 10, P3(*[v / 10 for v in p1]), r / 10)


# M3x30 motor bolts: head outside gearbox outer face, shank to X = gb_out_x - 30
bolts = []
for s in (1, -1):
    y = pv('shaft_y') - pv('tt_hole_back'); z = pv('shaft_z') + s * pv('tt_hole_dz')
    bolts.append(cyl((pv('gb_out_x') - 30, y, z), (pv('gb_out_x') + 3, y, z), 1.5))
avoid = {'disc': tcopy(disc), 'fork': tcopy(fork), 'pcb': tcopy(pcb)}
for i, m in enumerate(motor): avoid['motor%d' % i] = tcopy(m)
avoid['bolt_upper'], avoid['bolt_lower'] = bolts
brt = tcopy(br)
nut_rows = []
for nm, s in slots.items():
    for (x, y, d) in holes:
        if in_slot(x, y, s):
            nut = cyl((x, y, -pv('flange_t')), (x, y, -pv('flange_t') - 2.4), 3.2)  # M3 nut (corners) under flange
            washer = cyl((x, y, pv('plate_edge_x') * 0 + 10), (x, y, 10.6), 3.5)  # unused, plate top washer
            worst = []
            for k, tb in list(avoid.items()) + [('bracket', brt)]:
                ov = tdist(nut, tb)
                if ov > 1e-4: worst.append('%s overlap %.2f mm3' % (k, ov))
            # distance from nut to disc & fork (3D) using temporary body -> need real bodies: approximate via bodies' bbox gap
            nut_rows.append((nm, x, y, d, worst))
# disc to tab: faces of the bracket that belong to the tab (below the flange, at/after the tab inner face)
tab_faces = [f for f in br.faces if f.boundingBox.maxPoint.z * 10 < -pv('flange_t') - 0.5 and f.boundingBox.minPoint.x * 10 > pv('tab_in_x') - 0.05]
R['disc-tab min dist'] = min(dist(disc, f) for f in tab_faces)
relief = [f for f in br.faces if f.boundingBox.minPoint.z * 10 > -pv('flange_t') - 0.05 and f.boundingBox.maxPoint.z * 10 < -0.1
          and abs(f.boundingBox.minPoint.x * 10 - (pv('disc_x') - pv('disc_t') / 2 - pv('disc_cl'))) < 3]
R['disc-flange relief min dist'] = min(dist(disc, f) for f in relief) if relief else None
# 3 mm keep-out around each possible nut position
for i, r in enumerate(nut_rows):
    nm, x, y, d, worst = r
    keep = cyl((x, y, -pv('flange_t') + 0.01), (x, y, -pv('flange_t') - 2.4 - 3), 3.2 + 3)
    hits = [k for k, tb in (('disc', avoid['disc']), ('fork', avoid['fork']), ('pcb', avoid['pcb'])) if tdist(keep, tb) > 1e-4]
    nut_rows[i] = (nm, x, y, d, worst or 'no overlap', '3mm keep-out hits: %s' % (hits or 'none'))

# hex tabs reached from below: plan-view distance from motor/bracket mesh nodes to the hex outline
def hexpoly(cx, cy, r=10.0):
    return [(cx + r * math.cos(math.radians(a)), cy + r * math.sin(math.radians(a))) for a in range(0, 360, 60)]


def pt_in(p, poly_):
    x, y = p; c = False
    for i in range(len(poly_)):
        (x1, y1), (x2, y2) = poly_[i], poly_[(i + 1) % len(poly_)]
        if (y1 > y) != (y2 > y) and x < (x2 - x1) * (y - y1) / (y2 - y1) + x1: c = not c
    return c


def seg_d(p, a, b):
    (px, py), (ax, ay), (bx, by) = p, a, b
    dx, dy = bx - ax, by - ay; t = max(0, min(1, ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy)))
    return math.hypot(px - ax - t * dx, py - ay - t * dy)


def plan_pts(b, mirror=False):
    mc = b.meshManager.createMeshCalculator(); mc.setQuality(adsk.fusion.TriangleMeshQualityOptions.NormalQualityTriangleMesh)
    nodes = mc.calculate().nodeCoordinatesAsDouble
    return [((-1 if mirror else 1) * nodes[i] * 10, nodes[i + 1] * 10) for i in range(0, len(nodes), 3)]


hexes = [(sx * 95, sy * 13.66) for sx in (1, -1) for sy in (1, -1)] + [(sx * 13.66, sy * 95) for sx in (1, -1) for sy in (1, -1)]
for side, mir in (('right', False), ('left (mirrored)', True)):
    pts = []
    for b in motor + [br]: pts += plan_pts(b, mir)
    worst = 1e9
    for hc in hexes:
        hp = hexpoly(*hc)
        for p in pts:
            if pt_in(p, hp): worst = min(worst, -1)
            else: worst = min(worst, min(seg_d(p, hp[i], hp[(i + 1) % 6]) for i in range(6)))
    R['%s: plan clearance motor+bracket to pillar hex tabs' % side] = worst
# left side: mirror wheel and bracket across X=0 and test against the real plate
mir = adsk.core.Matrix3D.create(); mir.setCell(0, 0, -1)
wl = T.copy(wheel); T.transform(wl, mir)
bl = T.copy(br); T.transform(bl, mir)
grow = cyl((pv('gb_out_x') + pv('wheel_gap') - pv('wheel_edge_gap') + 0.01, pv('shaft_y'), pv('shaft_z')), (pv('gb_out_x') + pv('wheel_gap') + pv('wheel_w'), pv('shaft_y'), pv('shaft_z')), pv('wheel_d') / 2 + 0.0)
gl = T.copy(grow); T.transform(gl, mir)
R['left: wheel grown 5mm inward overlaps plate (mm3)'] = tdist(gl, tcopy(plate))
R['right: wheel grown 5mm inward overlaps plate (mm3)'] = tdist(grow, tcopy(plate))
R['left: bracket overlaps plate (mm3)'] = tdist(bl, tcopy(plate))
print('\n'.join('%-36s %s' % (k, (round(v, 2) if isinstance(v, float) else v)) for k, v in R.items()))
print('--- plate holes inside slots (nut check, M3 nut r3.2 x 2.4 under flange)')
for r in nut_rows: print(r)
