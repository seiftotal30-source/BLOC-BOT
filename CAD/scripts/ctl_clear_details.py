# Remove everything ctl_details.py added (details base features, combines, caps) and the
# entry-rim chamfer that references those holes, leaving the bare shells with the outer rim
# chamfer. Run before re-running ctl_details.py and ctl_entry_chamfers.py.
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py', encoding='utf-8').read())
print('ACTIVE:', app.activeDocument.name)
C = occ('Controller_Case').component
for f in list(C.features.chamferFeatures):
    if f.name == 'Chamfer_EntryRims': f.deleteMe(); print('deleted Chamfer_EntryRims')
k = occ('Controller_Caps')
if k: k.deleteMe(); print('deleted caps')
for f in list(C.features.combineFeatures) + list(C.features.baseFeatures):
    print('deleted', f.name); f.deleteMe()
for b in C.bRepBodies: print(b.name, bb(b))
