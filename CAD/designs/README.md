# Designs

One folder per printed design. Inside each, files are sorted by type:

| Subfolder / file | What it holds |
|---|---|
| `stl/` | Print-oriented STLs (each part already on the face it prints on) |
| `gcode/` | Sliced G-code, named `<Design>_<Printer>.gcode` |
| `3mf/` | The print plate as 3MF (and the assembled Fusion export where there is one) |
| `images/` | Fusion screenshots and viewport pictures (also used by the thesis) |
| `*.f3d` | Fusion backup of the design |
| `README.md` | Notes for that design, where it has some |

| Folder | Design |
|---|---|
| `motor_bracket/` | TT motor bracket (left and right) and the fit-test tab. `from_professional/` holds the professional's own files as he sent them: `Bracket.3mf` and `Bracket(Mirror).3mf` (assembled, not turned for printing) and `CE4_Bracket.gcode`. |
| `controller/` | Bloc-Con controller case (top, bottom) and button caps. `scad/` has the CadQuery/OpenSCAD viewer. |
| `waffle_plate/` | Robot base plate, 210 x 210 x 10 mm (fills a 220 mm bed; no room for a brim). |
| `hbloc/` | H bloc, the horizontal vise: arm, fixed jaw, sliding jaw, two lead screw halves, M4 thumb screw, 6 pins. `3mf/` also has `HBloc_assembled.3mf`, the assembled Fusion export. |
| `hbloc_l/` | H bloc L, the large H bloc for the 5-channel line sensor (96-100 mm board, 34 mm jaws, two M4 mounts). `3mf/HBloc_L.3mf` is the assembled Fusion export, not yet turned for printing. |
| `vbloc/` | VBloc, the vertical vise (same parts as H bloc), plus the thread test plate. `3mf/` also has `VBloc_assembled.3mf`. |
| `caster_support/` | Caster support, versions A and B. |
| `flyingfish_mount/` | Rear holder for the Flying Fish IR sensor (OpenSCAD source at the top of the folder). |

## Slicing

Open a design's 3MF in Cura (or any slicer), pick your printer and slice: the parts are laid
out together, centred on a 220 x 220 mm bed (Ender 3 Pro). On a bigger bed (Ender 5 Plus) use
the slicer's arrange/centre command. Default settings are the professional's Cura profile
(`CAD/print_profiles/CE3PRO_pro.curaprofile`: 0.28 mm adaptive layers, 3 walls, 238 C /
70 C bed, fan off, skirt) with 40 % infill on small parts and 20 % on big ones such as the
waffle plate (see `CAD/print_profiles/print_settings_ender5.md`). Command-line slicing:
`CAD/scripts/slice_pro.sh`.

Glued parts: on VBloc and H bloc, the fixed jaw onto the arm and the two screw halves (with the
pins); on the motor bracket, the tab and column into the flange. Use CA (super glue) or epoxy.

The 3MFs are made with `CAD/scripts/make_3mf.py` from the print-oriented STLs in each design's
`stl/` (the motor bracket straight from the Fusion design, each part on its largest flat face).

## Pictures

The `images/` of `hbloc`, `vbloc` and `waffle_plate` are Fusion viewport pictures (1600 x 1100 px,
shaded with edges): iso front-right, iso front-left, front, right, top (the waffle plate has
bottom and iso underside instead of front and right, since it is only 10 mm thick). The vise
pictures show the parts as assembled in the design. Sketches, the origin and construction
planes were hidden for these pictures.
