// BLOC-BOT controller (con)
// esp32 devkit, ky-023 joystick, 4 buttons, 128x64 oled
// sends drive commands to the robot (bloc) over esp-now and shows what it sends back

#include <Arduino.h>
#include "display.h"
#include "joystick.h"
#include "radio.h"
#include "screens.h"

void setup() {
  Serial.begin(115200);

  u8g2.begin();  // also starts the i2c bus
  showSplash();
  initRadio();
  initLocalMpu();  // keep the controller still during start-up
  initLocalUltrasonic();
  initButtons();
  calibrateJoystick();  // do not touch the stick here

  Serial.println("Ready!");
}

void loop() {
  handleButtons();

  switch (currentScreen) {
    case MENU_SCREEN:       drawMenu(); break;
    case ULTRASONIC_SCREEN: drawUltrasonic(); break;
    case MPU_SCREEN:        drawMpu(); break;
    case DRIVE_SCREEN:      drawDriveScreen(); break;
    case ABOUT_SCREEN:      drawAbout(); break;
    case SETTINGS_SCREEN:   drawSettings(); break;
  }

  delay(20);
}
