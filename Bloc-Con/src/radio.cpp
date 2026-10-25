#include <WiFi.h>
#include <esp_now.h>
#include "packets.h"
#include "radio.h"

RobotData robot;

static uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool isFresh(unsigned long t) {
  return t != 0 && millis() - t < DATA_TIMEOUT;
}

// a packet is accepted only if its id and its length both match
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onReceive(const esp_now_recv_info_t*, const uint8_t* data, int len) {
#else
static void onReceive(const uint8_t*, const uint8_t* data, int len) {
#endif
  if (len == sizeof(SpeedPacket) && data[0] == ID_SPEED) {
    SpeedPacket p;
    memcpy(&p, data, sizeof(p));
    robot.speedLeft = p.left;
    robot.speedRight = p.right;
    robot.speedTime = millis();
  } else if (len == sizeof(DistancePacket) && data[0] == ID_DISTANCE) {
    DistancePacket p;
    memcpy(&p, data, sizeof(p));
    robot.distMm = p.mm;
    robot.distTime = millis();
  } else if (len == sizeof(ImuPacket) && data[0] == ID_IMU) {
    ImuPacket p;
    memcpy(&p, data, sizeof(p));
    robot.roll = p.roll / 10.0;
    robot.pitch = p.pitch / 10.0;
    robot.gx = p.gx / 10.0;
    robot.gy = p.gy / 10.0;
    robot.gz = p.gz / 10.0;
    robot.imuTime = millis();
  }
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
  p.id = ID_DRIVE;
  p.fwd = constrain(fwd, -100, 100);
  p.turn = constrain(turn, -100, 100);
  esp_now_send(broadcastMac, (uint8_t*)&p, sizeof(p));
}
