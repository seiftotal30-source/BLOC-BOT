// BLOC-BOT: rear mount for the MH-Sensor-Series "Flying Fish" IR obstacle module.
// Test part (BlocBot-Sim). Units: mm.
//
// Robot frame: x right, y forward, z up, floor at z = 0. Plate rear edge at y = -89.75
// (the plate's own edge; only the hex pillar tabs reach out to y = -104).
// The board lies flat, long side along y, IR LED end pointing backwards (-y).
// The whole part is one profile in the XZ plane extruded along y, so it prints on its
// side with no supports.
//
//   export = true  -> print-oriented solid only (for the STL)
//   export = false -> mount in place on the robot, with ghost plate, caster and board

export = false;

// ---- robot (from the BLOC-BOT repo README and the "waffle plate" Fusion design)
plate_under_z = 47.75;   // plate underside above the floor
plate_rear_y  = -89.75;  // rear edge of the plate body, measured in Fusion. The README's
                         // 210 x 210 is the outline including the hex pillar tabs.
caster_y      = -80;     // rear caster, centred on x = 0
// Plate holes the flange bolts through (robot x, y), measured in Fusion: 4.5 mm through
// holes on the right-rear side of the plate. M3 screws, head on top of the plate.
plate_holes = [[50, -85], [57, -71]];

// ---- board (from the GrabCAD model "Sensor, IR, Arduino Type, Flying Fish MH-B",
//      BLOC-BOT/CAD/sensor-ir-arduino-type-flying-fish-mh-b-1.snapshot.3.zip)
pcb_l = 31.25;           // LED end to header end
pcb_w = 14.15;
pcb_t = 1.6;
led_axis_above = 1.3;    // LED centre above the PCB top
led_gap = 1.5;           // LED flange starts this far past the PCB edge
led_protrude = 10.2;     // LED tips past the PCB edge
led_pitch = 8;           // LED centres are +-4 from the board centre line
pin_below = 1.0;         // LED leads and header pins poke this far through the PCB bottom
pin_span = 11.2;         // width of the band those pin tips sit in (+-5.6 from centre)
hole_y = 24.25;          // board hole centre from the LED end (7 from the header end)
hole_d = 3.4;            // board hole is 3.0; 3.4 here = M3 clearance (prints lying sideways,
                         // so it comes out a little small)

// ---- placement
sensor_h = 35;           // LED centre line above the floor
x_offset = 35;           // board centre to the side of the caster (+x = right)
led_overhang = 0;        // PCB LED edge past the plate rear edge (+ = further out)

// ---- part
wall = 4;                // drop wall / near rail width
rail = 4;                // far rail width
shelf_t = 3;             // shelf under the board
channel_w = 12;          // relief channel in the shelf for the pin tips
channel_d = 1.5;         // its depth (pin tips stick out 1.0)
stop_l = 1.5;            // end stop at the header end; the board slides in from the LED end
lip = 0.7;               // how far the rails overlap the board edges. 1.0 left a 0.97 mm tall
                         // chip on the board only 0.14 mm from the lip (checked on the model)
lip_t = 1.2;             // lip thickness above the board
clr = 0.2;               // clearance each side of the board in the grooves
flange_t = 4;
flange_w = 16;           // flange reach sideways, away from the board; ends at robot
                         // x = 62, before the plate's rounded corner cut-out starts (x = 63)
screw_d = 3.4;           // M3 clearance holes, on the plate holes above
$fn = 48;

// ---- derived (local frame: plate underside z = 0, board x 0..pcb_w, LED end y = 0)
drop   = plate_under_z - sensor_h;          // 12.75 with the defaults
pcb_top = -drop - led_axis_above;
pcb_bot = pcb_top - pcb_t;
lip_top = pcb_top + clr + lip_t;
shelf_bot = pcb_bot - clr - shelf_t;
x_near = -wall;                              // outer face of the drop wall
x_far  = pcb_w + rail;
fx0 = x_near - flange_w;                     // flange outer edge
body_l = pcb_l + clr + stop_l;               // length of the part along y

module profile() {
    difference() {
        union() {
            translate([fx0, -flange_t]) square([flange_w + wall + lip, flange_t]);    // flange
            translate([x_near, shelf_bot]) square([wall + lip, -shelf_bot]);         // drop wall
            translate([x_near, shelf_bot]) square([x_far - x_near, shelf_t]);         // shelf
            translate([pcb_w - lip, shelf_bot]) square([rail + lip, lip_top - shelf_bot]); // far rail
        }
        translate([-clr, pcb_bot - clr]) square([pcb_w + 2 * clr, pcb_t + 2 * clr]); // board groove
        translate([lip, pcb_top]) square([pcb_w - 2 * lip, 50]);                      // room above the board
        translate([(pcb_w - channel_w) / 2, pcb_bot - clr - channel_d])
            square([channel_w, channel_d + 0.01]);                                  // pin-tip channel
    }
}

// robot (x, y) -> local (x, y); inverse of place() below
function to_local(p) = [x_offset + pcb_w / 2 - p[0], p[1] - (plate_rear_y - led_overhang)];

module mount_local() {
    difference() {
        union() {
            rotate([90, 0, 0]) mirror([0, 0, 1]) linear_extrude(body_l) profile();  // extrude along +y
            // end stop: fills the groove on both edge strips, clear of the pin-tip channel
            for (x0 = [-clr, (pcb_w + channel_w) / 2])
                translate([x0, pcb_l + clr, pcb_bot - clr - 0.01]) cube([(pcb_w - channel_w) / 2 + clr, stop_l, pcb_t + clr + 0.01]);
        }
        for (p = plate_holes)                           // screw holes on the plate holes
            translate([to_local(p)[0], to_local(p)[1], 0]) cylinder(d = screw_d, h = 50, center = true);
        translate([pcb_w / 2, hole_y, shelf_bot]) cylinder(d = hole_d, h = 10, center = true);
        // the far lip is cut away from the LED end up to past the pot, so the pot can slide in
        translate([pcb_w / 2 - 0.9, -1, pcb_top]) cube([8.01, 24.25 + 0.5 + 1, 10]);
    }
}

module dummy_board() {
    translate([0, 0, pcb_bot]) {
        c = pcb_w / 2;
        color("royalblue") difference() {
            cube([pcb_w, pcb_l, pcb_t]);
            translate([c, hole_y, -1]) cylinder(d = 3, h = 5);
        }
        for (s = [-1, 1]) {                                                // IR LED + receiver
            color(s < 0 ? "white" : "black") translate([c + s * led_pitch / 2, 0, pcb_t + led_axis_above]) rotate([90, 0, 0]) {
                translate([0, 0, led_gap]) cylinder(d = 5.8, h = 1.1);
                translate([0, 0, led_gap]) cylinder(d = 4.9, h = led_protrude - led_gap - 2.45);
                translate([0, 0, led_protrude - 2.45]) sphere(d = 4.9);
            }
            for (o = [2.73, 5.27])                                         // LED leads through the board
                color("silver") translate([c + s * o - 0.3, -led_gap, -pin_below]) cube([0.6, led_gap + 2.87, pin_below + pcb_t + led_axis_above]);
        }
        color("dodgerblue") translate([c - 0.4 + 0.0, 11.92, pcb_t]) cube([7.0, 12.33, 4.91]); // pot
        color("black") translate([c - 3.62, 24.25, pcb_t]) cube([7.25, 6.65, 1.9]);          // header body
        for (i = [-1, 0, 1]) color("gold") {                                                 // right-angle pins
            translate([c + i * 2.54 - 0.33, 29.32, -pin_below]) cube([0.66, 0.66, pin_below + pcb_t + 5.03]);
            translate([c + i * 2.54 - 0.33, 29.32, pcb_t + 4.37]) cube([0.66, 7.14, 0.66]);
        }
    }
}

// local -> robot: board centre at x_offset, flange towards +x (away from the caster),
// LED end at the plate rear edge, plate underside at plate_under_z
module place() {
    translate([x_offset + pcb_w / 2, plate_rear_y - led_overhang, plate_under_z])
        mirror([1, 0, 0]) children();
}

if (export) {
    // stand the extrusion on its header-end face: the profile is flat on the bed.
    // Mirrored like place() does, so the printed part is the one shown on the robot
    // (flange away from the caster), not its mirror image.
    rotate([-90, 0, 0]) translate([0, -body_l, 0]) mirror([1, 0, 0]) mount_local();
} else {
    place() { color("orange") mount_local(); %dummy_board(); }
    %translate([plate_rear_y, plate_rear_y, plate_under_z]) cube([-2 * plate_rear_y, 60, 10]); // plate (rear strip)
    %translate([0, caster_y, 0]) cylinder(r = 15, h = plate_under_z);             // caster envelope
    color("red", 0.4) translate([x_offset, plate_rear_y - 60, sensor_h]) rotate([-90, 0, 0]) cylinder(d = 1, h = 60); // beam axis
}

echo(str("drop below plate = ", drop, " mm; shelf bottom above floor = ", plate_under_z + shelf_bot,
         " mm; nearest part to caster centre x = ", x_offset + pcb_w / 2 - x_far, " mm"));
echo(str("flange robot x ", x_offset + pcb_w / 2 - (x_near + wall + lip), " .. ", x_offset + pcb_w / 2 - fx0,
         ", y ", plate_rear_y - led_overhang, " .. ", plate_rear_y - led_overhang + body_l));
for (p = plate_holes) {
    l = to_local(p);
    assert(l[0] > fx0 + screw_d / 2 + 1.5 && l[0] < x_near - screw_d / 2 - 0.5
           && l[1] > screw_d / 2 + 1.5 && l[1] < body_l - screw_d / 2 - 1.5,
           str("plate hole ", p, " is not on the flange"));
}
