# Pack print-ready parts into a 3MF file (one object per part), the format the BLOC-BOT print
# files are kept in. Uses only the Python standard library, so it runs inside Fusion's script
# runner (exec(open(r'E:\Claude Stuff\CAD\scripts\make_3mf.py').read())) or any Python 3.
#
#   parts = [('VBloc arm', read_stl(r'...\vbloc_base.stl')), ...]
#   write_3mf(r'...\VBloc.3mf', 'VBloc', parts)
#
# Coordinates are millimetres. The parts keep their print orientation and their positions
# relative to each other; the whole plate is moved so its centre is at (110, 110), the middle
# of a 220 x 220 bed (Ender 3 Pro), and each part sits on z = 0. Larger beds: use the
# slicer's "arrange" or "centre" command after opening.
import struct
import zipfile
from xml.sax.saxutils import escape


def read_stl(path):
    """Triangles [(a, b, c), ...] from a binary STL, each vertex an (x, y, z) tuple in mm."""
    data = open(path, 'rb').read()
    n = struct.unpack('<I', data[80:84])[0]
    tris = []
    for i in range(n):
        v = struct.unpack('<9f', data[84 + 50 * i + 12: 84 + 50 * i + 48])
        tris.append((v[0:3], v[3:6], v[6:9]))
    return tris


def write_3mf(path, title, parts, centre=(110.0, 110.0)):
    """parts: [(name, triangles), ...]. Returns (objects, triangles, plate size x, y)."""
    xs = [p[0] for _, tris in parts for t in tris for p in t]
    ys = [p[1] for _, tris in parts for t in tris for p in t]
    dx = centre[0] - (min(xs) + max(xs)) / 2
    dy = centre[1] - (min(ys) + max(ys)) / 2
    objects = []
    total = 0
    for i, (name, tris) in enumerate(parts, start=1):
        dz = -min(p[2] for t in tris for p in t)
        index = {}
        verts = []
        faces = []
        for t in tris:
            ids = []
            for p in t:
                key = (round(p[0] + dx, 4), round(p[1] + dy, 4), round(p[2] + dz, 4))
                if key not in index:
                    index[key] = len(verts)
                    verts.append(key)
                ids.append(index[key])
            if ids[0] != ids[1] and ids[1] != ids[2] and ids[0] != ids[2]:
                faces.append(ids)
        total += len(faces)
        vx = ''.join('<vertex x="%.4f" y="%.4f" z="%.4f"/>' % v for v in verts)
        fx = ''.join('<triangle v1="%d" v2="%d" v3="%d"/>' % tuple(f) for f in faces)
        objects.append('<object id="%d" name="%s" type="model"><mesh><vertices>%s</vertices>'
                       '<triangles>%s</triangles></mesh></object>' % (i, escape(name, {'"': '&quot;'}), vx, fx))
    items = ''.join('<item objectid="%d"/>' % i for i in range(1, len(parts) + 1))
    model = ('<?xml version="1.0" encoding="UTF-8"?>\n'
             '<model unit="millimeter" xml:lang="en-US" '
             'xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02">'
             '<metadata name="Title">%s</metadata>'
             '<resources>%s</resources><build>%s</build></model>' % (escape(title), ''.join(objects), items))
    types = ('<?xml version="1.0" encoding="UTF-8"?>\n'
             '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
             '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
             '<Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/>'
             '</Types>')
    rels = ('<?xml version="1.0" encoding="UTF-8"?>\n'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            '<Relationship Target="/3D/3dmodel.model" Id="rel0" '
            'Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/></Relationships>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', types)
        z.writestr('_rels/.rels', rels)
        z.writestr('3D/3dmodel.model', model)
    return len(parts), total, max(xs) - min(xs), max(ys) - min(ys)
