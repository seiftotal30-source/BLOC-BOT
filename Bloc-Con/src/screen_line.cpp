#include <Arduino.h>
#include "display.h"
#include "radio.h"
#include "screens.h"

// 5 bars, left to right as seen from behind the robot. taller bar = higher analog value.
// no threshold yet: this screen is for seeing what the bar reads over the floor and the tape
void drawLine() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tr);

  if (!isFresh(robot.lineTime)) {
    u8g2.drawStr(0, 12, "No line sensor data");
    u8g2.drawStr(0, 30, "Waiting for Bloc...");
    u8g2.drawStr(0, 46, "Sensors on GPIO");
    u8g2.drawStr(0, 58, "2, 4, 5, 6, 7");
    u8g2.sendBuffer();
    return;
  }

  u8g2.drawStr(0, 7, "Line sensors");

  const int barW = 20, gap = 5, x0 = 2;
  const int top = 10, bottom = 54;  // bar area, 44 px = 4095
  char num[6];
  u8g2.setFont(u8g2_font_4x6_tr);
  for (int i = 0; i < 5; i++) {
    int x = x0 + i * (barW + gap);
    int h = robot.line[i] * (bottom - top) / 4095;
    u8g2.drawFrame(x, top, barW, bottom - top);
    u8g2.drawBox(x, bottom - h, barW, h);
    snprintf(num, sizeof(num), "%u", robot.line[i]);
    u8g2.drawStr(x + (barW - u8g2.getStrWidth(num)) / 2, 62, num);
  }

  u8g2.sendBuffer();
}
