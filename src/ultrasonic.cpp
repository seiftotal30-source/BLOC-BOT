#include <Arduino.h>
#include "packets.h"
#include "pins.h"
#include "radio.h"
#include "ultrasonic.h"

#ifdef ULTRASONIC_TRIG_PIN

const unsigned long PING_PERIOD_MS = 100;  // datasheet wants at least 60 ms
const uint32_t ECHO_MAX_US = 25000;        // ~4.3 m, longer = no object

// the echo is timed by an interrupt so the loop never waits for it
static volatile uint32_t echoStartUs = 0;
static volatile uint32_t echoWidthUs = 0;
static volatile bool echoDone = false;

static void IRAM_ATTR onEcho() {
    uint32_t now = micros();
    if (digitalRead(ULTRASONIC_ECHO_PIN)) {
        echoStartUs = now;  // rising edge
    } else if (echoStartUs != 0) {
        echoWidthUs = now - echoStartUs;  // falling edge
        echoDone = true;
    }
}

void initUltrasonic() {
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ULTRASONIC_ECHO_PIN), onEcho, CHANGE);
}

void updateUltrasonic() {
    static unsigned long lastPing = 0;
    if (millis() - lastPing < PING_PERIOD_MS) return;
    lastPing = millis();

    // d = t * v / 2, v = 0.343 mm/us
    int16_t mm = -1;
    if (echoDone && echoWidthUs < ECHO_MAX_US) mm = echoWidthUs * 0.343 / 2;
    DistancePacket p = {ID_DISTANCE, mm};
    radioSend(&p, sizeof(p));

    // next ping: 10 us pulse on trig
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
