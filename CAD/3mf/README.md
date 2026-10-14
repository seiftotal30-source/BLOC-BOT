# 3MF print files

Every BLOC-BOT part we print, as 3MF. Open one in Cura (or any slicer), pick your printer and
slice: the parts are already turned to the face they print on (no supports needed) and laid
out together, centred on a 220 x 220 mm bed (Ender 3 Pro). On a bigger bed (Ender 5 Plus) use
the slicer's arrange/centre command. Default slicer settings are the professional's Cura profile
(`CAD/out/print_profiles/CE3PRO_pro.curaprofile`: 0.28 mm adaptive layers, 3 walls, 238 C /
70 C bed, fan off, skirt) with 40 % infill on small parts and 20 % on big ones such as the
waffle plate (see `CAD/out/print_settings_ender5.md`).

| File | What it is | Parts (each a separate object) |
|---|---|---|
| `VBloc.3mf` | vertical vise, reworked for printing | arm, fixed jaw, sliding jaw, lead screw top half, lead screw bottom half, M4 thumb screw, 6 pins |
| `VBloc_threads_test.3mf` | VBloc thread test: print first to check the M6 thread (slice at 0.16 mm) | sliding jaw, both lead screw halves, pins |
| `HBloc.3mf` | horizontal vise (not yet reworked like VBloc; lead screw prints standing) | base, sliding jaw, lead screw, M4 thumb screw |
| `MotorBracket_right.3mf` | the professional's 3-part TT motor bracket, right-hand | flange (top face down), motor tab (nut traps up), sensor column (on its flat) |
| `MotorBracket_left.3mf` | the same, left-hand (mirrored) | flange, motor tab, sensor column |
| `FlyingFish_mount.3mf` | rear holder for the Flying Fish IR sensor | 1 part, standing on its end face |
| `Waffle_plate.3mf` | the robot base plate, 210 x 210 x 10 mm | 1 part (fills a 220 mm bed; no room for a brim) |
| `Controller_case.3mf` | Bloc-Con controller case | bottom, top |
| `Controller_caps.3mf` | Bloc-Con button caps | 1 part |

Glued parts: VBloc's fixed jaw onto the arm and its two screw halves (with the pins), and the
motor bracket's tab and column into the flange. Use CA (super glue) or epoxy.

`from_professional/` holds the professional's own files as he sent them: `Bracket.3mf` and
`Bracket(Mirror).3mf` (exported from Fusion in the assembled position, not turned for
printing) and `CE4_Bracket.gcode`, the G-code he printed the bracket from on his Ender 3 Pro
(our default settings were read from it).

The 3MFs are made with `CAD/scripts/make_3mf.py` from the print-oriented STLs in `CAD/out/`
(the motor bracket straight from the Fusion design, each part on its largest flat face).
