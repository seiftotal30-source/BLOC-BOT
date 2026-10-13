# Circuits

Pictures of the wiring diagrams made for BLOC-BOT. Each PNG has its source page in `src/`
(open it in a browser to see it in light or dark mode, or edit it and render again).

| Picture | What it shows | Status |
|---|---|---|
| [drv8833_motor_circuit.png](drv8833_motor_circuit.png) | Both TT motors on one DRV8833 driver: power in, 5 V buck for the ESP32, GPIO 15–18 to AIN/BIN, STBY to 3.3 V | Current |
| [esc_bench_test.png](esc_bench_test.png) | One TT motor on a brushed RC ESC, signal on GPIO 13 | Replaced by the DRV8833 |

Test code for the DRV8833 circuit: [`firmware/drv8833_test`](../firmware/drv8833_test/drv8833_test.ino).
