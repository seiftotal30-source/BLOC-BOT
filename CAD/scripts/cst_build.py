# Builds both rear caster supports and their reference parts from the User Parameters.
# Re-running deletes and rebuilds every component except REF_Plate.
#
#   Support_A  4 mm plate with four printed pillars; the caster screws to the pillars.
#   Support_B  the same plate with a solid boss under it; the caster screws straight on.
#   REF_Caster_A / REF_Caster_B  stand-ins for the bought parts (GrabCAD
#              sizes, "(assumed)" details from the photo), for the checks and the pictures.
#
# Design decisions:
#  - Both supports bolt flat to the plate underside like the motor bracket flange: four M3
#    screws come down through real Ø4.5 plate holes, (+-27, -71) and (+-7, -34), into
#    nut traps on the support's underside, so no spanner is needed under the robot. All four
#    sit outside the caster's fixing plate, so version B's boss never covers a nut.
#  - The caster screws are M3 countersunk from the support's top face: that face is clamped
#    against the plate, so the heads must be flush; nuts go under the caster plate. Fit the caster to the
#    support first, then bolt the support to the plate.
#  - Prints top face down on the bed (B: boss up), no supports. Upright plan corners are
#    rounded (R4 flange, R3 boss = caster plate); only hole and trap rims are chamfered 0.4
#    (design rules).
exec(open(r'E:\Claude Stuff\CAD\scripts\cst_helpers.py').read())
print('ACTIVE:', app.activeDocument.name)

ft = pv('flange_t'); cx, cy = pv('cst_x'), pv('cst_y')
fix = [(s * pv('fix_rx'), pv('fix_ry')) for s in (-1, 1)] + [(s * pv('fix_fx'), pv('fix_fy')) for s in (-1, 1)]
cst = [(cx + sx * pv('cst_sx') / 2, cy + sy * pv('cst_sy') / 2) for sx in (-1, 1) for sy in (-1, 1)]
outline = [(-pv('sup_hw'), pv('sup_rear_y')), (pv('sup_hw'), pv('sup_rear_y')),
           (pv('sup_hw'), pv('sup_taper_y')), (pv('sup_front_hw'), pv('sup_front_y')),
           (-pv('sup_front_hw'), pv('sup_front_y')), (-pv('sup_hw'), pv('sup_taper_y'))]
csk_depth = (pv('csk_d') - pv('screw_d')) / 2  # 90 deg countersink
log = []


def support(name, boss):
    C = new_comp(name).component
    sk = sketch(C, 0, 'outline'); poly(sk, outline)
    body = extrude(C, profiles(sk), ft, False, FO.NewBodyFeatureOperation, name='flange').bodies.item(0)
    body.name = name
    fillet(C, upright_edges(body, outline, 0, -ft), pv('sup_corner_r'), 'flange_corners')
    depth = ft
    if boss:
        bh = pv('boss_h'); depth = ft + bh
        hw, hd = pv('cst_pw') / 2, pv('cst_pd') / 2
        rect = [(cx - hw, cy - hd), (cx + hw, cy - hd), (cx + hw, cy + hd), (cx - hw, cy + hd)]
        sk = sketch(C, -ft, 'boss'); poly(sk, rect)
        extrude(C, profiles(sk), bh, False, FO.JoinFeatureOperation, [body], 'boss')
        fillet(C, upright_edges(body, rect, -ft, -depth), pv('cst_pr'), 'boss_corners')
    else:
        depth = ft + pv('pillar_l')
        sk = sketch(C, -ft, 'pillars')
        for x, y in cst: circle(sk, x, y, pv('pillar_d'))
        extrude(C, profiles(sk), pv('pillar_l'), False, FO.JoinFeatureOperation, [body], 'pillars')
    # plate screws + nut traps
    sk = sketch(C, 0, 'fix_holes')
    for x, y in fix: circle(sk, x, y, pv('screw_d'))
    extrude(C, profiles(sk), ft, False, FO.CutFeatureOperation, [body], 'fix_holes')
    sk = sketch(C, -ft, 'nut_traps')
    for x, y in fix: hexagon(sk, x, y, pv('nut_af') + pv('nut_cl'))
    extrude(C, profiles(sk), pv('nut_depth'), True, FO.CutFeatureOperation, [body], 'nut_traps')
    # caster screws: through hole + 90 deg countersink at the top face
    sk = sketch(C, 0, 'caster_holes')
    for x, y in cst: circle(sk, x, y, pv('screw_d'))
    extrude(C, profiles(sk), depth, False, FO.CutFeatureOperation, [body], 'caster_holes')
    sk = sketch(C, 0, 'countersinks')
    for x, y in cst: circle(sk, x, y, pv('csk_d'))
    ei = C.features.extrudeFeatures.createInput(profiles(sk), FO.CutFeatureOperation)
    ei.setOneSideExtent(adsk.fusion.DistanceExtentDefinition.create(VI('%r mm' % csk_depth)),
                        ED.NegativeExtentDirection, VI('-45 deg'))
    ei.participantBodies = [body]
    C.features.extrudeFeatures.add(ei).name = 'countersinks'
    # entry chamfers: plate-screw rims on top, nut-trap rims, caster-hole rims underneath
    edges = adsk.core.ObjectCollection.create()
    for col in (rim_edges(body, 0, radius=pv('screw_d') / 2, pts=fix),
                rim_edges(body, -ft, hex_af=pv('nut_af') + pv('nut_cl'), pts=fix),
                rim_edges(body, -depth, radius=pv('screw_d') / 2, pts=cst)):
        for e in col: edges.add(e)
    log.append('%s chamfer edges %d (expect %d)' % (name, edges.count, 4 + 24 + 4))
    chamfer(C, edges, pv('rim_ch'), 'entry_chamfers')
    csk_ok = len(rim_edges(body, -csk_depth, radius=pv('screw_d') / 2, pts=cst))
    log.append('%s countersink bottoms found %d (expect 4)' % (name, csk_ok))
    log.append('%s bbox %s vol %.2f cm3 solid %s' % (name, bb(body), body.volume, body.isSolid))
    return body


def caster_model(top_z):
    """Stand-in for the nylon caster, fixing plate top at top_z, wheel trailing to the rear."""
    hw, hd, pt = pv('cst_pw') / 2, pv('cst_pd') / 2, pv('cst_pt')
    b = tbox(cx - hw, cy - hd, top_z - pt, cx + hw, cy + hd, top_z)
    for x, y in cst: tcut(b, tcyl((x, y, top_z + 1), (x, y, top_z - pt - 1), pv('cst_hole_d')))
    tunion(b, tcyl((cx, cy, top_z - pt), (cx, cy, top_z - pt - 5), 28))  # swivel housing (assumed)
    wr, ww = pv('cst_wheel_d') / 2, pv('cst_wheel_w')
    wy, wz = cy - pv('cst_trail'), top_z - pv('cst_h') + wr
    for s in (-1, 1):  # fork legs (assumed)
        x0 = cx + s * (ww / 2 + 0.5); x1 = cx + s * (ww / 2 + 2.5)
        tunion(b, tbox(min(x0, x1), wy - 5, wz - 3, max(x0, x1), cy + 5, top_z - pt - 5))
    tunion(b, tcyl((cx - ww / 2 - 3.5, wy, wz), (cx + ww / 2 + 3.5, wy, wz), 4))  # axle bolt
    tunion(b, tcyl((cx - ww / 2, wy, wz), (cx + ww / 2, wy, wz), 2 * wr))  # wheel
    return b


bA = support('Support_A', False)
bB = support('Support_B', True)

top_a = -ft - pv('pillar_l')
add_base_body(new_comp('REF_Caster_A').component, caster_model(top_a), 'Caster_A')
add_base_body(new_comp('REF_Caster_B').component, caster_model(-ft - pv('boss_h')), 'Caster_B')
o = occ('REF_Pillars_A')
if o: o.deleteMe()

for n in ('Support_B', 'REF_Caster_B'):
    occ(n).isLightBulbOn = False
print('\n'.join(log))
print('unhealthy', [(o.component.name, f.name) for o in root.occurrences for f in o.component.features
                    if hasattr(f, 'healthState') and f.healthState not in (0,)])
