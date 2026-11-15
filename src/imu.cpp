#include <Arduino.h>
#include <Wire.h>
#include "imu.h"
#include "packets.h"
#include "pins.h"
#include "radio.h"

#ifdef I2C_SDA_PIN

const uint8_t MPU_ADDR = 0x68;
const unsigned long IMU_SEND_MS = 50;  // 20 Hz

// mpu6050 registers
const uint8_t REG_GYRO_CONFIG = 0x1B;
const uint8_t REG_ACCEL_CONFIG = 0x1C;
const uint8_t REG_ACCEL_XOUT = 0x3B;  // accel, temp, gyro: 14 bytes
const uint8_t REG_GYRO_XOUT = 0x43;
const uint8_t REG_PWR_MGMT_1 = 0x6B;

static bool mpuOk = false;
static float gyroOff[3] = {0, 0, 0};
static float roll = 0, pitch = 0, gyroX = 0, gyroY = 0, gyroZ = 0;
static unsigned long lastUs = 0;

static bool mpuRead(uint8_t reg, uint8_t* buf, uint8_t n) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(MPU_ADDR, n) != n) return false;
    for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
    return true;
}

static void mpuWrite(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void initImu() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);
    Wire.beginTransmission(MPU_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("MPU6050 not found (SDA 8, SCL 9)");
        return;
    }
    mpuWrite(REG_PWR_MGMT_1, 0x00);    // wake up
    mpuWrite(REG_ACCEL_CONFIG, 0x00);  // +-2 g
    mpuWrite(REG_GYRO_CONFIG, 0x00);   // +-250 deg/s
    delay(100);

    // gyro offset = mean of 200 readings at rest
    long sum[3] = {0, 0, 0};
    uint8_t b[6];
    for (int i = 0; i < 200; i++) {
        if (mpuRead(REG_GYRO_XOUT, b, 6))
            for (int k = 0; k < 3; k++) sum[k] += (int16_t)((b[2 * k] << 8) | b[2 * k + 1]);
        delay(3);
    }
    for (int k = 0; k < 3; k++) gyroOff[k] = sum[k] / 200.0;
    mpuOk = true;
    Serial.println("MPU6050 ready");
}

// tilt from gravity (steady but noisy) blended with the gyro (smooth but drifts)
void updateImu() {
    if (!mpuOk) return;
    uint8_t b[14];
    if (!mpuRead(REG_ACCEL_XOUT, b, 14)) return;
    auto w = [&](int i) { return (int16_t)((b[i] << 8) | b[i + 1]); };

    float ax = w(0) / 16384.0, ay = w(2) / 16384.0, az = w(4) / 16384.0;  // g
    gyroX = (w(8) - gyroOff[0]) / 131.0;                                    // deg/s
    gyroY = (w(10) - gyroOff[1]) / 131.0;
    gyroZ = (w(12) - gyroOff[2]) / 131.0;

    float accRoll = atan2(ay, az) * 180.0 / PI;
    float accPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

    unsigned long nowUs = micros();
    float dt = (nowUs - lastUs) / 1e6;
    if (lastUs == 0 || dt > 0.2) {  // first run or long gap: start from the accel
        roll = accRoll;
        pitch = accPitch;
    } else {
        roll = 0.96 * (roll + gyroX * dt) + 0.04 * accRoll;
        pitch = 0.96 * (pitch + gyroY * dt) + 0.04 * accPitch;
    }
    lastUs = nowUs;
}

void sendImu() {
    static unsigned long lastSend = 0;
    if (!mpuOk || millis() - lastSend < IMU_SEND_MS) return;
    lastSend = millis();
    ImuPacket p = {ID_IMU, (int16_t)(roll * 10), (int16_t)(pitch * 10),
                   (int16_t)(gyroX * 10), (int16_t)(gyroY * 10), (int16_t)(gyroZ * 10)};
    radioSend(&p, sizeof(p));
}

#else
void initImu() {}
void updateImu() {}
void sendImu() {}
#endif
