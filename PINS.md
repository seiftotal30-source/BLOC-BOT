# BLOC-BOT pin map

One list for both boards, taken from the code on `main` (2026-10-22). Check a pin here before
wiring anything new, and update this file in the same commit when a pin changes.

- **In code**: used by the firmware on `main`.
- **Planned**: in `electronics/README.md` or tested on the bench, not in the robot firmware yet.

## Bloc-Con: controller (ESP32 DevKit V1, `Bloc-Con/src/main.cpp`)

Board labels: D21 = GPIO 21, VP = GPIO 36, VN = GPIO 39.

| GPIO | Use | Status | Notes |
|---|---|---|---|
| 21 / 22 | OLED SDA / SCL | in code | I2C (default pins); 0.96" SSD1306 now, 2.42" SSD1309 later (`USE_SSD1309`) |
| 5 | HC-SR04 TRIG | in code | strapping pin, fine as an output |
| 18 | HC-SR04 ECHO | in code | through a 1 kΩ / 2 kΩ divider (ECHO is 5 V); HC-SR04 VCC on VIN (5 V) |
| 34 / 35 | joystick X / Y | in code | ADC1, input-only |
| 12 | button UP | in code | **boot risk**: GPIO 12 high at reset stops the ESP32 from booting. Button to GND, no external pull-up; move it if Con ever fails to start |
| 13 | button DOWN | in code | to GND, `INPUT_PULLUP` |
| 14 | button FORWARD | in code | to GND, `INPUT_PULLUP` |
| 27 | button BACK | in code | to GND, `INPUT_PULLUP` |

Free: 4, 16, 17, 19, 23, 25, 26, 32, 33, 36 (VP), 39 (VN). Suggested: 36 for battery sense
(input-only ADC1), 32 for the joystick button.

Avoid: 0, 2, 15 (strapping), 6–11 (flash), 1 / 3 (USB serial). With ESP-NOW on, analog
reads only work on ADC1: 32–39.

## Bloc: robot (ESP32-S3-DevKitC-1 N16R8, `src/` + `include/pins_esp32s3.h`)

| GPIO | Use | Status | Notes |
|---|---|---|---|
| 13 / 16 | DRV8833 AIN1 / AIN2, left motor | in code | 13 replaced 15, which kept failing on forward |
| 18 / 17 | DRV8833 BIN1 / BIN2, right motor | in code | swapped so the right motor turns the right way |
| 40 / 41 | LM393 encoders left / right | in code | sensors on 3.3 V, D0 to the pin, A0 unused |
| 4 | red link LED | in code | **conflict** with the planned line sensor on 4, see below |
| 48 | onboard RGB LED | in code | `RGB_BUILTIN` in the Arduino core (some DevKitC v1.1 boards wire it to 38) |
| 8 / 9 | I2C SDA / SCL | planned | MPU6050 (tested 2026-10-21, address 0x68), OLED face, TCS34725 |
| 1 | battery sense | planned | ADC1, through the 100k / 33k divider |
| 2, 4, 5, 6, 7 | line sensors ×5 (TCRT5000) | planned | ADC1; 3 skipped (strapping) |
| 11 / 12 | HC-SR04 TRIG / ECHO | planned | ECHO through a 1k / 2k divider |
| 21 | WS2812 data | planned | |
| 47 | passive buzzer | planned | |

DRV8833 STBY (nSLEEP) goes straight to 3.3 V and nFAULT is not connected, so the 13 / 14
entries for them in `electronics/README.md` no longer apply (13 is now AIN1).

Free after everything planned: 10, 39, 42, and 38 (if the RGB LED is on 48); 45 / 46 only with
care. The Flying Fish IR sensor still needs one of these.

Avoid: 0, 3, 45, 46 (strapping), 19 / 20 (native USB), 43 / 44 (UART0), 35–37 (octal PSRAM on
N16R8), 26–32 (flash). With ESP-NOW on, analog reads only work on ADC1: GPIO 1–10.

## Open conflicts

1. **GPIO 4**: the link LED (in code) and the 5th line sensor (planned). Line sensors are
   analog and must stay on GPIO 1–10, so move the LED, e.g. to GPIO 10 or 42.
2. **GPIO 13**: AIN1 in code; `electronics/README.md` still lists nFAULT there. Drop nFAULT
   or move it to a free pin.
