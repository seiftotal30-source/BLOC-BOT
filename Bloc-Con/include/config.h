#ifndef CONFIG_H
#define CONFIG_H

// controller pins, esp32 devkit (see PINS.md)

// oled i2c on 25 / 26: same edge of the board as the oled in the case
// (21 / 22 sit on the far edge, next to the joystick)
// 0 = 0.96" ssd1306, 1 = 2.42" ssd1309, 2 = 1.3" sh1106 (fitted now)
#define OLED_TYPE 2
#define OLED_SDA 25
#define OLED_SCL 26

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
