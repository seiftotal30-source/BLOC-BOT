#include <Arduino.h>
#include "pins.h"
#include "motor.h"
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

// =====================================================
// BLOC-BOT MOTOR TEST FIRMWARE
// =====================================================
// Differential drive robot motor bench test
// Hardware:
// - 1x DRV8833 dual motor driver (replaced 2x DRV8871)
// - ESP32-CAM (current) or ESP32-S3 (future)
// - 18650 battery or lab power supply
//
// Features:
// - PWM soft-start ramping (prevents current spikes)
// - Serial command interface for testing
// - Failsafe motor cutoff (300ms timeout)
// - ESP-NOW control (to be added)
//
// Serial Commands:
// - w: Forward
// - s: Backward
// - a: Turn left
// - d: Turn right
// - x: Stop
// - q: Spin left
// - e: Spin right
// - +: Increase speed
// - -: Decrease speed
// - L<speed>: Set left motor (-255 to 255)
// - R<speed>: Set right motor (-255 to 255)
// =====================================================

int currentSpeed = 128;  // Default speed (50%)

// TT motors are rated 3-6 V; a full 2S pack is 8.4 V, so cap PWM at ~75 %
const int MAX_SERIAL_SPEED = 192;

// Serial test commands run for this long, then the failsafe stops the motors
const unsigned long SERIAL_RUN_MS = 2000;
unsigned long serialHoldUntil = 0;

// =====================================================
// ESP-NOW RECEIVER (controller -> robot)
// =====================================================

struct DrivePacket {
    uint8_t magic;  // 0xB1
    int8_t fwd;     // -100..100 (positive = forward)
    int8_t turn;    // -100..100 (positive = right)
};

const int MAX_DRIVE_SPEED = 204;  // 0-255; 80% (~6.2 V to the motors on a 7.7 V pack)

volatile bool newPacket = false;
volatile unsigned long lastPacketTime = 0;  // last drive packet from the controller
volatile int8_t rxFwd = 0;
volatile int8_t rxTurn = 0;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onReceive(const esp_now_recv_info_t*, const uint8_t* data, int len) {
#else
void onReceive(const uint8_t*, const uint8_t* data, int len) {
#endif
    if (len != sizeof(DrivePacket)) return;
    DrivePacket p;
    memcpy(&p, data, sizeof(p));
    if (p.magic != 0xB1) return;
    rxFwd = p.fwd;
    rxTurn = p.turn;
    lastPacketTime = millis();
    newPacket = true;
}

// =====================================================
// LINK LED (on while the controller is connected)
// =====================================================

// Onboard RGB LED only (green = connected). The external test LED on GPIO 4 was removed;
// GPIO 4 is kept for a line sensor (see PINS.md).
const unsigned long LINK_TIMEOUT = 500;  // ms without a drive packet = disconnected (controller sends every 50 ms)
bool linkLedOn = false;

void initLinkLed() {
#ifdef RGB_BUILTIN
    neopixelWrite(RGB_BUILTIN, 0, 0, 0);
#endif
}

void updateLinkLed() {
    bool connected = lastPacketTime != 0 && millis() - lastPacketTime < LINK_TIMEOUT;
    if (connected == linkLedOn) return;
    linkLedOn = connected;
#ifdef RGB_BUILTIN
    neopixelWrite(RGB_BUILTIN, 0, connected ? 40 : 0, 0);  // onboard RGB: green
#endif
    Serial.println(connected ? "Controller connected" : "Controller lost");
}

void applyPacket() {
    if (!newPacket) return;
    newPacket = false;
    int fwd = constrain((int)rxFwd, -100, 100);
    int turn = constrain((int)rxTurn, -100, 100);
    int left = constrain(fwd + turn, -100, 100) * MAX_DRIVE_SPEED / 100;
    int right = constrain(fwd - turn, -100, 100) * MAX_DRIVE_SPEED / 100;
    setMotors(left, right);

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 500) {
        lastPrint = millis();
        Serial.printf("RX fwd=%d turn=%d -> L=%d R=%d\n", fwd, turn, left, right);
    }
}

// =====================================================
// WHEEL SPEED (LM393 slot sensors -> controller)
// =====================================================

// Encoder disc (roue_codeuse_16): 16 slots on the TT output shaft = 16 pulses per wheel turn
const float WHEEL_DIAMETER_MM = 65.0;  // standard TT wheel, change if yours differs
const int ENCODER_SLOTS = 16;
const float MM_PER_PULSE = PI * WHEEL_DIAMETER_MM / ENCODER_SLOTS;  // ~12.8 mm
const unsigned long SPEED_SEND_MS = 100;
const uint32_t ENCODER_DEBOUNCE_US = 1000;  // LM393 boards can chatter on a slot edge

// Must match SpeedPacket in Bloc-Con/src/main.cpp
struct __attribute__((packed)) SpeedPacket {
    uint8_t magic;  // 0xB2
    int16_t left;   // mm/s (negative = reverse)
    int16_t right;  // mm/s (negative = reverse)
};

uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Speed is measured from the time between pulses, not from pulses per 100 ms:
// with 16 slots a 100 ms window holds only 2-5 pulses, so +-1 pulse made the reading jump by 128 mm/s.
const uint32_t ENCODER_STOP_US = 300000;  // no pulse for 300 ms = wheel stopped
const float SPEED_SMOOTHING = 0.4;        // 0..1, higher = faster but noisier

portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t leftPulses = 0, rightPulses = 0;        // pulses in the current window
volatile uint32_t leftPeriodSum = 0, rightPeriodSum = 0;  // sum of pulse-to-pulse times in the window (us)
volatile uint32_t leftLastUs = 0, rightLastUs = 0;
float leftMmFilt = 0, rightMmFilt = 0;

void IRAM_ATTR onLeftPulse() {
    uint32_t now = micros();
    uint32_t period = now - leftLastUs;
    if (period < ENCODER_DEBOUNCE_US) return;
    leftLastUs = now;
    portENTER_CRITICAL_ISR(&encoderMux);
    if (period < ENCODER_STOP_US) {  // skip the first pulse after a stop
        leftPulses++;
        leftPeriodSum += period;
    }
    portEXIT_CRITICAL_ISR(&encoderMux);
}

void IRAM_ATTR onRightPulse() {
    uint32_t now = micros();
    uint32_t period = now - rightLastUs;
    if (period < ENCODER_DEBOUNCE_US) return;
    rightLastUs = now;
    portENTER_CRITICAL_ISR(&encoderMux);
    if (period < ENCODER_STOP_US) {
        rightPulses++;
        rightPeriodSum += period;
    }
    portEXIT_CRITICAL_ISR(&encoderMux);
}

// mm/s from the average pulse period in this window. With no pulse in the window, the time since
// the last pulse gives an upper bound, so the reading falls smoothly to 0 instead of jumping.
float wheelSpeed(uint32_t pulses, uint32_t periodSum, uint32_t lastUs, float prev) {
    uint32_t since = micros() - lastUs;
    float mm;
    if (pulses > 0) mm = pulses * MM_PER_PULSE * 1e6 / periodSum;
    else if (since >= ENCODER_STOP_US) mm = 0;
    else mm = min(prev, MM_PER_PULSE * 1e6f / since);
    return prev + SPEED_SMOOTHING * (mm - prev);
}

void initEncoders() {
    pinMode(ENCODER_LEFT_PIN, INPUT_PULLUP);
    pinMode(ENCODER_RIGHT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_PIN), onLeftPulse, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT_PIN), onRightPulse, RISING);
}

// Every 100 ms: turn the pulse counts into mm/s and send them to the controller
void sendSpeed() {
    static unsigned long lastSend = 0;
    unsigned long now = millis();
    unsigned long dt = now - lastSend;
    if (dt < SPEED_SEND_MS) return;
    lastSend = now;

    portENTER_CRITICAL(&encoderMux);
    uint32_t l = leftPulses, r = rightPulses;
    uint32_t lSum = leftPeriodSum, rSum = rightPeriodSum;
    uint32_t lLast = leftLastUs, rLast = rightLastUs;
    leftPulses = rightPulses = 0;
    leftPeriodSum = rightPeriodSum = 0;
    portEXIT_CRITICAL(&encoderMux);

    leftMmFilt = wheelSpeed(l, lSum, lLast, leftMmFilt);
    rightMmFilt = wheelSpeed(r, rSum, rLast, rightMmFilt);

    // A single-channel sensor can't tell direction, so take it from the motor command
    int leftMm = (int)leftMmFilt;
    int rightMm = (int)rightMmFilt;
    if (getLeftSpeed() < 0) leftMm = -leftMm;
    if (getRightSpeed() < 0) rightMm = -rightMm;

    SpeedPacket p = {0xB2, (int16_t)leftMm, (int16_t)rightMm};
    esp_now_send(broadcastMac, (uint8_t*)&p, sizeof(p));
}

// =====================================================
// IMU (MPU6050 on I2C -> controller)
// =====================================================

#ifdef I2C_SDA_PIN
const uint8_t MPU_ADDR = 0x68;
const unsigned long IMU_SEND_MS = 50;  // 20 Hz to the controller

// Must match ImuPacket in Bloc-Con/src/main.cpp. Angles and rates are x10 (0.1 deg resolution).
struct __attribute__((packed)) ImuPacket {
    uint8_t magic;  // 0xB4
    int16_t roll;   // deg x10
    int16_t pitch;  // deg x10
    int16_t gx;     // deg/s x10
    int16_t gy;
    int16_t gz;
};

bool mpuOk = false;
float gyroOff[3] = {0, 0, 0};
float roll = 0, pitch = 0, gyroX = 0, gyroY = 0, gyroZ = 0;
unsigned long mpuLastUs = 0;

bool mpuRead(uint8_t reg, uint8_t* buf, uint8_t n) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(MPU_ADDR, n) != n) return false;
    for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
    return true;
}

void mpuWrite(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

// Keep the robot still for about a second after power-on: the gyro is zeroed here
void initImu() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);
    Wire.beginTransmission(MPU_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("MPU6050 not found (SDA 8, SCL 9)");
        return;
    }
    mpuWrite(0x6B, 0x00);  // wake up
    mpuWrite(0x1C, 0x00);  // accel +-2 g
    mpuWrite(0x1B, 0x00);  // gyro +-250 deg/s
    delay(100);

    long sum[3] = {0, 0, 0};
    uint8_t b[6];
    for (int i = 0; i < 200; i++) {
        if (mpuRead(0x43, b, 6))
            for (int k = 0; k < 3; k++) sum[k] += (int16_t)((b[2 * k] << 8) | b[2 * k + 1]);
        delay(3);
    }
    for (int k = 0; k < 3; k++) gyroOff[k] = sum[k] / 200.0;
    mpuOk = true;
    Serial.println("MPU6050 ready");
}

// Runs every loop: tilt from gravity blended with the gyro (complementary filter)
void updateImu() {
    if (!mpuOk) return;
    uint8_t b[14];
    if (!mpuRead(0x3B, b, 14)) return;
    auto w = [&](int i) { return (int16_t)((b[i] << 8) | b[i + 1]); };
    float ax = w(0) / 16384.0, ay = w(2) / 16384.0, az = w(4) / 16384.0;
    gyroX = (w(8) - gyroOff[0]) / 131.0;
    gyroY = (w(10) - gyroOff[1]) / 131.0;
    gyroZ = (w(12) - gyroOff[2]) / 131.0;

    float accRoll = atan2(ay, az) * 180.0 / PI;
    float accPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    unsigned long nowUs = micros();
    float dt = (nowUs - mpuLastUs) / 1e6;
    if (mpuLastUs == 0 || dt > 0.2) {
        roll = accRoll;
        pitch = accPitch;
    } else {
        roll = 0.96 * (roll + gyroX * dt) + 0.04 * accRoll;
        pitch = 0.96 * (pitch + gyroY * dt) + 0.04 * accPitch;
    }
    mpuLastUs = nowUs;
}

void sendImu() {
    static unsigned long lastSend = 0;
    if (!mpuOk || millis() - lastSend < IMU_SEND_MS) return;
    lastSend = millis();
    ImuPacket p = {0xB4, (int16_t)(roll * 10), (int16_t)(pitch * 10),
                   (int16_t)(gyroX * 10), (int16_t)(gyroY * 10), (int16_t)(gyroZ * 10)};
    esp_now_send(broadcastMac, (uint8_t*)&p, sizeof(p));
}
#else
void initImu() {}
void updateImu() {}
void sendImu() {}
#endif

// =====================================================
// ULTRASONIC (HC-SR04 -> controller)
// =====================================================

#ifdef ULTRASONIC_TRIG_PIN
const unsigned long ULTRASONIC_PERIOD_MS = 100;  // one ping every 100 ms, sent to the controller
const uint32_t ECHO_MAX_US = 25000;              // about 4.3 m; longer = no object

// Must match DistancePacket in Bloc-Con/src/main.cpp
struct __attribute__((packed)) DistancePacket {
    uint8_t magic;  // 0xB5
    int16_t mm;     // distance in mm, -1 = no object in range
};

// The echo pulse is timed by an interrupt, so the loop (and the motor ramp) never waits for it
volatile uint32_t echoStartUs = 0;
volatile uint32_t echoWidthUs = 0;
volatile bool echoDone = false;

void IRAM_ATTR onEcho() {
    uint32_t now = micros();
    if (digitalRead(ULTRASONIC_ECHO_PIN)) {
        echoStartUs = now;
    } else if (echoStartUs != 0) {
        echoWidthUs = now - echoStartUs;
        echoDone = true;
    }
}

void initUltrasonic() {
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ULTRASONIC_ECHO_PIN), onEcho, CHANGE);
}

// Each period: send the result of the last ping, then fire the next one
void updateUltrasonic() {
    static unsigned long lastPing = 0;
    if (millis() - lastPing < ULTRASONIC_PERIOD_MS) return;
    lastPing = millis();

    int16_t mm = -1;
    if (echoDone && echoWidthUs < ECHO_MAX_US) mm = echoWidthUs * 0.343 / 2;
    DistancePacket p = {0xB5, mm};
    esp_now_send(broadcastMac, (uint8_t*)&p, sizeof(p));

    echoDone = false;
    echoStartUs = 0;
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
}
#else
void initUltrasonic() {}
void updateUltrasonic() {}
#endif

void initRadio() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }
    esp_now_register_recv_cb(onReceive);

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcastMac, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
    Serial.print("ESP-NOW ready. Robot MAC: ");
    Serial.println(WiFi.macAddress());
}

void printHelp() {
    Serial.println("\n=== BLOC-BOT MOTOR TEST ===");
    Serial.println("Commands:");
    Serial.println("  w - Forward");
    Serial.println("  s - Backward");
    Serial.println("  a - Turn left");
    Serial.println("  d - Turn right");
    Serial.println("  q - Spin left");
    Serial.println("  e - Spin right");
    Serial.println("  x - Stop");
    Serial.println("  + - Increase speed");
    Serial.println("  - - Decrease speed");
    Serial.println("  L<value> - Set left motor (-255 to 255)");
    Serial.println("  R<value> - Set right motor (-255 to 255)");
    Serial.println("  h - Show this help");
    Serial.print("\nCurrent speed: ");
    Serial.println(currentSpeed);
}

void processSerialCommand() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();

        if (strchr("wsadqeLlRr", cmd))
            serialHoldUntil = millis() + SERIAL_RUN_MS;
        else if (cmd == 'x')
            serialHoldUntil = 0;

        switch (cmd) {
            case 'w':  // Forward
                setMotors(currentSpeed, currentSpeed);
                Serial.println("Forward");
                break;

            case 's':  // Backward
                setMotors(-currentSpeed, -currentSpeed);
                Serial.println("Backward");
                break;

            case 'a':  // Turn left (slow left motor)
                setMotors(currentSpeed / 2, currentSpeed);
                Serial.println("Turn left");
                break;

            case 'd':  // Turn right (slow right motor)
                setMotors(currentSpeed, currentSpeed / 2);
                Serial.println("Turn right");
                break;

            case 'q':  // Spin left (left backward, right forward)
                setMotors(-currentSpeed, currentSpeed);
                Serial.println("Spin left");
                break;

            case 'e':  // Spin right (left forward, right backward)
                setMotors(currentSpeed, -currentSpeed);
                Serial.println("Spin right");
                break;

            case 'x':  // Stop
                stopMotors();
                Serial.println("Stop");
                break;

            case '+':  // Increase speed
                currentSpeed += 32;
                if (currentSpeed > MAX_SERIAL_SPEED) currentSpeed = MAX_SERIAL_SPEED;
                Serial.print("Speed: ");
                Serial.println(currentSpeed);
                break;

            case '-':  // Decrease speed
                currentSpeed -= 32;
                if (currentSpeed < 32) currentSpeed = 32;
                Serial.print("Speed: ");
                Serial.println(currentSpeed);
                break;

            case 'h':  // Help
                printHelp();
                break;

            case 'L':  // Set left motor directly
            case 'l':
                {
                    int speed = Serial.parseInt();
                    setMotorSpeed('L', speed);
                    Serial.print("Left motor: ");
                    Serial.println(speed);
                }
                break;

            case 'R':  // Set right motor directly
            case 'r':
                {
                    int speed = Serial.parseInt();
                    setMotorSpeed('R', speed);
                    Serial.print("Right motor: ");
                    Serial.println(speed);
                }
                break;

            case '\n':
            case '\r':
                // Ignore newlines
                break;

            default:
                Serial.print("Unknown command: ");
                Serial.println(cmd);
                break;
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=== BLOC-BOT Starting ===");

    // Print board info
    #ifdef ARDUINO_ESP32_DEV
        Serial.println("Board: ESP32-CAM");
        Serial.println("IMPORTANT: GPIO12 must be LOW/floating at boot!");
        Serial.println("See pins_esp32cam.h for upload instructions.");
    #elif defined(ARDUINO_ESP32S3_DEV)
        Serial.println("Board: ESP32-S3");
    #endif

    // Initialize motors
    initMotors();
    initRadio();
    initLinkLed();
    initEncoders();
    initImu();
    initUltrasonic();

    // Show help
    printHelp();

    Serial.println("\nReady for commands!");
}

void loop() {
    // Process serial commands
    processSerialCommand();
    if (millis() < serialHoldUntil)
        resetFailsafe();  // keep a serial test command running
    applyPacket();
    updateLinkLed();
    sendSpeed();
    updateImu();
    sendImu();
    updateUltrasonic();

    // Update motor ramping and failsafe
    updateMotors();

    delay(10);  // 100 Hz update rate
}
