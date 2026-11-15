# Caster support

Two versions of the part that holds the nylon caster
([GrabCAD](https://grabcad.com/library/nylon-caster-wheel-1): plate 38 x 32 mm, Ø4 holes on
30 x 23 mm, 34 mm tall, Ø25.5 wheel) under the front of the waffle plate (the edge away from the drive wheels), on the robot's
centre line at y = -70 (the drive wheels are at y = +75). The plate sits turned 90 deg under the wheels and caster, as on the real robot. Pick one to print.

| | A: pillars | B: direct |
|---|---|---|
| Support | 4 mm plate (the motor bracket flange's thickness) + 4 printed Ø8 x 9.75 mm pillars | the same plate + a 38 x 32 x 9.75 mm boss |
| Caster screws to | the printed pillars (metal pillars not used) | the boss |
| Plate underside to floor | 4 + 9.75 + 34 = **47.75 mm** | 4 + 9.75 + 34 = **47.75 mm** |
| Target (bracket + TT motor + Ø65 wheel) | 47.75 mm | 47.75 mm |
| STL | `caster_support_A.stl` (64 x 59 x 13.75) | `caster_support_B.stl` (64 x 59 x 13.75) |

The Fusion design is **"Caster Mount"**, built by `CAD/scripts/cst_*.py`: `cst_params.py`
(all sizes), `cst_build.py` (rebuilds both versions and the stand-in caster), `cst_checks.py`,
`cst_export.py`, `cst_shots.py`.

## Fitting

- **Support to plate**: 4 x M3 x 16 screws, heads on top of the plate, down through the Ø4.5
  plate holes at (±27, -71) and (±7, -34) into M3 nuts pressed into the traps on the
  support's underside. No spanner needed under the robot.
- **Caster** (fit it to the support first; the screw heads end up against the plate):
  4 x M3 x 20 countersunk screws from the support's top face down through the pillars (A) or
  the boss (B) and the caster plate, nuts under the caster plate.

## Printing

Top face (the face against the plate) on the bed; the pillars (A) or boss (B) point up. No supports: the
holes are upright, the nut traps open upward and the countersinks narrow upward at 45 deg.
Edges follow `CAD/design_rules.md`: upright corners rounded (R4 plate, R3 boss), 0.4 mm
chamfers only on the hole and nut-trap rims. Infill 40 %.

Sliced with `CAD/scripts/slice_pro.sh`, the professional's profile at 0.28 mm, 40 % infill,
skirt (engine's time estimate from the last TIME_ELAPSED line; the ;TIME header is a placeholder):

| Part | G-code | Layers | Time | Filament |
|---|---|---|---|---|
| A | `CasterSupportA_Ender3Pro.gcode`, `CasterSupportA_Ender5Plus.gcode` | 45 | about 35 min | 4.47 m, about 13 g |
| B | `CasterSupportB_Ender3Pro.gcode`, `CasterSupportB_Ender5Plus.gcode` | 45 | about 41 min | 6.37 m, about 19 g |

## Check results (`cst_checks.py`)

| Check | A | B | Required |
|---|---|---|---|
| Caster wheel bottom vs floor | 0.00 | 0.00 | ±0.5 |
| Overlap with plate / caster | 0 | 0 | 0 |
| Screws over real Ø4.5 plate holes | 4/4, on centre | 4/4 | all |
| Nut trap to pillar / boss, in plan | 8.65 mm | 4.71 mm | ≥ 1 |
| Top face on solid plate | 77 % | 77 % | ≥ 50 % (bracket flange area: 38 %) |

## Still unverified

1. The caster's plate thickness (2.5), wheel width (15) and trail (8) are guessed from the
   photo; they only affect the stand-in model, not the printed parts.
2. Measure the real caster's hole spacing (30 x 23) before printing.
