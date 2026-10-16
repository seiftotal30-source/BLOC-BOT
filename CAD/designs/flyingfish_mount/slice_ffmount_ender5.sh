#!/bin/sh
# Slice flyingfish_mount.stl for a Creality Ender-5 with the CuraEngine from UltiMaker Cura 5.13.
# Same settings as the first G-code sliced in the Cura app: Creality "Standard Quality" (0.2 mm)
# with brim, bed 60 C, 40 % infill, 3 walls and -0.15 mm initial layer horizontal expansion.
# The command-line engine doesn't compute derived values, so the profile's values are set here.
#
# Usage (Git Bash):  sh CAD/designs/flyingfish_mount/slice_ffmount_ender5.sh
# The STL is centred on x = y = 0; the engine moves it to the bed centre (110, 110).

CURA="/d/UltiMaker Cura 5.13.0"
RES="$CURA/share/cura/resources"
DIR="/e/Claude Stuff/CAD/designs/flyingfish_mount"
export CURA_ENGINE_SEARCH_PATH="$(cygpath -w "$RES/definitions");$(cygpath -w "$RES/extruders")"
W() { cygpath -w "$1"; }

# settings without a default value; repeated per mesh
MESH="-s roofing_layer_count=0 -s flooring_layer_count=0"

"$CURA/CuraEngine.exe" slice -v -m4 -j "$(W "$RES/definitions/creality_ender5.def.json")" \
 -s layer_height=0.2 -s layer_height_0=0.2 -s infill_sparse_thickness=0.2 \
 -s line_width=0.4 -s wall_line_width=0.4 -s wall_line_width_0=0.4 -s wall_line_width_x=0.4 \
 -s skin_line_width=0.4 -s infill_line_width=0.4 -s skirt_brim_line_width=0.4 -s initial_layer_line_width_factor=100 \
 -s wall_line_count=3 -s wall_thickness=1.2 -s optimize_wall_printing_order=true -s z_seam_type=back \
 -s top_bottom_thickness=0.8 -s top_thickness=0.8 -s bottom_thickness=0.8 -s top_layers=4 -s bottom_layers=4 -s skin_overlap=10 \
 -s infill_sparse_density=40 -s infill_pattern=cubic -s infill_line_distance=3.0 -s infill_before_walls=false -s infill_wipe_dist=0 \
 -s xy_offset_layer_0=-0.15 \
 -s speed_print=80 -s speed_infill=80 -s speed_wall=40 -s speed_wall_0=40 -s speed_wall_x=40 -s speed_topbottom=40 \
 -s speed_layer_0=20 -s speed_print_layer_0=20 -s speed_travel_layer_0=100 -s speed_travel=200 -s skirt_brim_speed=20 \
 -s acceleration_enabled=false -s jerk_enabled=false \
 -s material_diameter=1.75 -s material_print_temperature=200 -s material_print_temperature_layer_0=200 \
 -s material_initial_print_temperature=200 -s material_final_print_temperature=200 \
 -s material_bed_temperature=60 -s material_bed_temperature_layer_0=60 -s material_flow=100 \
 -s retraction_enable=true -s retraction_amount=5 -s retraction_speed=45 -s retraction_retract_speed=45 -s retraction_prime_speed=45 \
 -s retraction_combing=noskin -s retraction_hop_enabled=false \
 -s cool_fan_enabled=true -s cool_fan_speed=100 -s cool_fan_speed_min=100 -s cool_fan_speed_max=100 -s cool_fan_speed_0=0 \
 -s cool_fan_full_layer=3 -s cool_min_layer_time=10 -s cool_min_speed=10 \
 -s support_enable=false -s adhesion_type=brim -s brim_width=8 -s brim_line_count=20 -s magic_spiralize=false \
 $MESH -e0 \
 -l "$(W "$DIR/stl/flyingfish_mount.stl")" $MESH \
 -o "$(W "$DIR/gcode/ffmount_ender5.gcode")"
