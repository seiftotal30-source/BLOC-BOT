#!/bin/sh
# Slice the parts of a vise (H bloc or VBloc) for a Creality Ender-5 with the
# CuraEngine that ships with UltiMaker Cura 5.13 (PLA, 0.4 mm nozzle).
#
# Usage (Git Bash):  sh slice_ender5.sh hbloc   or   sh slice_ender5.sh vbloc
# Reads   every CAD/designs/<part>/stl/<part>_*.stl  (already print-oriented and placed on the bed;
#         VBloc has a fifth part, the fixed jaw, since its base was split for gluing)
# Writes  CAD/designs/<part>/gcode/<HBloc|VBloc>_Ender5_PLA.gcode
#
# The command-line engine only reads default values, so every setting the Cura app
# normally computes is given here. The engine also shifts parts by half the bed size,
# so mesh_position_x/y = -110 cancels that for STLs already placed on a 220 x 220 bed.

PART="$1"
case "$PART" in
  hbloc) NAME="HBloc" ;;
  vbloc) NAME="VBloc" ;;
  *) echo "usage: sh slice_ender5.sh hbloc|vbloc"; exit 1 ;;
esac

CURA="/d/UltiMaker Cura 5.13.0"
RES="$CURA/share/cura/resources"
STL="/e/Claude Stuff/CAD/designs/$PART/stl"
OUT="/e/Claude Stuff/CAD/designs/$PART/gcode"
export CURA_ENGINE_SEARCH_PATH="$(cygpath -w "$RES/definitions");$(cygpath -w "$RES/extruders")"
W() { cygpath -w "$1"; }

# settings without a default value, plus the bed offset; repeated per mesh
MESH="-s roofing_layer_count=0 -s flooring_layer_count=0 -s mesh_position_x=-110 -s mesh_position_y=-110"

# one "-l <stl> <mesh settings>" group per part file
set --
for f in "$STL"/${PART}_*.stl; do
  [ -f "$f" ] || { echo "no STL files in $STL"; exit 1; }
  set -- "$@" -l "$(W "$f")" $MESH
done

"$CURA/CuraEngine.exe" slice -v -m4 -j "$(W "$RES/definitions/creality_ender5.def.json")" \
 -s layer_height=0.16 -s layer_height_0=0.2 -s infill_sparse_thickness=0.16 \
 -s line_width=0.4 -s wall_line_width=0.4 -s wall_line_width_0=0.4 -s wall_line_width_x=0.4 \
 -s skin_line_width=0.4 -s infill_line_width=0.4 -s skirt_brim_line_width=0.4 -s initial_layer_line_width_factor=100 \
 -s wall_thickness=1.2 -s wall_line_count=3 \
 -s top_bottom_thickness=0.96 -s top_thickness=0.96 -s bottom_thickness=0.96 -s top_layers=6 -s bottom_layers=6 \
 -s infill_sparse_density=35 -s infill_pattern=grid -s infill_line_distance=2.286 \
 -s speed_print=50 -s speed_infill=50 -s speed_wall=30 -s speed_wall_0=25 -s speed_wall_x=35 -s speed_topbottom=30 \
 -s speed_layer_0=20 -s speed_print_layer_0=20 -s speed_travel_layer_0=100 -s speed_travel=150 -s skirt_brim_speed=20 \
 -s acceleration_enabled=false -s jerk_enabled=false \
 -s material_diameter=1.75 -s material_print_temperature=200 -s material_print_temperature_layer_0=205 \
 -s material_initial_print_temperature=200 -s material_final_print_temperature=200 \
 -s material_bed_temperature=60 -s material_bed_temperature_layer_0=60 -s material_flow=100 \
 -s retraction_enable=true -s retraction_amount=5 -s retraction_speed=45 -s retraction_retract_speed=45 -s retraction_prime_speed=45 \
 -s cool_fan_enabled=true -s cool_fan_speed=100 -s cool_fan_speed_min=100 -s cool_fan_speed_max=100 -s cool_fan_speed_0=0 \
 -s cool_fan_full_layer=3 -s cool_min_layer_time=10 -s cool_min_speed=10 \
 -s support_enable=false -s adhesion_type=brim -s brim_width=6 -s brim_line_count=15 \
 -s magic_spiralize=false -s retraction_hop_enabled=false \
 $MESH -e0 \
 "$@" \
 -o "$(W "$OUT/${NAME}_Ender5_PLA.gcode")"
