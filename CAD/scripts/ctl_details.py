# Add every internal feature to the two shell halves, plus the printed caps.
#
# All geometry comes from the User Parameters (pv). Added material and cut-outs are built as
# temporary bodies, stored in one base feature per half, then joined / cut with Combine
# features. Run after ctl_shell.py (which rebuilds the bare halves); re-running both rebuilds
# the case from the current parameters.
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
print('ACTIVE:', app.activeDocument.name)
UNION = adsk.fusion.BooleanTypes.UnionBooleanType

W, H, T, w, c = pv('case_w'), pv('case_h'), pv('case_t'), pv('wall'), pv('clr')
zsp, ceil, floor = pv('z_split'), T - w, w
xi, yi = W / 2 - w, H / 2 - w                      # inner wall faces


def union(bodies):
    acc = TB.copy(bodies[0])
    for b in bodies[1:]: TB.booleanOperation(acc, b, UNION)
    return acc


def clip(edge, a0, a1, z_top, axis):
    """Retaining clip along a board edge: post outside the pocket + 0.5 mm lip over the board.
    edge = pocket wall coordinate, the board lies on the side the clip faces (sign of axis)."""
    t, lip = pv('rib_t'), 0.5
    s = 1 if axis[1] > 0 else -1          # +1: board is on the + side of `edge`
    if axis[0] == 'x':
        post = tbox(edge - s * t, a0, floor - 0.5, edge, a1, z_top + 0.6)
        lp = tbox(edge, a0, z_top + 0.1, edge + s * (lip + c), a1, z_top + 0.6)
    else:
        post = tbox(a0, edge - s * t, floor - 0.5, a1, edge, z_top + 0.6)
        lp = tbox(a0, edge, z_top + 0.1, a1, edge + s * (lip + c), z_top + 0.6)
    return [post, lp]


add_b, cut_b, add_t, cut_t = [], [], [], []
SCR = [(pv('scr%d_x' % i), pv('scr%d_y' % i)) for i in range(1, 5)]

# ---- case screws: Ø9 bosses + Ø3.4 clearance + head counterbore (bottom); Ø6 pilot bosses (top)
for x, y in SCR:
    add_b.append(tcyl(x, y, floor - 0.5, zsp, pv('m3_bboss_d')))
    cut_b.append(tcyl(x, y, -1, zsp + 1, pv('m3_clear')))
    cut_b.append(tcyl(x, y, -1, pv('m3_cb_h'), pv('m3_cb_d')))
    add_t.append(tcyl(x, y, zsp, ceil + 0.5, pv('m3_boss_d')))
    cut_t.append(tcyl(x, y, zsp - 1, ceil + 1, pv('m3_pilot')))   # 1 mm skin left under the top face

# ---- switches: pillars from the floor, locating-peg holes from the model
zs = T - w - pv('cap_flange_t') - pv('cap_gap') - pv('sw_plunger_h')
dx, dy, dp = pv('dpad_x'), pv('dpad_y'), pv('dpad_pitch')
SW = {'up': (dx, dy + dp, 'dpad'), 'down': (dx, dy - dp, 'dpad'), 'left': (dx - dp, dy, 'dpad'),
      'right': (dx + dp, dy, 'dpad'), 'back': (-pv('bf_pitch') / 2, pv('bf_y'), 'bf'),
      'forward': (pv('bf_pitch') / 2, pv('bf_y'), 'bf')}
sp = pv('sw_pillar')
for x, y, _ in SW.values():
    add_b.append(tbox(x - sp / 2, y - sp / 2, floor - 0.5, x + sp / 2, y + sp / 2, zs))
    for s in (-1, 1):
        cut_b.append(tcyl(x, y + s * pv('sw_peg_off'), zs - pv('sw_peg_h') - c, zs + 0.1, pv('sw_peg_d') + c))

# ---- caps: holes in the top
for x, y, k in SW.values():
    cut_t.append(tcyl(x, y, ceil - 1, T + 1, pv(k + '_cap_d') + 2 * c))

# ---- ESP32: 4 standoffs with locating pegs, hold-downs from the top, micro-USB opening
ex, ey, el, ew = pv('esp_x'), pv('esp_y'), pv('esp_l'), pv('esp_w')
ez = floor + pv('esp_standoff')                      # PCB underside
holes = [(2.4, 2.3), (2.4, 48.9), (25.9, 2.3), (25.9, 48.9)]   # model (x, y), Ø2.9
plan = lambda mx, my: (ex - my, ey + mx)            # model -> plan (USB end toward -X)
for mx, my in holes:
    x, y = plan(mx, my)
    add_b.append(tcyl(x, y, floor - 0.5, ez, pv('esp_post_d')))
    add_b.append(tcyl(x, y, ez - 0.1, ez + 1.2, pv('esp_peg_d')))
for mx, my in ((2.4, 48.9), (25.9, 48.9)):         # USB end: press on the PCB at the holes
    x, y = plan(mx, my)
    add_t.append(tcyl(x, y, ez + pv('esp_t') + 0.2, ceil + 0.5, pv('esp_post_d')))
x, y = plan(14.15, 12)                               # far end: press on the WROOM can (model top z 4.7)
add_t.append(tcyl(x, y, ez + 4.7 + 0.2, ceil + 0.5, 4))
uy, uz = ey + 14.0, ez + (1.36 + 4.3) / 2           # receptacle centre (model x 10..18, z 1.36..4.3)
cut_b.append(tbox(-W / 2 - 1, uy - pv('usbm_w') / 2 - c, uz - pv('usbm_h') / 2 - c,
                  -xi + 0.5, uy + pv('usbm_w') / 2 + c, uz + pv('usbm_h') / 2 + c))
cut_b.append(tbox(-W / 2 - 1, uy - pv('usbm_plug_w') / 2 - c, uz - pv('usbm_plug_h') / 2 - c,
                  -W / 2 + pv('usb_cb'), uy + pv('usbm_plug_w') / 2 + c, uz + pv('usbm_plug_h') / 2 + c))

# ---- TP4056: pads + clips, USB-C opening in the bottom edge
tcx, tl, tw = pv('tp_cx'), pv('tp_l'), pv('tp_w')
tz = floor + pv('tp_ledge'); ttop = tz + pv('tp_t')
x0, x1, y0, y1 = tcx - tw / 2 - c, tcx + tw / 2 + c, -yi, -yi + c + tl + c
for px in (x0 + c, x1 - c - 3):
    for py in (y0 + c, y1 - c - 3):
        add_b.append(tbox(px, py, floor - 0.5, px + 3, py + 3, tz))
add_b += clip(y1, tcx - 4, tcx + 4, ttop, ('y', -1))               # far end
for e, s in ((x0, 1), (x1, -1)):
    add_b += clip(e, y1 - 10, y1 - 4, ttop, ('x', s))              # sides, at the pad end
cz = tz + (1.35 + 4.14) / 2
cut_b.append(tbox(tcx - pv('usbc_w') / 2 - c, -H / 2 - 1, cz - pv('usbc_h') / 2 - c,
                  tcx + pv('usbc_w') / 2 + c, -yi + 0.5, cz + pv('usbc_h') / 2 + c))
cut_b.append(tbox(tcx - pv('usbc_plug_w') / 2 - c, -H / 2 - 1, cz - pv('usbc_plug_h') / 2 - c,
                  tcx + pv('usbc_plug_w') / 2 + c, -H / 2 + pv('usb_cb'), cz + pv('usbc_plug_h') / 2 + c))

# ---- MT3608: pads + clips on the open long edge (top edge is the wall)
mx0, ml, mw = pv('mt_x'), pv('mt_l'), pv('mt_w')
mz = floor + pv('mt_ledge'); mtop = mz + pv('mt_t')
my0 = yi - c - mw                                    # PCB lower edge
for px in (mx0, mx0 + ml - 3):
    for py in (my0, yi - c - 3):
        add_b.append(tbox(px, py, floor - 0.5, px + 3, py + 3, mz))
for a in (mx0 + 2, mx0 + ml - 8):                    # clear of the trimpot overhang (x 51.8..54)
    add_b += clip(my0 - c, a, a + 5, mtop, ('y', 1))

# ---- battery holder pocket ribs
bl, bw = pv('bat_l'), pv('bat_w'); rt, rh = pv('rib_t'), pv('rib_h')
by1 = -yi + pv('bat_wall_gap') + bw + c
add_b.append(tbox(-bl / 2 - c - rt, by1, floor - 0.5, bl / 2 + c + rt, by1 + rt, floor + rh))
for s in (-1, 1):
    add_b.append(tbox(s * (bl / 2 + c), -yi, floor - 0.5, s * (bl / 2 + c + rt), by1 + rt, floor + rh))

# ---- slide switch: cradle + actuator slot in the top edge wall
sx, sl, sw_, sh = pv('sl_x'), pv('sl_l'), pv('sl_w'), pv('sl_h')
sy0 = yi - c - sw_
add_b.append(tbox(sx - sl / 2 - c - rt, sy0 - c - rt, floor - 0.5, sx + sl / 2 + c + rt, sy0 - c, floor + sh - 1.5))
for s in (-1, 1):
    add_b.append(tbox(s * (sl / 2 + c) + sx, sy0 - c - rt, floor - 0.5, s * (sl / 2 + c + rt) + sx, yi, floor + sh - 1.5))
sb = pv('sl_base')
add_b.append(tbox(sx - sl / 2, sy0, floor - 0.5, sx + sl / 2, yi, floor + sb))   # pad lifts it off the corner fillet
saz = floor + sb + sh / 2
cut_b.append(tbox(sx - pv('sl_slot_w') / 2, yi - 0.5, saz - pv('sl_slot_h') / 2, sx + pv('sl_slot_w') / 2, H / 2 + 1, saz + pv('sl_slot_h') / 2))

# ---- OLED: window = active area + clr, glass pocket (lip = oled_lip), 4 bosses with pilots
ocx, ocy = pv('oled_cx'), pv('oled_cy')
aw, ah, acy = pv('oled_aa_w'), pv('oled_aa_h'), ocy + pv('oled_aa_dy')
cut_t.append(tbox(ocx - aw / 2 - c, acy - ah / 2 - c, ceil - 1, ocx + aw / 2 + c, acy + ah / 2 + c, T + 1))
gw, gh, gcy = pv('oled_glass_w'), pv('oled_glass_h'), ocy - pv('oled_glass_dy')
cut_t.append(tbox(ocx - gw / 2 - c, gcy - gh / 2 - c, ceil - 1, ocx + gw / 2 + c, gcy + gh / 2 + c, T - pv('oled_lip')))
pcb_front = T - pv('oled_lip') - 0.05 - pv('oled_glass_t')
for sxh in (-1, 1):
    for syh in (-1, 1):
        x, y = ocx + sxh * pv('oled_hole_sx') / 2, ocy + syh * pv('oled_hole_sy') / 2
        add_t.append(tcyl(x, y, pcb_front + pv('oled_pad_t'), ceil + 0.5, pv('oled_boss_d')))
        cut_t.append(tcyl(x, y, pcb_front - 1, T - 1, pv('bscrew_pilot')))

# ---- joystick: thumb opening in the top; 4 posts from the floor with pilots (M2.5 from above).
# Bosses hanging from the top would sit inside the cap skirt's sweep sphere at full tilt.
jx, jy = pv('joy_x'), pv('joy_y'); piv = T - pv('joy_pivot_dz')
cut_t.append(tcyl(jx, jy, ceil - 1, T + 1, pv('joy_open_d')))
jbot = piv - pv('joy_pcb_bot_dz')
for hx in ('joy_hole_x1', 'joy_hole_x2'):
    for hy in ('joy_hole_y1', 'joy_hole_y2'):
        x, y = jx + pv(hx), jy + pv(hy)
        add_b.append(tcyl(x, y, floor - 0.5, jbot, pv('joy_post_d')))
        cut_b.append(tcyl(x, y, jbot - 6, jbot + 1, pv('bscrew_pilot')))

# ---- apply to the halves
Cc = occ('Controller_Case').component
top, bot = Cc.bRepBodies.itemByName('controller_top'), Cc.bRepBodies.itemByName('controller_bottom')


def apply(target, adds, cuts, tag):
    bf = Cc.features.baseFeatures.add(); bf.startEdit()
    a = Cc.bRepBodies.add(union(adds), bf); a.name = tag + '_add'
    k = Cc.bRepBodies.add(union(cuts), bf); k.name = tag + '_cut'
    bf.finishEdit(); bf.name = tag + ' details'
    a = Cc.bRepBodies.itemByName(tag + '_add'); k = Cc.bRepBodies.itemByName(tag + '_cut')
    for tool, op in ((a, FO.JoinFeatureOperation), (k, FO.CutFeatureOperation)):
        tc = adsk.core.ObjectCollection.create(); tc.add(tool)
        ci = Cc.features.combineFeatures.createInput(target, tc); ci.operation = op
        Cc.features.combineFeatures.add(ci)
        target = Cc.bRepBodies.itemByName(target.name)


apply(bot, add_b, cut_b, 'bottom')
apply(top, add_t, cut_t, 'top')
for b in Cc.bRepBodies: print(b.name, bb(b), 'vol %.0f mm3' % (b.volume * 1000))

# ---- caps (flange under the top wall, stem through it)
old = occ('Controller_Caps')
if old: old.deleteMe()
co = root.occurrences.addNewComponent(adsk.core.Matrix3D.create()); K = co.component; K.name = 'Controller_Caps'
zb = ceil - pv('cap_flange_t')
caps = {name: union([tcyl(x, y, zb, T + pv('cap_protrude'), pv(k + '_cap_d')), tcyl(x, y, zb, ceil, pv('cap_flange_d'))])
        for name, (x, y, k) in SW.items()}
# no parameter reads between startEdit/finishEdit: the design is non-parametric while editing
bf = K.features.baseFeatures.add(); bf.startEdit()
for name, cap in caps.items():
    K.bRepBodies.add(cap, bf).name = 'cap_' + name
bf.finishEdit(); bf.name = 'caps'
print('caps', [(b.name, bb(b)) for b in K.bRepBodies])
