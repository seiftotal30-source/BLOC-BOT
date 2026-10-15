# One-off: creates the "Caster Mount" design document and copies the waffle plate into it.
#
# The plate is copied (as a dead base-feature body, component REF_Plate) from the "Motor
# Bracket" document, where it sits in the robot frame the caster scripts use. The new design
# is tagged with the BLOCBOT/doc = CasterMount attribute that cst_helpers.py checks for.
# Run once with "Motor Bracket" open; afterwards run cst_params.py and cst_build.py.
import adsk.core, adsk.fusion
app = adsk.core.Application.get()
TB = adsk.fusion.TemporaryBRepManager.get()

src = [d for d in app.documents if d.name.startswith('Motor Bracket')][0]
sdes = adsk.fusion.Design.cast(src.products.itemByProductType('DesignProductType'))
plate_occ = [o for o in sdes.rootComponent.occurrences if o.component.name.startswith('REF_Plate')
             or o.name.startswith('REF_Plate')][0]
plate = TB.copy(plate_occ.bRepBodies.item(0))
# On the real robot the plate sits turned 90 deg under the brackets, wheels and caster.
rot = adsk.core.Matrix3D.create(); rot.setToRotation(1.5707963267948966, adsk.core.Vector3D.create(0, 0, 1), adsk.core.Point3D.create(0, 0, 0))
TB.transform(plate, rot)

doc = app.documents.add(adsk.core.DocumentTypes.FusionDesignDocumentType)
des = adsk.fusion.Design.cast(app.activeProduct)
des.designType = adsk.fusion.DesignTypes.ParametricDesignType
des.attributes.add('BLOCBOT', 'doc', 'CasterMount')
root = des.rootComponent
o = root.occurrences.addNewComponent(adsk.core.Matrix3D.create())
C = o.component; C.name = 'REF_Plate'
base = C.features.baseFeatures.add()
base.startEdit()
C.bRepBodies.add(plate, base)
base.finishEdit()
base.name = 'waffle_plate_copy'
C.bRepBodies.item(0).name = 'Plate'
b = C.bRepBodies.item(0).boundingBox
print('plate bbox', [round(v * 10, 2) for v in (b.minPoint.x, b.minPoint.y, b.minPoint.z, b.maxPoint.x, b.maxPoint.y, b.maxPoint.z)])


def run(_context=None):
    pass
