#ifndef PINS_ESP32S3_H
#define PINS_ESP32S3_H

// robot pins, esp32-s3 n16r8 (see PINS.md)
// do not use 35-37 (octal psram), 0/3/45/46 (strapping), 19/20 (usb), 43/44 (uart0)

// drv8833, left motor on bridge a
#define MOTOR_LEFT_IN1   13  // was 15, that was a bad jumper wire
#define MOTOR_LEFT_IN2   16

// drv8833, right motor on bridge b
#define MOTOR_RIGHT_IN1  18  // 17/18 swapped so both wheels go forward together
#define MOTOR_RIGHT_IN2  17

// lm393 slot sensors, powered from 3.3 V
#define ENCODER_LEFT_PIN   40
#define ENCODER_RIGHT_PIN  41

// i2c: mpu6050 (0x68), later more sensors
#define I2C_SDA_PIN  8
#define I2C_SCL_PIN  9

// hc-sr04 on 5 V. echo -> 1k -> gpio 12, gpio 12 -> 2k -> gnd (5 V to 3.3 V)
#define ULTRASONIC_TRIG_PIN  11
#define ULTRASONIC_ECHO_PIN  12

// motor pwm
#define PWM_FREQ         20000  // 20 kHz
#define PWM_RESOLUTION   8      // 0-255

#define PWM_CHANNEL_LEFT_IN1   0
#define PWM_CHANNEL_LEFT_IN2   1
#define PWM_CHANNEL_RIGHT_IN1  2
#define PWM_CHANNEL_RIGHT_IN2  3

#endif
