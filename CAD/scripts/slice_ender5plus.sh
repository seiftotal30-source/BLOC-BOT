#!/bin/sh
# Slice parts for a Creality Ender 5 Plus with the CuraEngine from UltiMaker Cura 5.13, using
# the professional's settings from the motor bracket G-code (Cura "Low Quality" Creality
# profile with 60 % infill): 0.28 mm layers, 2 walls, 4 top/bottom layers, 60 % lines infill,
# 200 C / bed 50 C, 80 mm/s print, 40 mm/s walls and skin, 5 mm retraction, skirt.
#
# Usage (Git Bash):
#   sh slice_ender5plus.sh <output.gcode> <layer height> <skirt|brim> <part.stl> [<part.stl> ...]
# e.g.
#   sh slice_ender5plus.sh VBloc.gcode 0.28 skirt vbloc_*.stl            (his settings)
#   sh slice_ender5plus.sh test.gcode 0.16 brim vbloc_leadscrew_*.stl    (finer, for threads)
#
# The STLs are already print-oriented and laid out for a 220 x 220 bed (centre about 115);
# the engine shifts parts by half the bed width (175 mm), so mesh_position -110 moves the
# layout to the middle of the 350 x 350 bed. The Ender 5 Plus start G-code runs G29 (BLTouch
# bed levelling) before printing.
# The command-line engine doesn't compute derived values, so they are all set here.

[ $# -ge 4 ] || { echo "usage: sh slice_ender5plus.sh <output.gcode> <layer> <skirt|brim> <part.stl>..."; exit 1; }
OUTFILE="$1"; LH="$2"; ADH="$3"; shift 3
case "$ADH" in
  skirt) ADHESION="-s adhesion_type=skirt -s skirt_line_count=3 -s skirt_gap=10" ;;
  brim)  ADHESION="-s adhesion_type=brim -s brim_width=6 -s brim_line_count=15" ;;
  *) echo "adhesion must be skirt or brim"; exit 1 ;;
esac

CURA="/d/UltiMaker Cura 5.13.0"
RES="$CURA/share/cura/resources"
export CURA_ENGINE_SEARCH_PATH="$(cygpath -w "$RES/definitions");$(cygpath -w "$RES/extruders")"
W() { cygpath -w "$1"; }

# 4 top/bottom layers at 0.28 mm = 1.12 mm (Low Quality: first layer + 3 layers); keep 4 layers
# at finer heights too. Fan reaches full speed at layer 3 like the professional's file.
TB=$(awk -v h="$LH" 'BEGIN { printf "%.2f", 4 * h }')

# settings without a default value, plus the bed offset; repeated per mesh.
# XSHIFT / YSHIFT (mm, optional environment variables) move a subset of the layout to the
# bed centre, e.g.  XSHIFT=-18.6 YSHIFT=-6.8 sh slice_ender5plus.sh ...
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

"$CURA/CuraEngine.exe" slice -v -m4 -j "$(W "$RES/definitions/creality_ender5plus.def.json")" \
 -s layer_height=$LH -s layer_height_0=0.28 -s infill_sparse_thickness=$LH \
 -s line_width=0.4 -s wall_line_width=0.4 -s wall_line_width_0=0.4 -s wall_line_width_x=0.4 \
 -s skin_line_width=0.4 -s infill_line_width=0.4 -s skirt_brim_line_width=0.4 -s initial_layer_line_width_factor=100 \
 -s wall_line_count=2 -s wall_thickness=0.8 -s optimize_wall_printing_order=true -s z_seam_type=back \
 -s top_bottom_thickness=$TB -s top_thickness=$TB -s bottom_thickness=$TB -s top_layers=4 -s bottom_layers=4 -s skin_overlap=10 \
 -s infill_sparse_density=60 -s infill_pattern=lines -s infill_line_distance=0.667 -s infill_before_walls=false -s infill_wipe_dist=0 \
 -s speed_print=80 -s speed_infill=80 -s speed_wall=40 -s speed_wall_0=40 -s speed_wall_x=40 -s speed_topbottom=40 \
 -s speed_layer_0=20 -s speed_print_layer_0=20 -s speed_travel_layer_0=100 -s speed_travel=200 -s skirt_brim_speed=20 \
 -s acceleration_enabled=false -s jerk_enabled=false \
 -s material_diameter=1.75 -s material_print_temperature=200 -s material_print_temperature_layer_0=200 \
 -s material_initial_print_temperature=200 -s material_final_print_temperature=200 \
 -s material_bed_temperature=50 -s material_bed_temperature_layer_0=50 -s material_flow=100 \
 -s retraction_enable=true -s retraction_amount=5 -s retraction_speed=45 -s retraction_retract_speed=45 -s retraction_prime_speed=45 \
 -s retraction_combing=noskin -s retraction_hop_enabled=false \
 -s cool_fan_enabled=true -s cool_fan_speed=100 -s cool_fan_speed_min=100 -s cool_fan_speed_max=100 -s cool_fan_speed_0=0 \
 -s cool_fan_full_layer=3 -s cool_min_layer_time=10 -s cool_min_speed=10 \
 -s support_enable=false $ADHESION -s magic_spiralize=false \
 $MESH -e0 \
 "$@" \
 -o "$(W "$OUTFILE")"
