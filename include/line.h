#ifndef LINE_H
#define LINE_H

#include <Arduino.h>

// 5-channel line sensor bar under the front (gpio 2, 4, 5, 6, 7) and the line follower
void initLine();
void handleLineCmdPacket(const uint8_t* data, int len);  // radio callback: mode from the controller
void updateLine();  // every loop: read the sensors, calibrate or steer, send the state every 50 ms

#endif
