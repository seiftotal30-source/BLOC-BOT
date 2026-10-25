# Test plan

Only tests that give a clear number, table or photo for the Realisation and results
chapter, or something to show live at the defense. Save photos and numbers in
`thesis/results/<test_name>/` and add a row to `log.md`.

## Measurements for the thesis

| # | Test | How | Goes in the thesis as |
|---|---|---|---|
| T1 | **Real speed** | Time the robot over 2 m at full speed, 3 runs. Before that, turn a wheel 10 turns by hand and check the encoder count is 200 (20 slots), so the speed on the controller is right. | Speed in km/h, measured vs displayed |
| T2 | **Battery life** | Full charge, wheels lifted, constant command. NO BMS: do not run until it stops. Measure the pack with a multimeter every 10 min and stop at 6.0 V (3.0 V per cell); note the time. | Runtime in minutes |
| T3 | **Ultrasonic accuracy** | Box at 10, 20, 50 cm (tape measure; reading sticks at 59.9 cm beyond ~60 cm: suspect floor echo, check by lifting/tilting the sensor at a wall 1 m away), read the controller. | Small table: real vs measured distance |
| T4 | **Print fits** | Already done: screw hole Ø6.2 → 6.0, glue joint 0.14 → 0.20 mm/side. Photograph the parts. | Table + photos |
| T5 | **Print time, filament, weight** | From the slicer (time, grams) and a kitchen scale (full robot). | Parts table and total mass |

## Live demos for the defense

| # | Demo | What the jury sees |
|---|---|---|
| D1 | **Driving** | Robot driven from the controller, speed shown on the screen |
| D2 | **Safety stop** | Turn the controller off while driving and the robot stops by itself |
| D3 | **Modularity** | Move a bloc (e.g. the sensor) to another spot on the plate in seconds |
| D4 | **Line following** | Once it works: the robot follows a tape line |
