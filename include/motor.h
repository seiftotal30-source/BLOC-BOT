#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

// two tt motors on one drv8833, one h-bridge each
//   in1 = 1, in2 = 0  forward
//   in1 = 0, in2 = 1  reverse
//   in1 = 0, in2 = 0  coast
//   in1 = 1, in2 = 1  brake
// speed control: pwm on in1 (forward) or in2 (reverse), the other pin low

void initMotors();
void setMotorSpeed(char motor, int speed);      // 'L' or 'R', -255..255
void setMotors(int leftSpeed, int rightSpeed);  // targets, reached through the ramp
void stopMotors();                              // immediate, no ramp
void updateMotors();                            // call every loop: failsafe + ramp
void resetFailsafe();                           // call on every command
bool isFailsafeActive();
int getLeftSpeed();  // duty applied now, -255..255
int getRightSpeed();

#endif
