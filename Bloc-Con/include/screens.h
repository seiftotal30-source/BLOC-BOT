#ifndef SCREENS_H
#define SCREENS_H

// one screen is shown at a time, the menu opens the others

enum Screen {
  MENU_SCREEN,
  ULTRASONIC_SCREEN,
  MPU_SCREEN,
  LINE_SCREEN,
  DRIVE_SCREEN,
  ABOUT_SCREEN,
  SETTINGS_SCREEN
};

extern Screen currentScreen;

// ui.cpp
void showSplash();
void initButtons();
void handleButtons();
void drawMenu();
void drawAbout();
void drawSettings();

// screen_drive.cpp
void drawDriveScreen();
void stopRobot();  // sends a few stop commands

// screen_ultrasonic.cpp
void initLocalUltrasonic();
void drawUltrasonic();

// screen_mpu.cpp
void initLocalMpu();
void resetMpuFilter();
void drawMpu();

// screen_line.cpp
void drawLine();

#endif
