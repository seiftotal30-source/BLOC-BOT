# Test plan

Tests that can be run on the real hardware to give the Realisation and results chapter
real numbers. For each one, save the raw data (CSV, photos, scope screenshots) in
`thesis/results/<test_name>/`, together with the script that makes the plot, and add a
row to `log.md`.

Priority: **A** = do first (cheap, and the thesis needs it), **B** = good to have,
**C** = only once the feature exists.

## Robot: motion

| # | Test | How | Result / figure | Pri |
|---|---|---|---|---|
| T1 | **True top speed** | Mark 2 m on the floor and time 5 full-speed runs with a phone (slow-motion video is more accurate). Compare with the speed the controller shows. | Table: measured vs displayed speed. Checks the ~10 km/h reading. | A |
| T2 | **Encoder calibration** | Turn one wheel exactly 10 times by hand with serial printing on, and count the pulses. Expect 160 if the disc has 16 slots. Measure the wheel diameter with calipers. | Pulses per turn, real mm per pulse, fix in the code. | A |
| T3 | **Straight-line drift** | Full forward for 3 m and measure the sideways offset. Repeat 5 times. | Mean ± spread of drift. Shows the left/right motor mismatch. | A |
| T4 | **Speed vs PWM curve** | Set fixed PWM values over serial (`L`/`R` 50, 100, 150, 200) with the wheels lifted and log the encoder speed. | Plot: wheel speed vs duty cycle, both motors. | B |
| T5 | **Ramp / stopping** | Video at full speed, release the stick, and measure the stopping distance. | Stopping distance. Shows the soft-start ramp effect. | B |
| T6 | **Turning radius / spin** | Spin in place for 5 turns and time it. | Rotation speed (°/s). | B |
| T7 | **Payload** | Add known weights (water bottles) in 200 g steps and repeat T1. | Plot: speed vs load. Shows what the plate carries. | B |

## Radio link

| # | Test | How | Result / figure | Pri |
|---|---|---|---|---|
| T8 | **Range** | Drive away in a corridor and outdoors. Note the distance where the link LED goes off or the robot stops. 3 runs each. | Max range indoors / outdoors. | A |
| T9 | **Failsafe** | Switch the controller off while driving and measure the stopping time on video (should be ~300 ms + ramp). | Proof the safety stop works. | A |
| T10 | **Latency** | Film the controller screen and the wheel together at 240 fps, push the stick, and count frames until the wheel moves. | Command-to-motion delay (ms). | B |
| T11 | **Packet loss** | Add a counter to the packets and count missing ones at 1, 5, 10, 20 m. | Plot: loss % vs distance. | B |

## Power

| # | Test | How | Result / figure | Pri |
|---|---|---|---|---|
| T12 | **Battery life (robot)** | From full charge (8.4 V), drive in loops or run on a stand. Note the pack voltage every 5 min with the multimeter until 6.4 V. | Runtime + discharge curve. | A |
| T13 | **Current draw** | Multimeter in series with the pack: idle, wheels lifted, driving, stalled (briefly!). | Table of currents; compare with DRV8833 limits. | A |
| T14 | **Voltage sag** | Scope on the pack and on the ESP32's 5 V/3.3 V while starting at full speed. | Scope screenshot. Supports the DRV8871 → DRV8833 story. | B |
| T15 | **Controller battery life** | Full charge, leave it on the Drive screen, note when it dies. | Runtime. | B |

## Electronics (you have an oscilloscope)

| # | Test | How | Result / figure | Pri |
|---|---|---|---|---|
| T16 | **PWM signal** | Scope on IN1 at a few duty cycles. | Screenshots: 20 kHz, duty matches the command. | A |
| T17 | **Encoder signal** | Scope on the LM393 output while the wheel turns. | Clean square wave or bounce. Justifies the 1 ms debounce. | B |
| T18 | **HC-SR04 accuracy** | Place a box at 10, 20, 50, 100, 150 cm (tape measure), 10 readings each. | Plot: measured vs real distance, with error bars. | A |

## Mechanical / printing

| # | Test | How | Result / figure | Pri |
|---|---|---|---|---|
| T19 | **Print fits** | Already have results (screw hole Ø6.2 → 6.0, glue joint 0.14 → 0.20 mm/side). Photograph the test pieces. | Table: tolerance tried → result. | A |
| T20 | **Dimensional accuracy** | Calipers on the plate and a few blocs, compared with the CAD values. | Table: CAD vs printed, error %. | A |
| T21 | **Bloc swap time** | Time how long it takes to move a sensor bloc to another position. | Shows the modular idea in seconds. | B |
| T22 | **Print time & material** | From the slicer: time and grams of filament per part. | Cost/time table for the open-source section. | A |
| T23 | **Mass** | Kitchen scale: each part and the full robot. | Mass table (real one, not the proposal's). | A |

## Features in progress

| # | Test | How | Result / figure | Pri |
|---|---|---|---|---|
| T24 | **Line sensor readings** | Log the 5 channels over white floor / black tape at different heights. | Plot: raw value vs height. Confirms the HBloc_L sensor height. | C |
| T25 | **Line following** | Tape track and time a lap. Count how often it loses the line. | Success rate, lap time. | C |
| T26 | **Self-parking / obstacle stop** | Stop distance from a wall, 10 runs. | Mean ± spread of stop distance. | C |
