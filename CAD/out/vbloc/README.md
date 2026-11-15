# VBloc print files

VBloc is the vertical vise. It is printed as 7 parts plus small pins, all lying flat with no
supports, then glued. All G-code uses the professional's Cura profile (`CAD/out/print_profiles/
CE3PRO_pro.curaprofile`): 0.28 mm adaptive layers, 3 walls, 238 C nozzle, 70/67 C bed, fan off
(PETG temperatures), with 40 % triangles infill.
The same plates as 3MF (to slice for any printer) are `CAD/3mf/VBloc.3mf` and
`CAD/3mf/VBloc_threads_test.3mf`.

## Which file is what

Pick the G-code for your printer: **Ender3Pro** = Creality Ender 3 Pro (the professional's
printer, 220 x 220 bed), **Ender5Plus** = Creality Ender 5 Plus (350 x 350 bed, starts with
G29 bed levelling). Both are centred on their bed.

| File | What it is |
|---|---|
| `VBloc_Ender3Pro.gcode` / `VBloc_Ender5Plus.gcode` | **Print this.** Every part below on one plate, 0.28 mm layers, skirt. About 42 min, 15 g. |
| `VBloc_threads_test_0.16_Ender3Pro.gcode` / `..._Ender5Plus.gcode` | **Optional test first.** Only the two screw halves, the sliding jaw and the pins, at finer 0.16 mm layers with a brim, to check the M6 thread. About 41 min, 8.5 g. |
| `vbloc_base.stl` | Arm (the long rail the sliding jaw runs on), with the M4 mounting hole underneath. Prints flat. Outer corners rounded R2; 0.4 mm chamfer at the mouth of the slide groove, where the sliding jaw goes in. |
| `vbloc_fixedjaw.stl` | Fixed jaw. Glues onto the end of the arm with its two tenons. Prints on its outer face. Its screw hole is Ø6.2 (was 6.6, which let the screw wobble on the first print) with 0.4 mm chamfers on both rims. Top corners R1, tenon tips R0.5; 0.4 mm chamfers on the board slot rims. |
| `vbloc_slidingjaw.stl` | Sliding jaw with the M6 thread. Prints on its back face. Corners R1, tongue edges R0.5; 0.4 mm chamfers on both thread rims and the board slot rims. |
| `vbloc_leadscrew_top.stl` | Lead screw, top half (thread + half the knob). Prints on its flat cut face. The knob is an octagon, 14 mm across the flats, with R2 corners and R1 rims so it does not dig into your fingers (the first version had sharp notches). |
| `vbloc_leadscrew_bottom.stl` | Lead screw, bottom half. Identical shape, also on its cut face. |
| `vbloc_cap.stl` | M4 thumb screw that fixes the vise to the plate from below. A steel M4 screw is stronger. |
| `vbloc_pins.stl` | 6 alignment pins, 1.75 mm: 3 long (3.8 mm) and 3 short (2.2 mm). You need 2 of each; the third is a spare. 1.75 mm filament cut to length works too. |

The STLs are already turned to their print positions and laid out together; don't rotate them
if you slice them yourself.

Edges follow the motor bracket: corners that stand upright while printing are rounded, and
45 deg chamfers (0.4 mm) go only where something goes in: the screw hole, the thread, the
board slot and the mouth of the slide groove. Outer outlines stay sharp, as on his bracket.

## Assembly

1. **Lead screw:** put the two long pins in the knob holes and the two short pins in the
   thread holes of one half, glue (CA or epoxy), press the other half on so the thread lines
   up. Wipe glue out of the thread; sand the seam lightly if the screw turns stiffly.
2. **Base:** dry-fit the fixed jaw's two tenons into the notches at the end of the arm, then
   glue with the jaw's outer face flush with the end of the arm.
3. Slide the sliding jaw onto the arm (tongue in the groove), push the lead screw through
   the fixed jaw and screw it into the sliding jaw.

Re-slice with `CAD/scripts/slice_pro.sh ender3pro|ender5plus ...` (settings are in
`CAD/out/print_settings_ender5.md`).
