# VBloc print files

VBloc is the vertical vise. It is printed as 7 parts plus small pins, all lying flat with no
supports, then glued. The G-code is for a **Creality Ender 5 Plus** (350 x 350 bed, starts
with G28 + G29 bed levelling) with the professional's settings: 0.28 mm layers, 2 walls,
60 % infill, 200 C nozzle, 50 C bed (60 C is better on the glass bed; set it on the printer).

## Which file is what

| File | What it is |
|---|---|
| `VBloc_Ender5Plus.gcode` | **Print this.** Every part below on one plate, 0.28 mm layers, skirt. About 1 h 03 min, 16 g PLA, centred on the bed. |
| `VBloc_threads_test_0.16_Ender5Plus.gcode` | **Optional test first.** Only the two screw halves, the sliding jaw and the pins, at finer 0.16 mm layers with a brim, to check the M6 thread. About 46 min, 7 g. |
| `vbloc_base.stl` | Arm (the long rail the sliding jaw runs on), with the M4 mounting hole underneath. Prints flat. |
| `vbloc_fixedjaw.stl` | Fixed jaw. Glues onto the end of the arm with its two tenons. Prints on its outer face. |
| `vbloc_slidingjaw.stl` | Sliding jaw with the M6 thread. Prints on its back face. |
| `vbloc_leadscrew_top.stl` | Lead screw, top half (thread + half the knob). Prints on its flat cut face. |
| `vbloc_leadscrew_bottom.stl` | Lead screw, bottom half. Identical shape, also on its cut face. |
| `vbloc_cap.stl` | M4 thumb screw that fixes the vise to the plate from below. A steel M4 screw is stronger. |
| `vbloc_pins.stl` | 6 alignment pins, 1.75 mm: 3 long (3.8 mm) and 3 short (2.2 mm). You need 2 of each; the third is a spare. 1.75 mm filament cut to length works too. |

The STLs are already turned to their print positions and laid out together; don't rotate them
if you slice them yourself.

## Assembly

1. **Lead screw:** put the two long pins in the knob holes and the two short pins in the
   thread holes of one half, glue (CA or epoxy), press the other half on so the thread lines
   up. Wipe glue out of the thread; sand the seam lightly if the screw turns stiffly.
2. **Base:** dry-fit the fixed jaw's two tenons into the notches at the end of the arm, then
   glue with the jaw's outer face flush with the end of the arm.
3. Slide the sliding jaw onto the arm (tongue in the groove), push the lead screw through
   the fixed jaw and screw it into the sliding jaw.

Re-slice with `CAD/scripts/slice_ender5plus.sh` (settings and file list are in
`CAD/out/print_settings_ender5.md`).
