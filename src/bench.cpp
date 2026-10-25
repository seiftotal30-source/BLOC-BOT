#include <Arduino.h>
#include "bench.h"
#include "motor.h"

// tt motors are 3-6 V and a full 2S pack is 8.4 V, so cap at ~75 %
const int MAX_SERIAL_SPEED = 192;

// a serial command runs this long, then the failsafe stops the motors
const unsigned long SERIAL_RUN_MS = 2000;

static int currentSpeed = 128;  // 50 %
static unsigned long holdUntil = 0;

void printHelp() {
    Serial.println("\n=== BLOC-BOT BENCH TEST ===");
    Serial.println("  w / s - forward / backward");
    Serial.println("  a / d - turn left / right");
    Serial.println("  q / e - spin left / right");
    Serial.println("  x     - stop");
    Serial.println("  + / - - faster / slower");
    Serial.println("  L<n> / R<n> - set left / right motor (-255..255)");
    Serial.println("  h     - this help");
    Serial.print("Current speed: ");
    Serial.println(currentSpeed);
}

static void runCommand(char cmd) {
    if (strchr("wsadqeLlRr", cmd))
        holdUntil = millis() + SERIAL_RUN_MS;
    else if (cmd == 'x')
        holdUntil = 0;

    switch (cmd) {
        case 'w':
            setMotors(currentSpeed, currentSpeed);
            Serial.println("Forward");
            break;
        case 's':
            setMotors(-currentSpeed, -currentSpeed);
            Serial.println("Backward");
            break;
        case 'a':  // slow the left wheel
            setMotors(currentSpeed / 2, currentSpeed);
            Serial.println("Turn left");
            break;
        case 'd':  // slow the right wheel
            setMotors(currentSpeed, currentSpeed / 2);
            Serial.println("Turn right");
            break;
        case 'q':
            setMotors(-currentSpeed, currentSpeed);
            Serial.println("Spin left");
            break;
        case 'e':
            setMotors(currentSpeed, -currentSpeed);
            Serial.println("Spin right");
            break;
        case 'x':
            stopMotors();
            Serial.println("Stop");
            break;
        case '+':
            currentSpeed = min(currentSpeed + 32, MAX_SERIAL_SPEED);
            Serial.printf("Speed: %d\n", currentSpeed);
            break;
        case '-':
            currentSpeed = max(currentSpeed - 32, 32);
            Serial.printf("Speed: %d\n", currentSpeed);
            break;
        case 'h':
            printHelp();
            break;
        case 'L':
        case 'l': {
            int speed = Serial.parseInt();
            setMotorSpeed('L', speed);
            Serial.printf("Left motor: %d\n", speed);
            break;
        }
        case 'R':
        case 'r': {
            int speed = Serial.parseInt();
            setMotorSpeed('R', speed);
            Serial.printf("Right motor: %d\n", speed);
            break;
        }
        case '\n':
        case '\r':
            break;
        default:
            Serial.printf("Unknown command: %c\n", cmd);
            break;
    }
}

void handleSerial() {
    if (Serial.available() > 0) runCommand(Serial.read());
    if (millis() < holdUntil) resetFailsafe();  // keep the command running
}
