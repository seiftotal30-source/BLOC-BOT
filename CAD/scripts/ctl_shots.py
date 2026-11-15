# Screenshots into CAD\out: iso, top, exploded, and a section through the D-pad and the display.
# Exploded and section views are drawn with custom graphics (temporary copies of the bodies), so
# the design itself is never moved or cut; visibility is restored at the end.
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
import os
print('ACTIVE:', app.activeDocument.name)
out = r'E:\Claude Stuff\CAD\out'
vp = app.activeViewport
IW, IH = 1800, 1100
VO = adsk.core.ViewOrientations


def shot(name, orient=None, eye_dir=None):
    cam = vp.camera
    if orient is not None:
        cam.viewOrientation = orient
    else:
        t = P3(0, 0, 1.2)
        cam.target = t; cam.eye = P3(t.x + eye_dir[0], t.y + eye_dir[1], t.z + eye_dir[2]); cam.upVector = V3(0, 0, 1)
    cam.isSmoothTransition = False; cam.isFitView = True
    vp.camera = cam; vp.refresh(); adsk.doEvents()
    p = os.path.join(out, name); vp.saveAsImageFile(p, IW, IH); print('saved', p)


def leaf(o):
    r = [b for b in o.bRepBodies if b.isLightBulbOn]
    for ch in o.childOccurrences:
        if ch.isLightBulbOn: r += leaf(ch)
    return r


root.isOriginFolderLightBulbOn = False  # origin axes and planes would show in the shots
case, caps = occ('Controller_Case'), occ('Controller_Caps')
refs = [o for o in root.occurrences if o.isLightBulbOn and o.component.name.startswith('REF_')]
shot('controller_iso.png', VO.IsoTopRightViewOrientation)
shot('controller_top.png', VO.TopViewOrientation)

# ---- custom-graphics scenes
CG = adsk.fusion.CustomGraphicsBasicMaterialColorEffect
col = lambda r, g, b: adsk.core.Color.create(r, g, b, 255)
def mat(r, g, b):
    return CG.create(col(r, g, b), col(r // 3, g // 3, b // 3), col(60, 60, 60), col(0, 0, 0), 30.0, 1.0)
M_TOP, M_BOT, M_CAP, M_REF = mat(200, 200, 205), mat(120, 125, 135), mat(230, 130, 40), mat(60, 110, 180)
PLATES = {'REF_OLED': mat(40, 40, 60), 'REF_Joystick': mat(30, 30, 30), 'REF_Battery18650': mat(40, 150, 80),
          'REF_SlideSwitch': mat(200, 60, 60)}
scene = [(b, M_TOP, 'top') for b in case.bRepBodies if b.name == 'controller_top']
scene += [(b, M_BOT, 'bot') for b in case.bRepBodies if b.name == 'controller_bottom']
scene += [(b, M_CAP, 'cap') for b in caps.bRepBodies]
for o in refs:
    scene += [(b, PLATES.get(o.component.name, M_REF), 'ref') for b in leaf(o)]
vis = [(o, o.isLightBulbOn) for o in root.occurrences]
for o, _ in vis: o.isLightBulbOn = False


def draw(items):
    g = root.customGraphicsGroups.add()
    for tb, m in items:
        try:
            e = g.addBRepBody(tb); e.color = m
        except Exception:
            pass
    return g


# exploded: bottom 0, internal parts +35, top +75, caps +100 (mm)
LIFT = {'bot': 0, 'ref': 35, 'top': 75, 'cap': 100}
items = []
for b, m, k in scene:
    try:
        t = TB.copy(b); mx = adsk.core.Matrix3D.create(); mx.translation = V3(0, 0, LIFT[k] / 10); TB.transform(t, mx)
        items.append((t, m))
    except Exception:
        pass
g = draw(items)
shot('controller_exploded.png', eye_dir=(30, -38, 26))
g.deleteMe()

# section: vertical plane through the directional cross and the display centre; keep the near half
p0 = (pv('dpad_x'), pv('dpad_y')); p1 = (pv('oled_cx'), pv('oled_cy'))
dx, dy = p1[0] - p0[0], p1[1] - p0[1]; L = math.hypot(dx, dy); d = (dx / L, dy / L); n = (d[1], -d[0])
if n[1] > 0: n = (-n[0], -n[1])                      # keep the half toward the viewer (-Y side)
cx, cy = (p0[0] + p1[0]) / 2, (p0[1] + p1[1]) / 2
ob = adsk.core.OrientedBoundingBox3D.create(P3((cx - n[0] * 200) / 10, (cy - n[1] * 200) / 10, 1.2),
                                            V3(-n[0], -n[1], 0), V3(d[0], d[1], 0), 40, 60, 20)
half = TB.createBox(ob)                               # everything on the far (+Y) side of the plane
items = []
for b, m, k in scene:
    try:
        t = TB.copy(b); TB.booleanOperation(t, TB.copy(half), adsk.fusion.BooleanTypes.DifferenceBooleanType)
        if t.volume > 0: items.append((t, m))
    except Exception:
        pass
g = draw(items)
shot('controller_section.png', eye_dir=(-n[0] * 30 + d[0] * 8, -n[1] * 30 + d[1] * 8, 14))  # from the removed side, onto the cut face
g.deleteMe()
print('section plane through cross', [round(v, 1) for v in p0], 'and display', [round(v, 1) for v in p1])

for o, v in vis: o.isLightBulbOn = v
vp.refresh()
