#include <Arduino.h>
#include "config.h"
#include "joystick.h"

static int centerX = 2048;
static int centerY = 2048;

void calibrateJoystick() {
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  Serial.println("Calibrating joystick, do not touch it");
  long sumX = 0, sumY = 0;
  for (int i = 0; i < 50; i++) {
    sumX += analogRead(JOY_X);
    sumY += analogRead(JOY_Y);
    delay(5);
  }
  centerX = sumX / 50;
  centerY = sumY / 50;
  Serial.printf("Joystick center X: %d  Y: %d\n", centerX, centerY);
}

void readJoystick(int& x, int& y) {
  // average 8 readings, the esp32 adc is noisy
  long totalX = 0, totalY = 0;
  for (int i = 0; i < 8; i++) {
    totalX += analogRead(JOY_X);
    totalY += analogRead(JOY_Y);
  }
  x = (totalX / 8 - centerX) / 20;
  y = (centerY - totalY / 8) / 20;

  // dead zone: small offsets at rest count as 0
  if (abs(x) < 4) x = 0;
  if (abs(y) < 4) y = 0;
}
