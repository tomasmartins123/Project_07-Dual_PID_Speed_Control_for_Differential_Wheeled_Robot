// ============================================================================
// CRIA 7 PROJECT - Kp value testing
// ============================================================================

const int DIR1 = A0;
const int PWM1 = 5;
const int LEFT_SENSOR = 2;

const float TRANSITIONS_PER_REV = 40.0;
const unsigned long SAMPLE_INTERVAL = 150; // 150 ms for better resolution (10 RPM per step)

float targetRPM = 100.0;

// --- AUTOMATIC KP TEST ---
float kpValues[5] = {1.0, 1.5, 2.0, 2.5, 3.0};
int subTrial = 0;
bool testCompleted = false;
unsigned long subTrialStartTime = 0;

// --- SENSOR VARIABLES ---
volatile unsigned long leftTransitions = 0;
volatile unsigned long lastLeftTransition = 0;

unsigned long lastSampleTime = 0;
int leftPWM = 0;
float filteredRPM = 0.0; // Holds the smoothed speed

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
  Serial.println("Time_ms,Trial,Kp,Setpoint,Measured_RPM,PWM_Out");

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

    if (timeInSubTrial >= 7000) { // 5s Test + 2s Pause completed
      subTrial++;
      subTrialStartTime = now;
      timeInSubTrial = 0; // Reset cycle internal timer

      // If all 5 Kp values have been tested, conclude the test
      if (subTrial >= 5) {
        testCompleted = true;
        analogWrite(PWM1, 0);
        Serial.println("# END_TEST");
        return;
      }
    }

    float currentKp = kpValues[subTrial];

    // 6. Control Logic vs Pause
    if (timeInSubTrial >= 5000) {
      // --- PAUSE PHASE (Last 2 seconds) ---
      // Force motor stop and clear filter memory for a clean start on the next Kp
      leftPWM = 0;
      filteredRPM = 0.0;
    } else {
      // --- TEST PHASE (First 5 seconds) ---
      // Active Proportional Controller
      float leftError = targetRPM - filteredRPM;
      float leftSignal = currentKp * leftError;
      leftPWM = constrain((int)leftSignal, 0, 255);
    }

    // 7. Motor Actuation
    analogWrite(PWM1, leftPWM);

    // 8. Telemetry Output
    Serial.print(now); Serial.print(",");
    Serial.print(subTrial + 1); Serial.print(",");
    Serial.print(currentKp, 2); Serial.print(",");
    Serial.print(targetRPM, 1); Serial.print(",");
    Serial.print(filteredRPM, 1); Serial.print(",");
    Serial.println(leftPWM);
  }
}