# Circuits

Pictures of the wiring diagrams made for BLOC-BOT. Each PNG has its source page in `src/`
(open it in a browser to see it in light or dark mode, or edit it and render again).

| Picture | What it shows | Status |
|---|---|---|
| [drv8833_motor_circuit.png](drv8833_motor_circuit.png) | Both TT motors on one DRV8833 driver: power in, 5 V buck for the ESP32, GPIO 13, 16, 17, 18 to AIN/BIN (GPIO 15 dropped), STBY to 3.3 V, link LED on GPIO 4, LM393 speed sensors DO on GPIO 40 (L) / 41 (R) | Current |
| [bloccon_controller_circuit.png](bloccon_controller_circuit.png) | Bloc-Con controller: 18650 → TP4056 → switch → MT3608 (5 V) → ESP32; OLED I²C 21/22, joystick 34/35, HC-SR04 5/18, buttons 12, 13, 14, 27 to GND | Current |
| [esc_bench_test.png](esc_bench_test.png) | One TT motor on a brushed RC ESC, signal on GPIO 13 | Replaced by the DRV8833 |

Test code for the DRV8833 circuit: [`firmware/drv8833_test`](../firmware/drv8833_test/drv8833_test.ino).
