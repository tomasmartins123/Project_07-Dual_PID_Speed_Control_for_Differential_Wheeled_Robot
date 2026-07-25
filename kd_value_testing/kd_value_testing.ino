// ============================================================================
// CRIA 7 PROJECT - Kd Value Testing
// ============================================================================

const int DIR1 = A0;
const int PWM1 = 5;
const int LEFT_SENSOR = 2;

const float TRANSITIONS_PER_REV = 40.0;
const unsigned long SAMPLE_INTERVAL = 150; // 150 ms for better resolution (10 RPM per step)

float targetRPM = 100.0;

// --- FIXED PARAMETERS (SELECTED KP AND KI) ---
const float Kp_fixed = 2.0;  // Optimal Proportional Gain
const float Ki_fixed = 1.25; // Optimal Integral Gain

// --- AUTOMATIC KD TEST ---
float kdValues[5] = {0.02, 0.04, 0.06, 0.08, 0.1};
int subTrial = 0;
bool testCompleted = false;
unsigned long subTrialStartTime = 0;

// --- SENSOR AND CONTROLLER VARIABLES ---
volatile unsigned long leftTransitions = 0;
volatile unsigned long lastLeftTransition = 0;

unsigned long lastSampleTime = 0;
int leftPWM = 0;
float filteredRPM = 0.0;      // Holds the smoothed speed
float errorAccumulator = 0.0; // Accumulated integral term
float previousError = 0.0;    // Holds the previous cycle error for the derivative term

// =================================
// INTERRUPT (Anti-Debounce Filter)
// =================================
void countLeftTransition() {
  unsigned long now = micros();
  if (now - lastLeftTransition > 1000) {
    leftTransitions++;
    lastLeftTransition = now;
  }
}

void setup() {
  pinMode(DIR1, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(LEFT_SENSOR, INPUT);

  digitalWrite(DIR1, LOW);

  attachInterrupt(digitalPinToInterrupt(LEFT_SENSOR), countLeftTransition, CHANGE);

  Serial.begin(115200);
  delay(2000);

  // CSV Header matching Python processing scripts
  Serial.println("Time_ms,Trial,Kd,Setpoint,Measured_RPM,PWM_Out");

  subTrialStartTime = millis();
  lastSampleTime = millis();
}

void loop() {
  if (testCompleted) return;

  unsigned long now = millis();

  // Executes only every SAMPLE_INTERVAL (150 ms)
  if (now - lastSampleTime >= SAMPLE_INTERVAL) {

    // 1. Read transitions with interrupt protection
    noInterrupts();
    unsigned long leftMeasured = leftTransitions;
    leftTransitions = 0;
    interrupts();

    // 2. Elapsed time
    float dt = (now - lastSampleTime) / 1000.0;
    lastSampleTime = now;

    // 3. Instantaneous RPM Calculation
    float rawRPM = (leftMeasured / TRANSITIONS_PER_REV) * (60.0 / dt);

    // 4. LOW-PASS FILTER (Exponential Moving Average)
    filteredRPM = (0.3 * rawRPM) + (0.7 * filteredRPM);

    // 5. Test Timing Management
    unsigned long timeInSubTrial = now - subTrialStartTime;

    if (timeInSubTrial >= 12000) { // 10s Test + 2s Pause completed
      subTrial++;
      subTrialStartTime = now;
      timeInSubTrial = 0; // Reset cycle internal timer

      // If all 5 Kd values have been tested, conclude the test
      if (subTrial >= 5) {
        testCompleted = true;
        analogWrite(PWM1, 0);
        Serial.println("# END_TEST");
        return;
      }
    }

    float currentKd = kdValues[subTrial];

    // 6. Control Logic vs Pause
    if (timeInSubTrial >= 10000) {
      // --- PAUSE PHASE (Last 2 seconds) ---
      // Force motor stop, clear filter and reset memory terms
      leftPWM = 0;
      filteredRPM = 0.0;
      errorAccumulator = 0.0;
      previousError = targetRPM; // Prepare clean startup without initial derivative spike
    } else {
      // --- TEST PHASE (First 10 seconds) ---
      // Active PID Controller
      float leftError = targetRPM - filteredRPM;

      // Accumulate error over time (Integral Term)
      errorAccumulator += leftError * dt;
      errorAccumulator = constrain(errorAccumulator, -100.0, 100.0); // Anti-Windup

      // Rate of error change over time (Derivative Term)
      float errorDerivative = (leftError - previousError) / dt;

      // PID Control Signal
      float leftSignal = (Kp_fixed * leftError) + (Ki_fixed * errorAccumulator) + (currentKd * errorDerivative);
      leftPWM = constrain((int)leftSignal, 0, 255);

      // Save error for next cycle
      previousError = leftError;
    }

    // 7. Motor Actuation
    analogWrite(PWM1, leftPWM);

    // 8. Telemetry Output
    Serial.print(now); Serial.print(",");
    Serial.print(subTrial + 1); Serial.print(",");
    Serial.print(currentKd, 2); Serial.print(",");
    Serial.print(targetRPM, 1); Serial.print(",");
    Serial.print(filteredRPM, 1); Serial.print(",");
    Serial.println(leftPWM);
  }
}