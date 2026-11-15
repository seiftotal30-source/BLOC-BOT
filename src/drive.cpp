#include "drive.h"
#include "motor.h"
#include "packets.h"

// 204/255 = 80 % duty, about 6.2 V on a 7.7 V pack (tt motors are 3-6 V)
const int MAX_DRIVE_SPEED = 204;

// no command for this long = controller lost (it sends every 50 ms)
const unsigned long LINK_TIMEOUT = 500;

static volatile bool newCommand = false;
static volatile unsigned long lastCommandTime = 0;
static volatile int8_t rxFwd = 0;
static volatile int8_t rxTurn = 0;
static bool linkLedOn = false;

void handleDrivePacket(const uint8_t* data, int len) {
    if (len != sizeof(DrivePacket) || data[0] != ID_DRIVE) return;
    DrivePacket p;
    memcpy(&p, data, sizeof(p));
    rxFwd = p.fwd;
    rxTurn = p.turn;
    lastCommandTime = millis();
    newCommand = true;
}

// left = fwd + turn, right = fwd - turn, then scaled to the pwm cap
void applyDriveCommand() {
    if (!newCommand) return;
    newCommand = false;

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

// onboard rgb led only
void initLinkLed() {
#ifdef RGB_BUILTIN
    neopixelWrite(RGB_BUILTIN, 0, 0, 0);
#endif
}

void updateLinkLed() {
    bool connected = lastCommandTime != 0 && millis() - lastCommandTime < LINK_TIMEOUT;
    if (connected == linkLedOn) return;
    linkLedOn = connected;
#ifdef RGB_BUILTIN
    neopixelWrite(RGB_BUILTIN, 0, connected ? 40 : 0, 0);
#endif
    Serial.println(connected ? "Controller connected" : "Controller lost");
}
