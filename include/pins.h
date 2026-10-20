#ifndef PINS_H
#define PINS_H

// =====================================================
// BLOC-BOT PIN DEFINITIONS
// =====================================================

// Select the appropriate pin definitions based on the board
#ifdef ARDUINO_ESP32_DEV
    // ESP32-CAM has very limited available GPIOs
    // Many pins are reserved for camera, SD card, and boot
    #include "pins_esp32cam.h"
#elif defined(ARDUINO_ESP32S3_DEV)
    // ESP32-S3 has more GPIOs available
    #include "pins_esp32s3.h"
#else
    #error "Unknown board! Define pins for your ESP32 board."
#endif

#endif // PINS_H
