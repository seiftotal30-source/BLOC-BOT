#include <Arduino.h>
#include "line.h"
#include "packets.h"
#include "pins.h"
#include "radio.h"

#ifdef LINE_PIN_1

const unsigned long LINE_SEND_MS = 50;  // 20 Hz
const int LINE_SAMPLES = 4;             // averaged per reading, the outputs are a bit noisy

static const uint8_t linePins[5] = {LINE_PIN_1, LINE_PIN_2, LINE_PIN_3, LINE_PIN_4, LINE_PIN_5};

void initLine() {
    for (int i = 0; i < 5; i++) pinMode(linePins[i], INPUT);
    analogReadResolution(12);  // 0..4095
}

void updateLine() {
    static unsigned long lastSend = 0;
    if (millis() - lastSend < LINE_SEND_MS) return;
    lastSend = millis();

    LinePacket p;
    p.id = ID_LINE;
    for (int i = 0; i < 5; i++) {
        uint32_t sum = 0;
        for (int s = 0; s < LINE_SAMPLES; s++) sum += analogRead(linePins[i]);
        p.raw[i] = sum / LINE_SAMPLES;
    }
    radioSend(&p, sizeof(p));
}

#else
void initLine() {}
void updateLine() {}
#endif
