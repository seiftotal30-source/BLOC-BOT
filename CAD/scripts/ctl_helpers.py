# Shared helpers for the Controller build scripts (exec'd inside Fusion).
#
# Every controller script starts by exec'ing this file. It refuses to run unless the active
# document is "Controller", the guard that keeps scripts away from "waffle plate" and
# "Motor Bracket". Positions are read from User Parameters (pv) so the build follows them.
import adsk.core, adsk.fusion, math
app = adsk.core.Application.get()
assert app.activeDocument.name == 'Controller' or app.activeDocument.name.startswith('Controller v'), \
    'WRONG DOC: ' + app.activeDocument.name
des = adsk.fusion.Design.cast(app.activeProduct)
root = des.rootComponent
um = des.unitsManager
VI = adsk.core.ValueInput.createByString
P3 = adsk.core.Point3D.create
V3 = adsk.core.Vector3D.create
FO = adsk.fusion.FeatureOperations
TB = adsk.fusion.TemporaryBRepManager.get()
REF = r'E:\Claude Stuff\CAD\controller'


def pv(n):
    """User parameter value in mm."""
    return des.userParameters.itemByName(n).value * 10


def setp(name, expr, comment='', unit='mm'):
    """Create or update a User Parameter."""
    p = des.userParameters.itemByName(name)
    if p: p.expression = expr; p.comment = comment or p.comment
    else: des.userParameters.add(name, VI(expr), unit, comment)


def occ(name):
    r = [o for o in root.occurrences if o.component.name == name]
    return r[0] if r else None


def bodies_of(o):
    out = list(o.bRepBodies)
    for c in o.childOccurrences: out += bodies_of(c)
    return out


def bbox_mm(boxes):
    lo = [min(b.minPoint.x for b in boxes), min(b.minPoint.y for b in boxes), min(b.minPoint.z for b in boxes)]
    hi = [max(b.maxPoint.x for b in boxes), max(b.maxPoint.y for b in boxes), max(b.maxPoint.z for b in boxes)]
    return [round(v * 10, 2) for v in lo + hi]


def bb(b):
    x = b.boundingBox
    return [round(v * 10, 2) for v in (x.minPoint.x, x.minPoint.y, x.minPoint.z, x.maxPoint.x, x.maxPoint.y, x.maxPoint.z)]


def tbox(x0, y0, z0, x1, y1, z1):
    """Temporary box body from mm corners."""
    c = P3((x0 + x1) / 20, (y0 + y1) / 20, (z0 + z1) / 20)
    ob = adsk.core.OrientedBoundingBox3D.create(c, V3(1, 0, 0), V3(0, 1, 0), abs(x1 - x0) / 10, abs(y1 - y0) / 10, abs(z1 - z0) / 10)
    return TB.createBox(ob)


def tcyl(x, y, z0, z1, d):
    return TB.createCylinderOrCone(P3(x / 10, y / 10, z0 / 10), d / 20, P3(x / 10, y / 10, z1 / 10), d / 20)


def inter_vol(a, b):
    ta = TB.copy(a); TB.booleanOperation(ta, TB.copy(b), adsk.fusion.BooleanTypes.IntersectionBooleanType)
    return ta.volume * 1000  # mm3


def run(_context=None):
    # The Fusion MCP wrapper calls run() after exec'ing a script; a missing run() raises and
    # rolls the whole script's transaction back, so every script gets this no-op.
    pass
