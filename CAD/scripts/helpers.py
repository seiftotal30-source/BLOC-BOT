# Shared helpers for the Motor Bracket build scripts (exec'd inside Fusion).
#
# Every script starts by exec'ing this file. It refuses to run unless the active document is
# "Motor Bracket", the guard that keeps scripts from ever touching the waffle plate document.
# The sketch helpers take model-space coordinates as parameter EXPRESSIONS (e.g.
# 'shaft_y - tab_front_off') and turn each into a driving dimension from the sketch origin,
# so the geometry follows the User Parameters instead of being drawn at fixed numbers.
import adsk.core, adsk.fusion, math
app = adsk.core.Application.get()
assert app.activeDocument.name.startswith('Motor Bracket'), 'WRONG DOC: ' + app.activeDocument.name
des = adsk.fusion.Design.cast(app.activeProduct)
root = des.rootComponent
um = des.unitsManager
VI = adsk.core.ValueInput.createByString
P3 = adsk.core.Point3D.create
FO = adsk.fusion.FeatureOperations
ED = adsk.fusion.ExtentDirections
AX = {'X': 0, 'Y': 1, 'Z': 2}
ev = lambda e: um.evaluateExpression(e, 'mm')  # -> cm
pv = lambda n: des.userParameters.itemByName(n).value * 10  # -> mm


def comp(name):
    return [o for o in root.occurrences if o.component.name == name][0].component


C = comp('Bracket') if any(o.component.name == 'Bracket' for o in root.occurrences) else None


def amap(sk):
    o = sk.sketchToModelSpace(P3(0, 0, 0)); oo = (o.x, o.y, o.z); res = {}
    for key, sv in (('x', P3(1, 0, 0)), ('y', P3(0, 1, 0))):
        p = sk.sketchToModelSpace(sv); d = (p.x - o.x, p.y - o.y, p.z - o.z)
        k = max(range(3), key=lambda j: abs(d[j]))
        res['XYZ'[k]] = (key, oo[k])
    return res


def inplane(sk):
    return [a for a in 'XYZ' if a in amap(sk)]


def normal_axis_val(sk):
    ax = inplane(sk); n = [a for a in 'XYZ' if a not in ax][0]
    o = sk.sketchToModelSpace(P3(0, 0, 0))
    return n, {'X': o.x, 'Y': o.y, 'Z': o.z}[n] * 10


def mpt(sk, c):
    n, nv = normal_axis_val(sk); d = dict(c); d[n] = '%r mm' % nv
    return sk.modelToSketchSpace(P3(ev(d['X']), ev(d['Y']), ev(d['Z'])))


_dimreg = {}


def dimpt(sk, spt, axis, expr):
    """Locate spt along a model axis by an origin dimension driven by expr.
    Fusion rejects a second origin dimension with an identical value, so a point that
    shares a coordinate with an already-dimensioned point is aligned to it instead."""
    key, o = amap(sk)[axis]
    reg = _dimreg.setdefault(sk.entityToken, [])
    val = round(getattr(spt.geometry, key), 6)
    for k2, v2, p2 in reg:
        if k2 == key and abs(v2 - val) < 1e-6 and p2 != spt:
            gc = sk.geometricConstraints
            # equal sketch-x -> vertical alignment, equal sketch-y -> horizontal alignment
            (gc.addVerticalPoints if key == 'x' else gc.addHorizontalPoints)(p2, spt)
            return None
    orient = (adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation if key == 'x'
              else adsk.fusion.DimensionOrientations.VerticalDimensionOrientation)
    tp = P3(spt.geometry.x + 0.3, spt.geometry.y + 0.3, 0)
    d = sk.sketchDimensions.addDistanceDimension(sk.originPoint, spt, orient, tp)
    d.parameter.expression = ('abs(%s)' % expr) if abs(o) < 1e-9 else 'abs((%s) - (%r mm))' % (expr, o * 10)
    reg.append((key, val, spt))
    return d


def rect(sk, c1, c2):
    a = inplane(sk)
    corners = [c1, {a[0]: c2[a[0]], a[1]: c1[a[1]]}, c2, {a[0]: c1[a[0]], a[1]: c2[a[1]]}]
    return poly(sk, corners)


def circle(sk, c, d_expr, dims=True):
    cc = sk.sketchCurves.sketchCircles.addByCenterRadius(mpt(sk, c), ev(d_expr) / 2)
    g = cc.centerSketchPoint.geometry
    sk.sketchDimensions.addDiameterDimension(cc, P3(g.x + 0.5, g.y + 0.5, 0)).parameter.expression = d_expr
    if dims:
        for a in inplane(sk): dimpt(sk, cc.centerSketchPoint, a, c[a])
    return cc


def poly(sk, corners):
    """Closed polyline through model corners (dicts of exprs); every vertex dimensioned."""
    L = sk.sketchCurves.sketchLines; pts = [mpt(sk, c) for c in corners]; lines = []
    first = None; prev = None
    for i in range(len(pts)):
        a = prev if prev is not None else pts[i]
        b = pts[(i + 1) % len(pts)] if i < len(pts) - 1 else first
        ln = L.addByTwoPoints(a, b); lines.append(ln)
        if first is None: first = ln.startSketchPoint
        prev = ln.endSketchPoint
    for ln, c in zip(lines, corners):
        for a in inplane(sk): dimpt(sk, ln.startSketchPoint, a, c[a])
    return lines


def normal_sign(sk, axis):
    o = sk.sketchToModelSpace(P3(0, 0, 0)); px = sk.sketchToModelSpace(P3(1, 0, 0)); py = sk.sketchToModelSpace(P3(0, 1, 0))
    a = adsk.core.Vector3D.create(px.x - o.x, px.y - o.y, px.z - o.z); b = adsk.core.Vector3D.create(py.x - o.x, py.y - o.y, py.z - o.z)
    n = a.crossProduct(b); return [n.x, n.y, n.z][AX[axis]]


def extrude(comp_, sk, profs, op, dist, axis, sign, bodies=None):
    coll = adsk.core.ObjectCollection.create()
    for p in profs: coll.add(p)
    ei = comp_.features.extrudeFeatures.createInput(coll, op)
    d = ED.PositiveExtentDirection if normal_sign(sk, axis) * sign > 0 else ED.NegativeExtentDirection
    ei.setOneSideExtent(adsk.fusion.DistanceExtentDefinition.create(VI(dist)), d)
    if bodies: ei.participantBodies = bodies
    return comp_.features.extrudeFeatures.add(ei)


def oplane(comp_, base, expr, axis, name):
    n = base.geometry.normal; s = [n.x, n.y, n.z][AX[axis]]
    pi = comp_.constructionPlanes.createInput(); pi.setByOffset(base, VI(expr if s > 0 else '-(%s)' % expr))
    p = comp_.constructionPlanes.add(pi); p.name = name; return p


def bb(b):
    x = b.boundingBox
    return [round(v * 10, 2) for v in (x.minPoint.x, x.minPoint.y, x.minPoint.z, x.maxPoint.x, x.maxPoint.y, x.maxPoint.z)]


def on(v, target_mm, tol=0.02):
    return abs(v * 10 - target_mm) < tol
