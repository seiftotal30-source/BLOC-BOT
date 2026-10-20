# Test plan

Only tests that give a clear number, table or photo for the Realisation and results
chapter, or something to show live at the defense. Save photos and numbers in
`thesis/results/<test_name>/` and add a row to `log.md`.

## Measurements for the thesis

| # | Test | How | Goes in the thesis as |
|---|---|---|---|
| T1 | **Real speed** | Time the robot over 2 m at full speed, 3 runs. Before that, turn a wheel 10 turns by hand and check the encoder count is 160 (16 slots), so the speed on the controller is right. | Speed in km/h, measured vs displayed |
| T2 | **Battery life** | Full charge, drive around (or wheels lifted) until it stops, note the time. | Runtime in minutes |
| T3 | **Ultrasonic accuracy** | Box at 10, 20, 50, 100 cm (tape measure), read the controller. | Small table: real vs measured distance |
| T4 | **Print fits** | Already done: screw hole Ø6.2 → 6.0, glue joint 0.14 → 0.20 mm/side. Photograph the parts. | Table + photos |
| T5 | **Print time, filament, weight** | From the slicer (time, grams) and a kitchen scale (full robot). | Parts table and total mass |

## Live demos for the defense

| # | Demo | What the jury sees |
|---|---|---|
| D1 | **Driving** | Robot driven from the controller, speed shown on the screen |
| D2 | **Safety stop** | Turn the controller off while driving and the robot stops by itself |
| D3 | **Modularity** | Move a bloc (e.g. the sensor) to another spot on the plate in seconds |
| D4 | **Line following** | Once it works: the robot follows a tape line |
