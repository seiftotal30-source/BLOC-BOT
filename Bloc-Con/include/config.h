#ifndef CONFIG_H
#define CONFIG_H

// controller pins, esp32 devkit (see PINS.md)

// oled on the default i2c pins (sda 21, scl 22)
// 0 = 0.96" ssd1306 (fitted now), 1 = 2.42" ssd1309 (planned)
#define USE_SSD1309 0

// buttons to gnd, internal pull-ups
// gpio 12 is a strapping pin: never add an external pull-up on it
#define BTN_UP       12
#define BTN_DOWN     13
#define BTN_FORWARD  14
#define BTN_BACK     27

// joystick on adc1 (adc2 is shared with wi-fi), powered from 3.3 V
#define JOY_X 34
#define JOY_Y 35

// hc-sr04 wired to the controller, bench-test fallback only
// echo through a 1k/2k divider
#define TRIG 5
#define ECHO 18

#endif
