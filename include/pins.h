#ifndef PINS_H
#define PINS_H

// the robot only runs on the esp32-s3
#if defined(ARDUINO_ESP32S3_DEV)
#include "pins_esp32s3.h"
#else
#error "BLOC-BOT robot pins are only defined for the ESP32-S3"
#endif

#endif
