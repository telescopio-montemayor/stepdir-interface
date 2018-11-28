// Timeout in microseconds.
// If the driver does not receive an update from the host after that amount of time the axis is stopped.
#define UPDATE_TIMEOUT              (100 * 1000LL)
#define MONITOR_PIN                 10

#define AXIS_0_PIN_STEP             11
#define AXIS_0_PIN_DIR              12
#define AXIS_0_PIN_ENABLE           13

#define AXIS_1_PIN_STEP             7
#define AXIS_1_PIN_DIR              8
#define AXIS_1_PIN_ENABLE           9


float targetSpeed_0 = 0;
unsigned long last_step_time_0 = 0;
unsigned long last_update_time_0 = 0;
unsigned long stepInterval_0 = 1000;

float targetSpeed_1 = 0;
unsigned long last_step_time_1 = 0;
unsigned long last_update_time_1 = 0;
unsigned long stepInterval_1 = 1000;


void setup()
{
  Serial.begin(57600);
  while (Serial.available() > 0) {
    Serial.read();
  }

  pinMode(MONITOR_PIN,       OUTPUT);

  pinMode(AXIS_0_PIN_STEP,   OUTPUT);
  pinMode(AXIS_0_PIN_DIR,    OUTPUT);
  pinMode(AXIS_0_PIN_ENABLE, OUTPUT);

  pinMode(AXIS_1_PIN_STEP,   OUTPUT);
  pinMode(AXIS_1_PIN_DIR,    OUTPUT);
  pinMode(AXIS_1_PIN_ENABLE, OUTPUT);

  digitalWrite(AXIS_0_PIN_DIR, 1);
  digitalWrite(AXIS_1_PIN_DIR, 1);

  last_step_time_0 = micros();
  last_step_time_1 = micros();
  last_update_time_0 = micros();
  last_update_time_1 = micros();
}

void loop()
{
  unsigned long now = micros();
  if ((targetSpeed_0 != 0)
      && ((now - last_step_time_0) > stepInterval_0)
      && ((now - last_update_time_0) < UPDATE_TIMEOUT)) {
    last_step_time_0 = now;
    digitalWrite(AXIS_0_PIN_STEP, 1);
  }

  if ((targetSpeed_1 != 0)
      && ((now - last_step_time_1) > stepInterval_1)
      && ((now - last_update_time_1) < UPDATE_TIMEOUT)) {
    last_step_time_1 = now;
    digitalWrite(AXIS_1_PIN_STEP, 1);
  }

  delayMicroseconds(5);
  digitalWrite(AXIS_0_PIN_STEP, 0);
  digitalWrite(AXIS_1_PIN_STEP, 0);


  // Format is:
  // A+12345\n for channel 0
  // B+12345\n for channel 1
  if (Serial.available() >= 9) {
    int axis = 0;
    digitalWrite(MONITOR_PIN, 1);

    do {
      axis = Serial.read();

      if (axis == 'A' || axis == 'B') {
        break;
      }
    } while (Serial.available() >= 7);

    if (axis == 'A') {
      targetSpeed_0 = Serial.parseFloat();
      if (targetSpeed_0 != 0) {
        stepInterval_0 = fabs(1000000.0 / targetSpeed_0);
      }
      last_update_time_0 = micros();
    }

    if (axis == 'B') {
      targetSpeed_1 = Serial.parseFloat();
      if (targetSpeed_1 != 0) {
        stepInterval_1 = fabs(1000000.0 / targetSpeed_1);
      }
      last_update_time_1 = micros();
    }

    if (targetSpeed_0 > 0) {
      digitalWrite(AXIS_0_PIN_DIR, 1);
    } else {
      digitalWrite(AXIS_0_PIN_DIR, 0);
    }

    if (targetSpeed_1 > 0) {
      digitalWrite(AXIS_1_PIN_DIR, 1);
    } else {
      digitalWrite(AXIS_1_PIN_DIR, 0);
    }

    digitalWrite(MONITOR_PIN, 0);
  }
}
