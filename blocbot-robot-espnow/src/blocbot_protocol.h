// Shared BLOC-BOT radio protocol. Keep this file identical in
// firmware/blocbot_robot and firmware/blocbot_controller.
#pragma once
#include <stdint.h>

#define BLOCBOT_PROTO_VERSION 1
#define BLOCBOT_WIFI_CHANNEL  1   // both boards must use the same channel

enum : uint8_t { MSG_DRIVE = 1, MSG_TELEMETRY = 2 };

typedef struct __attribute__((packed)) {
  uint8_t  version;      // BLOCBOT_PROTO_VERSION
  uint8_t  type;         // MSG_DRIVE
  uint16_t seq;          // increments every packet, wraps
  int16_t  motor_left;   // -255..+255
  int16_t  motor_right;  // -255..+255
  uint8_t  mode;         // 0 = Learner, 1 = Researcher
  uint8_t  buttons;      // bitfield, b0..b5
  uint8_t  estop;        // 1 = stop immediately
  uint8_t  reserved;
} DriveCmd;              // 12 bytes
