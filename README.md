# BLOC-BOT: TT motor bracket

A 3D-printed bracket that hangs a dual-shaft TT motor under the BLOC-BOT waffle plate
(210 x 210 x 10 mm). It also holds the LM393 slotted sensor that reads the encoder disc on
the motor's inner shaft. Two motors and a rear caster make up the classroom robot.

The Fusion design is **"Motor Bracket"** in the Bloc-Bot project. It is fully parametric, so
change dimensions in *Modify > Change Parameters*. The plate is read from the **"waffle
plate"** document and never modified.

## Folder layout

| Path | What it is |
|---|---|
| `CAD/scripts/` | Fusion Python scripts that build, check and export the bracket |
| `CAD/out/` | Printable STLs, the `.f3d` backup of the design, screenshots |
| `CAD/ref/` | Reference models extracted from the zips (motor, sensor, encoder disc) |
| `CAD/*.zip` | The reference models as downloaded |
| `File 1` to `File 4` | Saved design session transcripts |

## Scripts (run inside Fusion, with "Motor Bracket" as the active document)

- `helpers.py`: shared sketch/extrude helpers; refuses to run in any other document.
- `build_bracket.py`: rebuilds the bracket component from the User Parameters.
- `checks.py`: clearance and interference checks against the reference parts.
- `export_stl.py`: writes print-oriented, watertight STLs to `CAD/out/`.

## Current design (right side; the left is a mirror)

- Axle at **Y = +75**, gearbox outer face at **X = 90**, wheel inner face at **X = 95**.
- Flange 57 x 53.6 x 4 mm, flat on the plate underside, three slots over real plate holes.
- Motor tab on the gearbox's flat (inner) face with two Ø3.4 holes and hex nut traps.
- LM393 lies flat **under** the disc with the fork pointing up at the disc's bottom rim,
  screwed to a column. (A vertical sensor wall was the original plan; it doesn't fit: the
  board would hit the floor or the plate.)
- Prints flange-down with no supports.

### Last check results

| Check | Result | Required |
|---|---|---|
| Disc to tab | 3.25 mm | >= 2 |
| Disc faces to fork arms | 1.8 mm each side | no contact |
| Wheel to bracket | 5.0 mm | >= 3 |
| Wheel to plate edge / hex tabs | 5.0 mm | >= 5 |
| Pillar hex tabs covered? | no, 2.94 mm clear in plan | not covered |
| Plate underside above floor | 47.75 mm | about 47 |
| Lowest bracket point above floor | 17.45 mm | |
| Nuts to disc / fork | >= 3 mm for every usable hole | >= 3 |

## Assembly notes

- **Left motor goes in upside-down** (turned 180 deg about the front-back axis), so its
  flat face points inward like the right one.
- Motor bolts: M3x30 work, but their tails stick out 6.9 mm past the tab; **M3x25** is enough
  with the nut traps. Use low heads on the wheel side (the gap to the wheel is 5 mm).
- Board screw: the sensor model's hole is Ø2.7. Use M3 if it fits your board, else M2.5.
- Print `CAD/out/test_tab.stl` first and fit it on a real motor before printing the brackets.

## Still unverified

1. **Gearbox-to-wheel gap**: assumed 5 mm (`wheel_gap`), because no wheel model was
   available. It sets where the motor sits, so measure it on the real wheel.
2. **Encoder disc**: the model is 16-slot Ø24, not 20-slot Ø20.
3. **Board hole size** (see above).
4. The board is held by a single screw and could pivot; add a locating pin or glue if needed.
