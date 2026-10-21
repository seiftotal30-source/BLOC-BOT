#include "motor.h"
#include "pins.h"

// Arduino-ESP32 3.x replaced ledcSetup/ledcAttachPin and writes by pin; keep the channel-based calls
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void ledcSetup(uint8_t, uint32_t, uint8_t) {}
static void ledcAttachPin(uint8_t pin, uint8_t ch) { ledcAttachChannel(pin, PWM_FREQ, PWM_RESOLUTION, ch); }
#define ledcWrite(ch, duty) ledcWriteChannel(ch, duty)
#endif

// =====================================================
// MOTOR CONTROL VARIABLES
// =====================================================

// Current and target speeds
static int leftCurrentSpeed = 0;
static int rightCurrentSpeed = 0;
static int leftTargetSpeed = 0;
static int rightTargetSpeed = 0;

// Soft-start ramping
const int RAMP_RATE = 5;  // Speed change per update (adjust for desired ramp time)

// Failsafe
const unsigned long FAILSAFE_TIMEOUT = 300;  // ms
static unsigned long lastCommandTime = 0;
static bool failsafeActive = false;

// =====================================================
// HELPER: Set PWM for one motor (internal)
// =====================================================

void setMotorPWM(uint8_t in1Pin, uint8_t in2Pin, uint8_t ch1, uint8_t ch2, int speed) {
    // Clamp speed to -255..+255
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;

    if (speed > 0) {
        // Forward: IN1=PWM, IN2=LOW
        ledcWrite(ch1, speed);
        ledcWrite(ch2, 0);
    } else if (speed < 0) {
        // Reverse: IN1=LOW, IN2=PWM
        ledcWrite(ch1, 0);
        ledcWrite(ch2, -speed);
    } else {
        // Stop: Both LOW (coast)
        ledcWrite(ch1, 0);
        ledcWrite(ch2, 0);
    }
}

// =====================================================
// INITIALIZATION
// =====================================================

void initMotors() {
    // Setup PWM channels for left motor
    ledcSetup(PWM_CHANNEL_LEFT_IN1, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_LEFT_IN2, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_LEFT_IN1, PWM_CHANNEL_LEFT_IN1);
    ledcAttachPin(MOTOR_LEFT_IN2, PWM_CHANNEL_LEFT_IN2);

    // Setup PWM channels for right motor
    ledcSetup(PWM_CHANNEL_RIGHT_IN1, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_RIGHT_IN2, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_RIGHT_IN1, PWM_CHANNEL_RIGHT_IN1);
    ledcAttachPin(MOTOR_RIGHT_IN2, PWM_CHANNEL_RIGHT_IN2);

    // Start with motors stopped
    stopMotors();

    Serial.println("Motors initialized");
}

// =====================================================
// SET MOTOR SPEED (with ramping handled in update)
// =====================================================

void setMotorSpeed(char motor, int speed) {
    if (motor == 'L' || motor == 'l') {
        leftTargetSpeed = speed;
    } else if (motor == 'R' || motor == 'r') {
        rightTargetSpeed = speed;
    }

    resetFailsafe();
}

void setMotors(int leftSpeed, int rightSpeed) {
    leftTargetSpeed = leftSpeed;
    rightTargetSpeed = rightSpeed;
    resetFailsafe();
}

// =====================================================
// EMERGENCY STOP
// =====================================================

void stopMotors() {
    leftTargetSpeed = 0;
    rightTargetSpeed = 0;
    leftCurrentSpeed = 0;
    rightCurrentSpeed = 0;

    setMotorPWM(MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, PWM_CHANNEL_LEFT_IN1, PWM_CHANNEL_LEFT_IN2, 0);
    setMotorPWM(MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, PWM_CHANNEL_RIGHT_IN1, PWM_CHANNEL_RIGHT_IN2, 0);

    Serial.println("Motors stopped");
}

// =====================================================
// MOTOR UPDATE (Soft-start ramping + failsafe)
// =====================================================

void updateMotors() {
    // Check failsafe
    if (millis() - lastCommandTime > FAILSAFE_TIMEOUT) {
        if (!failsafeActive) {
            Serial.println("FAILSAFE: No commands received, stopping motors");
            failsafeActive = true;
            stopMotors();
        }
        return;
    } else {
        failsafeActive = false;
    }

    // Ramp left motor
    if (leftCurrentSpeed < leftTargetSpeed) {
        leftCurrentSpeed += RAMP_RATE;
        if (leftCurrentSpeed > leftTargetSpeed) leftCurrentSpeed = leftTargetSpeed;
    } else if (leftCurrentSpeed > leftTargetSpeed) {
        leftCurrentSpeed -= RAMP_RATE;
        if (leftCurrentSpeed < leftTargetSpeed) leftCurrentSpeed = leftTargetSpeed;
    }

    // Ramp right motor
    if (rightCurrentSpeed < rightTargetSpeed) {
        rightCurrentSpeed += RAMP_RATE;
        if (rightCurrentSpeed > rightTargetSpeed) rightCurrentSpeed = rightTargetSpeed;
    } else if (rightCurrentSpeed > rightTargetSpeed) {
        rightCurrentSpeed -= RAMP_RATE;
        if (rightCurrentSpeed < rightTargetSpeed) rightCurrentSpeed = rightTargetSpeed;
    }

    // Apply PWM
    setMotorPWM(MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, PWM_CHANNEL_LEFT_IN1, PWM_CHANNEL_LEFT_IN2, leftCurrentSpeed);
    setMotorPWM(MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, PWM_CHANNEL_RIGHT_IN1, PWM_CHANNEL_RIGHT_IN2, rightCurrentSpeed);
}

// =====================================================
// FAILSAFE MANAGEMENT
// =====================================================

void resetFailsafe() {
    lastCommandTime = millis();
}

bool isFailsafeActive() {
    return failsafeActive;
}

int getLeftSpeed() {
    return leftCurrentSpeed;
}

int getRightSpeed() {
    return rightCurrentSpeed;
}
