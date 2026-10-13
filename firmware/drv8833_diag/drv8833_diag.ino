// DRV8833 diagnostic: drives the pins with plain HIGH/LOW (no PWM) from Serial
// Monitor commands, so each step can be checked with a multimeter.
// Serial Monitor: 115200 baud. Any line ending works.
//
// Wiring: AIN1=15, AIN2=16, BIN1=17, BIN2=18, all GNDs together, VM to the 7.3 V supply.
// STBY: either wire it to ESP32 3.3V (leave STBY_PIN = -1),
//       or wire it to GPIO 14 and set STBY_PIN = 14 so the sketch drives it HIGH.
//
// Lift the wheels off the table. Each motor command runs for 1.5 s, then stops.

const int AIN1 = 15;   // left motor
const int AIN2 = 16;
const int BIN1 = 17;   // right motor
const int BIN2 = 18;
const int STBY_PIN = -1;   // -1 = STBY wired to 3.3V; otherwise the GPIO it is wired to

const int PINS[] = { AIN1, AIN2, BIN1, BIN2 };
const char* NAMES[] = { "AIN1", "AIN2", "BIN1", "BIN2" };

void allLow() {
  for (int i = 0; i < 4; i++) digitalWrite(PINS[i], LOW);
}

void pulse(const char* label, int in1, int in2) {
  Serial.printf("%s: %d=HIGH, %d=LOW for 1.5 s\n", label, in1, in2);
  allLow();
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(1500);
  allLow();
  Serial.println("  stopped");
}

void holdPin(int i) {
  allLow();
  digitalWrite(PINS[i], HIGH);
  Serial.printf("%s (GPIO %d) is HIGH, the others LOW. Measure it to GND: expect ~3.3 V.\n",
                NAMES[i], PINS[i]);
  Serial.println("  Send 0 to set everything LOW.");
}

void pwmRamp() {
  Serial.println("PWM ramp on both motors, forward, up to 70 %");
  for (int d = 0; d <= 178; d += 2) {
    analogWrite(AIN1, d); analogWrite(AIN2, 0);
    analogWrite(BIN1, d); analogWrite(BIN2, 0);
    delay(30);
  }
  delay(1000);
  analogWrite(AIN1, 0); analogWrite(BIN1, 0);
  // Hand the pins back to plain digital output for the other commands.
  for (int i = 0; i < 4; i++) { pinMode(PINS[i], OUTPUT); digitalWrite(PINS[i], LOW); }
  Serial.println("  stopped");
}

void help() {
  Serial.println();
  Serial.println("=== DRV8833 diagnostic ===");
  Serial.println(" a  left forward    b  left reverse");
  Serial.println(" c  right forward   d  right reverse");
  Serial.println(" 1..4  hold AIN1 / AIN2 / BIN1 / BIN2 HIGH (for the multimeter)");
  Serial.println(" 0  all LOW         p  PWM ramp both motors");
  Serial.println(" h  this help");
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  for (int i = 0; i < 4; i++) { pinMode(PINS[i], OUTPUT); digitalWrite(PINS[i], LOW); }
  if (STBY_PIN >= 0) {
    pinMode(STBY_PIN, OUTPUT);
    digitalWrite(STBY_PIN, HIGH);
    Serial.printf("STBY driven HIGH on GPIO %d\n", STBY_PIN);
  } else {
    Serial.println("STBY expected on 3.3V. Check it reads ~3.3 V to GND.");
  }
  help();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'a': pulse("Left forward",  AIN1, AIN2); break;
    case 'b': pulse("Left reverse",  AIN2, AIN1); break;
    case 'c': pulse("Right forward", BIN1, BIN2); break;
    case 'd': pulse("Right reverse", BIN2, BIN1); break;
    case '1': case '2': case '3': case '4': holdPin(c - '1'); break;
    case '0': allLow(); Serial.println("All LOW"); break;
    case 'p': pwmRamp(); break;
    case 'h': help(); break;
    default: break;   // ignore newlines and other characters
  }
}
