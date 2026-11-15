#include <Arduino.h>
#include "packets.h"
#include "pins.h"
#include "radio.h"
#include "ultrasonic.h"

#ifdef ULTRASONIC_TRIG_PIN

// one hc-sr04, its echo timed by an interrupt so the loop never waits for it
struct Sonar {
    uint8_t trig, echo;
    volatile uint32_t startUs;
    volatile uint32_t widthUs;
    volatile bool done;
    int16_t mm;  // last result, -1 = nothing in range
};

// [0] left (trig 39, echo 10), [1] right (trig 11, echo 12)
#ifdef ULTRASONIC2_TRIG_PIN
static Sonar sonars[] = {
    {ULTRASONIC2_TRIG_PIN, ULTRASONIC2_ECHO_PIN, 0, 0, false, -1},
    {ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN, 0, 0, false, -1},
};
const int RIGHT = 1;
#else
static Sonar sonars[] = {
    {ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN, 0, 0, false, -1},
};
const int RIGHT = 0;
#endif
const int SONAR_COUNT = sizeof(sonars) / sizeof(sonars[0]);

// the sensors take turns so one never hears the other's ping:
// one ping every 50 ms, each sensor every 100 ms, both results sent to con at 10 Hz
const unsigned long PING_PERIOD_MS = 100 / SONAR_COUNT;
const uint32_t ECHO_MAX_US = 25000;  // ~4.3 m, longer = no object
static int current = 0;              // the sensor pinged last
static bool printing = false;

static void IRAM_ATTR onEcho(void* arg) {
    Sonar* s = (Sonar*)arg;
    uint32_t now = micros();
    if (digitalRead(s->echo)) {
        s->startUs = now;  // rising edge
    } else if (s->startUs != 0) {
        s->widthUs = now - s->startUs;  // falling edge
        s->done = true;
    }
}

static void ping(Sonar& s) {
    s.done = false;
    s.startUs = 0;
    digitalWrite(s.trig, HIGH);  // 10 us pulse on trig
    delayMicroseconds(10);
    digitalWrite(s.trig, LOW);
}

void initUltrasonic() {
    for (int i = 0; i < SONAR_COUNT; i++) {
        pinMode(sonars[i].trig, OUTPUT);
        digitalWrite(sonars[i].trig, LOW);
        pinMode(sonars[i].echo, INPUT);
        attachInterruptArg(digitalPinToInterrupt(sonars[i].echo), onEcho, &sonars[i], CHANGE);
    }
}

void updateUltrasonic() {
    static unsigned long lastPing = 0;
    if (millis() - lastPing < PING_PERIOD_MS) return;
    lastPing = millis();

    // result of the last ping. d = t * v / 2, v = 0.343 mm/us
    Sonar& s = sonars[current];
    s.mm = (s.done && s.widthUs < ECHO_MAX_US) ? s.widthUs * 0.343 / 2 : -1;

    // both sensors read once: send them
    if (current == RIGHT) {
        int16_t left = SONAR_COUNT > 1 ? sonars[0].mm : -1;
        DistancePacket p = {ID_DISTANCE, left, sonars[RIGHT].mm};
        radioSend(&p, sizeof(p));
        if (printing) Serial.printf("left %d mm  right %d mm\n", p.left, p.right);
    }

    current = (current + 1) % SONAR_COUNT;
    ping(sonars[current]);
}

#else
static bool printing = false;
void initUltrasonic() {}
void updateUltrasonic() {}
#endif

void setUltrasonicPrint(bool on) { printing = on; }
bool ultrasonicPrinting() { return printing; }
