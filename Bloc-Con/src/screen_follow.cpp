#include <Arduino.h>
#include "display.h"
#include "packets.h"
#include "radio.h"
#include "screens.h"

// Start: line follower. opens in calibration, FORWARD starts following, BACK stops.
// the mode is sent every 50 ms: bloc stops on its own if this screen goes quiet for 300 ms

const unsigned long SEND_MS = 50;

static uint8_t followMode = LINE_OFF;
static unsigned long lastSend = 0;
static unsigned long openedAt = 0;

// bloc restarts its calibration when it gets LINE_CALIBRATE. its flags only count once a
// state packet from after that has arrived, not an old one from an earlier calibration
static const unsigned long FRESH_AFTER_MS = 300;

void openFollow() {
  followMode = LINE_CALIBRATE;
  lastSend = 0;
  openedAt = millis();
}

void followForward() {
  bool fresh = (long)(robot.lineTime - openedAt) > (long)FRESH_AFTER_MS;
  if (followMode == LINE_CALIBRATE && fresh && (robot.lineFlags & LINE_CALIBRATED)) followMode = LINE_FOLLOW;
}

void stopFollow() {
  followMode = LINE_OFF;
  for (int i = 0; i < 3; i++) sendLineCmd(LINE_OFF);
}

void drawFollow() {
  if (millis() - lastSend >= SEND_MS) {
    lastSend = millis();
    sendLineCmd(followMode);
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  if (!isFresh(robot.lineTime)) {
    u8g2.drawStr(0, 12, "Line follower");
    u8g2.drawStr(0, 30, "Waiting for Bloc...");
    u8g2.sendBuffer();
    return;
  }

  uint8_t f = robot.lineFlags;
  const char* title;
  if (followMode == LINE_CALIBRATE) title = "CALIBRATE";
  else if (f & LINE_LOST) title = "LINE LOST - stopped";
  else if (f & LINE_FOLLOWING) title = "FOLLOWING";
  else title = "OFF";
  u8g2.drawStr(0, 9, title);

  // one box per sensor, X = ignored (its floor/line range was too small during calibration)
  u8g2.setFont(u8g2_font_4x6_tr);
  for (int i = 0; i < 5; i++) {
    int x = 4 + i * 25;
    bool usable = robot.lineUsable & (1 << i);
    u8g2.drawFrame(x, 14, 20, 12);
    if (!usable) {
      u8g2.drawLine(x, 14, x + 19, 25);
      u8g2.drawLine(x + 19, 14, x, 25);
    }
  }

  // line position: a marker on a scale from far left to far right
  u8g2.drawHLine(4, 36, 120);
  u8g2.drawVLine(64, 32, 9);
  if (f & LINE_SEEN) {
    int mx = 64 + robot.linePos * 58 / 100;
    u8g2.drawBox(mx - 2, 32, 5, 9);
  } else {
    u8g2.drawStr(48, 47, "no line");
  }

  if (followMode == LINE_CALIBRATE) {
    u8g2.drawStr(0, 56, "Slide the bar over line");
    bool ready = (f & LINE_CALIBRATED) && (long)(robot.lineTime - openedAt) > (long)FRESH_AFTER_MS;
    u8g2.drawStr(0, 63, ready ? "and floor. FORWARD=go" : "and floor a few times");
  } else {
    u8g2.drawStr(0, 63, "BACK = stop");
  }

  u8g2.sendBuffer();
}
