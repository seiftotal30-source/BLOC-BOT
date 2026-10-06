# Ender-5 print settings for H bloc and VBloc

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

| Part | Orientation | Why |
|---|---|---|
| Base | flat on its bottom | largest flat face; only short bridges |
| Sliding jaw | on its back face | thread hole is vertical, so the thread prints cleanly with no support |
| Lead screw | standing on its knob | thread is vertical; the brim keeps the tall, thin screw steady |
| Thumb screw | head down | thread is vertical |

## Results

| | H bloc | VBloc |
|---|---|---|
| Layers | 318 | 400 |
| Estimated time | about 1 h 43 min | about 2 h |
| Filament | 4.0 m (about 11.8 g) | 4.65 m (about 13.9 g) |
| - base | 7.0 g | 7.5 g |
| - sliding jaw | 2.1 g | 3.3 g |
| - lead screw | 1.7 g | 2.0 g |
| - thumb screw | 0.5 g | 0.5 g |
| - brim | 0.6 g | 0.6 g |
| Print area | X 67-162, Y 76-141 mm | X 67-160, Y 65-146 mm |
| Thread print clearance | 0.2 mm added | not yet (may bind) |

Weights are for PLA (1.24 g/cm3, 1.75 mm filament); PETG would be about 2 % heavier. Both
vises fall inside the 10-15 g per bloc estimated in the weight and motor sizing session.

The G-code header shows "Filament used: 0m": the Cura app normally writes that line, the
command-line engine doesn't. It doesn't affect the print. The filament was counted from the
G-code's extrusion moves, leaving out retraction re-primes; an earlier count that included
them gave 7.1 m / 21 g and 8.0 m / 24 g, which was wrong (more than the solid parts weigh).
