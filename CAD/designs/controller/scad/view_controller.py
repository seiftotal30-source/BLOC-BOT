import cadquery as cq
from OCP.StlAPI import StlAPI_Reader
from OCP.TopoDS import TopoDS_Shape

OUT = r"C:\Users\Wassim\Desktop\PROJECT\BLOC-BOT\CAD\designs\controller\stl"
GAP = 25  # mm between parts in the exploded view


def load_stl(name):
    shape = TopoDS_Shape()
    StlAPI_Reader().Read(shape, OUT + "\\" + name)
    return cq.Shape.cast(shape)


parts = [
    ("bottom", "controller_bottom.stl", "steelblue"),
    ("top", "controller_top.stl", "orange"),
    ("caps", "controller_caps.stl", "green"),
]

z = 0
for name, file, color in parts:
    s = load_stl(file)
    bb = s.BoundingBox()
    s = s.translate(cq.Vector(0, 0, z - bb.zmin))
    z += bb.zlen + GAP
    show_object(s, name=name, options={"color": color, "alpha": 0.0})
    print(name, round(bb.xlen, 1), "x", round(bb.ylen, 1), "x", round(bb.zlen, 1), "mm")
