#!/usr/bin/perl
# Generates the two BLOC-BOT schematics (robot, controller) as KiCad files.
# Modules are drawn as boxes with their header pins; nets are joined by name labels.
# Run:  perl gen_schematics.pl   (from circuits/kicad), then export with kicad-cli.
use strict;
use warnings;

my $G = 2.54;
sub uid { sprintf("%08x-%04x-4%03x-a%03x-%012x", int(rand(2**32)), int(rand(2**16)),
                  int(rand(2**12)), int(rand(2**12)), int(rand(2**48))) }
sub f { my $v = shift; my $s = sprintf("%.3f", $v); $s =~ s/\.?0+$//; $s eq "-0" ? "0" : $s }
my $FONT = '(effects (font (size 1.524 1.524)))';  # a bit larger than default, for the thesis
my $HIDE = '(effects (font (size 1.27 1.27)) hide)';

# A part: name (library symbol), ref, value, x, y, width, left pins, right pins.
# Each pin: [pin name, net name] ; net "NC" = no connect.
sub symbol_def {
    my ($p) = @_;
    my ($name, $w, $L, $R) = @$p{qw(sym w left right)};
    my $rows = @$L > @$R ? scalar @$L : scalar @$R;
    my $top = ($rows - 1) / 2 * $G;
    my ($y1, $y2) = ($top + $G, -$top - $G);
    my $s = "    (symbol \"bloc:$name\" (pin_names (offset 1.016)) (exclude_from_sim no) (in_bom yes) (on_board yes)\n";
    $s .= "      (property \"Reference\" \"$p->{prefix}\" (at 0 " . f($y1 + 3.81) . " 0) $FONT)\n";
    $s .= "      (property \"Value\" \"$name\" (at 0 " . f($y1 + 1.524) . " 0) $FONT)\n";
    $s .= "      (property \"Footprint\" \"\" (at 0 0 0) $HIDE)\n";
    $s .= "      (property \"Datasheet\" \"\" (at 0 0 0) $HIDE)\n";
    $s .= "      (property \"Description\" \"\" (at 0 0 0) $HIDE)\n";
    $s .= "      (symbol \"${name}_0_1\" (rectangle (start " . f(-$w / 2) . " " . f($y1) . ") (end " . f($w / 2) . " " . f($y2)
        . ") (stroke (width 0.254) (type default)) (fill (type background))))\n";
    $s .= "      (symbol \"${name}_1_1\"\n";
    my $n = 1;
    for my $side ([$L, -1], [$R, 1]) {
        my ($pins, $dir) = @$side;
        for my $i (0 .. $#$pins) {
            my $py = $top - $i * $G;
            my $px = $dir * ($w / 2 + $G);
            my $ang = $dir < 0 ? 0 : 180;
            $s .= "        (pin passive line (at " . f($px) . " " . f($py) . " $ang) (length 2.54) (name \"$pins->[$i][0]\" $FONT) (number \"$n\" $FONT))\n";
            $n++;
        }
    }
    $s .= "      )\n    )\n";
    return $s;
}

sub build {
    my ($project, $title, $paper, $notes, @parts) = @_;
    my $root = uid();
    my (%seen, $lib, $body) = ((), "", "");
    for my $p (@parts) {
        $lib .= symbol_def($p) unless $seen{$p->{sym}}++;
        my ($X, $Y, $w, $L, $R) = @$p{qw(x y w left right)};
        $X = sprintf("%.0f", $X / $G) * $G; $Y = sprintf("%.0f", $Y / $G) * $G;
        my $rows = @$L > @$R ? scalar @$L : scalar @$R;
        my $top = ($rows - 1) / 2 * $G;
        my $y1 = $top + $G;
        $body .= "  (symbol (lib_id \"bloc:$p->{sym}\") (at " . f($X) . " " . f($Y) . " 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no) (uuid \"" . uid() . "\")\n";
        $body .= "    (property \"Reference\" \"$p->{ref}\" (at " . f($X) . " " . f($Y - $y1 - 3.81) . " 0) $FONT)\n";
        $body .= "    (property \"Value\" \"$p->{value}\" (at " . f($X) . " " . f($Y - $y1 - 1.524) . " 0) $FONT)\n";
        $body .= "    (property \"Footprint\" \"\" (at " . f($X) . " " . f($Y) . " 0) $HIDE)\n";
        $body .= "    (property \"Datasheet\" \"\" (at " . f($X) . " " . f($Y) . " 0) $HIDE)\n";
        $body .= "    (property \"Description\" \"\" (at " . f($X) . " " . f($Y) . " 0) $HIDE)\n";
        my $npins = @$L + @$R;
        $body .= "    (pin \"$_\" (uuid \"" . uid() . "\"))\n" for 1 .. $npins;
        $body .= "    (instances (project \"$project\" (path \"/$root\" (reference \"$p->{ref}\") (unit 1))))\n  )\n";

        # stub wire + net label (or no-connect) on every pin
        for my $side ([$L, -1], [$R, 1]) {
            my ($pins, $dir) = @$side;
            for my $i (0 .. $#$pins) {
                my $net = $pins->[$i][1];
                my $ax = $X + $dir * ($w / 2 + $G);
                my $ay = $Y - ($top - $i * $G);
                if ($net eq 'NC') {
                    $body .= "  (no_connect (at " . f($ax) . " " . f($ay) . ") (uuid \"" . uid() . "\"))\n";
                    next;
                }
                my $ex = $ax + $dir * 2 * $G;
                $body .= "  (wire (pts (xy " . f($ax) . " " . f($ay) . ") (xy " . f($ex) . " " . f($ay) . ")) (stroke (width 0) (type default)) (uuid \"" . uid() . "\"))\n";
                my ($ang, $just) = $dir < 0 ? (180, 'right bottom') : (0, 'left bottom');
                $body .= "  (label \"$net\" (at " . f($ex) . " " . f($ay) . " $ang) (fields_autoplaced yes) (effects (font (size 1.524 1.524)) (justify $just)) (uuid \"" . uid() . "\"))\n";
            }
        }
    }
    my $text = "";
    for my $t (@$notes) {
        my ($s, $x, $y, $size) = @$t;
        $text .= "  (text \"$s\" (exclude_from_sim no) (at " . f($x) . " " . f($y) . " 0) (effects (font (size $size $size)) (justify left bottom)) (uuid \"" . uid() . "\"))\n";
    }
    my $out = "(kicad_sch (version 20231120) (generator \"eeschema\") (generator_version \"8.0\")\n";
    $out .= "  (uuid \"$root\")\n  (paper \"$paper\")\n";
    $out .= "  (title_block (title \"$title\") (date \"2026-10-25\") (rev \"1\") (company \"BLOC-BOT\") (comment 1 \"Modules drawn as boxes with their header pins; nets joined by name\"))\n";
    $out .= "  (lib_symbols\n$lib  )\n$body$text";
    $out .= "  (sheet_instances (path \"/\" (page \"1\")))\n)\n";

    mkdir $project unless -d $project;
    open my $fh, '>', "$project/$project.kicad_sch" or die $!;
    print $fh $out;
    close $fh;
    open $fh, '>', "$project/$project.kicad_pro" or die $!;
    print $fh "{\n  \"meta\": { \"filename\": \"$project.kicad_pro\", \"version\": 1 }\n}\n";
    close $fh;
    print "wrote $project/$project.kicad_sch\n";
}

# helper to write parts compactly
sub part { my %h = @_; $h{prefix} //= substr($h{ref}, 0, 1) =~ /[A-Z]/ ? ($h{ref} =~ /^([A-Z]+)/)[0] : 'U'; return \%h }

# ---------------------------------------------------------------- robot
build('robot', 'BLOC-BOT robot (Bloc)', 'A4',
    [],  # explanations are in the thesis caption
    part(sym => 'Battery_2S', ref => 'BT1', value => '2S 18650 pack 7.4 V', x => 45, y => 50, w => 20.32,
         left => [], right => [['+', 'VBAT'], ['-', 'GND']]),
    part(sym => 'Mini360', ref => 'U7', value => 'Mini360 buck, set to 5 V', x => 45, y => 75, w => 20.32,
         left => [['IN+', 'VBAT'], ['IN-', 'GND']], right => [['OUT+', '+5V'], ['OUT-', 'GND']]),
    part(sym => 'ESP32-S3_board', ref => 'U1', value => 'ESP32-S3 board (N16R8)', x => 110, y => 65, w => 25.4,
         left => [['5V', '+5V'], ['3V3', '+3V3'], ['GND', 'GND']],
         right => [['GPIO13', 'AIN1'], ['GPIO16', 'AIN2'], ['GPIO17', 'BIN1'], ['GPIO18', 'BIN2'],
                   ['GPIO40', 'ENC_L'], ['GPIO41', 'ENC_R'], ['GPIO11', 'TRIG'], ['GPIO12', 'ECHO_3V3'],
                   ['GPIO8', 'SDA'], ['GPIO9', 'SCL']]),
    part(sym => 'DRV8833_module', ref => 'U2', value => 'DRV8833 module', x => 180, y => 55, w => 20.32,
         left => [['NC', 'NC'], ['AIN2', 'AIN2'], ['AIN1', 'AIN1'], ['STBY', '+3V3'], ['BIN1', 'BIN1'],
                  ['BIN2', 'BIN2'], ['NC', 'NC'], ['GND', 'GND']],
         right => [['VM', 'VBAT'], ['NC', 'NC'], ['GND', 'GND'], ['AO1', 'ML_A'], ['AO2', 'ML_B'],
                   ['BO2', 'MR_B'], ['BO1', 'MR_A'], ['GND', 'GND']]),
    part(sym => 'TT_motor', ref => 'M1', value => 'TT motor, left', x => 240, y => 45, w => 15.24,
         left => [['A', 'ML_A'], ['B', 'ML_B']], right => []),
    part(sym => 'TT_motor', ref => 'M2', value => 'TT motor, right', x => 240, y => 65, w => 15.24,
         left => [['A', 'MR_A'], ['B', 'MR_B']], right => []),
    part(sym => 'LM393_speed_sensor', ref => 'U3', value => 'LM393 slot sensor, left', x => 180, y => 100, w => 15.24,
         left => [['VCC', '+3V3'], ['GND', 'GND'], ['DO', 'ENC_L'], ['AO', 'NC']], right => []),
    part(sym => 'LM393_speed_sensor', ref => 'U4', value => 'LM393 slot sensor, right', x => 240, y => 100, w => 15.24,
         left => [['VCC', '+3V3'], ['GND', 'GND'], ['DO', 'ENC_R'], ['AO', 'NC']], right => []),
    part(sym => 'HC-SR04', ref => 'U5', value => 'HC-SR04', x => 180, y => 135, w => 15.24,
         left => [['VCC', '+5V'], ['TRIG', 'TRIG'], ['ECHO', 'ECHO_5V'], ['GND', 'GND']], right => []),
    part(sym => 'R', ref => 'R1', value => '1k', x => 240, y => 130, w => 10.16,
         left => [['1', 'ECHO_5V']], right => [['2', 'ECHO_3V3']]),
    part(sym => 'R', ref => 'R2', value => '2k', x => 240, y => 142, w => 10.16,
         left => [['1', 'ECHO_3V3']], right => [['2', 'GND']]),
    part(sym => 'MPU6050_module', ref => 'U6', value => 'MPU6050 module', x => 110, y => 120, w => 15.24,
         left => [['VCC', '+3V3'], ['GND', 'GND'], ['SCL', 'SCL'], ['SDA', 'SDA']], right => []),
);

# ---------------------------------------------------------------- controller
build('controller', 'BLOC-BOT controller (Con)', 'A4',
    [],  # explanations are in the thesis caption
    part(sym => 'Cell_18650', ref => 'BT1', value => '18650 cell 2000 mAh', x => 30, y => 40, w => 15.24,
         left => [], right => [['+', 'CELL+'], ['-', 'CELL-']]),
    part(sym => 'TP4056_module', ref => 'U4', value => 'TP4056 module (USB-C, protection)', x => 85, y => 40, w => 20.32,
         left => [['B+', 'CELL+'], ['B-', 'CELL-']], right => [['OUT+', 'VSW_IN'], ['OUT-', 'GND']]),
    part(sym => 'Switch', ref => 'SW5', value => 'power switch', x => 140, y => 38, w => 10.16,
         left => [['1', 'VSW_IN']], right => [['2', 'VBOOST_IN']]),
    part(sym => 'MT3608', ref => 'U5', value => 'MT3608 boost, set to 5 V', x => 195, y => 40, w => 20.32,
         left => [['VIN+', 'VBOOST_IN'], ['VIN-', 'GND']], right => [['VOUT+', '+5V'], ['VOUT-', 'GND']]),
    part(sym => 'ESP32_DevKit', ref => 'U1', value => 'ESP32 DevKit (30 pins)', x => 110, y => 90, w => 25.4,
         left => [['VIN', '+5V'], ['3V3', '+3V3'], ['GND', 'GND']],
         right => [['GPIO34', 'JOY_X'], ['GPIO35', 'JOY_Y'], ['GPIO12', 'BTN_UP'], ['GPIO13', 'BTN_DOWN'],
                   ['GPIO14', 'BTN_FWD'], ['GPIO27', 'BTN_BACK'], ['GPIO21', 'SDA'], ['GPIO22', 'SCL']]),
    part(sym => 'KY-023', ref => 'U2', value => 'KY-023 joystick', x => 195, y => 85, w => 15.24,
         left => [['GND', 'GND'], ['+5V', '+3V3'], ['VRx', 'JOY_X'], ['VRy', 'JOY_Y'], ['SW', 'NC']], right => []),
    part(sym => 'OLED_SSD1306', ref => 'U3', value => 'OLED 0.96in SSD1306 (0x3C)', x => 195, y => 120, w => 15.24,
         left => [['GND', 'GND'], ['VCC', '+3V3'], ['SCL', 'SCL'], ['SDA', 'SDA']], right => []),
    part(sym => 'Push_button', ref => 'SW1', value => 'UP', x => 255, y => 75, w => 10.16,
         left => [['1', 'BTN_UP']], right => [['2', 'GND']]),
    part(sym => 'Push_button', ref => 'SW2', value => 'DOWN', x => 255, y => 88, w => 10.16,
         left => [['1', 'BTN_DOWN']], right => [['2', 'GND']]),
    part(sym => 'Push_button', ref => 'SW3', value => 'FORWARD', x => 255, y => 101, w => 10.16,
         left => [['1', 'BTN_FWD']], right => [['2', 'GND']]),
    part(sym => 'Push_button', ref => 'SW4', value => 'BACK', x => 255, y => 114, w => 10.16,
         left => [['1', 'BTN_BACK']], right => [['2', 'GND']]),
);
