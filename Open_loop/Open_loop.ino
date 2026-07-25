// ============================================================================
// CRIA 7 PROJECT - Open-Loop Control Step
// ============================================================================

// =================================
// PINS
// =================================

const int DIR1 = A0;
const int PWM1 = 5;

const int DIR2 = A2;
const int PWM2 = 6;

const int LEFT_SENSOR = 2;
const int RIGHT_SENSOR = 3;


// =================================
// CALIBRATION AND CONFIGURATION
// =================================

const float TRANSITIONS_PER_REV = 40.0;

const unsigned long SAMPLE_INTERVAL = 100; // 100 ms = 10 Hz


// =================================
// LEFT SENSOR VARIABLES
// =================================

volatile unsigned long leftTransitions = 0;

volatile unsigned long lastLeftTransition = 0;


// =================================
// RIGHT SENSOR VARIABLES
// =================================

volatile unsigned long rightTransitions = 0;

volatile unsigned long lastRightTransition = 0;


// =================================
// TIMING AND COMMANDS
// =================================

unsigned long lastSampleTime = 0;

const int FIXED_PWM = 100;


// =================================
// SETUP
// =================================

void setup() {

  pinMode(DIR1, OUTPUT);
  pinMode(PWM1, OUTPUT);

  pinMode(DIR2, OUTPUT);
  pinMode(PWM2, OUTPUT);

  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);


  // Set direction of travel
  digitalWrite(DIR1, LOW);
  digitalWrite(DIR2, LOW);


  attachInterrupt(
    digitalPinToInterrupt(LEFT_SENSOR),
    countLeftTransition,
    CHANGE
  );


  attachInterrupt(
    digitalPinToInterrupt(RIGHT_SENSOR),
    countRightTransition,
    CHANGE
  );


  Serial.begin(9600);

  delay(2000);


  Serial.println("--- STEP 1: OPEN LOOP (FIXED PWM) ---");

  Serial.println("Time_ms,PWM_Left,RPM_Left,PWM_Right,RPM_Right");


  // Apply fixed command to motors
  analogWrite(PWM1, FIXED_PWM);

  analogWrite(PWM2, FIXED_PWM);


  lastSampleTime = millis();
}


// =================================
// MAIN LOOP
// =================================

void loop() {

  unsigned long now = millis();


  // Real-time reading and sampling (every 100 ms)
  if (now - lastSampleTime >= SAMPLE_INTERVAL) {


    // Atomic read of accumulated transitions
    noInterrupts();

    unsigned long leftMeasured = leftTransitions;

    unsigned long rightMeasured = rightTransitions;


    leftTransitions = 0;

    rightTransitions = 0;

    interrupts();


    // Real elapsed time in seconds
    float dt = (now - lastSampleTime) / 1000.0;

    lastSampleTime = now;


    // RPM calculation using 40 transitions per revolution calibration
    float leftRPM =
      (leftMeasured / TRANSITIONS_PER_REV)
      * (60.0 / dt);


    float rightRPM =
      (rightMeasured / TRANSITIONS_PER_REV)
      * (60.0 / dt);


    // Print data in CSV format for Serial Monitor / Plotter
    Serial.print(now);

    Serial.print(",");

    Serial.print(FIXED_PWM);

    Serial.print(",");

    Serial.print(leftRPM, 2);

    Serial.print(",");

    Serial.print(FIXED_PWM);

    Serial.print(",");

    Serial.println(rightRPM, 2);
  }
}


// =================================
// LEFT MOTOR INTERRUPT
// =================================

void countLeftTransition() {

  unsigned long now = micros();


  if (
    now - lastLeftTransition
    > 1000
  ) {

    leftTransitions++;

    lastLeftTransition = now;
  }
}


// =================================
// RIGHT MOTOR INTERRUPT
// =================================

void countRightTransition() {

  unsigned long now = micros();


  if (
    now - lastRightTransition
    > 1000
  ) {

    rightTransitions++;

    lastRightTransition = now;
  }
}