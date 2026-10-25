#ifndef IMU_H
#define IMU_H

// mpu6050 on i2c (sda 8, scl 9)
void initImu();    // keep the robot still: the gyro is zeroed here
void updateImu();  // every loop, complementary filter
void sendImu();    // every 50 ms to the controller

#endif
