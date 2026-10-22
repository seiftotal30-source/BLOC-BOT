#include <Arduino.h>
#include "pins.h"
#include "motor.h"
#include <WiFi.h>
#include <esp_now.h>

// =====================================================
// BLOC-BOT MOTOR TEST FIRMWARE
// =====================================================
// Differential drive robot motor bench test
// Hardware:
// - 2x DRV8871 motor drivers (one per TT motor)
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
volatile unsigned long lastPacketTime = 0;  // any packet from the controller (drive or heartbeat)
volatile int8_t rxFwd = 0;
volatile int8_t rxTurn = 0;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onReceive(const esp_now_recv_info_t*, const uint8_t* data, int len) {
#else
void onReceive(const uint8_t*, const uint8_t* data, int len) {
#endif
    if (len == 1 && data[0] == 0xB3) {  // heartbeat: link only, never moves the motors
        lastPacketTime = millis();
        return;
    }
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
const unsigned long LINK_TIMEOUT = 500;  // ms without a packet = disconnected (heartbeat every 100 ms)
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

    // Update motor ramping and failsafe
    updateMotors();

    delay(10);  // 100 Hz update rate
}
