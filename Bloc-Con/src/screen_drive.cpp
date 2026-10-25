#include <Arduino.h>
#include "display.h"
#include "joystick.h"
#include "radio.h"
#include "screens.h"

const unsigned long SEND_MS = 50;  // the robot stops after 300 ms without a command

// joystick value that gives full speed. the end of the travel is about 2048 / 20 = 102,
// so 90 leaves a small margin and uses almost the whole stick (it was 30 before)
const int JOY_FULL = 90;

static unsigned long lastSend = 0;

void stopRobot() {
  for (int i = 0; i < 3; i++) sendDrive(0, 0);
}

void drawDriveScreen() {
  int joyX, joyY;
  readJoystick(joyX, joyY);

  // JOY_FULL = full command (100), clipped in sendDrive. stick up = forward, right = turn right
  if (millis() - lastSend >= SEND_MS) {
    lastSend = millis();
    sendDrive(-joyY * 100 / JOY_FULL, joyX * 100 / JOY_FULL);
  }

  // circle with a dot that follows the stick, edge = full speed
  const int cx = 64, cy = 32;
  const float maxRadius = 30.0;
  float fx = joyX * maxRadius / JOY_FULL, fy = joyY * maxRadius / JOY_FULL;
  float mag = sqrt(fx * fx + fy * fy);
  if (mag > maxRadius) {
    fx = fx / mag * maxRadius;
    fy = fy / mag * maxRadius;
  }

  u8g2.clearBuffer();
  u8g2.drawCircle(cx, cy, 30);
  u8g2.drawDisc(cx, cy, 2);
  u8g2.drawDisc(cx + fx, cy + fy, 4);

  char buf[32];
  u8g2.setFont(u8g2_font_5x8_tr);
  sprintf(buf, "X:%d Y:%d", joyX, joyY);
  u8g2.drawStr(0, 62, buf);

  // robot speed: mean of both wheels, mm/s -> km/h
  u8g2.setFont(u8g2_font_6x12_tr);
  if (isFresh(robot.speedTime)) {
    float kmh = (robot.speedLeft + robot.speedRight) / 2.0 * 0.0036;
    sprintf(buf, "%.1f", kmh);
    u8g2.drawStr(98, 12, buf);
  } else {
    u8g2.drawStr(98, 12, "--");
  }
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(98, 22, "km/h");

  u8g2.sendBuffer();
}
