#include "motor.h"
#include "pins.h"

// arduino-esp32 3.x dropped ledcSetup/ledcAttachPin, keep the channel calls working
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void ledcSetup(uint8_t, uint32_t, uint8_t) {}
static void ledcAttachPin(uint8_t pin, uint8_t ch) { ledcAttachChannel(pin, PWM_FREQ, PWM_RESOLUTION, ch); }
#define ledcWrite(ch, duty) ledcWriteChannel(ch, duty)
#endif

const int RAMP_STEP = 5;                     // duty change per 10 ms loop, ~0.4 s from 0 to 204
const unsigned long FAILSAFE_TIMEOUT = 300;  // ms without a command = stop

static int leftCurrent = 0, rightCurrent = 0;  // duty applied now, -255..255
static int leftTarget = 0, rightTarget = 0;    // duty asked for
static unsigned long lastCommandTime = 0;
static bool failsafeActive = false;

// forward: in1 = pwm, in2 = 0. reverse: in1 = 0, in2 = pwm. 0: both low (coast)
static void setMotorPWM(uint8_t ch1, uint8_t ch2, int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {
        ledcWrite(ch1, speed);
        ledcWrite(ch2, 0);
    } else if (speed < 0) {
        ledcWrite(ch1, 0);
        ledcWrite(ch2, -speed);
    } else {
        ledcWrite(ch1, 0);
        ledcWrite(ch2, 0);
    }
}

static void applyPWM() {
    setMotorPWM(PWM_CHANNEL_LEFT_IN1, PWM_CHANNEL_LEFT_IN2, leftCurrent);
    setMotorPWM(PWM_CHANNEL_RIGHT_IN1, PWM_CHANNEL_RIGHT_IN2, rightCurrent);
}

// move current toward target by at most one step
static int ramp(int current, int target) {
    if (current < target) return min(current + RAMP_STEP, target);
    if (current > target) return max(current - RAMP_STEP, target);
    return current;
}

void initMotors() {
    ledcSetup(PWM_CHANNEL_LEFT_IN1, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_LEFT_IN2, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_LEFT_IN1, PWM_CHANNEL_LEFT_IN1);
    ledcAttachPin(MOTOR_LEFT_IN2, PWM_CHANNEL_LEFT_IN2);

    ledcSetup(PWM_CHANNEL_RIGHT_IN1, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_RIGHT_IN2, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_RIGHT_IN1, PWM_CHANNEL_RIGHT_IN1);
    ledcAttachPin(MOTOR_RIGHT_IN2, PWM_CHANNEL_RIGHT_IN2);

    stopMotors();
    Serial.println("Motors initialized");
}

void setMotorSpeed(char motor, int speed) {
    if (motor == 'L' || motor == 'l') leftTarget = speed;
    else if (motor == 'R' || motor == 'r') rightTarget = speed;
    resetFailsafe();
}

void setMotors(int leftSpeed, int rightSpeed) {
    leftTarget = leftSpeed;
    rightTarget = rightSpeed;
    resetFailsafe();
}

// immediate stop, no ramp
void stopMotors() {
    leftTarget = rightTarget = 0;
    leftCurrent = rightCurrent = 0;
    applyPWM();
    Serial.println("Motors stopped");
}

void updateMotors() {
    if (millis() - lastCommandTime > FAILSAFE_TIMEOUT) {
        if (!failsafeActive) {
            Serial.println("FAILSAFE: no command, stopping motors");
            failsafeActive = true;
            stopMotors();
        }
        return;
    }
    failsafeActive = false;

    leftCurrent = ramp(leftCurrent, leftTarget);
    rightCurrent = ramp(rightCurrent, rightTarget);
    applyPWM();
}

void resetFailsafe() { lastCommandTime = millis(); }
bool isFailsafeActive() { return failsafeActive; }
int getLeftSpeed() { return leftCurrent; }
int getRightSpeed() { return rightCurrent; }
