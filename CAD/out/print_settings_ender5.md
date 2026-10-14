# Print settings for H bloc and VBloc

## Ender 3 Pro and Ender 5 Plus with the professional's profile (current default)

The professional's own Cura profile, `CAD/out/print_profiles/CE3PRO_pro.curaprofile` (Cura
5.13, Creality "Standard" quality, made for his Ender 3 Pro), is the default. Import it in the
Cura app under Preferences > Profiles > Import. `CAD/scripts/slice_pro.sh` slices with the same
values for either printer, and the parts sit in the middle of the bed:

- `ender3pro`: Creality Ender 3 Pro, 220 x 220 bed, no bed levelling probe.
- `ender5plus`: Creality Ender 5 Plus, 350 x 350 bed, starts with G28 + G29 (BLTouch).
  (`slice_ender5plus.sh` is the same as `slice_pro.sh ender5plus`.)

Cura's start and end G-code for these printers contain `{setting}` placeholders that only the
Cura app fills in (the Ender 3 Pro heat-up, and the "present print" move at the end on both).
The command-line engine leaves them as text, which a printer can't read, so the script writes
those lines out with the real values (bed 70 C, nozzle 238 C; end move to Y 220 or X/Y 350).

| Setting | Value |
|---|---|
| Layers | 0.28 mm, adaptive (Cura picks 0.24-0.32 mm by shape; VBloc comes out 0.24 after a 0.28 first layer) |
| Lines | 0.45 mm (inner walls 0.5, infill 0.57), first layer 120 % |
| Walls / top / bottom | 3 walls, 4 top and 4 bottom layers, zigzag top/bottom |
| Infill | triangles: 40 % on small parts (VBloc, H bloc, brackets), 20 % on big ones such as the waffle plate (his profile had 60 %; user rule) |
| Speeds | 65 mm/s print, infill and inner walls; 50 outer wall; 45 top/bottom; 25 first layer; 162.5 travel |
| Temperatures | 238 C nozzle; bed 70 C first layer, then 67 C |
| Cooling | fan off |
| Retraction | 5 mm at 45 mm/s, at most 12 per 6 mm of filament |
| Adhesion | skirt, 2 lines, 10 mm away (thread test: brim, 25 lines) |

238 C nozzle, a 67-70 C bed and no fan are PETG settings. For PLA they are too hot: use about
200-210 C, a 55-60 C bed and the fan on, or the PLA will string and sag.

Settings the profile leaves to Cura formulas were worked out the way Cura does (comments in
`slice_pro.sh`). One of them matters: from 40 % infill up Cura turns "infill skin support" off;
the command-line engine left it on, which also switched the fan on under the top skins.

| VBloc file (`_Ender3Pro` and `_Ender5Plus` versions) | What | Result |
|---|---|---|
| `vbloc/VBloc_<printer>.gcode` | all parts (arm, fixed jaw, sliding jaw, both screw halves, thumb screw, 6 pins), his settings | 33 layers, about 42 min, 4.8 m (14.8 g PETG, 14.4 g PLA) |
| `vbloc/VBloc_threads_test_0.16_<printer>.gcode` | screw halves, sliding jaw and pins at 0.16 mm with a brim, to test the M6 thread first | 64 layers, about 41 min, 2.8 m (8.5 g PETG) |

The same plates are in `CAD/3mf/` as `VBloc.3mf` and `VBloc_threads_test.3mf`.

The lead screw is split lengthwise into two identical halves that print cut face down; they
are glued together with 4 pins (two 3.8 mm in the knob, two 2.2 mm along the thread, plus a
spare of each in `vbloc_pins.stl`; 1.75 mm filament works as pins too). At 0.28 mm the
thread is coarse, which is why the 0.16 mm test file exists.

## Ender-5 (university printer), first settings

These are the settings H bloc and VBloc were first sliced with. Both vises have since been
reworked and re-sliced with the profile above; the old Ender-5 G-code files were removed, and
the rows below are kept as the record of that earlier slice.

The G-code files in `hbloc/` and `vbloc/` were sliced for a **Creality Ender-5** with the
CuraEngine that ships with **UltiMaker Cura 5.13** (printer profile `creality_ender5`).
`CAD/scripts/slice_ender5.sh` reproduces them:

```sh
sh CAD/scripts/slice_ender5.sh hbloc
sh CAD/scripts/slice_ender5.sh vbloc
```

To slice in the Cura app instead, load the four STLs of a vise, pick the Creality Ender-5
printer and enter the values below. The STLs are already oriented and placed on the bed,
so don't rotate them.

## Printer and material

| Setting | Value |
|---|---|
| Printer | Creality Ender-5 (220 x 220 x 300 mm, Bowden extruder) |
| Nozzle | 0.4 mm |
| Filament | PLA, 1.75 mm |
| Start / end G-code | Cura's default Ender-5 G-code (heats bed, then nozzle, homes, draws a purge line) |

## Settings used

| Group | Setting (Cura name) | Value |
|---|---|---|
| Quality | Layer height (`layer_height`) | 0.16 mm |
| | Initial layer height (`layer_height_0`) | 0.2 mm |
| | Line width, all features (`line_width`, `wall_line_width*`, `skin_line_width`, `infill_line_width`, `skirt_brim_line_width`) | 0.4 mm |
| | Initial layer line width (`initial_layer_line_width_factor`) | 100 % |
| Walls | Wall line count (`wall_line_count`) | 3 |
| | Wall thickness (`wall_thickness`) | 1.2 mm |
| Top/bottom | Top and bottom layers (`top_layers`, `bottom_layers`) | 6 each |
| | Top and bottom thickness (`top_thickness`, `bottom_thickness`) | 0.96 mm |
| Infill | Density (`infill_sparse_density`) | 35 % |
| | Pattern (`infill_pattern`) | Grid |
| | Infill line distance (`infill_line_distance`) | 2.286 mm |
| | Infill layer thickness (`infill_sparse_thickness`) | 0.16 mm |
| Material | Printing temperature (`material_print_temperature`) | 200 °C |
| | Initial layer temperature (`material_print_temperature_layer_0`) | 205 °C |
| | Initial / final printing temperature | 200 °C |
| | Build plate temperature, all layers (`material_bed_temperature*`) | 60 °C |
| | Flow (`material_flow`) | 100 % |
| Speed | Print speed (`speed_print`) | 50 mm/s |
| | Infill speed (`speed_infill`) | 50 mm/s |
| | Outer wall (`speed_wall_0`) | 25 mm/s |
| | Inner walls (`speed_wall_x`) | 35 mm/s |
| | Top/bottom (`speed_topbottom`) | 30 mm/s |
| | Initial layer (`speed_layer_0`, `speed_print_layer_0`) | 20 mm/s |
| | Brim (`skirt_brim_speed`) | 20 mm/s |
| | Travel (`speed_travel`) / initial layer travel | 150 / 100 mm/s |
| | Acceleration and jerk control (`acceleration_enabled`, `jerk_enabled`) | off (firmware values from start G-code) |
| Travel | Retraction (`retraction_enable`) | on |
| | Retraction distance (`retraction_amount`) | 5 mm (Bowden) |
| | Retraction / prime speed | 45 mm/s |
| | Z hop (`retraction_hop_enabled`) | off |
| Cooling | Fan (`cool_fan_speed`, min and max) | 100 % |
| | Initial fan speed (`cool_fan_speed_0`) | 0 % |
| | Full fan speed from layer (`cool_fan_full_layer`) | 3 |
| | Minimum layer time (`cool_min_layer_time`) | 10 s |
| | Minimum speed (`cool_min_speed`) | 10 mm/s |
| Support | Generate support (`support_enable`) | off |
| Adhesion | Type (`adhesion_type`) | Brim |
| | Brim width / line count (`brim_width`, `brim_line_count`) | 6 mm / 15 lines |
| Special | Spiralize (`magic_spiralize`) | off |

## Extra settings for the command-line engine

The Cura app works out many settings from formulas before slicing; the command-line
engine only reads fixed default values. These extra settings were needed to make it
slice correctly. You do **not** need them in the Cura app.

| Setting | Value | Why |
|---|---|---|
| `roofing_layer_count` | 0 | Has no default value; the engine stops with an error without it. 0 = no separate top-surface skin, same as the Cura default. |
| `flooring_layer_count` | 0 | Same as above for the bottom-surface skin. |
| `mesh_position_x`, `mesh_position_y` | -110 mm | The engine shifts every part by half the bed size (110 mm). The STLs are already placed on the 220 x 220 mm bed, so this cancels the shift. Without it the parts land off the bed (up to X 266 mm). |
| `infill_line_distance` | 2.286 mm | Normally calculated by Cura from the density: 0.4 mm x 100 / 35 % x 2 (grid). |
| `infill_sparse_thickness` | 0.16 mm | Normally equal to the layer height; set by hand. |
| All line widths, speeds and temperatures above | as listed | Cura derives most of them from one main value; the engine needs each one set. |

The engine is run with `-m4` (4 threads) and the search path set to Cura's
`definitions` and `extruders` folders, so it finds the Ender-5 profile and its extruder.

## Part orientation on the plate

VBloc follows the motor bracket redesign's rules: every part prints flat on a face with no
supports, its layers run along the direction it's loaded, round parts got a flat along their
length so they can lie down, and a part that needs two print directions is split and glued.

| Part | H bloc | VBloc | Why (VBloc) |
|---|---|---|---|
| Base | flat on its bottom | **split in two and glued**: the arm flat on its bottom, the fixed jaw on its outer face | the jaw's layers run along the jaw, which bends when clamping; its screw hole prints round; no overhangs; both pieces 6 mm tall |
| Sliding jaw | on its back face | on its back face | thread hole vertical; layers along the jaw |
| Lead screw | standing on its knob | **lying on a 0.44 mm flat** cut along the thread and knob | layers along the screw, which is pulled when clamping; 9.5 mm tall instead of 64; overhang 50 mm2 instead of 576 |
| Thumb screw | head down | **lying on a 0.35 mm flat** along the shank and head | same as the lead screw; overhang 2 mm2 instead of 99 |

**Gluing the VBloc base:** the fixed jaw has two 4 x 6 x 4 mm tenons that drop into notches
at the end of the arm (0.14 mm clearance per side, notches 0.2 mm deeper than the tenons,
about 245 mm2 of glue surface). Dry-fit first, then glue with CA (super glue) or epoxy,
with the jaw's outer face flush with the arm's end. When clamping, the knob's push and the
clamped part's push meet inside the jaw itself, so the glued joint carries little load.

## Results

| | H bloc | VBloc |
|---|---|---|
| Layers | 318 | 59 |
| Estimated time | about 1 h 43 min | about 1 h 47 min |
| Filament | 4.0 m (about 11.8 g) | 4.73 m (about 14.1 g) |
| - base (VBloc: arm) | 7.0 g | 4.9 g |
| - fixed jaw (VBloc only) | - | 2.7 g |
| - sliding jaw | 2.1 g | 3.3 g |
| - lead screw | 1.7 g | 1.9 g |
| - thumb screw | 0.5 g | 0.4 g |
| - brim | 0.6 g | 1.0 g |
| Print area | X 67-162, Y 76-141 mm | X 57-161, Y 59-170 mm |
| Thread print clearance | 0.2 mm added | 0.2 mm added |

Weights are for PLA (1.24 g/cm3, 1.75 mm filament); PETG would be about 2 % heavier. Both
vises fall inside the 10-15 g per bloc estimated in the weight and motor sizing session.

The G-code header shows "Filament used: 0m": the Cura app normally writes that line, the
command-line engine doesn't. It doesn't affect the print. The filament was counted from the
G-code's extrusion moves, leaving out retraction re-primes; an earlier count that included
them gave 7.1 m / 21 g and 8.0 m / 24 g, which was wrong (more than the solid parts weigh).
