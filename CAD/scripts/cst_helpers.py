# Shared helpers for the Caster Mount build scripts (exec'd inside Fusion).
#
# Every caster script starts by exec'ing this file. It refuses to run unless the active
# design carries the BLOC-BOT "CasterMount" attribute (set by cst_new_doc.py), the guard
# that keeps these scripts away from "waffle plate", "Motor Bracket" and the others.
#
# Frame: the motor bracket's. Origin = plate centre on its underside, X right, Y forward
# (the drive wheels are at Y = +75), Z up; the floor is at Z = -floor_drop.
import adsk.core, adsk.fusion, math
app = adsk.core.Application.get()
des = adsk.fusion.Design.cast(app.activeProduct)
_tag = des.attributes.itemByName('BLOCBOT', 'doc') if des else None
assert _tag and _tag.value == 'CasterMount', 'WRONG DOC: ' + app.activeDocument.name
root = des.rootComponent
VI = adsk.core.ValueInput.createByString
P3 = adsk.core.Point3D.create
V3 = adsk.core.Vector3D.create
FO = adsk.fusion.FeatureOperations
ED = adsk.fusion.ExtentDirections
TB = adsk.fusion.TemporaryBRepManager.get()


def run(_context=None):
    """The MCP runner calls run() after exec; without it every change is rolled back."""
    pass


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


def new_comp(name):
    """Delete the component if it exists, then add it again empty."""
    o = occ(name)
    if o: o.deleteMe()
    o = root.occurrences.addNewComponent(adsk.core.Matrix3D.create())
    o.component.name = name
    return o


def bb(b):
    x = b.boundingBox
    return [round(v * 10, 2) for v in (x.minPoint.x, x.minPoint.y, x.minPoint.z, x.maxPoint.x, x.maxPoint.y, x.maxPoint.z)]


def on(a, b, tol=1e-3):
    """Equal within tol (values in cm, as the API gives them)."""
    return abs(a - b) < tol


# ---- sketches (mm in, cm to the API). Planes are parallel to XY, so sketch x, y = model x, y.
def zplane(C, z, name):
    pi = C.constructionPlanes.createInput()
    pi.setByOffset(C.xYConstructionPlane, VI('%r mm' % z))
    pl = C.constructionPlanes.add(pi); pl.name = name; pl.isLightBulbOn = False
    return pl


def sketch(C, z, name):
    pl = C.xYConstructionPlane if z == 0 else zplane(C, z, name + '_plane')
    sk = C.sketches.add(pl); sk.name = name
    return sk


def poly(sk, pts):
    L = sk.sketchCurves.sketchLines
    pp = [P3(x / 10, y / 10, 0) for x, y in pts]
    first = prev = L.addByTwoPoints(pp[0], pp[1])
    for p in pp[2:]:
        prev = L.addByTwoPoints(prev.endSketchPoint, p)
    L.addByTwoPoints(prev.endSketchPoint, first.startSketchPoint)


def circle(sk, x, y, d):
    sk.sketchCurves.sketchCircles.addByCenterRadius(P3(x / 10, y / 10, 0), d / 20)


def hexagon(sk, x, y, af):
    """Hexagon across flats af, two flats parallel to X."""
    r = af / math.sqrt(3)
    poly(sk, [(x + r * math.cos(math.radians(a)), y + r * math.sin(math.radians(a))) for a in range(0, 360, 60)])


def profiles(sk):
    c = adsk.core.ObjectCollection.create()
    for p in sk.profiles: c.add(p)
    return c


def extrude(C, profs, dist, up, op, bodies=None, name=None):
    """Extrude dist mm up (+Z) or down (-Z) from the sketch plane."""
    ei = C.features.extrudeFeatures.createInput(profs, op)
    ei.setOneSideExtent(adsk.fusion.DistanceExtentDefinition.create(VI('%r mm' % dist)),
                        ED.PositiveExtentDirection if up else ED.NegativeExtentDirection)
    if bodies: ei.participantBodies = bodies
    f = C.features.extrudeFeatures.add(ei)
    if name: f.name = name
    return f


def upright_edges(body, xy_pts, z_top, z_bot):
    """Vertical straight edges of body at the plan points xy_pts (mm), spanning z_top..z_bot."""
    out = adsk.core.ObjectCollection.create()
    for e in body.edges:
        if not isinstance(e.geometry, adsk.core.Line3D): continue
        s, t = e.startVertex.geometry, e.endVertex.geometry
        if not (on(s.x, t.x) and on(s.y, t.y)): continue
        if not ({round(s.z * 10, 2), round(t.z * 10, 2)} == {round(z_top, 2), round(z_bot, 2)}): continue
        if any(on(s.x, x / 10) and on(s.y, y / 10) for x, y in xy_pts): out.add(e)
    return out


def fillet(C, edges, r, name):
    fi = C.features.filletFeatures.createInput()
    fi.addConstantRadiusEdgeSet(edges, VI('%r mm' % r), False)
    f = C.features.filletFeatures.add(fi); f.name = name
    return f


def chamfer(C, edges, d, name):
    ci = C.features.chamferFeatures.createInput2()
    ci.chamferEdgeSets.addEqualDistanceChamferEdgeSet(edges, VI('%r mm' % d), False)
    f = C.features.chamferFeatures.add(ci); f.name = name
    return f


def rim_edges(body, z, radius=None, hex_af=None, pts=None):
    """Hole rims on the plane z (mm): circles of radius (mm) or the straight sides of
    hexagons of across-flats hex_af centred on pts."""
    out = adsk.core.ObjectCollection.create()
    for e in body.edges:
        g = e.geometry; b = e.boundingBox
        if not (on(b.minPoint.z, z / 10) and on(b.maxPoint.z, z / 10)): continue
        if radius is not None and isinstance(g, adsk.core.Circle3D) and on(g.radius, radius / 10):
            if pts is None or any(on(g.center.x, x / 10) and on(g.center.y, y / 10) for x, y in pts):
                out.add(e)
        if hex_af is not None and isinstance(g, adsk.core.Line3D):
            m = P3((b.minPoint.x + b.maxPoint.x) / 2, (b.minPoint.y + b.maxPoint.y) / 2, 0)
            if any(on(math.hypot(m.x - x / 10, m.y - y / 10), hex_af / 20, 2e-3) for x, y in pts):
                out.add(e)
    return out


# ---- temporary B-Rep primitives (mm), for the reference models and the checks
def tbox(x0, y0, z0, x1, y1, z1):
    c = P3((x0 + x1) / 20, (y0 + y1) / 20, (z0 + z1) / 20)
    ob = adsk.core.OrientedBoundingBox3D.create(c, V3(1, 0, 0), V3(0, 1, 0),
                                                abs(x1 - x0) / 10, abs(y1 - y0) / 10, abs(z1 - z0) / 10)
    return TB.createBox(ob)


def tcyl(p0, p1, d):
    """Cylinder from point p0 to p1 (mm tuples), diameter d."""
    return TB.createCylinderOrCone(P3(*[v / 10 for v in p0]), d / 20, P3(*[v / 10 for v in p1]), d / 20)


def tunion(a, b):
    TB.booleanOperation(a, b, adsk.fusion.BooleanTypes.UnionBooleanType); return a


def tcut(a, b):
    TB.booleanOperation(a, b, adsk.fusion.BooleanTypes.DifferenceBooleanType); return a


def inter_vol(a, b):
    """Overlap volume (mm3) of two bodies (real or temporary), via temporary copies."""
    ta, tb_ = TB.copy(a), TB.copy(b)
    TB.booleanOperation(ta, tb_, adsk.fusion.BooleanTypes.IntersectionBooleanType)
    return round(ta.volume * 1000, 3) if ta and ta.volume > 0 else 0.0


def add_base_body(C, tbody, name):
    """Put a temporary body into component C through a base feature."""
    base = C.features.baseFeatures.add()
    base.startEdit()
    b = C.bRepBodies.add(tbody, base)
    base.finishEdit()
    base.name = name
    b = C.bRepBodies.item(C.bRepBodies.count - 1); b.name = name
    return b
