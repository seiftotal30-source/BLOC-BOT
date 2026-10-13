// DRV8833 motor test for BLOC-BOT (ESP32-S3).
// Order: left forward, left reverse, right forward, right reverse,
//        then both forward, both reverse. Repeats forever.
// Wiring: AIN1=15, AIN2=16, BIN1=17, BIN2=18, STBY to 3.3V, all GNDs together.
// Lift the wheels off the table. Serial Monitor at 115200.

const int AIN1 = 15;   // left motor
const int AIN2 = 16;
const int BIN1 = 17;   // right motor
const int BIN2 = 18;

const int SPEED   = 178;   // ~70% of 255, keeps the TT motors near 6 V
const int RUN_MS  = 2000;  // how long each step runs
const int STOP_MS = 1000;  // pause between steps

// speed: -SPEED..SPEED (positive = forward, negative = reverse, 0 = coast)
void setMotor(int in1, int in2, int speed) {
  speed = constrain(speed, -SPEED, SPEED);
  if (speed > 0)      { analogWrite(in1, speed); analogWrite(in2, 0); }
  else if (speed < 0) { analogWrite(in1, 0);     analogWrite(in2, -speed); }
  else                { analogWrite(in1, 0);     analogWrite(in2, 0); }
}

void drive(int left, int right) {
  setMotor(AIN1, AIN2, left);
  setMotor(BIN1, BIN2, right);
}

void step(const char* label, int left, int right) {
  Serial.println(label);
  drive(left, right);
  delay(RUN_MS);
  drive(0, 0);
  delay(STOP_MS);
}

void setup() {
  Serial.begin(115200);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  drive(0, 0);
  delay(2000);
  Serial.println("BLOC-BOT motor test start");
}

void loop() {
  Serial.println("--- one at a time ---");
  step("Left  forward",  SPEED, 0);
  step("Left  reverse", -SPEED, 0);
  step("Right forward", 0,  SPEED);
  step("Right reverse", 0, -SPEED);

  Serial.println("--- both together ---");
  step("Both  forward",  SPEED,  SPEED);
  step("Both  reverse", -SPEED, -SPEED);

  Serial.println("Cycle done, repeating in 3 s\n");
  delay(3000);
}
