// Controller STLs. The top plate is stored in print orientation
// (outer face down on the bed, posts pointing up), so it is flipped
// 180 deg about X to show it as assembled (posts pointing down into the shell).
gap = 25;        // extra lift for the exploded view (0 = closed)
plate_t = 3;     // approx. top plate thickness - adjust if the fit looks off
bottom_h = 19;
top_h = 11.7;

color("steelblue") import("../out/controller_bottom.stl");

translate([0, 0, bottom_h + plate_t + gap])
  rotate([180, 0, 0]) color("orange") import("../out/controller_top.stl");

translate([0, 0, bottom_h + plate_t + 2*gap]) color("green")
  import("../out/controller_caps.stl");
