# BLOC-BOT: electronics plan

Status: **proposed** (2026-10-06). Nothing here is wired or tested yet. The robot MCU may
move from the ESP32 DevKit V1 to an **ESP32-S3-DevKitC-1 N16R8**; the pin map below is for
the S3.

![Robot circuit](robot_circuit.svg)

## Power

Robot: 2S 18650 pack (6.4–8.4 V) → 2S BMS with balancing → 3 A fuse → main switch, then:

- battery straight to both DRV8871 VM pins, with 220–470 µF at each driver;
- battery → buck converter, 5 V, at least 2 A → ESP32-S3 5V/VIN, WS2812 LEDs;
- battery → 100k/33k divider → GPIO 1 (battery sense);
- 0.1 µF ceramic across each motor's terminals.

Controller (Bloc-Con): 1 × 18650 → TP4056 + MT3608 boost → 5 V → ESP32 VIN.

All grounds are tied together: battery, buck, drivers, ESP32 and every sensor.

## Robot pin map (ESP32-S3, proposed)

| GPIO | Use | Notes |
|---|---|---|
| 1 | Battery sense | ADC1, through the divider |
| 2, 4, 5, 6, 7 | Line sensors ×5 (TCRT5000) | ADC1; GPIO 3 skipped (strapping) |
| 8 / 9 | I2C SDA / SCL | OLED face, MPU6050, TCS34725 |
| 11 / 12 | HC-SR04 TRIG / ECHO | ECHO through a 1k/2k divider (5 V → 3.3 V) |
| 15 / 16 | DRV8871 left IN1 / IN2 | |
| 17 / 18 | DRV8871 right IN1 / IN2 | |
| 21 | WS2812 data | keep brightness low (up to 60 mA per LED) |
| 40 / 41 | LM393 encoders L / R | sensors powered at 3.3 V |
| 47 | Passive buzzer | PWM tones |
| spare | 10, 13, 14, 38, 39, 42; 45/46 with care | 38 is the onboard RGB LED on DevKitC v1.1 |

Not usable on N16R8: 35–37 (octal PSRAM). Kept for upload/serial: 43/44 (UART0) and 19/20
(native USB). Avoid 0, 3, 45, 46 for anything that could be pulled high at boot. With
WiFi/ESP-NOW running, only ADC1 (GPIO 1–10) can read analog values.

Count: about 27 usable GPIOs, 9 for the base robot, 11 with all the add-ons above,
leaving about 7 spare.

## Build steps

1. **Bench tests, one part at a time:** ESP32 + one DRV8871 + one TT motor (direction and
   PWM); one encoder (count pulses by hand, then with the motor); controller buttons,
   joystick, OLED and HC-SR04 with the existing Bloc-Con code.
2. **Power:** build the robot power chain and the controller power chain above.
3. **Integration:** both motors and encoders on the plate, battery sense, then the
   ESP-NOW link from Bloc-Con to the robot (the empty "Start" menu item).
4. **Firmware protections:** PWM soft-start, encoder stall cutoff, low-battery cutoff,
   motors stop if the link is lost for more than 300 ms.
5. **Add-ons:** OLED face, RGB LEDs, buzzer, line sensors, IMU, colour sensor.
6. **Final wiring:** breadboard → perfboard or PCB, screw terminals for motors and battery.
7. **Floor tests:** measure current draw and runtime (data for the thesis).

## Known risks

| Issue | Why | Fix |
|---|---|---|
| DRV8871 cuts out at half charge | needs ≥ 6.5 V; 2S is ~6.4 V empty and sags under load on ~1 A cells | low-battery cutoff at 6.8–7.0 V; less usable runtime |
| TT motors over-volted | rated 3–6 V, pack gives up to 8.4 V | cap PWM duty at ~70 % |
| ESP32 resets when motors start | inrush pulls the supply down (brownout) | soft-start, bulk caps, separate 5 V buck |
| Noisy encoder counts | motor noise on signal wires | motor caps, separate encoder and motor wiring, software debounce |
| Controller won't boot | UP button on GPIO 12 (strapping pin) | button to GND with INPUT_PULLUP, no external pull-up; or move it |
| HC-SR04 echo is 5 V | ESP32 pins are 3.3 V | 1k/2k divider on ECHO |
| ADC2 dead with WiFi | ESP-NOW uses the WiFi radio | analog only on ADC1 |
| 2S cells drift apart | unbranded, unmatched cells | balancing BMS; proper 2S charger or charge cells separately |
| Odd logic behaviour | no common ground | tie all grounds together |
| DRV8871 not working | missing ILIM resistor | check the module has one (usually ~2 A) |

## Simulation and documentation tools

| Tool | Use |
|---|---|
| **Wokwi** (VS Code + PlatformIO) | Runs the real ESP32 firmware with OLED, buttons, joystick, HC-SR04. No DC motor driver part. |
| **KiCad** | Proper schematics, and a PCB later |
| **Fritzing** | Breadboard-style wiring pictures |
| Proteus (optional) | Only if the supervisor requires it; it does not really simulate the ESP32 |
