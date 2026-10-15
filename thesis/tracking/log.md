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
| F13 | controller_iso.png | fig:controller-iso | Fusion iso top-right, closed case with references, 1800x1100 (CAD/scripts/ctl_shots.py) | 2026-10-05 | ready |
| F14 | controller_top.png | fig:controller-top | Fusion top view, closed case (ctl_shots.py) | 2026-10-05 | ready |
| F15 | controller_exploded.png | fig:controller-exploded | Custom-graphics copies lifted 0/35/75/100 mm (bottom/parts/top/caps), camera (30,-38,26) (ctl_shots.py) | 2026-10-05 | ready |
| F16 | controller_section.png | fig:controller-section | Custom-graphics copies cut by the vertical plane through the directional cross (57.5,-9.5) and display (0,19.7) centres (ctl_shots.py); F13-F16 re-shot 2026-10-14 (cross of 4 + 2 buttons, rim chamfer) | 2026-10-05 | ready |
| F17 | electronics/robot_circuit.svg | (not added yet) | Hand-written SVG block diagram of the proposed ESP32-S3 robot circuit; convert to PDF before including. 2026-10-10: two DRV8871 boxes replaced by one DRV8833, GPIO 13/14 added | 2026-10-06 | draft |
| F18 | caster/caster_A_iso.png | fig:caster-iso (left) | Fusion "Caster Mount", ortho, eye (-120,-200,-120) target (0,-65,-20); support A, pillars, caster, plate (CAD/scripts/cst_shots.py) | 2026-10-14 | ready |
| F19 | caster/caster_B_iso.png | fig:caster-iso (right) | As F18, support B and caster (cst_shots.py) | 2026-10-14 | ready |
| F20 | caster/caster_A_side.png | fig:caster-side (left) | Fusion ortho from -X, eye (-300,-62,-24); support A, pillars, caster (cst_shots.py) | 2026-10-14 | ready |
| F21 | caster/caster_B_side.png | fig:caster-side (right) | As F20, support B and caster (cst_shots.py) | 2026-10-14 | ready |
| F22 | caster/caster_A_part.png | fig:caster-parts (left) | Fusion ortho from below, eye (-90,-150,-100); support A alone (cst_shots.py) | 2026-10-14 | ready |
| F23 | caster/caster_B_part.png | fig:caster-parts (right) | As F22, support B alone (cst_shots.py) | 2026-10-14 | ready |

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
| grabcad_rc_model | GrabCAD Remote Controller (Slyman Rambo) | Controller shell shape (chamfered outline) | URL missing |
| esp32_devkit_model | ESP32 DevKit V1 STEP model | ESP32 board, holes, micro-USB | author, URL missing |
| oled_242_model | 2.42" 12864 OLED STEP model | Display window, lip, bosses | author, URL missing |
| ky023_model | KY-023 joystick STEP model | Opening, travel, posts | author, URL missing |
| b3f_switch_model | OMRON B3F-4050 STEP model | Switch heights, pegs, caps | author, URL missing |
| tp4056_model | TP4056 Type-C STEP model | Charger pocket, USB-C opening | author, URL missing |
| mt3608_model | MT3608 STEP model | Boost board pocket | author, URL missing |
| drv8833_datasheet | TI DRV8833 datasheet (SLVSAR1E) | Motor driver limits, wiring, control tables | yes |
| caster_wheel_model | GrabCAD Nylon Caster Wheel (AJ, 2018) | Caster plate, hole pattern, height, wheel size | yes (page text; files not downloaded) |

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
| S09 | 2026-10-05 | OLED model | Bodies and faces | PCB 72 x 42.8 x 1.2; holes Ø3.0 on 68 x 38.8; glass 62 x 39.8 x 3.7; active area (grey face) 56.6 x 29.5, 2.65 off PCB centre; header pins 8.54 behind the PCB | Window, lip, bosses |
| S10 | 2026-10-05 | KY-023 model | Faces; cap rotated about the gimbal pivot | Pivot at the pin axes; cap skirt torus = sphere r 13.65 about the pivot; cap meets the module case at 30 deg -> 29 deg travel; holes Ø3.0 at (-12.5/14, -11.25/9.25) | Opening Ø28.5, posts |
| S11 | 2026-10-05 | B3F-4050 model | Bodies and horizontal faces | Seating plane z 0; plunger top 7.35; pegs Ø1.6 x 1.35 at y ±4.5; model includes a 13 x 13 keycap (ignored) | Pillars, cap heights |
| S12 | 2026-10-05 | ESP32 model | Bodies, PCB holes | PCB 28.3 x 51.2 x 1.6; holes Ø2.9 at 23.5 x 46.6; pins 8.2 below; micro-USB 7.98 x 2.94 protruding 0.57 | Standoffs, side USB opening |
| S13 | 2026-10-05 | TP4056 / MT3608 models | Bodies, PCB holes | TP4056 28.3 x 17.4 x 1.5, USB-C 8.42 x 2.79, no mounting holes; MT3608 37.2 x 17.23 x 1.52, 6.45 tall, no mounting holes | Clip pockets, USB-C opening |
| S14 | 2026-10-05 | GrabCAD RC Transmitter | Import and outline | 200 x 170 x 36 box with chamfered corners, two halves, no grips | Shell shape |
| S15 | 2026-10-10 | ti.com, DRV8833 product page and datasheet SLVSAR1E (Rev. E, Jul 2015) | Supply range, current, protection, logic levels, pin functions, control tables | VM 2.7–10.8 V (11.8 V abs. max); 1.5 A RMS / 2 A peak per bridge (HTSSOP/WQFN), 0.5 A RMS (TSSOP); OCP 2–3.3 A, retry 1.35 ms; UVLO below 2.5 V; VIH 2 V (nSLEEP 2.5 V); nSLEEP high to enable, 500 kΩ pull-down; xISEN to GND without current control | Switch from DRV8871 to DRV8833: wiring, pin map, risks |
| S16 | 2026-10-14 | grabcad.com/library/nylon-caster-wheel-1 (built-in browser; plain fetch got 403) | Caster sizes | Fixing plate 38 x 32, holes Ø4 on 30 x 23, overall height 34, wheel Ø25.5; files need a login | Both caster supports |
| S17 | 2026-10-14 | CAD/3mf/from_professional/Bracket.3mf | Vertex heights of the three parts | Flange 4 mm thick (12 mm only at the tab socket); flange X 33..90, Y 47.45..101.06 | Support plate thickness (flange_t) |
| S18 | 2026-10-14 | Waffle plate (copied into "Caster Mount") | Holes near the rear centre line; underside solid fraction | Plate turned 90 deg as on the real robot; Ø4.5 holes at (±27, -71) and (±7, -34) used; underside flat with holes; 77 % of the support top on solid plate, bracket flange area 38 % | Fixing screw positions, contact check |
