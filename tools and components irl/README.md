# Tools and components (real photos)

Photos of the actual hardware we own, with the details read off each one.
Add new photos to `components/` or `tools/` and list them here.

## Components

| Photo | What it is | Notes |
|---|---|---|
| [drv8871-front.jpg](components/drv8871-front.jpg) | DRV8871 motor driver (x2) | Screw terminals left to right: MOTOR (OUT2, OUT1), POWER (⊕ VM, ⊖ GND). Header left to right: IN2, IN1, VM, GND. 47 µF 50 V cap on board. |
| [drv8871-back-pin-labels.jpg](components/drv8871-back-pin-labels.jpg) | DRV8871, back | Confirms labels: OUT2, OUT1, VM, GND / IN2, IN1, VM, GND. Header VM is battery voltage: never connect it to the ESP32. VM min 6.5 V. |
| [esp32-s3-n16r8-devkit.jpg](components/esp32-s3-n16r8-devkit.jpg) | ESP32-S3-N16R8 DevKitC-1 clone (robot MCU) | Two USB-C ports. Left side: 3V3, 3V3, RST, 4, 5, 6, 7, 15, 16, 17, 18, 8, 3, 46, 9, 10, 11, 12, 13, 14, 5Vin, GND. Right side: GND, TX, RX, 1, 2, 42, 41, 40, 39, 38, 37, 36, 35, 0, 45, 48, 47, 21, 20, 19, GND, GND. Avoid 35–37 (PSRAM), 0/3/45/46 (strapping), 19/20 (USB). Motor test uses 15/16 (left) and 17/18 (right). |

## Tools

| Photo | What it is | Notes |
|---|---|---|
| [charger-hzs-002-label.jpg](tools/charger-hzs-002-label.jpg) | Li-ion charger HZS-002 | Input AC 100–240 V, output 4.2 V / 600 mA (shared). Cheap: check cells with a meter and remove at 4.15–4.2 V. Never leave unattended. |
| [charger-hzs-002-slots.jpg](tools/charger-hzs-002-slots.jpg) | HZS-002, slots | 2 independent slots. + end at the top tabs, flat − end on the sliding contacts. |
| [oscilloscope-ch1-menu.jpg](tools/oscilloscope-ch1-menu.jpg) | 200 MHz 1 GS/s DSO, CH1 menu | Coupling DC 1 MΩ, BW Limit, Volts/Div, Probe (F1–F4). |
| [oscilloscope-ch1-10x-20mhz.jpg](tools/oscilloscope-ch1-10x-20mhz.jpg) | DSO, CH1 set for DC measuring | DC 1 MΩ, BW 20 MHz, Probe 10× (must match the probe switch). |
| [oscilloscope-measure-menu.jpg](tools/oscilloscope-measure-menu.jpg) | DSO, MEASURE menu | MasterSrc must be CH1. All Para On (F3) or User Def (F4) → Mean = DC voltage. |
