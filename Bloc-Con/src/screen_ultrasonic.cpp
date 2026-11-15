#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "radio.h"
#include "screens.h"

// the sweep is only an animation, the sensor does not turn
static int sweepAngle = 0;
static bool sweepForward = true;

void initLocalUltrasonic() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

// hc-sr04 wired to the controller (fallback), cm or -1
static float localDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);  // us, 0 = timeout
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;  // d = t * v / 2
}

// "L 34.5" style label, or "L --" when nothing is in range
static void drawReading(int x, const char* side, float cm) {
  char buf[16];
  if (cm > 0) sprintf(buf, "%s %.1f", side, cm);
  else sprintf(buf, "%s --", side);
  u8g2.drawStr(x, 8, buf);
}

// a dot on the radar for one reading, at a fixed angle (90 = straight ahead)
static void drawBlip(int cx, int cy, int angle, float cm) {
  if (cm <= 0 || cm > 120) return;
  float rad = angle * PI / 180.0;
  int radius = map((int)cm, 0, 120, 0, 60);
  u8g2.drawDisc(cx + cos(rad) * radius, cy - sin(rad) * radius, 3);
}

void drawUltrasonic() {
  // the robot's two front sensors when its packets arrive, else the local one
  bool fromBloc = isFresh(robot.distTime);
  float left = -1, right = -1, local = -1;
  if (fromBloc) {
    left = robot.distLeftMm < 0 ? -1 : robot.distLeftMm / 10.0;
    right = robot.distRightMm < 0 ? -1 : robot.distRightMm / 10.0;
  } else {
    local = localDistance();
  }
  const char* source = fromBloc ? "Bloc" : "Con";

  const int cx = 64, cy = 63;
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(128 - u8g2.getStrWidth(source), 6, source);

  u8g2.setFont(u8g2_font_5x8_tr);
  if (fromBloc) {
    drawReading(0, "L", left);
    drawReading(52, "R", right);
  } else if (local > 0) {
    char buf[30];
    sprintf(buf, "Distance: %.1f cm", local);
    u8g2.drawStr(0, 8, buf);
  } else {
    u8g2.drawStr(0, 8, "No Object");
  }

  // radar rings, 60 px = 120 cm
  for (int r = 15; r <= 60; r += 15) u8g2.drawCircle(cx, cy, r);

  float rad = sweepAngle * PI / 180.0;
  u8g2.drawLine(cx, cy, cx + cos(rad) * 60, cy - sin(rad) * 60);

  // the sensors are fixed: left blip left of centre, right blip right of it
  if (fromBloc) {
    drawBlip(cx, cy, 110, left);
    drawBlip(cx, cy, 70, right);
  } else {
    drawBlip(cx, cy, 90, local);
  }
  u8g2.sendBuffer();

  // sweep back and forth over 0-180 deg
  sweepAngle += sweepForward ? 4 : -4;
  if (sweepAngle >= 180) sweepForward = false;
  if (sweepAngle <= 0) sweepForward = true;
}
