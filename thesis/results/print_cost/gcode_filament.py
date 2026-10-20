"""Sum filament actually extruded in G-code files (handles M82/M83 and G92 E resets)."""
import math
import re
import sys

DIAM = 1.75
AREA = math.pi * (DIAM / 2) ** 2  # mm^2

E_RE = re.compile(r"\bE(-?\d*\.?\d+)")


def filament_mm(path):
    absolute = True
    e_pos = 0.0
    total = 0.0
    with open(path, "r", errors="ignore") as f:
        for line in f:
            code = line.split(";", 1)[0].strip()
            if not code:
                continue
            word = code.split()[0].upper()
            if word in ("M82", "G90"):  # G90/G91 also switch E in Marlin
                absolute = True
            elif word in ("M83", "G91"):
                absolute = False
            elif word == "G92":
                m = E_RE.search(code)
                if m:
                    e_pos = float(m.group(1))
            elif word in ("G0", "G1", "G2", "G3"):
                m = E_RE.search(code)
                if not m:
                    continue
                e = float(m.group(1))
                if absolute:
                    delta = e - e_pos
                    e_pos = e
                else:
                    delta = e
                total += delta  # retracts and un-retracts cancel out
    return total


for p in sys.argv[1:]:
    mm = filament_mm(p)
    cm3 = mm * AREA / 1000
    print(f"{p}\t{mm / 1000:.2f} m\t{cm3:.2f} cm3")
