#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>

// esp-now in broadcast mode, no pairing

// data received from the robot, with the time it arrived (0 = never)
struct RobotData {
  volatile int16_t speedLeft = 0, speedRight = 0;  // mm/s
  volatile unsigned long speedTime = 0;
  volatile int16_t distMm = -1;  // -1 = nothing in range
  volatile unsigned long distTime = 0;
  volatile float roll = 0, pitch = 0, gx = 0, gy = 0, gz = 0;  // deg, deg/s
  volatile unsigned long imuTime = 0;
  volatile uint16_t line[5] = {0, 0, 0, 0, 0};  // line sensors, analog 0..4095, left to right
  volatile int8_t linePos = 0;     // -100 (line far left) .. 100 (far right)
  volatile uint8_t lineFlags = 0;  // LINE_* in packets.h
  volatile uint8_t lineUsable = 0; // bit i = sensor i+1 usable after calibration
  volatile unsigned long lineTime = 0;
};

extern RobotData robot;

// older than this = the robot is not sending
const unsigned long DATA_TIMEOUT = 500;

bool isFresh(unsigned long t);  // true if t is set and less than DATA_TIMEOUT old
void initRadio();
void sendDrive(int fwd, int turn);  // -100..100 each
void sendLineCmd(uint8_t mode);     // LINE_OFF, LINE_CALIBRATE, LINE_FOLLOW

#endif
