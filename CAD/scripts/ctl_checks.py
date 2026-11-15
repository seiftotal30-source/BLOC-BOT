# Read-only checks for the closed controller. Re-run after any rebuild.
#  1. collisions: every printed body and every reference part, pairwise (touching = 0 mm3 is OK)
#  2. cap travel: each cap moved down by cap_gap + sw_travel hits only its own plunger
#  3. joystick: cap swept to joy_tilt in 8 directions clears every other part
#  4. OLED: a prism straight up from the active-area face does not touch the top shell
#  5. USB: receptacle cross-section and plug overmold envelope are clear to the outside
#  6. wiring channel (56 x 38 x 8 mm, under the display) is empty
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
print('ACTIVE:', app.activeDocument.name)
W, H, T, w, c = pv('case_w'), pv('case_h'), pv('case_t'), pv('wall'), pv('clr')
TOL = 0.001  # mm3


def wbox(b):
    x = b.boundingBox
    return (x.minPoint.x * 10, x.minPoint.y * 10, x.minPoint.z * 10, x.maxPoint.x * 10, x.maxPoint.y * 10, x.maxPoint.z * 10)


def ov(a, b, e=0.0):
    return all(a[i] < b[i + 3] - e and b[i] < a[i + 3] - e for i in range(3))


def leaf_bodies(o):
    out = [b for b in o.bRepBodies if b.isLightBulbOn and b.isVisible]
    for ch in o.childOccurrences:
        if ch.isLightBulbOn: out += leaf_bodies(ch)
    return out


def vol_or_none(tb):
    try: return tb.volume * 1000
    except Exception: return None


def inter(a, b):
    try:
        ta = TB.copy(a); TB.booleanOperation(ta, TB.copy(b), adsk.fusion.BooleanTypes.IntersectionBooleanType)
        return vol_or_none(ta)
    except Exception:
        return None


# ---- groups
case = occ('Controller_Case'); caps = occ('Controller_Caps')
G = {}
for b in case.bRepBodies: G[b.name] = [b]
for b in caps.bRepBodies: G[b.name] = [b]
sw_names = [n.upper() for n, _, _ in buttons()]
k = 0
for o in root.occurrences:
    if not o.isLightBulbOn or not o.component.name.startswith('REF_'): continue
    name = o.component.name
    if name == 'REF_Switch12': name = 'REF_Switch_' + sw_names[k]; k += 1
    G[name] = leaf_bodies(o)
BB = {g: [(b, wbox(b)) for b in bs] for g, bs in G.items()}
GB = {g: (min(x[1][0] for x in v), min(x[1][1] for x in v), min(x[1][2] for x in v),
          max(x[1][3] for x in v), max(x[1][4] for x in v), max(x[1][5] for x in v)) for g, v in BB.items() if v}

# ---- 1. collisions
names = sorted(GB); hits = []; unk = 0; pairs = 0
for i, a in enumerate(names):
    for bname in names[i + 1:]:
        if not ov(GB[a], GB[bname]): continue
        tot = 0.0
        for ba, xa in BB[a]:
            for bb_, xb in BB[bname]:
                if not ov(xa, xb): continue
                pairs += 1
                v = inter(ba, bb_)
                if v is None: unk += 1
                elif v > TOL: tot += v
        if tot > TOL: hits.append((a, bname, round(tot, 3)))
print('1. COLLISIONS: %d body pairs tested, %d not evaluable' % (pairs, unk))
for h in hits: print('   OVERLAP %s <-> %s : %s mm3' % h)
if not hits: print('   none')

# ---- 2. cap travel
others = [g for g in names if not g.startswith('cap_')]
print('2. CAP TRAVEL (free play %.2f + switch travel %.2f)' % (pv('cap_gap'), pv('sw_travel')))
mm_ = app.measureManager
for cn, sn in zip(['cap_' + n for n, _, _ in buttons()], sw_names):
    cap = G[cn][0]
    plunger = [b for b in G['REF_Switch_' + sn] if b.name == 'Body3'][0]
    gap = mm_.measureMinimumDistance(cap, plunger).value * 10
    m = adsk.core.Matrix3D.create(); m.translation = V3(0, 0, -(pv('cap_gap') + pv('sw_travel')) / 10)
    tc = TB.copy(cap); TB.transform(tc, m); tcb = tc.boundingBox
    xb = (tcb.minPoint.x * 10, tcb.minPoint.y * 10, tcb.minPoint.z * 10, tcb.maxPoint.x * 10, tcb.maxPoint.y * 10, tcb.maxPoint.z * 10)
    press = inter(tc, plunger) or 0
    bad = []
    for g in others:
        for b, x in BB[g]:
            if b == plunger or not ov(xb, x): continue
            v = inter(tc, b)
            if v and v > TOL: bad.append((g, round(v, 3)))
    top_after = xb[5] - T
    print('   %-12s rest gap to plunger %.2f mm | pressed: plunger overlap %.2f mm3 (>0 = actuated) | other hits %s | cap top %.2f above surface'
          % (cn, gap, press, bad or 'none', top_after))

# ---- 3. joystick sweep
jo = occ('REF_Joystick')
def find(o, name):
    for ch in o.childOccurrences:
        if ch.component.name == name: return ch
        r = find(ch, name)
        if r: return r
jcap = find(jo, 'Thumb-Button').bRepBodies.item(0)
piv = P3(pv('joy_x') / 10, pv('joy_y') / 10, (T - pv('joy_pivot_dz')) / 10)
tilt = des.userParameters.itemByName('joy_tilt').value
tgt = [g for g in names if g != 'REF_Joystick']
worst = []
for k8 in range(8):
    a = math.radians(45 * k8); ax = V3(-math.sin(a), math.cos(a), 0)
    for deg in (10, 20, tilt):
        m = adsk.core.Matrix3D.create(); m.setToRotation(math.radians(deg), ax, piv)
        tc = TB.copy(jcap); TB.transform(tc, m); tb = tc.boundingBox
        xb = (tb.minPoint.x * 10, tb.minPoint.y * 10, tb.minPoint.z * 10, tb.maxPoint.x * 10, tb.maxPoint.y * 10, tb.maxPoint.z * 10)
        for g in tgt:
            for b, x in BB[g]:
                if ov(xb, x):
                    v = inter(tc, b)
                    if v and v > TOL: worst.append((45 * k8, deg, g, round(v, 3)))
dmin = min(mm_.measureMinimumDistance(jcap, G['controller_top'][0]).value * 10, 99)
print('3. JOYSTICK SWEEP to %.0f deg in 8 directions: %s (rest clearance cap-top shell %.2f mm)'
      % (tilt, worst or 'clear', dmin))

# ---- 4. OLED active area
scr = find(occ('REF_OLED'), 'screen').bRepBodies.item(0)
aa = [f for f in scr.faces if f.appearance and f.appearance.name == 'Opaque(179,179,179)']
aa = max(aa, key=lambda f: f.area); x = aa.boundingBox
ab = [v * 10 for v in (x.minPoint.x, x.minPoint.y, x.minPoint.z, x.maxPoint.x, x.maxPoint.y, x.maxPoint.z)]
prism = tbox(ab[0], ab[1], ab[5] + 0.01, ab[3], ab[4], T + 5)
v = inter(prism, G['controller_top'][0])
print('4. OLED ACTIVE AREA %.1f x %.1f at z %.2f: blocked volume above it %s mm3 -> %s'
      % (ab[3] - ab[0], ab[4] - ab[1], ab[5], v, 'fully visible' if (v or 0) < TOL else 'BLOCKED'))
glass = scr.boundingBox
print('   glass front z %.2f, lip underside z %.2f, lip overlap on glass: L %.1f R %.1f bottom %.1f top %.1f mm' % (
    glass.maxPoint.z * 10, T - pv('oled_lip'),
    (ab[0] - c) - glass.minPoint.x * 10, glass.maxPoint.x * 10 - (ab[3] + c),
    (ab[1] - c) - glass.minPoint.y * 10, glass.maxPoint.y * 10 - (ab[4] + c)))

# ---- 5. USB
def usb(label, rec_body, axis, sign, plug_w, plug_h):
    x = wbox(rec_body)
    shell = G['controller_bottom'] + G['controller_top']
    if axis == 'x':
        face = x[0] if sign < 0 else x[3]
        lane = tbox(-W / 2 - 10, x[1], x[2], face - 0.01, x[4], x[5])
        cy, cz = (x[1] + x[4]) / 2, (x[2] + x[5]) / 2
        env = tbox(-W / 2 - 30, cy - plug_w / 2, cz - plug_h / 2, -W / 2 + pv('usb_cb') - 0.01, cy + plug_w / 2, cz + plug_h / 2)
        depth = face - (-W / 2 + pv('usb_cb'))
    else:
        face = x[1]
        lane = tbox(x[0], -H / 2 - 10, x[2], x[3], face - 0.01, x[5])
        cx, cz = (x[0] + x[3]) / 2, (x[2] + x[5]) / 2
        env = tbox(cx - plug_w / 2, -H / 2 - 30, cz - plug_h / 2, cx + plug_w / 2, -H / 2 + pv('usb_cb') - 0.01, cz + plug_h / 2)
        depth = face - (-H / 2 + pv('usb_cb'))
    vl = sum(inter(lane, b) or 0 for b in shell); ve = sum(inter(env, b) or 0 for b in shell)
    print('5. %s: receptacle lane blocked %.3f mm3, plug overmold envelope blocked %.3f mm3, receptacle face %.2f mm inside the overmold stop -> %s'
          % (label, vl, ve, depth, 'reachable' if vl < TOL and ve < TOL else 'BLOCKED'))


usb('ESP32 micro-USB (left edge)', find(occ('REF_ESP32'), 'USB_Micro-B_Molex_47346-0001').bRepBodies.item(0), 'x', -1, pv('usbm_plug_w'), pv('usbm_plug_h'))
usb('TP4056 USB-C (bottom edge)', find(occ('REF_TP4056'), 'USB_Type_C_Connector').bRepBodies.item(0), 'y', -1, pv('usbc_plug_w'), pv('usbc_plug_h'))

# ---- 6. wiring channel
ch = tbox(-28, 0, w, 28, 38, w + 8); chb = (-28, 0, w, 28, 38, w + 8)
inch = [g for g in names for b, x in BB[g] if ov(chb, x) and (inter(ch, b) or 0) > TOL]
print('6. WIRING CHANNEL 56 x 38 x 8 mm under the display: %s' % (sorted(set(inch)) or 'empty'))
print('OUTER SIZE', [round(GB['controller_bottom'][i + 3] - GB['controller_bottom'][i], 2) for i in range(2)], 'x', round(GB['controller_top'][5], 2))
