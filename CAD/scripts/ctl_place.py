# Place the reference models (not printed) at their layout positions from the User Parameters.
# Also creates the 6 switch occurrences and simple stand-in bodies for the parts that have no
# model (18650 holder, slide switch). Re-run after changing any position parameter.
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
print('ACTIVE:', app.activeDocument.name)


def place(o, xa, ya, za, model_pt, target):
    """Rotate model axes onto xa/ya/za, then move model_pt to target (all mm)."""
    m = adsk.core.Matrix3D.create()
    m.setWithCoordinateSystem(P3(0, 0, 0), V3(*xa), V3(*ya), V3(*za))
    p = P3(model_pt[0] / 10, model_pt[1] / 10, model_pt[2] / 10); p.transformBy(m)
    m.translation = V3(target[0] / 10 - p.x, target[1] / 10 - p.y, target[2] / 10 - p.z)
    o.transform2 = m


W, H, T, w, c = pv('case_w'), pv('case_h'), pv('case_t'), pv('wall'), pv('clr')
# GrabCAD shell: shape reference only, kept hidden
g = occ('REF_GrabCAD_RC')
if g: g.isLightBulbOn = False

# Timeline features (stand-ins, extra switch occurrences) must come before any move:
# adding a feature discards pending occurrence moves.
def standin(name, boxes):
    o = occ(name)
    if o is None:
        o = root.occurrences.addNewComponent(adsk.core.Matrix3D.create()); o.component.name = name
    cmp = o.component
    for f in list(cmp.features.baseFeatures): f.deleteMe()
    for b in list(cmp.bRepBodies): b.deleteMe()
    bf = cmp.features.baseFeatures.add(); bf.startEdit()
    for bx in boxes: cmp.bRepBodies.add(tbox(*bx), bf)
    bf.finishEdit()
    return o


bl, bw, bh = pv('bat_l'), pv('bat_w'), pv('bat_h')
by0 = -H / 2 + w + pv('bat_wall_gap')
bx = pv('bat_cx')
standin('REF_Battery18650', [(bx - bl / 2, by0, w, bx + bl / 2, by0 + bw, w + bh)])
sx, sl, sw_, sh = pv('sl_x'), pv('sl_l'), pv('sl_w'), pv('sl_h')
yb = H / 2 - w - c
aw = pv('sl_act_w')
sz = w + pv('sl_base')
standin('REF_SlideSwitch', [(sx - sl / 2, yb - sw_, sz, sx + sl / 2, yb, sz + sh),
                            (sx - aw / 2, yb, sz + sh / 2 - 1, sx + aw / 2, yb + pv('sl_act_out'), sz + sh / 2 + 1)])

swc = occ('REF_Switch12').component
# the model ships with a 13x13 keycap (Body8); our printed caps replace it
for b in swc.bRepBodies:
    if b.name == 'Body8': b.isLightBulbOn = False
sw_occ = [o for o in root.occurrences if o.component == swc]
while len(sw_occ) < 6:
    sw_occ.append(root.occurrences.addExistingComponent(swc, adsk.core.Matrix3D.create()))

place(occ('REF_OLED'), (1, 0, 0), (0, 0, 1), (0, -1, 0), (39.94, 4.9, 23.84),
      (pv('oled_cx'), pv('oled_cy'), T - pv('oled_lip') - 0.05))
place(occ('REF_Joystick'), (1, 0, 0), (0, 1, 0), (0, 0, 1), (0, 0, 0),
      (pv('joy_x'), pv('joy_y'), T - pv('joy_pivot_dz')))
place(occ('REF_ESP32'), (0, 1, 0), (-1, 0, 0), (0, 0, 1), (0, 0, 0),
      (pv('esp_x'), pv('esp_y'), w + pv('esp_standoff')))
place(occ('REF_TP4056'), (0, -1, 0), (1, 0, 0), (0, 0, 1), (0, 0, 0),
      (pv('tp_cx'), -H / 2 + w + c + pv('tp_l') / 2, w + pv('tp_ledge')))
place(occ('REF_MT3608'), (1, 0, 0), (0, 0, 1), (0, -1, 0), (0, 0, 0),
      (pv('mt_x'), H / 2 - w - c, w + pv('mt_ledge')))

# switches: b1, b2 below the display, then up, down, left, right
zs = T - w - pv('cap_flange_t') - pv('cap_gap') - pv('sw_plunger_h')
SW = [(x, y) for _, x, y in buttons()]
for o, (x, y) in zip(sw_occ, SW):
    place(o, (1, 0, 0), (0, 1, 0), (0, 0, 1), (0, 0, 0), (x, y, zs))

try:
    des.snapshots.add()  # only needed when the timeline has a pending move
except RuntimeError:
    pass
for o in root.occurrences:
    if o.component.name.startswith('REF_') and o.isLightBulbOn:
        print('%-18s' % o.component.name, bbox_mm([o.boundingBox]))
print('switch seat z', round(zs, 2))
