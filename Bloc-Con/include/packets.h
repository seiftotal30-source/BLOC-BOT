#ifndef PACKETS_H
#define PACKETS_H

#include <Arduino.h>

// esp-now packets between con (controller) and bloc (robot)
// keep this file the same as include/packets.h in the robot project

const uint8_t ID_DRIVE = 0xB1;     // con -> bloc
const uint8_t ID_SPEED = 0xB2;     // bloc -> con
const uint8_t ID_IMU = 0xB4;       // bloc -> con
const uint8_t ID_DISTANCE = 0xB5;  // bloc -> con

struct __attribute__((packed)) DrivePacket {
  uint8_t id;
  int8_t fwd;   // -100..100, + = forward
  int8_t turn;  // -100..100, + = right
};

struct __attribute__((packed)) SpeedPacket {
  uint8_t id;
  int16_t left;   // mm/s, - = reverse
  int16_t right;  // mm/s
};

struct __attribute__((packed)) ImuPacket {
  uint8_t id;
  int16_t roll;   // deg x10
  int16_t pitch;  // deg x10
  int16_t gx;     // deg/s x10
  int16_t gy;
  int16_t gz;
};

struct __attribute__((packed)) DistancePacket {
  uint8_t id;
  int16_t mm;  // -1 = nothing in range
};

#endif
