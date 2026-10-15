# Screenshots of both caster supports for the README and the thesis, into CAD/out/caster/.
#   caster_<v>_iso.png   from below and behind, under the plate
#   caster_<v>_side.png  from the left side, plate edge to floor
#   caster_<v>_part.png  the support alone, underside up
exec(open(r'E:\Claude Stuff\CAD\scripts\cst_helpers.py').read())
import os
out = r'E:\Claude Stuff\CAD\out\caster'
os.makedirs(out, exist_ok=True)
vp = app.activeViewport
for c in [root] + [o.component for o in root.occurrences]:
    for p in c.constructionPlanes: p.isLightBulbOn = False
    c.isSketchFolderLightBulbOn = False


def show(names):
    for o in root.occurrences:
        o.isLightBulbOn = o.component.name in names


def shot(name, eye, target, w=1200, h=900):
    cam = vp.camera
    cam.cameraType = adsk.core.CameraTypes.OrthographicCameraType
    cam.target = P3(*[v / 10 for v in target]); cam.eye = P3(*[v / 10 for v in eye])
    cam.upVector = V3(0, 0, 1); cam.isFitView = True
    vp.camera = cam; vp.refresh(); adsk.doEvents()
    p = os.path.join(out, name); vp.saveAsImageFile(p, w, h); print('saved', p)


for v in ('A', 'B'):
    parts = ['Support_' + v, 'REF_Caster_' + v]
    show(parts + ['REF_Plate'])
    shot('caster_%s_iso.png' % v, (-120, -200, -120), (0, -65, -20))
    show(parts)
    shot('caster_%s_side.png' % v, (-300, -62, -24), (0, -62, -24))
    show(['Support_' + v])
    shot('caster_%s_part.png' % v, (-90, -150, -100), (0, -62, -5))
show(['REF_Plate', 'Support_A', 'REF_Caster_A'])
