# Controller User Parameters (all mm unless noted). Re-run after editing, then re-run the build.
#
# Plan coordinates: origin at the case centre, X to the right, Y toward the top edge (display),
# Z up from the bottom outer face. "(model)" values were read from the imported reference
# models in CAD\controller\; "(assumed)" values had no model and are listed in the report.
exec(open(r'E:\Claude Stuff\CAD\scripts\ctl_helpers.py').read())
print('ACTIVE:', app.activeDocument.name)

P = [
    # --- shell ---
    ('case_w', '178', 'outer width (X)'),
    ('case_h', '87', 'outer height (Y)'),
    ('case_t', '24.5', 'outer thickness (Z)'),
    ('wall', '2', 'wall / floor / top thickness'),
    ('fillet_r', '3', 'outer edge fillet'),
    ('corner_ch', '6', 'plan corner chamfer, GrabCAD RC Transmitter style'),
    ('z_split', '19', 'parting plane height (both USB openings stay in the bottom shell)'),
    ('clr', '0.4', 'clearance on every cutout and pocket'),
    # --- OLED 2.42" SSD1309 (model) ---
    ('oled_pcb_w', '72', 'PCB width (model)'),
    ('oled_pcb_h', '42.8', 'PCB height (model)'),
    ('oled_pcb_t', '1.2', 'PCB thickness (model)'),
    ('oled_glass_w', '62', 'glass width (model)'),
    ('oled_glass_h', '39.8', 'glass height (model)'),
    ('oled_glass_t', '3.7', 'glass height above PCB front (model)'),
    ('oled_aa_w', '56.6', 'active area width (model, grey face)'),
    ('oled_aa_h', '29.5', 'active area height (model, grey face)'),
    ('oled_aa_dy', '2.65', 'active area centre offset from PCB centre toward the top edge (model)'),
    ('oled_glass_dy', '0.5', 'glass centre offset from PCB centre toward the bottom edge (model)'),
    ('oled_hole_sx', '68', 'mounting hole spacing X (model)'),
    ('oled_hole_sy', '38.8', 'mounting hole spacing Y (model)'),
    ('oled_hole_d', '3.0', 'mounting hole dia (model)'),
    ('oled_lip', '1', 'bezel lip thickness in front of the glass'),
    ('oled_boss_d', '5', 'OLED boss dia'),
    ('oled_pad_t', '0.05', 'copper pad thickness around the holes, boss stops on it (model)'),
    ('oled_cx', '0', 'PCB centre X'),
    ('oled_cy', 'case_h/2 - wall - clr - oled_pcb_h/2', 'PCB centre Y (top edge)'),
    ('bscrew_pilot', '2.1', 'pilot for M2.5 self-tapping board screws (assumed screw)'),
    # --- KY-023 joystick (model) ---
    ('joy_x', '-59.5', 'stick centre X'),
    ('joy_y', '-9.5', 'stick centre Y'),
    ('joy_pivot_dz', '4', 'top surface above the gimbal pivot'),
    ('joy_sweep_r', '13.65', 'cap skirt sphere radius about the pivot (model torus 12.48+1.17)'),
    ('joy_tilt', '29', 'deg, max tilt before the cap touches the module case (model)'),
    ('joy_open_d', '2*(joy_sweep_r + 0.6)', 'thumb opening dia (sweep sphere + clearance)'),
    ('joy_pcb_top_dz', '6.99', 'PCB top below pivot (model)'),
    ('joy_hole_x1', '-12.5', 'hole X rel. stick (model)'),
    ('joy_hole_x2', '14', 'hole X rel. stick (model)'),
    ('joy_hole_y1', '-11.25', 'hole Y rel. stick (model)'),
    ('joy_hole_y2', '9.25', 'hole Y rel. stick (model)'),
    ('joy_pcb_bot_dz', '8.6', 'PCB underside below pivot (model)'),
    ('joy_post_d', '4.6', 'joystick support post dia (from the bottom shell, M2.5 screws from above)'),
    # --- 12x12 tactile switch, OMRON B3F-4050 (model) ---
    ('sw_body', '12', 'body square (model)'),
    ('sw_plunger_h', '7.35', 'plunger top above seating plane (model)'),
    ('sw_peg_off', '4.5', 'locating peg offset (model)'),
    ('sw_peg_d', '1.6', 'locating peg dia (model)'),
    ('sw_peg_h', '1.35', 'locating peg length (model)'),
    ('sw_travel', '0.25', 'switch travel (assumed, B3F datasheet)'),
    ('sw_pillar', '10', 'support pillar square (legs clear outside it)'),
    # --- caps ---
    ('cap_gap', '0.1', 'free play between cap and plunger at rest'),
    ('cap_flange_t', '1.2', 'flange thickness'),
    ('cap_protrude', '2', 'cap height above the top surface'),
    ('dpad_cap_d', '9', 'D-pad cap dia'),
    ('bf_cap_d', '8', 'BACK/FORWARD cap dia'),
    ('cap_flange_d', '13', 'flange dia'),
    ('dpad_x', '-joy_x', 'D-pad centre X'),
    ('dpad_y', 'joy_y', 'D-pad centre Y'),
    ('dpad_pitch', '20', 'D-pad switch spacing from centre'),
    ('bf_y', '-11', 'BACK/FORWARD centre Y'),
    ('bf_pitch', '15', 'BACK/FORWARD spacing'),
    # --- ESP32 DevKit V1 30-pin (model) ---
    ('esp_l', '51.2', 'PCB length (model)'),
    ('esp_w', '28.3', 'PCB width (model)'),
    ('esp_t', '1.6', 'PCB thickness (model)'),
    ('esp_pin_below', '8.2', 'header pins below PCB (model)'),
    ('esp_standoff', '9', 'PCB underside above floor (pins + clearance = wiring space)'),
    ('esp_hole_d', '2.9', 'hole dia (model)'),
    ('esp_post_d', '3.2', 'standoff dia (clears header pins by 0.4)'),
    ('esp_peg_d', 'esp_hole_d - clr', 'locating peg dia'),
    ('esp_x', '-case_w/2 + wall + clr + esp_l', 'plan X of the PCB end away from USB'),
    ('esp_y', '3.3', 'plan Y of the PCB lower edge'),
    ('usbm_w', '7.98', 'micro-USB receptacle width (model)'),
    ('usbm_h', '2.94', 'micro-USB receptacle height (model)'),
    ('usbm_plug_w', '11', 'micro-USB plug overmold width (assumed)'),
    ('usbm_plug_h', '8', 'micro-USB plug overmold height (assumed)'),
    ('usb_cb', '1.2', 'outside counterbore depth for the plug overmold'),
    # --- TP4056 Type-C (model) ---
    ('tp_l', '28.3', 'PCB length (model)'),
    ('tp_w', '17.4', 'PCB width (model)'),
    ('tp_t', '1.5', 'PCB thickness (model)'),
    ('tp_cx', '-58.3', 'PCB centre X (between the joystick posts)'),
    ('tp_ledge', '1', 'PCB underside above floor'),
    ('usbc_w', '8.42', 'USB-C receptacle width (model)'),
    ('usbc_h', '2.79', 'USB-C receptacle height (model)'),
    ('usbc_plug_w', '12.4', 'USB-C plug overmold width (assumed)'),
    ('usbc_plug_h', '6.5', 'USB-C plug overmold height (assumed)'),
    # --- MT3608 (model) ---
    ('mt_l', '37.2', 'PCB length (model)'),
    ('mt_w', '17.23', 'PCB width (model)'),
    ('mt_t', '1.52', 'PCB thickness (model)'),
    ('mt_h', '6.45', 'height incl. inductor (model)'),
    ('mt_x', '44.4', 'PCB left edge X'),
    ('mt_ledge', '1', 'PCB underside above floor'),
    # --- 18650 holder (assumed, no model) ---
    ('bat_l', '77', 'holder length (assumed)'),
    ('bat_w', '21', 'holder width (assumed)'),
    ('bat_h', '20', 'holder height (assumed)'),
    ('bat_wall_gap', '1', 'holder to wall gap (clears the 1 mm inner corner fillet)'),
    ('rib_t', '1.2', 'pocket rib / clip thickness'),
    ('rib_h', '6', 'battery pocket rib height'),
    # --- slide switch (assumed, no model) ---
    ('sl_l', '12', 'body length (assumed)'),
    ('sl_w', '6', 'body depth (assumed)'),
    ('sl_h', '6.5', 'body height (assumed)'),
    ('sl_x', '36.4', 'centre X'),
    ('sl_base', '1', 'pad under the switch (clears the inner corner fillet)'),
    ('sl_act_w', '2', 'actuator width (assumed)'),
    ('sl_act_out', '4', 'actuator length out of the body (assumed)'),
    ('sl_slot_w', '7', 'actuator slot length incl. throw'),
    ('sl_slot_h', '3.5', 'actuator slot height'),
    # --- case screws: 4 x M3 into pilots in the top shell ---
    ('m3_pilot', '2.6', 'M3 pilot hole'),
    ('m3_boss_d', '6', 'top-shell pilot boss dia'),
    ('m3_clear', '3.4', 'clearance hole in the bottom shell'),
    ('m3_cb_d', '6', 'head counterbore dia (M3 head 5.5 + clr)'),
    ('m3_cb_h', '3', 'head counterbore depth'),
    ('m3_bboss_d', '9', 'bottom-shell boss dia'),
    ('scr1_x', '-80', ''), ('scr1_y', '36.5', ''),
    ('scr2_x', '80', ''), ('scr2_y', '12', ''),
    ('scr3_x', '-80', ''), ('scr3_y', '-35', ''),
    ('scr4_x', '80', ''), ('scr4_y', '-35', ''),
]
for n, e, c in P:
    setp(n, e, c, '' if n == 'joy_tilt' else 'mm')
print('%d parameters set' % len(P))
for n in ('oled_cy', 'joy_open_d', 'esp_x'):
    print(n, round(pv(n), 3))
for n in ('joy_boss_d',):  # retired: the joystick now sits on posts from the bottom shell
    p = des.userParameters.itemByName(n)
    if p: p.deleteMe()
