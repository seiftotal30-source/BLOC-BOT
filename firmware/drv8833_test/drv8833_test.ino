// DRV8833 test: left motor, then right motor, forward and reverse.
// Wiring: AIN1=15, AIN2=16, BIN1=17, BIN2=18, STBY to 3.3V, all GNDs together.
// Lift the wheels off the table before running. Serial Monitor at 115200.

const int AIN1 = 15;   // left motor
const int AIN2 = 16;
const int BIN1 = 17;   // right motor
const int BIN2 = 18;

const int MAX_DUTY = 178;  // ~70% of 255, keeps the TT motors near 6 V

// speed: -MAX_DUTY..MAX_DUTY  (positive = forward, negative = reverse, 0 = coast)
void setMotor(int in1, int in2, int speed) {
  speed = constrain(speed, -MAX_DUTY, MAX_DUTY);
  if (speed > 0)      { analogWrite(in1, speed);  analogWrite(in2, 0); }
  else if (speed < 0) { analogWrite(in1, 0);      analogWrite(in2, -speed); }
  else                { analogWrite(in1, 0);      analogWrite(in2, 0); }
}

void stopAll() {
  setMotor(AIN1, AIN2, 0);
  setMotor(BIN1, BIN2, 0);
}

void run(const char* label, int in1, int in2, int speed) {
  Serial.println(label);
  setMotor(in1, in2, speed);
  delay(2000);
  stopAll();
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  stopAll();
  delay(2000);
  Serial.println("DRV8833 test start");
}

void loop() {
  run("Left  forward", AIN1, AIN2,  150);
  run("Left  reverse", AIN1, AIN2, -150);
  run("Right forward", BIN1, BIN2,  150);
  run("Right reverse", BIN1, BIN2, -150);

  Serial.println("Both ramp up");
  for (int s = 0; s <= MAX_DUTY; s += 5) {
    setMotor(AIN1, AIN2, s);
    setMotor(BIN1, BIN2, s);
    delay(60);
  }
  delay(1000);
  stopAll();
  Serial.println("Done, repeating in 3 s\n");
  delay(3000);
}
