# Build the case shell in component "Controller_Case": chamfered plan outline (GrabCAD RC Transmitter
# style), extrude case_t, fillet every outer edge fillet_r, hollow it to `wall`, split at
# z_split into controller_top / controller_bottom. Extents, fillet, shell and split plane are
# driven by parameter expressions; the outline sketch is regenerated from case_w/case_h/corner_ch
# when this script is re-run (it deletes and rebuilds the Controller_Case component).
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
print('ACTIVE:', app.activeDocument.name)

old = occ('Controller_Case')
if old: old.deleteMe()
o = root.occurrences.addNewComponent(adsk.core.Matrix3D.create())
C = o.component; C.name = 'Controller_Case'

W, H, ch = pv('case_w'), pv('case_h'), pv('corner_ch')
sk = C.sketches.add(C.xYConstructionPlane); sk.name = 'outline'
x, y = W / 2, H / 2
pts = [(-x + ch, -y), (x - ch, -y), (x, -y + ch), (x, y - ch), (x - ch, y), (-x + ch, y), (-x, y - ch), (-x, -y + ch)]
L = sk.sketchCurves.sketchLines
for i in range(8):
    a, b = pts[i], pts[(i + 1) % 8]
    L.addByTwoPoints(P3(a[0] / 10, a[1] / 10, 0), P3(b[0] / 10, b[1] / 10, 0))
ei = C.features.extrudeFeatures.createInput(sk.profiles.item(0), FO.NewBodyFeatureOperation)
ei.setDistanceExtent(False, VI('case_t'))
body = C.features.extrudeFeatures.add(ei).bodies.item(0)

# fillet: vertical corner edges first, then the top and bottom perimeters
fil = C.features.filletFeatures
vert = adsk.core.ObjectCollection.create()
for e in body.edges:
    s, t = e.startVertex.geometry, e.endVertex.geometry
    if abs(s.x - t.x) < 1e-6 and abs(s.y - t.y) < 1e-6: vert.add(e)
fi = fil.createInput(); fi.addConstantRadiusEdgeSet(vert, VI('fillet_r'), True); fil.add(fi)
body = C.bRepBodies.item(0)
hor = adsk.core.ObjectCollection.create()
for f in body.faces:
    if isinstance(f.geometry, adsk.core.Plane) and abs(f.geometry.normal.z) > 0.99:
        for e in f.edges: hor.add(e)
fi = fil.createInput(); fi.addConstantRadiusEdgeSet(hor, VI('fillet_r'), True); fil.add(fi)
body = C.bRepBodies.item(0)

# hollow (no open face) -> closed shell with wall thickness
coll = adsk.core.ObjectCollection.create(); coll.add(body)
si = C.features.shellFeatures.createInput(coll, False); si.insideThickness = VI('wall')
C.features.shellFeatures.add(si)
body = C.bRepBodies.item(0)

pi = C.constructionPlanes.createInput(); pi.setByOffset(C.xYConstructionPlane, VI('z_split'))
pl = C.constructionPlanes.add(pi); pl.name = 'split'
spi = C.features.splitBodyFeatures.createInput(body, pl, True)
C.features.splitBodyFeatures.add(spi)
pl.isLightBulbOn = False  # otherwise it shows in every screenshot
for b in C.bRepBodies:
    b.name = 'controller_top' if b.boundingBox.maxPoint.z * 10 > pv('z_split') + 1 else 'controller_bottom'
    print(b.name, bb(b), 'vol %.0f mm3' % (b.volume * 1000))
