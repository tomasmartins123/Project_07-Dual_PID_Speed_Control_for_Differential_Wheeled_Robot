// ============================================================================
// CRIA 7 PROJECT - Dual-Wheel PID Control with Selected Gain Values
// ============================================================================

const int DIR1 = A0;
const int PWM1 = 5;
const int LEFT_SENSOR = 2;

const int DIR2 = A1;
const int PWM2 = 6;
const int RIGHT_SENSOR = 3;

const float TRANSITIONS_PER_REV = 40.0;
const unsigned long SAMPLE_INTERVAL = 150;

float targetRPM = 100.0;

// --- SELECTED PID GAINS ---
const float Kp = 2.00;
const float Ki = 1.25;
const float Kd = 0.06;

// --- TEST DURATION ---
const unsigned long TEST_DURATION_MS = 15000; // 15 seconds
unsigned long testStartTime = 0;
bool testCompleted = false;

// --- SENSOR AND CONTROLLER VARIABLES (LEFT) ---
volatile unsigned long leftTransitions = 0;
volatile unsigned long lastLeftTransition = 0;
float filteredLeftRPM = 0.0;
float leftErrorAccumulator = 0.0;
float previousLeftError = 0.0;
int leftPWM = 0;

// --- SENSOR AND CONTROLLER VARIABLES (RIGHT) ---
volatile unsigned long rightTransitions = 0;
volatile unsigned long lastRightTransition = 0;
float filteredRightRPM = 0.0;
float rightErrorAccumulator = 0.0;
float previousRightError = 0.0;
int rightPWM = 0;

unsigned long lastSampleTime = 0;

void countLeftTransition() {
  unsigned long now = micros();
  if (now - lastLeftTransition > 1000) {
    leftTransitions++;
    lastLeftTransition = now;
  }
}

void countRightTransition() {
  unsigned long now = micros();
  if (now - lastRightTransition > 1000) {
    rightTransitions++;
    lastRightTransition = now;
  }
}

void setup() {
  pinMode(DIR1, OUTPUT); pinMode(PWM1, OUTPUT); pinMode(LEFT_SENSOR, INPUT);
  pinMode(DIR2, OUTPUT); pinMode(PWM2, OUTPUT); pinMode(RIGHT_SENSOR, INPUT);

  digitalWrite(DIR1, HIGH);
  digitalWrite(DIR2, HIGH);

  attachInterrupt(digitalPinToInterrupt(LEFT_SENSOR), countLeftTransition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_SENSOR), countRightTransition, CHANGE);

  Serial.begin(115200);
  delay(2000);

  Serial.println("Time_ms,Setpoint,Left_RPM,Right_RPM,Left_PWM,Right_PWM");

  testStartTime = millis();
  lastSampleTime = millis();
}

void loop() {
  if (testCompleted) return;

  unsigned long now = millis();

  // Termination condition at 15 seconds
  if (now - testStartTime >= TEST_DURATION_MS) {
    testCompleted = true;
    analogWrite(PWM1, 0);
    analogWrite(PWM2, 0);
    Serial.println("# END_TEST");
    return;
  }

  if (now - lastSampleTime >= SAMPLE_INTERVAL) {

    noInterrupts();
    unsigned long leftMeasured = leftTransitions;
    unsigned long rightMeasured = rightTransitions;
    leftTransitions = 0;
    rightTransitions = 0;
    interrupts();

    float dt = (now - lastSampleTime) / 1000.0;
    lastSampleTime = now;

    // RPM and EMA Filter
    float rawLeftRPM = (leftMeasured / TRANSITIONS_PER_REV) * (60.0 / dt);
    filteredLeftRPM = (0.3 * rawLeftRPM) + (0.7 * filteredLeftRPM);

    float rawRightRPM = (rightMeasured / TRANSITIONS_PER_REV) * (60.0 / dt);
    filteredRightRPM = (0.3 * rawRightRPM) + (0.7 * filteredRightRPM);

    // Left Wheel PID
    float leftError = targetRPM - filteredLeftRPM;
    leftErrorAccumulator += leftError * dt;
    leftErrorAccumulator = constrain(leftErrorAccumulator, -100.0, 100.0);
    float leftErrorDerivative = (leftError - previousLeftError) / dt;
    float leftSignal = (Kp * leftError) + (Ki * leftErrorAccumulator) + (Kd * leftErrorDerivative);
    leftPWM = constrain((int)leftSignal, 0, 255);
    previousLeftError = leftError;

    // Right Wheel PID
    float rightError = targetRPM - filteredRightRPM;
    rightErrorAccumulator += rightError * dt;
    rightErrorAccumulator = constrain(rightErrorAccumulator, -100.0, 100.0);
    float rightErrorDerivative = (rightError - previousRightError) / dt;
    float rightSignal = (Kp * rightError) + (Ki * rightErrorAccumulator) + (Kd * rightErrorDerivative);
    rightPWM = constrain((int)rightSignal, 0, 255);
    previousRightError = rightError;

    // Actuation
    analogWrite(PWM1, leftPWM);
    analogWrite(PWM2, rightPWM);

    // Telemetry Output
    Serial.print(now); Serial.print(",");
    Serial.print(targetRPM, 1); Serial.print(",");
    Serial.print(filteredLeftRPM, 1); Serial.print(",");
    Serial.print(filteredRightRPM, 1); Serial.print(",");
    Serial.print(leftPWM); Serial.print(",");
    Serial.println(rightPWM);
  }
}