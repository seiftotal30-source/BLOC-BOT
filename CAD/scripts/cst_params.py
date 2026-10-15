# Caster Mount User Parameters (all mm). Re-run after editing, then re-run cst_build.py.
#
# Two versions of the rear caster support, both bolted flat to the plate underside like the
# motor bracket flange:
#   A "pillars": a 4 mm plate (the bracket flange's thickness) with four printed pillars;
#     the caster screws to the pillars with M3 (the metal pillars are not used).
#   B "direct": no pillars; a solid boss under the same plate brings the caster down so that
#     support + caster = plate underside to floor (bracket + motor + wheel).
# "(GrabCAD)" values are from the caster's page (grabcad.com/library/nylon-caster-wheel-1);
# "(assumed)" values had no source and must be measured on the real parts.
exec(open(r'E:\Claude Stuff\CAD\scripts\cst_helpers.py').read())
print('ACTIVE:', app.activeDocument.name)

P = [
    # --- robot (from the motor bracket design) ---
    ('floor_drop', '47.75', 'plate underside above the floor: bracket + TT motor + Ø65 wheel'),
    ('flange_t', '4', 'support plate thickness = the motor bracket flange (professional 3MF)'),
    # --- caster (GrabCAD) ---
    ('cst_h', '34', 'overall height, top of the fixing plate to the floor (GrabCAD)'),
    ('cst_pw', '38', 'fixing plate size across the robot, X (GrabCAD)'),
    ('cst_pd', '32', 'fixing plate size front-back, Y (GrabCAD)'),
    ('cst_sx', '30', 'hole spacing X (GrabCAD)'),
    ('cst_sy', '23', 'hole spacing Y (GrabCAD)'),
    ('cst_hole_d', '4', 'fixing hole dia (GrabCAD)'),
    ('cst_wheel_d', '25.5', 'wheel dia (GrabCAD)'),
    ('cst_pt', '2.5', 'fixing plate thickness (assumed)'),
    ('cst_pr', '3', 'fixing plate corner radius (assumed)'),
    ('cst_wheel_w', '15', 'wheel width (assumed, from the photo)'),
    ('cst_trail', '8', 'swivel axis to wheel axle, in plan (assumed)'),
    # --- where the caster goes ---
    ('cst_x', '0', 'caster centre X: on the robot centre line'),
    ('cst_y', '-70', 'caster centre Y: rear, plate edge is at -89.75'),
    # --- support plate outline (plan) ---
    ('sup_hw', '32', 'half width of the rear part'),
    ('sup_rear_y', '-88', 'rear edge (plate edge -89.75)'),
    ('sup_front_y', '-29', 'front edge'),
    ('sup_taper_y', '-66', 'where the sides start to taper in toward the front'),
    ('sup_front_hw', '14', 'half width at the front edge'),
    ('sup_corner_r', '4', 'upright plan corners, as the bracket flange (R4)'),
    # --- fixing to the waffle plate: M3 screws down through Ø4.5 plate holes, nut traps ---
    ('fix_rx', '27', 'rear screws at (+-27, -71): Ø4.5 plate holes (plate turned 90 deg)'),
    ('fix_ry', '-71', ''),
    ('fix_fx', '7', 'front screws at (+-7, -34): Ø4.5 plate holes'),
    ('fix_fy', '-34', ''),
    ('screw_d', '3.3', 'M3 clearance (design rules: 0.15 per side)'),
    ('nut_af', '5.5', 'M3 nut across flats'),
    ('nut_cl', '0.2', 'nut trap clearance on the flats'),
    ('nut_depth', '2.6', 'nut trap depth (M3 nut 2.4)'),
    ('rim_ch', '0.4', 'entry chamfer on hole and trap rims'),
    # --- caster screws: M3 countersunk from the support top (flush against the plate) ---
    ('csk_d', '6.3', 'countersink dia at the top face (M3 countersunk head 6.0)'),
    # --- drop under the plate: follows from the rest (9.75) ---
    ('boss_h', 'floor_drop - cst_h - flange_t', 'boss under the plate, B'),
    # --- version A: printed pillars, part of the support ---
    ('pillar_l', 'boss_h', 'pillar length, same drop as the boss'),
    ('pillar_d', '8', 'pillar dia (caster holes are 4 from the plate edge)'),
]
for n, e, c in P:
    setp(n, e, c)
print('params', des.userParameters.count, 'boss_h', round(pv('boss_h'), 2),
      'A floor error', round(pv('flange_t') + pv('pillar_l') + pv('cst_h') - pv('floor_drop'), 2))
