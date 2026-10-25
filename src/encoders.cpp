#include <Arduino.h>
#include "encoders.h"
#include "motor.h"
#include "packets.h"
#include "pins.h"
#include "radio.h"

// 20 slots on the disc = 20 pulses per wheel turn (counted on the real disc)
const float WHEEL_DIAMETER_MM = 65.0;
const int ENCODER_SLOTS = 20;
const float MM_PER_PULSE = PI * WHEEL_DIAMETER_MM / ENCODER_SLOTS;  // ~10.2 mm

const unsigned long SPEED_SEND_MS = 100;
const uint32_t DEBOUNCE_US = 1000;  // ignore edges closer than 1 ms (slot edge chatter)
const uint32_t STOP_US = 300000;    // no pulse for 300 ms = wheel stopped
const float SMOOTHING = 0.4;        // 0..1, higher = faster but noisier

// speed comes from the time between pulses, not the pulse count:
// a 100 ms window only holds a few pulses, so counting made the reading jump by ~100 mm/s

struct Wheel {
    volatile uint32_t pulses = 0;     // pulses in the current window
    volatile uint32_t periodSum = 0;  // sum of pulse periods in the window (us)
    volatile uint32_t lastUs = 0;     // time of the last pulse
    float mmFilt = 0;                 // filtered speed, mm/s
};

static Wheel leftWheel, rightWheel;
static portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR onPulse(Wheel& w) {
    uint32_t now = micros();
    uint32_t period = now - w.lastUs;
    if (period < DEBOUNCE_US) return;
    w.lastUs = now;
    portENTER_CRITICAL_ISR(&encoderMux);
    if (period < STOP_US) {  // skip the first pulse after a stop
        w.pulses++;
        w.periodSum += period;
    }
    portEXIT_CRITICAL_ISR(&encoderMux);
}

static void IRAM_ATTR onLeftPulse() { onPulse(leftWheel); }
static void IRAM_ATTR onRightPulse() { onPulse(rightWheel); }

// mm/s from the mean period in this window, then smoothed.
// no pulse in the window: the time since the last one gives an upper bound, so it falls to 0 smoothly
static float wheelSpeed(uint32_t pulses, uint32_t periodSum, uint32_t lastUs, float prev) {
    uint32_t since = micros() - lastUs;
    float mm;
    if (pulses > 0) mm = pulses * MM_PER_PULSE * 1e6 / periodSum;
    else if (since >= STOP_US) mm = 0;
    else mm = min(prev, MM_PER_PULSE * 1e6f / since);
    return prev + SMOOTHING * (mm - prev);
}

void initEncoders() {
    pinMode(ENCODER_LEFT_PIN, INPUT_PULLUP);
    pinMode(ENCODER_RIGHT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_PIN), onLeftPulse, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT_PIN), onRightPulse, RISING);
}

void sendSpeed() {
    static unsigned long lastSend = 0;
    if (millis() - lastSend < SPEED_SEND_MS) return;
    lastSend = millis();

    // copy and reset both windows in one go
    portENTER_CRITICAL(&encoderMux);
    uint32_t l = leftWheel.pulses, r = rightWheel.pulses;
    uint32_t lSum = leftWheel.periodSum, rSum = rightWheel.periodSum;
    uint32_t lLast = leftWheel.lastUs, rLast = rightWheel.lastUs;
    leftWheel.pulses = rightWheel.pulses = 0;
    leftWheel.periodSum = rightWheel.periodSum = 0;
    portEXIT_CRITICAL(&encoderMux);

    leftWheel.mmFilt = wheelSpeed(l, lSum, lLast, leftWheel.mmFilt);
    rightWheel.mmFilt = wheelSpeed(r, rSum, rLast, rightWheel.mmFilt);

    // one-channel sensor: direction comes from the motor command
    int leftMm = (int)leftWheel.mmFilt;
    int rightMm = (int)rightWheel.mmFilt;
    if (getLeftSpeed() < 0) leftMm = -leftMm;
    if (getRightSpeed() < 0) rightMm = -rightMm;

    SpeedPacket p = {ID_SPEED, (int16_t)leftMm, (int16_t)rightMm};
    radioSend(&p, sizeof(p));
}
