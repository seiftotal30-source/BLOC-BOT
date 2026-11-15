#ifndef DRIVE_H
#define DRIVE_H

#include <Arduino.h>

// drive commands from the controller

void handleDrivePacket(const uint8_t* data, int len);  // radio callback
void applyDriveCommand();  // mix the last command into the motors
void initLinkLed();
void updateLinkLed();      // green while commands arrive

#endif
