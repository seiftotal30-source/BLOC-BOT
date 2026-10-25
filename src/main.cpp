// BLOC-BOT robot (bloc)
// esp32-s3, drv8833 + 2 tt motors, lm393 encoders, hc-sr04, mpu6050
// driven by the controller (con) over esp-now

#include <Arduino.h>
#include "bench.h"
#include "drive.h"
#include "encoders.h"
#include "imu.h"
#include "motor.h"
#include "radio.h"
#include "ultrasonic.h"

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== BLOC-BOT starting ===");

    initMotors();
    initRadio();
    initLinkLed();
    initEncoders();
    initImu();  // keep the robot still for ~1 s here
    initUltrasonic();

    printHelp();
}

// one pass every 10 ms, nothing in here waits for a sensor
void loop() {
    handleSerial();       // bench test commands
    applyDriveCommand();  // last command from the controller
    updateLinkLed();
    sendSpeed();          // every 100 ms
    updateImu();
    sendImu();            // every 50 ms
    updateUltrasonic();   // every 100 ms
    updateMotors();       // failsafe + ramp
    delay(10);
}
