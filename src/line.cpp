#include <Arduino.h>
#include "line.h"
#include "motor.h"
#include "packets.h"
#include "pins.h"
#include "radio.h"

#ifdef LINE_PIN_1

const unsigned long LINE_SEND_MS = 50;      // state to the controller, 20 Hz
const unsigned long CMD_TIMEOUT_MS = 300;   // no mode packet for this long = back to off
const int LINE_SAMPLES = 4;                 // averaged per reading, the outputs are a bit noisy

// calibration: a sensor whose floor/line range is smaller than this is ignored (too weak)
const int MIN_RANGE = 150;
// normalised darkness, 0 = floor, 1000 = line. below NOISE counts as floor
const int NOISE = 200;
const int SEEN = 500;  // the line is seen when one sensor is darker than this

// steering. duty 0..255, MAX_DRIVE_SPEED in drive.cpp is 204
const int BASE_SPEED = 90;          // straight-line duty, kept low for the first tests
const float KP = 0.9;               // duty per unit of position (-100..100)
const float KD = 4.0;               // duty per unit of position change per loop
const int SEARCH_SPEED = 80;        // spin toward the last side the line was seen on
const unsigned long LOST_STOP_MS = 1500;  // stop if the line is not found again in this time

static const uint8_t linePins[5] = {LINE_PIN_1, LINE_PIN_2, LINE_PIN_3, LINE_PIN_4, LINE_PIN_5};
static const int weights[5] = {-100, -50, 0, 50, 100};  // sensor position across the bar

static uint16_t raw[5];
static uint16_t calMin[5], calMax[5];
static uint8_t usable = 0;  // bit i = sensor i has a usable range

static volatile uint8_t mode = LINE_OFF;
static volatile unsigned long lastCmdTime = 0;
static uint8_t activeMode = LINE_OFF;  // mode applied in the loop

static int position = 0, lastPosition = 0;
static bool seen = false, lost = false;
static unsigned long lastSeenTime = 0;

static void resetCalibration() {
    for (int i = 0; i < 5; i++) {
        calMin[i] = 4095;
        calMax[i] = 0;
    }
    usable = 0;
}

void initLine() {
    for (int i = 0; i < 5; i++) pinMode(linePins[i], INPUT);
    analogReadResolution(12);  // 0..4095
    resetCalibration();
}

void handleLineCmdPacket(const uint8_t* data, int len) {
    if (len != sizeof(LineCmdPacket) || data[0] != ID_LINE_CMD) return;
    mode = data[1];
    lastCmdTime = millis();
}

static void readSensors() {
    for (int i = 0; i < 5; i++) {
        uint32_t sum = 0;
        for (int s = 0; s < LINE_SAMPLES; s++) sum += analogRead(linePins[i]);
        raw[i] = sum / LINE_SAMPLES;
    }
}

// the line is darker = lower reading, so darkness = how far below the floor value
static void computePosition() {
    long sum = 0, weighted = 0;
    int darkest = 0;
    for (int i = 0; i < 5; i++) {
        if (!(usable & (1 << i))) continue;
        int range = calMax[i] - calMin[i];
        int dark = constrain((long)(calMax[i] - raw[i]) * 1000 / range, 0, 1000);
        darkest = max(darkest, dark);
        dark = max(dark - NOISE, 0);
        sum += dark;
        weighted += (long)dark * weights[i];
    }
    seen = darkest >= SEEN && sum > 0;
    if (seen) {
        position = weighted / sum;
        lastSeenTime = millis();
    }
}

static void calibrate() {
    for (int i = 0; i < 5; i++) {
        calMin[i] = min(calMin[i], raw[i]);
        calMax[i] = max(calMax[i], raw[i]);
        if (calMax[i] - calMin[i] >= MIN_RANGE) usable |= 1 << i;
    }
}

static void follow() {
    if (seen) {
        lost = false;
        float turn = KP * position + KD * (position - lastPosition);
        lastPosition = position;
        int left = constrain((int)(BASE_SPEED + turn), -BASE_SPEED * 2, BASE_SPEED * 2);
        int right = constrain((int)(BASE_SPEED - turn), -BASE_SPEED * 2, BASE_SPEED * 2);
        setMotors(left, right);
    } else if (!lost && millis() - lastSeenTime < LOST_STOP_MS) {
        // spin on the spot toward the side the line left from
        if (lastPosition >= 0) setMotors(SEARCH_SPEED, -SEARCH_SPEED);
        else setMotors(-SEARCH_SPEED, SEARCH_SPEED);
    } else {
        if (!lost) Serial.println("Line lost, stopped");
        lost = true;
        setMotors(0, 0);  // keeps the failsafe fed, so the robot stays stopped but in follow mode
    }
}

void updateLine() {
    readSensors();

    // the controller must keep sending the mode, like drive commands
    uint8_t want = (millis() - lastCmdTime < CMD_TIMEOUT_MS) ? mode : LINE_OFF;
    if (want == LINE_FOLLOW && usable == 0) want = LINE_OFF;  // nothing calibrated yet

    if (want != activeMode) {
        if (want == LINE_CALIBRATE) resetCalibration();
        if (want == LINE_FOLLOW) {
            lost = false;
            lastSeenTime = millis();
            lastPosition = 0;
        }
        if (activeMode == LINE_FOLLOW) stopMotors();  // leaving follow mode: stop now
        Serial.printf("Line mode %d -> %d\n", activeMode, want);
        activeMode = want;
    }

    if (activeMode == LINE_CALIBRATE) calibrate();
    computePosition();
    if (activeMode == LINE_FOLLOW) follow();

    static unsigned long lastSend = 0;
    if (millis() - lastSend < LINE_SEND_MS) return;
    lastSend = millis();

    LinePacket p;
    p.id = ID_LINE;
    for (int i = 0; i < 5; i++) p.raw[i] = raw[i];
    p.position = constrain(position, -100, 100);
    p.flags = (seen ? LINE_SEEN : 0) | (usable ? LINE_CALIBRATED : 0) |
              (activeMode == LINE_FOLLOW ? LINE_FOLLOWING : 0) | (lost ? LINE_LOST : 0);
    p.usable = usable;
    radioSend(&p, sizeof(p));
}

#else
void initLine() {}
void handleLineCmdPacket(const uint8_t*, int) {}
void updateLine() {}
#endif
