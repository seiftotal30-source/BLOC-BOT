#include <WiFi.h>
#include <esp_now.h>
#include "radio.h"
#include "drive.h"
#include "line.h"

static uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// called by esp-now for every packet received
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onReceive(const esp_now_recv_info_t*, const uint8_t* data, int len) {
#else
static void onReceive(const uint8_t*, const uint8_t* data, int len) {
#endif
    handleDrivePacket(data, len);
    handleLineCmdPacket(data, len);
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

    Serial.print("ESP-NOW ready, robot MAC: ");
    Serial.println(WiFi.macAddress());
}

void radioSend(const void* data, size_t len) {
    esp_now_send(broadcastMac, (const uint8_t*)data, len);
}
