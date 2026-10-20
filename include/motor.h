#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

// =====================================================
// MOTOR CONTROL FOR DRV8871 DRIVERS
// =====================================================
// DRV8871 is a single H-bridge driver (one motor per chip)
// Control modes:
// IN1=HIGH, IN2=LOW  -> Forward
// IN1=LOW,  IN2=HIGH -> Reverse
// IN1=LOW,  IN2=LOW  -> Coast (motor freewheels)
// IN1=HIGH, IN2=HIGH -> Brake (slow decay)
//
// For PWM speed control, we use:
// Forward: IN1=PWM, IN2=LOW
// Reverse: IN1=LOW, IN2=PWM
// =====================================================

// Initialize motor driver PWM channels
void initMotors();

// Set motor speed (-255 to +255)
// Negative = reverse, Positive = forward, 0 = stop
// Uses soft-start ramping to avoid current spikes
void setMotorSpeed(char motor, int speed);

// Set both motors (for arcade drive mixing)
void setMotors(int leftSpeed, int rightSpeed);

// Emergency stop (immediate)
void stopMotors();

// Update motor ramping (call this in main loop)
void updateMotors();

// Reset failsafe timer (call when receiving commands)
void resetFailsafe();

// Check if motors are stopped due to failsafe
bool isFailsafeActive();

// Current (ramped) motor speeds, -255..+255
int getLeftSpeed();
int getRightSpeed();

#endif // MOTOR_H
