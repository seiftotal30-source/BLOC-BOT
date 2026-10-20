#ifndef PINS_ESP32CAM_H
#define PINS_ESP32CAM_H

// =====================================================
// ESP32-CAM PIN DEFINITIONS
// =====================================================
// CRITICAL: ESP32-CAM has VERY limited free GPIOs!
// Many pins are used by camera, SD card, and flash LED.
//
// RESERVED PINS (DO NOT USE):
// GPIO0  - Boot mode / Camera
// GPIO1  - Serial TX (FTDI upload)
// GPIO2  - Flash LED / Boot strapping
// GPIO3  - Serial RX (FTDI upload)
// GPIO4  - Flash LED (often has external LED)
// GPIO5  - Camera
// GPIO6-11 - SPI Flash (NEVER USE!)
// GPIO12 - Camera / Boot strapping
// GPIO13 - Camera
// GPIO14 - Camera
// GPIO15 - Camera / Boot strapping
// GPIO16 - PSRAM
//
// SAFE TO USE (with caution):
// GPIO1  - Serial TX (only if not uploading)
// GPIO3  - Serial RX (only if not uploading)
// GPIO13 - Safe after boot (camera not used)
// GPIO14 - Safe after boot (camera not used)
// GPIO15 - Safe after boot (camera not used)
// GPIO12 - Safe for output only (boot strapping pin)
// GPIO2  - Safe for output (has pull-down at boot)
//
// For motor driver testing with 2x DRV8871:
// Each DRV8871 needs 2 pins (IN1, IN2) for one motor
// Total: 4 pins for 2 motors
// =====================================================

// DRV8871 Motor Drivers (2 required for differential drive)
// Left motor (DRV8871 #1)
#define MOTOR_LEFT_IN1   13  // Safe: Not used when camera disabled
#define MOTOR_LEFT_IN2   15  // Safe: Not used when camera disabled

// Right motor (DRV8871 #2)
#define MOTOR_RIGHT_IN1  14  // Safe: Not used when camera disabled
#define MOTOR_RIGHT_IN2  12  // CAUTION: Strapping pin, must be LOW or floating at boot

// PWM Configuration
#define PWM_FREQ         20000  // 20 kHz for motor drivers
#define PWM_RESOLUTION   8      // 8-bit (0-255)

// PWM Channels (ESP32 has 16 channels, 0-15)
#define PWM_CHANNEL_LEFT_IN1   0
#define PWM_CHANNEL_LEFT_IN2   1
#define PWM_CHANNEL_RIGHT_IN1  2
#define PWM_CHANNEL_RIGHT_IN2  3

// UPLOAD INSTRUCTIONS FOR ESP32-CAM:
// 1. Connect FTDI adapter: GND-GND, 5V-5V, TX-U0R (GPIO3), RX-U0T (GPIO1)
// 2. Connect GPIO0 to GND (boot mode)
// 3. Press reset button
// 4. Upload firmware
// 5. Disconnect GPIO0 from GND
// 6. Press reset to run

// IMPORTANT WARNING:
// GPIO12 (MOTOR_RIGHT_IN2) must be LOW or floating during boot.
// If your motor driver pulls this pin HIGH, the ESP32 may fail to boot.
// Solution: Add a pull-down resistor (10kΩ) or disconnect motor during upload.

#endif // PINS_ESP32CAM_H
