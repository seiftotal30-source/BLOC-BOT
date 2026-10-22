#ifndef PINS_ESP32S3_H
#define PINS_ESP32S3_H

// =====================================================
// ESP32-S3 PIN DEFINITIONS (Future hardware)
// =====================================================
// ESP32-S3 has many more available GPIOs compared to ESP32-CAM
// =====================================================

// DRV8871 Motor Drivers (2 required for differential drive)
// Left motor (DRV8871 #1)
#define MOTOR_LEFT_IN1   13  // moved off GPIO 15, which kept failing on forward
#define MOTOR_LEFT_IN2   16

// Right motor (DRV8871 #2)
#define MOTOR_RIGHT_IN1  18  // 17/18 swapped so the right motor turns the right way
#define MOTOR_RIGHT_IN2  17

// LM393 slot sensors (wheel speed), powered from 3.3V
#define ENCODER_LEFT_PIN   40
#define ENCODER_RIGHT_PIN  41

// I2C bus: MPU6050 (0x68), later the OLED face and TCS34725
#define I2C_SDA_PIN  8
#define I2C_SCL_PIN  9

// HC-SR04 (VCC on 5V). ECHO is 5 V: ECHO -> 1k -> GPIO 12, GPIO 12 -> 2k -> GND
#define ULTRASONIC_TRIG_PIN  11
#define ULTRASONIC_ECHO_PIN  12

// PWM Configuration
#define PWM_FREQ         20000  // 20 kHz for motor drivers
#define PWM_RESOLUTION   8      // 8-bit (0-255)

// PWM Channels
#define PWM_CHANNEL_LEFT_IN1   0
#define PWM_CHANNEL_LEFT_IN2   1
#define PWM_CHANNEL_RIGHT_IN1  2
#define PWM_CHANNEL_RIGHT_IN2  3

#endif // PINS_ESP32S3_H
