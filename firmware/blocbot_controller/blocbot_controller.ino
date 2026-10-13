// Bloc-Con (ESP32 DevKit V1): reads the joystick and drives BLOC-BOT over ESP-NOW.
// Joystick (KY-023): VRx=34, VRy=35, SW=32 (pressed = stop the robot).
// Sends to the broadcast address, so no MAC address needs to be typed in.
// Keep the joystick centred while the controller powers up: that is the zero point.

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "blocbot_protocol.h"

const int JOY_X  = 34;
const int JOY_Y  = 35;
const int JOY_SW = 32;

const bool INVERT_X = false;   // flip if left/right is backwards
const bool INVERT_Y = false;   // flip if forward/back is backwards
const int  DEADZONE = 150;     // ADC counts around centre that count as zero
const uint32_t SEND_EVERY_MS = 20;   // 50 packets per second

const uint8_t BROADCAST[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

int centreX = 2048, centreY = 2048;
uint16_t seq = 0;

int readCentre(int pin) {
  long sum = 0;
  for (int i = 0; i < 32; i++) { sum += analogRead(pin); delay(2); }
  return sum / 32;
}

// ADC reading -> -255..255 around the measured centre, with a dead zone
int axis(int pin, int centre, bool invert) {
  int d = analogRead(pin) - centre;
  if (abs(d) < DEADZONE) return 0;
  int span = d > 0 ? (4095 - centre) : centre;
  int v = (d - (d > 0 ? DEADZONE : -DEADZONE)) * 255 / max(span - DEADZONE, 1);
  v = constrain(v, -255, 255);
  return invert ? -v : v;
}

void setup() {
  Serial.begin(115200);
  pinMode(JOY_SW, INPUT_PULLUP);
  analogReadResolution(12);

  centreX = readCentre(JOY_X);
  centreY = readCentre(JOY_Y);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(BLOCBOT_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed, restarting");
    delay(1000);
    ESP.restart();
  }
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = BLOCBOT_WIFI_CHANNEL;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.printf("Bloc-Con ready. Joystick centre X=%d Y=%d\n", centreX, centreY);
}

void loop() {
  static uint32_t lastSend = 0, lastPrint = 0;
  uint32_t now = millis();
  if (now - lastSend < SEND_EVERY_MS) return;
  lastSend = now;

  int x = axis(JOY_X, centreX, INVERT_X);   // turn
  int y = axis(JOY_Y, centreY, INVERT_Y);   // forward / back
  bool stop = digitalRead(JOY_SW) == LOW;

  // Arcade mix: push forward to drive, sideways to turn.
  int left  = constrain(y + x, -255, 255);
  int right = constrain(y - x, -255, 255);

  DriveCmd cmd = {};
  cmd.version     = BLOCBOT_PROTO_VERSION;
  cmd.type        = MSG_DRIVE;
  cmd.seq         = seq++;
  cmd.motor_left  = stop ? 0 : left;
  cmd.motor_right = stop ? 0 : right;
  cmd.estop       = stop ? 1 : 0;
  esp_now_send(BROADCAST, (const uint8_t*)&cmd, sizeof(cmd));

  if (now - lastPrint >= 250) {
    lastPrint = now;
    Serial.printf("x %4d  y %4d  ->  L %4d  R %4d %s\n", x, y, cmd.motor_left, cmd.motor_right,
                  stop ? " STOP" : "");
  }
}
