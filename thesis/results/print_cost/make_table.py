"""Build print_cost.csv from the latest G-code files in CAD/out.

Run from the BLOC-BOT root:  python thesis/results/print_cost/make_table.py
Filament is summed from the E moves (gcode_filament.py) because the Cura headers in
these files are placeholders (0 m, 6666 s). Print time is Cura's last ;TIME_ELAPSED.
"""
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from gcode_filament import AREA, filament_mm  # noqa: E402

DENSITY = 1.27  # g/cm3, typical PETG (check the spool label)
PRICE = 4.0     # DA per gram: 4000 DA/kg (user, 2026-10-20)
WASTE = 0.05    # 0-5 % of the plastic is wasted (user estimate)

GCODE_PARTS = [
    ("Motor brackets (left + right)", "CAD/out/motor_bracket/MotorBracket_Ender3Pro_pro.gcode"),
    ("HBloc_L (line sensor)", "CAD/out/hbloc/HBlocL_Ender3Pro_pro.gcode"),
    ("VBloc", "CAD/out/vbloc/VBloc_Ender3Pro_pro.gcode"),
    ("HBloc", "CAD/out/hbloc/HBloc_Ender3Pro_pro.gcode"),
    ("Caster support B", "CAD/out/CasterSupportB_Ender5Plus.gcode"),
]

# Parts with no G-code: (name, grams, hours, source)
MANUAL_PARTS = [
    ("Waffle plate (old version, in use)", 200.0, 45.0,
     "user: ~200 g, 1 d 21 h, early print before the team had experience"),
]


def elapsed_s(path):
    last = 0.0
    with open(path, errors="ignore") as f:
        for line in f:
            if line.startswith(";TIME_ELAPSED:"):
                last = float(line.split(":", 1)[1])
    return last


def hm(seconds):
    h, m = divmod(round(seconds / 60), 60)
    return f"{h} h {m:02d}"


rows = []
for name, path in GCODE_PARTS:
    cm3 = filament_mm(path) * AREA / 1000
    rows.append((name, cm3 * DENSITY, elapsed_s(path), "G-code: " + path))
for name, grams, hours, source in MANUAL_PARTS:
    rows.append((name, grams, hours * 3600, source))

out = os.path.join(os.path.dirname(__file__), "print_cost.csv")
with open(out, "w", newline="") as fh:
    w = csv.writer(fh)
    w.writerow(["part", "mass_g", "print_time", "cost_DA_no_waste", "cost_DA_5pct_waste", "source"])
    for name, g, s, src in rows:
        w.writerow([name, round(g, 1), hm(s), round(g * PRICE), round(g * (1 + WASTE) * PRICE), src])
    g_tot = sum(r[1] for r in rows)
    s_tot = sum(r[2] for r in rows)
    w.writerow(["TOTAL", round(g_tot, 1), hm(s_tot), round(g_tot * PRICE),
                round(g_tot * (1 + WASTE) * PRICE), ""])

with open(out) as fh:
    print(fh.read())
