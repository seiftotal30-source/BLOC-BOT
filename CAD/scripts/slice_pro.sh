#!/bin/sh
# Slice parts with the professional's Ender 3 Pro Cura profile (CAD/print_profiles/
# CE3PRO_pro.curaprofile, Cura 5.13, Creality "Standard" quality): 0.28 mm adaptive layers,
# 3 walls, 4 top/bottom layers, triangles infill, 238 C nozzle, bed 70 C first layer then
# 67 C, fan off, 65 mm/s print, 50 mm/s outer wall, 45 mm/s top/bottom, 0.45 mm lines, 5 mm
# retraction, skirt. Uses the CuraEngine from UltiMaker Cura 5.13 and Cura's printer profiles.
#
# Usage (Git Bash):
#   sh slice_pro.sh <printer> <output.gcode> <layer height> <skirt|brim> <part.stl> [<part.stl> ...]
#   printer: ender3pro  (the professional's printer, 220 x 220 bed)
#            ender5plus (350 x 350 bed, BLTouch: its start G-code runs G29 bed levelling)
# e.g.
#   sh slice_pro.sh ender3pro VBloc.gcode 0.28 skirt vbloc_*.stl            (his settings)
#   sh slice_pro.sh ender5plus test.gcode 0.16 brim vbloc_leadscrew_*.stl   (finer, for threads)
#   INFILL=20 sh slice_pro.sh ender3pro Plate.gcode 0.28 skirt plate.stl     (big parts)
#
# Infill (user rule, replaces his 60 %): 40 % for small parts (the default), 20 % for big ones
# such as the waffle plate; set it with the INFILL environment variable.
#
# The STLs are already print-oriented and laid out around (110, 110) mm. The engine shifts
# parts by half the bed width, so mesh_position -110 puts that layout in the middle of either
# bed. The command-line engine doesn't compute derived values, so they are all set here.

[ $# -ge 5 ] || { echo "usage: sh slice_pro.sh <ender3pro|ender5plus> <output.gcode> <layer> <skirt|brim> <part.stl>..."; exit 1; }
PRINTER="$1"; OUTFILE="$2"; LH="$3"; ADH="$4"; shift 4
case "$PRINTER" in
  ender5plus) DEF="creality_ender5plus.def.json"; START="" ;;
  # Cura's Ender 3 Pro start G-code uses {setting} placeholders that only the Cura app fills
  # in, so it is given here with the values written out (bed 70 C, nozzle 238 C, 175 C while
  # the bed heats).
  ender3pro)  DEF="creality_ender3pro.def.json"
              START="; Ender 3 Pro start G-code (Cura's, with the temperatures filled in)
G92 E0 ; Reset Extruder
G28 ; Home all axes
G1 Z5.0 F3000 ; Move Z Axis up a bit during heating to not damage bed
M104 S175 ; Start heating up the nozzle most of the way
M190 S70 ; Heat the bed and wait
M109 S238 ; Finish heating the nozzle
G1 Z2.0 F3000 ; Move Z Axis up little to prevent scratching of Heat Bed
G1 X0.1 Y20 Z0.3 F5000.0 ; Move to start position
G1 X0.1 Y200.0 Z0.3 F1500.0 E15 ; Draw the first line
G1 X0.4 Y200.0 Z0.3 F5000.0 ; Move to side a little
G1 X0.4 Y20 Z0.3 F1500.0 E30 ; Draw the second line
G92 E0 ; Reset Extruder
G1 Z2.0 F3000 ; Move Z Axis up little to prevent scratching of Heat Bed
G1 X5 Y20 Z0.3 F5000.0 ; Move over to prevent blob squish" ;;
  *) echo "printer must be ender3pro or ender5plus"; exit 1 ;;
esac
# The end G-code "present print" move also uses {machine_width} / {machine_depth} placeholders
# in Cura's profiles; write it out with the bed size (Ender 3 Pro: Y 220, Ender 5 Plus: 350).
case "$PRINTER" in
  ender3pro)  PRESENT="G1 X0 Y220 ;Present print" ;;
  ender5plus) PRESENT="G1 X350 Y350 ;Present print" ;;
esac
END="G91 ;Relative positioning
G1 E-2 F2700 ;Retract a bit
G1 E-2 Z0.2 F2400 ;Retract and raise Z
G1 X5 Y5 F3000 ;Wipe out
G1 Z10 ;Raise Z more
G90 ;Absolute positioning

$PRESENT
M106 S0 ;Turn-off fan
M104 S0 ;Turn-off hotend
M140 S0 ;Turn-off bed

M84 X Y E ;Disable all steppers but Z"
case "$ADH" in
  skirt) ADHESION="-s adhesion_type=skirt -s skirt_line_count=2 -s skirt_gap=10" ;;
  brim)  ADHESION="-s adhesion_type=brim -s brim_line_count=25 -s brim_width=13.5" ;;
  *) echo "adhesion must be skirt or brim"; exit 1 ;;
esac

CURA="/d/UltiMaker Cura 5.13.0"
RES="$CURA/share/cura/resources"
export CURA_ENGINE_SEARCH_PATH="$(cygpath -w "$RES/definitions");$(cygpath -w "$RES/extruders")"
W() { cygpath -w "$1"; }

# Values below are the professional's profile (CE3PRO_pro.curaprofile). Where his profile leaves
# a setting to a Cura formula, the value is worked out here the way Cura would:
#   infill_line_distance = 0.57 x 100 / infill % x 3 (triangles): 4.275 at 40 %, 8.55 at 20 %
#   speed_infill = speed_print = 65; speed_travel = 65 x 2.5 = 162.5;
#   speed_print_layer_0 / skirt speed = speed_layer_0 = 25; first-layer travel = 25 x 5 = 125
#   top/bottom thickness 0.9 mm, but his explicit 4 top + 4 bottom layers win
#   skin_support: Cura turns it on only below 40 % infill (so on at 20 %); its fan speed is
#   set to 0 because his profile has the fan off
#   adaptive layers: Creality's +/- 0.04 mm variation in 0.04 mm steps, 0.2 threshold
# Retraction speed (45 mm/s) isn't in his profile; it is the value in his bracket G-code.

# settings without a default value, plus the bed offset; repeated per mesh.
# XSHIFT / YSHIFT (mm, optional environment variables) move a subset of the layout to the
# bed centre, e.g.  XSHIFT=-18.6 YSHIFT=-6.8 sh slice_pro.sh ...
INFILL="${INFILL:-40}"
ILD=$(awk -v d="$INFILL" 'BEGIN { printf "%.3f", 0.57 * 100 / d * 3 }')
SKINSUP=$(awk -v d="$INFILL" 'BEGIN { print (d < 40) ? "true" : "false" }')
MX=$(awk -v s="${XSHIFT:-0}" 'BEGIN { print -110 + s }')
MY=$(awk -v s="${YSHIFT:-0}" 'BEGIN { print -110 + s }')
MESH="-s roofing_layer_count=0 -s flooring_layer_count=0 -s mesh_position_x=$MX -s mesh_position_y=$MY"

# turn the part list into "-l <file> <mesh settings>" groups (the loop reads the original list)
N=$#
for f in "$@"; do
  [ -f "$f" ] || { echo "missing $f"; exit 1; }
  set -- "$@" -l "$(W "$f")" $MESH
done
shift $N

"$CURA/CuraEngine.exe" slice -v -m4 -j "$(W "$RES/definitions/$DEF")" \
 ${START:+-s} ${START:+"machine_start_gcode=$START"} -s "machine_end_gcode=$END" \
 -s layer_height=$LH -s layer_height_0=0.28 -s infill_sparse_thickness=$LH \
 -s adaptive_layer_height_enabled=true -s adaptive_layer_height_variation=0.04 \
 -s adaptive_layer_height_variation_step=0.04 -s adaptive_layer_height_threshold=0.2 \
 -s line_width=0.45 -s wall_line_width=0.45 -s wall_line_width_0=0.45 -s wall_line_width_x=0.5 \
 -s skin_line_width=0.45 -s infill_line_width=0.57 -s skirt_brim_line_width=0.45 -s initial_layer_line_width_factor=120 \
 -s min_wall_line_width=0.34 -s fill_outline_gaps=true -s hole_xy_offset=0 -s slicing_tolerance=inclusive \
 -s wall_line_count=3 -s wall_thickness=2 -s optimize_wall_printing_order=true -s wall_0_wipe_dist=0.2 \
 -s z_seam_type=back -s z_seam_position=front \
 -s top_bottom_thickness=0.9 -s top_thickness=0.9 -s bottom_thickness=0.9 -s top_layers=4 -s bottom_layers=4 -s initial_bottom_layers=4 \
 -s top_bottom_pattern=zigzag -s skin_overlap=10 \
 -s infill_sparse_density=$INFILL -s infill_pattern=triangles -s infill_line_distance=$ILD -s infill_overlap=30 \
 -s infill_before_walls=false -s infill_wipe_dist=0 -s skin_support=$SKINSUP -s skin_support_fan_speed=0 -s bridge_fan_speed=0 \
 -s speed_print=65 -s speed_infill=65 -s speed_wall=32.5 -s speed_wall_0=50 -s speed_wall_x=65 -s speed_topbottom=45 \
 -s speed_roofing=45 -s speed_layer_0=25 -s speed_print_layer_0=25 -s speed_travel_layer_0=125 -s speed_travel=162.5 \
 -s skirt_brim_speed=25 -s acceleration_enabled=false -s jerk_enabled=false \
 -s material_diameter=1.75 -s material_print_temperature=238 -s material_print_temperature_layer_0=238 \
 -s material_initial_print_temperature=238 -s material_final_print_temperature=238 \
 -s material_bed_temperature=67 -s material_bed_temperature_layer_0=70 -s material_flow=100 \
 -s retraction_enable=true -s retraction_amount=5 -s retraction_speed=45 -s retraction_retract_speed=45 -s retraction_prime_speed=45 \
 -s retraction_count_max=12 -s retraction_extrusion_window=6 -s retraction_min_travel=1.5 \
 -s retraction_combing=noskin -s retraction_hop_enabled=false \
 -s cool_fan_enabled=false -s cool_fan_speed=0 -s cool_fan_speed_min=0 -s cool_fan_speed_max=0 -s cool_fan_speed_0=0 \
 -s cool_min_layer_time=10 -s cool_min_speed=10 \
 -s support_enable=false $ADHESION -s magic_spiralize=false \
 $MESH -e0 \
 "$@" \
 -o "$(W "$OUTFILE")"
