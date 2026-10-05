# Builds the parametric right-hand bracket in component "Bracket" (robot frame, mm params).
#
# Frame: origin = plate centre on its underside, X out toward the wheel, Y forward, Z up.
# Every dimension is a User Parameter, and every sketch is fully constrained, so editing a
# parameter in Fusion (Modify > Change Parameters) rebuilds the bracket. Re-running this
# script deletes the "Bracket" component and builds it again from the current parameters.
#
# Design decisions this script encodes (see README for the full story):
#  - Motor geometry comes from the "TT Motor Dual Shaft" model, not guesses: its flat face
#    (no bosses) is the INNER face, so the tab sits there. The left motor is therefore
#    mounted upside-down so the same face points inward.
#  - The LM393 cannot hang vertically at shaft height (it would hit the floor or the plate),
#    so it lies flat UNDER the disc with the fork pointing up at the disc's bottom rim, held
#    by one screw into a column. This replaced the original "sensor wall".
#  - The flange rests directly on the gearbox top: shaft_z = -(flange_t + gb_h/2) makes that
#    contact the height datum. The 0.3 mm clearance applies at the tab.
exec(open(r'E:\Claude Stuff\CAD\scripts\helpers.py').read())
print('ACTIVE:', app.activeDocument.name)

for o in list(root.occurrences):
    if o.component.name == 'Bracket':
        o.deleteMe()
bo = root.occurrences.addNewComponent(adsk.core.Matrix3D.create())
C = bo.component; C.name = 'Bracket'
XY = C.xYConstructionPlane
log = []

def chk(sk):
    log.append('%s: full=%s' % (sk.name, sk.isFullyConstrained))

# 1 flange (plate side flat at Z=0)
sk = C.sketches.add(XY); sk.name = 'flange'
rect(sk, {'X': 'flange_in_x', 'Y': 'flange_rear_y'}, {'X': 'gb_out_x', 'Y': 'flange_front_y'}); chk(sk)
body = extrude(C, sk, [sk.profiles.item(0)], FO.NewBodyFeatureOperation, 'flange_t', 'Z', -1).bodies.item(0)
body.name = 'Bracket_R'

# 2 motor tab, against gearbox inner face (with clearance), down to motor bottom.
#   Front edge stays tab_front_off (13) behind the shaft so it clears the Ø24 disc; rear
#   edge stops short of the motor's strap (model "Part 8", 28.15 behind the shaft), which
#   is why the flange rear also ends there.
sk = C.sketches.add(XY); sk.name = 'tab'
rect(sk, {'X': 'tab_in_x', 'Y': 'flange_rear_y'}, {'X': 'tab_in_x + tab_t', 'Y': 'shaft_y - tab_front_off'}); chk(sk)
extrude(C, sk, [sk.profiles.item(0)], FO.JoinFeatureOperation, 'flange_t + gb_h', 'Z', -1).name = 'tab'

# 3 LM393 column (board screws to its lower end).
#   The board runs FORWARD from the fork: running it rearward put the screw right where
#   the M3x30 motor-bolt tails stick out of the tab. A vertical column also prints
#   flange-down with no supports, unlike a cantilevered ledge.
sk = C.sketches.add(XY); sk.name = 'column'
circle(sk, {'X': 'disc_x - board_hole_dx', 'Y': 'board_hole_y'}, 'col_d'); chk(sk)
extrude(C, sk, [sk.profiles.item(0)], FO.JoinFeatureOperation, '-pcb_top_z', 'Z', -1).name = 'column'

# 4 joint fillets: flange-tab (inner + front edge) and flange-column.
#   Not on the tab's gearbox side: a fillet there would hit the gearbox's top edge.
fz = -pv('flange_t'); tx = pv('tab_in_x'); fy = pv('shaft_y') - pv('tab_front_off')
edges = adsk.core.ObjectCollection.create()
for e in body.edges:
    g = e.geometry
    if isinstance(g, adsk.core.Line3D):
        s, t = e.startVertex.geometry, e.endVertex.geometry
        if on(s.z, fz) and on(t.z, fz) and on(s.x, tx) and on(t.x, tx):
            edges.add(e)
        elif on(s.z, fz) and on(t.z, fz) and on(s.y, fy) and on(t.y, fy):
            edges.add(e)
    elif isinstance(g, adsk.core.Circle3D) and on(g.center.z, fz) and on(g.radius, pv('col_d') / 2):
        edges.add(e)
log.append('fillet edges %d' % edges.count)
fi = C.features.filletFeatures.createInput(); fi.addConstantRadiusEdgeSet(edges, VI('joint_fillet'), True)
C.features.filletFeatures.add(fi).name = 'joint_fillets'

# 5 gusset at the tab's rear end (clear of the bolt nuts and the disc)
pl = oplane(C, C.xZConstructionPlane, 'flange_rear_y', 'Y', 'gusset_plane')
sk = C.sketches.add(pl); sk.name = 'gusset'
poly(sk, [{'X': 'tab_in_x', 'Z': '-flange_t'}, {'X': 'tab_in_x - gusset_leg', 'Z': '-flange_t'},
          {'X': 'tab_in_x', 'Z': '-flange_t - gusset_leg'}]); chk(sk)
extrude(C, sk, [sk.profiles.item(0)], FO.JoinFeatureOperation, 'gusset_t', 'Y', +1).name = 'gusset'

# 6 TT bolt holes + hex nut traps on the tab's disc-side face.
#   The upper hole is only 2.5 mm below the flange, so a loose nut would hit the flange;
#   a trap (flats horizontal, so the top is a printable bridge) holds it captive instead.
pl = oplane(C, C.yZConstructionPlane, 'tab_in_x', 'X', 'tab_inner_plane')
sk = C.sketches.add(pl); sk.name = 'tt_holes'
for s in ('+', '-'):
    circle(sk, {'Y': 'shaft_y - tt_hole_back', 'Z': 'shaft_z %s tt_hole_dz' % s}, 'tt_hole_d')
chk(sk)
extrude(C, sk, list(sk.profiles), FO.CutFeatureOperation, 'tab_t + 1 mm', 'X', +1, [body]).name = 'tt_holes'
sk = C.sketches.add(pl); sk.name = 'nut_traps'
R = '((nut_af + 0.2 mm)/sqrt(3))'
for s in ('+', '-'):
    cy = '(shaft_y - tt_hole_back)'; cz = '(shaft_z %s tt_hole_dz)' % s; h = '%s*sqrt(3)/2' % R
    poly(sk, [{'Y': '%s + %s' % (cy, R), 'Z': cz}, {'Y': '%s + %s/2' % (cy, R), 'Z': '%s + %s' % (cz, h)},
              {'Y': '%s - %s/2' % (cy, R), 'Z': '%s + %s' % (cz, h)}, {'Y': '%s - %s' % (cy, R), 'Z': cz},
              {'Y': '%s - %s/2' % (cy, R), 'Z': '%s - %s' % (cz, h)}, {'Y': '%s + %s/2' % (cy, R), 'Z': '%s - %s' % (cz, h)}])
chk(sk)
extrude(C, sk, list(sk.profiles), FO.CutFeatureOperation, 'nut_depth', 'X', +1, [body]).name = 'nut_traps'

# 7 plate slots (rect + two end circles), cut through the flange.
#   Positions were picked from the waffle plate's real hole list so each slot passes over
#   real holes: A over two Ø3.2, B over three Ø4.5, C (side-to-side) over two Ø4.5. Slots
#   avoid the area over the motor (no room for a nut there) and keep every usable nut
#   >= 3 mm from the disc and fork. C was moved inboard (slotC_x 47) so the Ø4.5 hole at
#   (57, 64), which is within 3 mm of the disc, can't be used.
sk = C.sketches.add(XY); sk.name = 'slots'
hl = '((slot_len - slot_w)/2)'
for nm, along in (('A', 'Y'), ('B', 'Y'), ('C', 'X')):
    cx, cy = 'slot%s_x' % nm, 'slot%s_y' % nm
    if along == 'Y':
        rect(sk, {'X': '%s - slot_w/2' % cx, 'Y': '%s - %s' % (cy, hl)}, {'X': '%s + slot_w/2' % cx, 'Y': '%s + %s' % (cy, hl)})
        for s in ('-', '+'): circle(sk, {'X': cx, 'Y': '%s %s %s' % (cy, s, hl)}, 'slot_w')
    else:
        rect(sk, {'X': '%s - %s' % (cx, hl), 'Y': '%s - slot_w/2' % cy}, {'X': '%s + %s' % (cx, hl), 'Y': '%s + slot_w/2' % cy})
        for s in ('-', '+'): circle(sk, {'X': '%s %s %s' % (cx, s, hl), 'Y': cy}, 'slot_w')
chk(sk)
cut = extrude(C, sk, list(sk.profiles), FO.CutFeatureOperation, 'flange_t', 'Z', -1, [body]); cut.name = 'slots'

# 8 slot edge chamfers (top + bottom rims of every slot)
ce = adsk.core.ObjectCollection.create(); seen = set()
for f in cut.faces:
    for e in f.edges:
        b = e.boundingBox
        if abs(b.maxPoint.z - b.minPoint.z) < 1e-6 and (on(b.minPoint.z, 0) or on(b.minPoint.z, fz)) and e.tempId not in seen:
            seen.add(e.tempId); ce.add(e)
log.append('chamfer edges %d' % ce.count)
chi = C.features.chamferFeatures.createInput2()
chi.chamferEdgeSets.addEqualDistanceChamferEdgeSet(ce, VI('slot_chamfer'), True)
C.features.chamferFeatures.add(chi).name = 'slot_chamfers'

# 8b relief pocket in the flange underside: the Ø24 disc rises 0.75 mm above the gearbox
#    top (radius 12 vs half-height 11.25), so without this pocket it would rub the flange.
sk = C.sketches.add(oplane(C, XY, '-flange_t', 'Z', 'flange_under_plane')); sk.name = 'disc_relief'
ph = 'sqrt((disc_d/2 + disc_cl)^2 - (gb_h/2)^2)'
rect(sk, {'X': 'disc_x - disc_t/2 - disc_cl', 'Y': 'shaft_y - %s' % ph}, {'X': 'disc_x + disc_t/2 + disc_cl', 'Y': 'shaft_y + %s' % ph}); chk(sk)
extrude(C, sk, [sk.profiles.item(0)], FO.CutFeatureOperation, 'disc_d/2 - gb_h/2 + disc_cl', 'Z', +1, [body]).name = 'disc_relief'

# 9 M3 self-tap pilot in the column end. The sensor model's board hole is Ø2.7, so check
#   that an M3 fits your board; if not, use an M2.5 screw.
pl = oplane(C, XY, 'pcb_top_z', 'Z', 'pcb_top_plane')
sk = C.sketches.add(pl); sk.name = 'pilot'
circle(sk, {'X': 'disc_x - board_hole_dx', 'Y': 'board_hole_y'}, 'pilot_d'); chk(sk)
extrude(C, sk, [sk.profiles.item(0)], FO.CutFeatureOperation, 'pilot_depth', 'Z', +1, [body]).name = 'pilot'

print('\n'.join(log))
print('body', bb(body), 'faces', body.faces.count, 'solid', body.isSolid, 'vol cm3', round(body.volume, 3))
print('timeline health', [(f.name, f.healthState) for f in C.features if f.healthState != 0])
