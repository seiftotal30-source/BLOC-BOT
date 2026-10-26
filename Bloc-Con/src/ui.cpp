#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "screens.h"

Screen currentScreen = MENU_SCREEN;

static const char* menu[] = {"Start", "Settings", "Ultrasonic", "MPU6050", "Line", "Drive", "About", "Reset"};
static const int MENU_SIZE = 8;
static const int MENU_ROWS = 6;  // rows that fit on 64 px, the list scrolls past this
static int selected = 0;

// a press is ignored for 180 ms after the last one (bounce, held button)
static const unsigned long DEBOUNCE_MS = 180;
static unsigned long lastButtonTime = 0;

static bool pressed(int pin) {
  return digitalRead(pin) == LOW;  // buttons pull to gnd
}

void initButtons() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_FORWARD, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
}

static void openSelected() {
  switch (selected) {
    case 0:  // start: line follower
      currentScreen = FOLLOW_SCREEN;
      openFollow();
      break;
    case 1:
      currentScreen = SETTINGS_SCREEN;
      break;
    case 2:
      currentScreen = ULTRASONIC_SCREEN;
      break;
    case 3:
      currentScreen = MPU_SCREEN;
      resetMpuFilter();
      break;
    case 4:
      currentScreen = LINE_SCREEN;
      break;
    case 5:
      currentScreen = DRIVE_SCREEN;
      break;
    case 6:
      currentScreen = ABOUT_SCREEN;
      break;
    case 7:
      ESP.restart();
      break;
  }
}

void handleButtons() {
  if (millis() - lastButtonTime < DEBOUNCE_MS) return;

  if (currentScreen == MENU_SCREEN) {
    if (pressed(BTN_UP)) {
      selected = (selected + MENU_SIZE - 1) % MENU_SIZE;
      lastButtonTime = millis();
    }
    if (pressed(BTN_DOWN)) {
      selected = (selected + 1) % MENU_SIZE;
      lastButtonTime = millis();
    }
    if (pressed(BTN_FORWARD)) {
      openSelected();
      lastButtonTime = millis();
      return;  // the same press must not also act on the screen it just opened
    }
  }

  if (currentScreen == FOLLOW_SCREEN && pressed(BTN_FORWARD)) {
    followForward();
    lastButtonTime = millis();
  }

  // back always returns to the menu
  if (pressed(BTN_BACK)) {
    if (currentScreen == DRIVE_SCREEN) stopRobot();
    if (currentScreen == FOLLOW_SCREEN) {
      stopFollow();
      stopRobot();
    }
    currentScreen = MENU_SCREEN;
    lastButtonTime = millis();
  }
}

// "BLOC-BOT" typed letter by letter, then underlined
void showSplash() {
  const char* title = "BLOC-BOT";
  const int len = strlen(title);

  // biggest bold font that fits 128 px
  const uint8_t* fonts[] = {u8g2_font_fub25_tr, u8g2_font_fub20_tr, u8g2_font_fub17_tr, u8g2_font_fub14_tr};
  const uint8_t* font = fonts[3];
  for (int i = 0; i < 4; i++) {
    u8g2.setFont(fonts[i]);
    if (u8g2.getStrWidth(title) <= 124) {
      font = fonts[i];
      break;
    }
  }

  u8g2.setFont(font);
  const int fullWidth = u8g2.getStrWidth(title);
  const int x0 = (128 - fullWidth) / 2;
  const int baseline = 38;

  char shown[16];
  for (int n = 1; n <= len; n++) {
    memcpy(shown, title, n);
    shown[n] = '\0';
    u8g2.clearBuffer();
    u8g2.setFont(font);
    u8g2.drawStr(x0, baseline, shown);
    u8g2.sendBuffer();
    delay(110);
  }

  for (int w = 0; w <= fullWidth; w += 6) {
    u8g2.clearBuffer();
    u8g2.setFont(font);
    u8g2.drawStr(x0, baseline, title);
    u8g2.drawBox(x0, baseline + 6, w, 2);
    u8g2.sendBuffer();
    delay(12);
  }

  u8g2.setFont(u8g2_font_5x8_tr);
  const char* tag = "CONTROLLER";
  u8g2.drawStr((128 - u8g2.getStrWidth(tag)) / 2, 58, tag);
  u8g2.sendBuffer();
  delay(1200);
}

void drawMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);

  int first = max(selected - (MENU_ROWS - 1), 0);
  for (int i = first; i < MENU_SIZE && i < first + MENU_ROWS; i++) {
    int y = 10 + (i - first) * 10;
    if (i == selected) {  // selected row: white bar, black text
      u8g2.drawBox(0, y - 9, 128, 11);
      u8g2.setDrawColor(0);
      u8g2.drawStr(5, y, menu[i]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(5, y, menu[i]);
    }
  }
  u8g2.sendBuffer();
}

static void drawTextScreen(const char* a, const char* b, const char* c, int yA, int yB, int yC) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(10, yA, a);
  u8g2.drawStr(10, yB, b);
  u8g2.drawStr(10, yC, c);
  u8g2.sendBuffer();
}

void drawAbout() { drawTextScreen("BlocBot", "ESP32 Robot UI", "Press BACK", 20, 35, 50); }
void drawSettings() { drawTextScreen("Settings", "Coming Soon", "Press BACK", 20, 40, 55); }
