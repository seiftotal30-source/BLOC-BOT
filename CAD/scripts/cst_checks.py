# Checks for both caster supports. Re-run after cst_build.py.
#
# Pass marks: caster wheel bottom within 0.5 mm of the floor (the drive wheels' contact);
# no overlap with the plate or the caster; each fixing hole over a real plate
# hole that an M3 passes; nut traps >= 1 mm from a pillar or the boss; top face at least
# 50 % on solid plate (the rest is over plate holes; the motor bracket's flange area is 38 %).
exec(open(r'E:\Claude Stuff\CAD\scripts\cst_helpers.py').read())
print('ACTIVE:', app.activeDocument.name)

plate = occ('REF_Plate').bRepBodies.item(0)
floor = -pv('floor_drop')
fix = [(s * pv('fix_rx'), pv('fix_ry')) for s in (-1, 1)] + [(s * pv('fix_fx'), pv('fix_fy')) for s in (-1, 1)]
cx, cy = pv('cst_x'), pv('cst_y')
cst = [(cx + sx * pv('cst_sx') / 2, cy + sy * pv('cst_sy') / 2) for sx in (-1, 1) for sy in (-1, 1)]
R = []

# plate holes the fixing screws pass through
holes = []
for f in plate.faces:
    g = f.geometry
    if isinstance(g, adsk.core.Cylinder) and on(abs(g.axis.z), 1):
        holes.append((g.origin.x * 10, g.origin.y * 10, g.radius * 20))
for x, y in fix:
    h = min(holes, key=lambda h: math.hypot(h[0] - x, h[1] - y))
    R.append(('fix (%g, %g): plate hole dia, offset' % (x, y), '%.2f, %.2f' % (h[2], math.hypot(h[0] - x, h[1] - y)),
              'dia >= 3.3, offset 0'))

for v in ('A', 'B'):
    sup = occ('Support_' + v).bRepBodies.item(0)
    cas = occ('REF_Caster_' + v).bRepBodies.item(0)
    low = cas.boundingBox.minPoint.z * 10
    R.append((v + ': wheel bottom - floor', round(low - floor, 2), '-0.5 .. +0.5 (minus = caster pushes the rear up)'))
    R.append((v + ': overlap with plate (mm3)', inter_vol(sup, plate), 0))
    R.append((v + ': overlap with caster (mm3)', inter_vol(sup, cas), 0))
    R.append((v + ': support min z / total height', '%.2f / %.2f' % (sup.boundingBox.minPoint.z * 10,
              sup.boundingBox.maxPoint.z * 10 - low), ''))
    # contact: top face area on solid plate
    top = sum(f.area for f in sup.faces if isinstance(f.geometry, adsk.core.Plane)
              and on(f.boundingBox.maxPoint.z, 0) and on(f.boundingBox.minPoint.z, 0)) * 100
    slab = TB.copy(sup)
    TB.booleanOperation(slab, tbox(-200, -200, -0.2, 200, 200, 0), adsk.fusion.BooleanTypes.IntersectionBooleanType)
    tr = adsk.core.Matrix3D.create(); tr.translation = V3(0, 0, 0.02); TB.transform(slab, tr)  # top 0.2 mm, lifted into the plate
    on_plate = inter_vol(slab, plate) / 0.2
    R.append((v + ': top face on solid plate', '%.0f %% of %.0f mm2' % (100 * on_plate / top, top), '>= 50 % (bracket flange area: 38 %)'))
    R.append((v + ': volume (cm3)', round(sup.volume, 2), ''))


# nut traps (corner radius) against pillar corners and the boss outline, in plan
nr = (pv('nut_af') + pv('nut_cl')) / math.sqrt(3)
pr = pv('pillar_d') / 2
d_pil = min(math.hypot(x - px, y - py) - nr - pr for x, y in fix for px, py in cst)
hw, hd = pv('cst_pw') / 2, pv('cst_pd') / 2
d_boss = min(math.hypot(max(abs(x - cx) - hw, 0), max(abs(y - cy) - hd, 0)) - nr for x, y in fix)
R.append(('A: nut trap to pillar, plan (mm)', round(d_pil, 2), '>= 1'))
R.append(('B: nut trap to boss, plan (mm)', round(d_boss, 2), '>= 1'))
R.append(('nut trap floor thickness (mm)', round(pv('flange_t') - pv('nut_depth'), 2), ''))

for k, val, req in R:
    print('%-48s %-22s %s' % (k, val, req))
