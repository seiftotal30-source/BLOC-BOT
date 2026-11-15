# 0.4 mm entry chamfers (design_rules.md): top rims of the button cap holes and the outer rims
# of the screw-head counterbores. Run after ctl_details.py.
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
print('ACTIVE:', app.activeDocument.name)
C = occ('Controller_Case').component
T = pv('case_t'); rh = (pv('btn_cap_d') + 2 * pv('clr')) / 2; rc = pv('m3_cb_d') / 2
coll = adsk.core.ObjectCollection.create(); n = {'cap holes': 0, 'counterbores': 0}
for b in C.bRepBodies:
    for e in b.edges:
        g = e.geometry
        if not isinstance(g, adsk.core.Circle3D): continue
        z, r = g.center.z * 10, g.radius * 10
        if b.name == 'controller_top' and abs(z - T) < 0.01 and abs(r - rh) < 0.01: coll.add(e); n['cap holes'] += 1
        if b.name == 'controller_bottom' and abs(z) < 0.01 and abs(r - rc) < 0.01: coll.add(e); n['counterbores'] += 1
print(n)
ci = C.features.chamferFeatures.createInput2()
ci.chamferEdgeSets.addEqualDistanceChamferEdgeSet(coll, VI('0.4 mm'), False)
f = C.features.chamferFeatures.add(ci); f.name = 'Chamfer_EntryRims'; print('health', f.healthState)
