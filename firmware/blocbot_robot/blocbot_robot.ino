// BLOC-BOT robot (ESP32-S3): drives the two TT motors from Bloc-Con over ESP-NOW.
// Motors: DRV8833, AIN1=15, AIN2=16 (left), BIN1=17, BIN2=18 (right), STBY to 3.3V.
// Safety: motors stop if no command arrives for 300 ms, or when the controller sends estop.
// Serial Monitor at 115200 shows the robot's MAC address and the commands it receives.

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "blocbot_protocol.h"

const int AIN1 = 15;   // left motor
const int AIN2 = 16;
const int BIN1 = 17;   // right motor
const int BIN2 = 18;

const int  MAX_DUTY    = 178;   // ~70% of 255, keeps the TT motors near 6 V
const int  RAMP_STEP   = 12;    // max duty change per 10 ms, softens starts
const uint32_t LINK_TIMEOUT_MS = 300;

volatile int16_t  targetLeft = 0, targetRight = 0;
volatile uint32_t lastPacketMs = 0;
volatile uint32_t packets = 0;
int curLeft = 0, curRight = 0;

void setMotor(int in1, int in2, int speed) {
  if (speed > 0)      { analogWrite(in1, speed); analogWrite(in2, 0); }
  else if (speed < 0) { analogWrite(in1, 0);     analogWrite(in2, -speed); }
  else                { analogWrite(in1, 0);     analogWrite(in2, 0); }
}

// -255..255 from the controller -> -MAX_DUTY..MAX_DUTY
int scale(int v) {
  v = constrain(v, -255, 255);
  return v * MAX_DUTY / 255;
}

int rampTo(int cur, int target) {
  if (target > cur + RAMP_STEP) return cur + RAMP_STEP;
  if (target < cur - RAMP_STEP) return cur - RAMP_STEP;
  return target;
}

void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  if (len != sizeof(DriveCmd)) return;
  DriveCmd cmd;
  memcpy(&cmd, data, sizeof(cmd));
  if (cmd.version != BLOCBOT_PROTO_VERSION || cmd.type != MSG_DRIVE) return;

  if (cmd.estop) { targetLeft = 0; targetRight = 0; }
  else { targetLeft = scale(cmd.motor_left); targetRight = scale(cmd.motor_right); }
  lastPacketMs = millis();
  packets++;
}

void setup() {
  Serial.begin(115200);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  setMotor(AIN1, AIN2, 0);
  setMotor(BIN1, BIN2, 0);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(BLOCBOT_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed, restarting");
    delay(1000);
    ESP.restart();
  }
  esp_now_register_recv_cb(onReceive);

  delay(500);
  Serial.print("BLOC-BOT robot ready. MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  static uint32_t lastRamp = 0, lastPrint = 0;
  uint32_t now = millis();

  bool linkOk = (now - lastPacketMs) < LINK_TIMEOUT_MS;
  int wantLeft  = linkOk ? targetLeft  : 0;
  int wantRight = linkOk ? targetRight : 0;

  if (now - lastRamp >= 10) {
    lastRamp = now;
    curLeft  = rampTo(curLeft,  wantLeft);
    curRight = rampTo(curRight, wantRight);
    setMotor(AIN1, AIN2, curLeft);
    setMotor(BIN1, BIN2, curRight);
  }

  if (now - lastPrint >= 500) {
    lastPrint = now;
    Serial.printf("link %s  packets %lu  L %4d  R %4d\n",
                  linkOk ? "OK  " : "LOST", (unsigned long)packets, curLeft, curRight);
  }
}
