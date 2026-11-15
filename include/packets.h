#ifndef PACKETS_H
#define PACKETS_H

#include <Arduino.h>

// esp-now packets between con (controller) and bloc (robot)
// keep this file the same as Bloc-Con/include/packets.h

const uint8_t ID_DRIVE = 0xB1;     // con -> bloc
const uint8_t ID_SPEED = 0xB2;     // bloc -> con
const uint8_t ID_IMU = 0xB4;       // bloc -> con
const uint8_t ID_DISTANCE = 0xB5;  // bloc -> con
const uint8_t ID_LINE = 0xB6;      // bloc -> con
const uint8_t ID_LINE_CMD = 0xB7;  // con -> bloc

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

// line sensor state flags (LinePacket.flags)
const uint8_t LINE_SEEN = 0x01;        // the line is under at least one sensor
const uint8_t LINE_CALIBRATED = 0x02;  // at least one sensor has a usable floor/line range
const uint8_t LINE_FOLLOWING = 0x04;   // bloc is steering along the line
const uint8_t LINE_LOST = 0x08;        // following, but the line was lost for too long: stopped

struct __attribute__((packed)) LinePacket {
    uint8_t id;
    uint16_t raw[5];  // analog 0..4095, sensor 1 (left) .. 5 (right)
    int8_t position;  // -100 (line far left) .. 100 (far right), valid with LINE_SEEN
    uint8_t flags;    // LINE_*
    uint8_t usable;   // bit i set = sensor i+1 changed enough during calibration
};

// line follower mode, sent by con every 50 ms while its Start screen is open.
// bloc drops back to LINE_OFF if none arrives for 300 ms
const uint8_t LINE_OFF = 0;
const uint8_t LINE_CALIBRATE = 1;  // stand still, learn each sensor's floor and line values
const uint8_t LINE_FOLLOW = 2;

struct __attribute__((packed)) LineCmdPacket {
    uint8_t id;
    uint8_t mode;  // LINE_OFF, LINE_CALIBRATE, LINE_FOLLOW
};

#endif
