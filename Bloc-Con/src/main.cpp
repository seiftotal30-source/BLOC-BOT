#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <esp_now.h>

// =====================================================
// OLED
// =====================================================

// 0 = 0.96" SSD1306 (current test screen), 1 = 2.42" SSD1309 (final screen)
#define USE_SSD1309 0

#if USE_SSD1309
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(
#else
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
#endif
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================================
// ULTRASONIC
// =====================================================

#define TRIG 5
#define ECHO 18

// =====================================================
// BUTTONS
// =====================================================

#define BTN_UP       12
#define BTN_DOWN     13
#define BTN_FORWARD  14
#define BTN_BACK     27

// =====================================================
// JOYSTICK
// =====================================================

#define JOY_X 34
#define JOY_Y 35

int centerX = 2048;
int centerY = 2048;

// =====================================================
// MENU
// =====================================================

const char* menu[] = {
  "Start",
  "Settings",
  "Ultrasonic",
  "Drive",
  "About",
  "Reset"
};

const int menuSize = 6;
int selected = 0;

// =====================================================
// SCREENS
// =====================================================

enum ScreenState {
  MENU_SCREEN,
  ULTRASONIC_SCREEN,
  DRIVE_SCREEN,
  ABOUT_SCREEN,
  SETTINGS_SCREEN
};

ScreenState currentScreen = MENU_SCREEN;

// =====================================================
// BUTTON DEBOUNCE
// =====================================================

unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 180;

// =====================================================
// RADAR
// =====================================================

int sweepAngle = 0;
bool sweepForward = true;

// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

void showSplash();
void drawMenu();
void drawUltrasonic();
void drawDriveScreen();
void handleButtons();
float getDistance();
bool buttonPressed(int pin);

// =====================================================
// ESP-NOW (controller -> robot, broadcast)
// =====================================================

struct DrivePacket {
  uint8_t magic;  // 0xB1
  int8_t fwd;     // -100..100 (positive = forward)
  int8_t turn;    // -100..100 (positive = right)
};

// robot -> controller: wheel speed from the LM393 encoders
struct __attribute__((packed)) SpeedPacket {
  uint8_t magic;  // 0xB2
  int16_t left;   // mm/s (negative = reverse)
  int16_t right;  // mm/s (negative = reverse)
};

uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned long lastSendTime = 0;
bool wasDriving = false;

volatile int16_t speedLeft = 0;
volatile int16_t speedRight = 0;
volatile unsigned long lastSpeedTime = 0;
const unsigned long speedTimeout = 500;  // ms without a packet = no feedback

void onReceive(const uint8_t* mac, const uint8_t* data, int len) {
  if (len != sizeof(SpeedPacket)) return;
  SpeedPacket p;
  memcpy(&p, data, sizeof(p));
  if (p.magic != 0xB2) return;
  speedLeft = p.left;
  speedRight = p.right;
  lastSpeedTime = millis();
}

void initRadio() {
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, broadcastMac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

void sendDrive(int fwd, int turn) {
  DrivePacket p;
  p.magic = 0xB1;
  p.fwd = constrain(fwd, -100, 100);
  p.turn = constrain(turn, -100, 100);
  esp_now_send(broadcastMac, (uint8_t*)&p, sizeof(p));
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  u8g2.begin();
  showSplash();
  initRadio();

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_FORWARD, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  // Joystick calibration
  long sumX = 0;
  long sumY = 0;

  Serial.println("Calibrating joystick...");
  Serial.println("DO NOT TOUCH THE JOYSTICK");

  for (int i = 0; i < 50; i++) {
    sumX += analogRead(JOY_X);
    sumY += analogRead(JOY_Y);
    delay(5);
  }

  centerX = sumX / 50;
  centerY = sumY / 50;

  Serial.print("Joystick center X: ");
  Serial.println(centerX);

  Serial.print("Joystick center Y: ");
  Serial.println(centerY);

  Serial.println("Ready!");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {
  handleButtons();

  switch (currentScreen) {

    case MENU_SCREEN:
      drawMenu();
      break;

    case ULTRASONIC_SCREEN:
      drawUltrasonic();
      break;

    case DRIVE_SCREEN:
      drawDriveScreen();
      break;

    case ABOUT_SCREEN:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tr);
      u8g2.drawStr(10, 20, "BlocBot");
      u8g2.drawStr(10, 35, "ESP32 Robot UI");
      u8g2.drawStr(10, 50, "Press BACK");
      u8g2.sendBuffer();
      break;

    case SETTINGS_SCREEN:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tr);
      u8g2.drawStr(10, 20, "Settings");
      u8g2.drawStr(10, 40, "Coming Soon");
      u8g2.drawStr(10, 55, "Press BACK");
      u8g2.sendBuffer();
      break;
  }

  delay(20);
}

// =====================================================
// BUTTON HANDLER
// =====================================================

void handleButtons() {
  if (millis() - lastButtonTime < debounceDelay)
    return;

  if (currentScreen == MENU_SCREEN) {

    if (buttonPressed(BTN_UP)) {
      selected--;
      if (selected < 0)
        selected = menuSize - 1;
      lastButtonTime = millis();
    }

    if (buttonPressed(BTN_DOWN)) {
      selected++;
      if (selected >= menuSize)
        selected = 0;
      lastButtonTime = millis();
    }

    if (buttonPressed(BTN_FORWARD)) {

      switch (selected) {

        case 0:
          // Start
          break;

        case 1:
          currentScreen = SETTINGS_SCREEN;
          break;

        case 2:
          currentScreen = ULTRASONIC_SCREEN;
          break;

        case 3:
          currentScreen = DRIVE_SCREEN;
          break;

        case 4:
          currentScreen = ABOUT_SCREEN;
          break;

        case 5:
          ESP.restart();
          break;
      }

      lastButtonTime = millis();
    }
  }

  if (buttonPressed(BTN_BACK)) {
    if (currentScreen == DRIVE_SCREEN) {
      for (int i = 0; i < 3; i++) sendDrive(0, 0);
      wasDriving = false;
    }
    currentScreen = MENU_SCREEN;
    lastButtonTime = millis();
  }
}

// =====================================================
// BUTTON READ
// =====================================================

bool buttonPressed(int pin) {
  return digitalRead(pin) == LOW;
}

// =====================================================
// ULTRASONIC DISTANCE
// =====================================================

float getDistance() {

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(
    ECHO,
    HIGH,
    30000
  );

  if (duration == 0)
    return -1;

  float distance = duration * 0.0343 / 2.0;

  return distance;
}

// =====================================================
// SPLASH SCREEN
// =====================================================

void showSplash() {

  const char* title = "BLOC-BOT";
  const int len = strlen(title);

  // Biggest bold font that fits the 128 px width
  const uint8_t* fonts[] = {
    u8g2_font_fub25_tr,
    u8g2_font_fub20_tr,
    u8g2_font_fub17_tr,
    u8g2_font_fub14_tr
  };

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

  // Type the letters one by one
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

  // Underline sweeps across, then the tagline appears
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

// =====================================================
// MENU DISPLAY
// =====================================================

void drawMenu() {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  for (int i = 0; i < menuSize; i++) {

    int y = 10 + i * 10;

    if (i == selected) {

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

// =====================================================
// ULTRASONIC RADAR SCREEN
// =====================================================

void drawUltrasonic() {

  float distance = getDistance();

  int cx = 64;
  int cy = 63;

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_5x8_tr);

  if (distance > 0) {

    char buffer[30];

    sprintf(
      buffer,
      "Distance: %.1f cm",
      distance
    );

    u8g2.drawStr(0, 8, buffer);

  } else {

    u8g2.drawStr(0, 8, "No Object");
  }

  u8g2.drawCircle(cx, cy, 15);
  u8g2.drawCircle(cx, cy, 30);
  u8g2.drawCircle(cx, cy, 45);
  u8g2.drawCircle(cx, cy, 60);

  float rad = sweepAngle * PI / 180.0;

  int sx = cx + cos(rad) * 60;
  int sy = cy - sin(rad) * 60;

  u8g2.drawLine(cx, cy, sx, sy);

  if (distance > 0 && distance <= 120) {

    int radius = map(
      (int)distance,
      0,
      120,
      0,
      60
    );

    int objectX = cx + cos(rad) * radius;
    int objectY = cy - sin(rad) * radius;

    u8g2.drawDisc(objectX, objectY, 3);
  }

  u8g2.sendBuffer();

  if (sweepForward)
    sweepAngle += 4;
  else
    sweepAngle -= 4;

  if (sweepAngle >= 180)
    sweepForward = false;

  if (sweepAngle <= 0)
    sweepForward = true;
}

// =====================================================
// DRIVE SCREEN
// =====================================================

void drawDriveScreen() {

  long totalX = 0;
  long totalY = 0;
  for (int i = 0; i < 8; i++) {
    totalX += analogRead(JOY_X);
    totalY += analogRead(JOY_Y);
  }

  int rawX = totalX / 8;
  int rawY = totalY / 8;

  int joyX = rawX - centerX;
  int joyY = centerY - rawY;

  joyX = joyX / 20;
  joyY = joyY / 20;

  if (abs(joyX) < 4)
    joyX = 0;

  if (abs(joyY) < 4)
    joyY = 0;

  // Send to robot every 50 ms (robot stops after 300 ms of silence)
  if (millis() - lastSendTime >= 50) {
    lastSendTime = millis();
    sendDrive(-joyY * 100 / 30, joyX * 100 / 30);
  }

  int cx = 64;
  int cy = 32;

  const float maxRadius = 30.0;

  float fx = joyX;
  float fy = joyY;

  float magnitude = sqrt(fx * fx + fy * fy);

  if (magnitude > maxRadius) {
    fx = (fx / magnitude) * maxRadius;
    fy = (fy / magnitude) * maxRadius;
  }

  int dotX = cx + fx;
  int dotY = cy + fy;

  u8g2.clearBuffer();

  u8g2.drawCircle(cx, cy, 30);
  u8g2.drawDisc(cx, cy, 2);
  u8g2.drawDisc(dotX, dotY, 4);

  u8g2.setFont(u8g2_font_5x8_tr);

  char buffer[32];

  sprintf(
    buffer,
    "X:%d Y:%d",
    joyX,
    joyY
  );

  u8g2.drawStr(0, 62, buffer);

  // Robot speed feedback in km/h (average of both wheels, mm/s -> km/h)
  u8g2.setFont(u8g2_font_6x12_tr);
  if (millis() - lastSpeedTime < speedTimeout) {
    float kmh = (speedLeft + speedRight) / 2.0 * 0.0036;
    sprintf(buffer, "%.1f", kmh);
    u8g2.drawStr(98, 12, buffer);
  } else {
    u8g2.drawStr(98, 12, "--");
  }
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(98, 22, "km/h");

  u8g2.sendBuffer();
}
