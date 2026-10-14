# Design rules for BLOC-BOT printed parts

What we learned from the professional's motor bracket redesign and from the first VBloc
print. Follow these for every new printed part and when reworking an old one (VBloc and H bloc
are both reworked this way).

## Print direction first

- Decide each part's print face before modelling. Every part prints flat on its biggest face
  with **no supports**.
- If a part needs two print directions, **split it and glue it** (CA or epoxy). Glue is part
  of the method, not a fallback: the motor bracket is 3 glued parts, VBloc's base and lead
  screw are split and glued.
- Load along the layers, not across them: a jaw or tab that bends should have its layers
  running along it.
- Round parts that must lie down get a shallow **flat** along their length (the bracket's
  sensor column: 0.56 mm deep). The flat can double as a key in a D-shaped socket.
- Where a split removes a one-piece joint, give the glue more area: a rib, a deeper socket,
  tenons in mortises.

## Fits

| Joint | Clearance per side |
|---|---|
| Flat tab in a slot (glued) | 0.14 mm |
| Round peg in a socket (glued) | 0.20 mm |
| Printed screw through a clearance hole | about 0.15 mm (M6: hole 6.2 mm). VBloc's first 6.6 mm hole let the screw wobble |
| Printed thread (nut side) | 0.2 mm added to the thread |

Upright printed holes come out about 0.1 mm small, which is why a 0.15 mm gap still turns.
Socket and slot ends are rounded with the clearance carried round the radius.

## Edges: break them by print direction

- **Round (fillet) only edges that stand upright while printing**: the bracket's flange
  corners R4, tab corners R2, slot and pocket ends; VBloc's jaw and arm corners.
- **Chamfer only where something goes in, and use 45 deg, never a fillet, on flat edges.**
  On his bracket the only chamfers are 0.4 mm on the rims of the slots, holes and sockets
  that a tab, bolt or column enters (both faces), plus the rib's 45 deg slope so it prints
  without support. No outer outline, bed edge or big face is chamfered. On the vises that
  means the screw hole, the thread, the board slot or groove, and the mouth of the slide
  groove; nothing else. (A fillet on a flat edge would print as an overhang underneath and a
  staircase on top.)
- Exception: rounds on the **top** of a part are fine when nothing hangs below them, e.g. a
  split screw lying on its cut face.

## Parts you touch

- Anything turned by hand must be skin-friendly: no notches or sharp ridges. VBloc's knob is
  an octagon, 14 mm across the flats, with R2 corners and R1 rims (the notched first version
  hurt). On a split screw the cut runs through the middle of two flats so each half has
  upright sides then 45 deg slopes.

## Print settings

The professional's Cura profile `CAD/out/print_profiles/CE3PRO_pro.curaprofile`, with infill
**40 % on small parts and 20 % on big ones** such as the waffle plate. Details and the
command-line slicer: `CAD/out/print_settings_ender5.md` and `CAD/scripts/slice_pro.sh`.
Every printable plate is also saved as a 3MF in `CAD/3mf/`.

## Fusion scripting notes

For anyone scripting the designs through the Fusion API:

- An exception inside a script undoes every feature the script made; add risky features
  (fillets, chamfers) one per script, or test them first.
- Chamfers fail when the chain stops on a fillet arc next to an unchamfered edge; end the
  chain on a straight edge or chamfer a face's whole outline.
- Meshing several bodies with "high quality" in one script froze Fusion for 30+ minutes.
  Export one body per script with a set surface tolerance (0.01 mm), and keep a backup
  (`CAD/out/*.f3d`) before heavy exports.
- Fusion's STL export call can report success and write nothing; the scripts write STLs from
  the mesh calculator instead.
