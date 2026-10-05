# Thesis tracking log

One running record of every figure, graph, reference and search made for the thesis.
Add a row whenever one is created or used. IDs never change once given, even if an item is
dropped (mark it `dropped`).

## Figures

Images are in `CAD/out/`. LaTeX blocks are in `thesis/figures.tex`.

| ID | File | LaTeX label | How it was made | Date | Status |
|---|---|---|---|---|---|
| F01 | shot_iso.png | fig:bracket-iso | Fusion, iso top-right, plate hidden, 1600x1100 | 2026-10-05 | ready |
| F02 | shot_top.png | fig:bracket-top | Fusion, top view, plate hidden | 2026-10-05 | ready |
| F03 | shot_front.png | fig:bracket-front | Fusion "front" view (looks along +Y), plate hidden | 2026-10-05 | ready |
| F04 | shot_side.png | fig:sensor-side | Fusion left view; plate, wheel, left bracket hidden | 2026-10-05 | ready |
| F05 | shot_with_plate_below.png | fig:plate-below | Fusion iso bottom-right, plate shown | 2026-10-05 | ready |
| F06 | shot_with_plate_top.png | fig:plate-top | Fusion iso top-right, plate shown | 2026-10-05 | ready |
| F07 | angle_bracket_iso_front.png | fig:bracket-angles (a) | Fusion iso top-right, bracket only | 2026-10-05 | ready |
| F08 | angle_bracket_iso_rear.png | fig:bracket-angles (b) | Fusion iso top-left, bracket only | 2026-10-05 | ready |
| F09 | angle_bracket_under_front.png | fig:bracket-angles (c) | Fusion iso bottom-right, bracket only | 2026-10-05 | ready |
| F10 | angle_bracket_under_rear.png | fig:bracket-angles (d) | Fusion iso bottom-left, bracket only | 2026-10-05 | ready |
| F11 | angle_assembly_under_front.png | fig:assembly-below (a) | Fusion iso bottom-right; bracket, motor, disc, LM393 | 2026-10-05 | ready |
| F12 | angle_assembly_under_rear.png | fig:assembly-below (b) | Fusion iso bottom-left; bracket, motor, disc, LM393 | 2026-10-05 | ready |

## Graphs

Plots made from data (with the data file and the script that made them).

| ID | File | LaTeX label | Data source | Script | Date | Status |
|---|---|---|---|---|---|---|
| (none yet) | | | | | | |

## References

Entries are in `thesis/references.bib`.

| Key | What it is | Used for | Verified? |
|---|---|---|---|
| ttmotor_model | TT Motor Dual Shaft CAD model (Oj Simpson) | All motor dimensions | URL missing |
| lm393_sensor_model | Speed sensor MH-Sensor-Series CAD model (P. Pennel) | Board, fork and disc dimensions | URL missing |
| fusion360 | Autodesk Fusion | CAD modelling, checks, renders | yes |
| fusion_api | Fusion API Reference Manual | Scripted modelling and checks | URL missing |
| git | Git 2.55.0 | Version control of the project | yes |

## Searches and lookups

Every search or data lookup that fed a design decision, with what it found.

| # | Date | Where | Query / action | Result | Used for |
|---|---|---|---|---|---|
| S01 | 2026-10-05 | Fusion cloud (Bloc-Bot) | Document search "waffle" | Found "waffle plate" (210 x 210 x 10 mm) | Plate geometry source |
| S02 | 2026-10-05 | Waffle plate model | All vertical hole faces | 108 x Ø3.2, 192 x Ø4.5, 44 x Ø6.0 through-holes, with centres | Slot placement |
| S03 | 2026-10-05 | Waffle plate model | Outline and hex tabs | Side edge X = 90; front-right corner cut-out r 27.06 (others r 29.21); pillar hex tabs circumradius 10 at (±95, ±13.66), (±13.66, ±95) | Placement rules |
| S04 | 2026-10-05 | TT motor model | Faces, holes, bosses | Gearbox 22.5 x 18.8; holes 20.6 behind shaft at ±8.75; front end +11.2; strap to −28.15; flat face = inner side | Tab, holes, motor orientation |
| S05 | 2026-10-05 | LM393 sensor model | Board, fork, hole | PCB 31.25 x 14.25 x 1.6; fork gap 5.0; slot floor 3.65 above PCB underside; hole Ø2.7 at 21.06 from fork centre | Sensor layout, column |
| S06 | 2026-10-05 | Encoder disc model | Faces | Ø24, 1.4 thick, 16 windows r 6.61–11.5, hub Ø7.2 x 4 | Disc position, flange relief |
| S07 | 2026-10-05 | Fusion API docs | Member search `readonly\|editable` | PersonalUseLimits can be read but not changed by scripts | Document-limit troubleshooting |
| S08 | 2026-10-05 | Fusion licensing | Licence query | Personal Use: 10 editable documents | Document-limit troubleshooting |
