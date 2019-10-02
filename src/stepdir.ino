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

// When low ignore commands from host and use joystick/button pad instead
#define MANUAL_TOGGLE_PIN             6
#define MANUAL_SPEED_PIN              A0
#define MANUAL_CONTROL_AXIS_0_PIN     A1
#define MANUAL_CONTROL_AXIS_1_PIN     A2

// Runs axis 0 at tracking speed if manual mode is selected (and the joystick is centered)
#define TRACKING_TOGGLE_PIN           5
// 15.04106858 degrees/hour
// 0.00417807460555556 degrees/second
//
// On our mount, one revolution on the worm translates to 1 degree on the output shaft (1:360)
// so we need to turn the axis 0 at a rate of
//
// 0.00417807460555556 revolutions/second
//
// We use 25600 microsteps. Then the tracking speed is:
//
// 106.958709902222 (micro)steps/second
//
#define TRACKING_SPEED                (106.958709902222)

// Sometimes the joystick/pad wiring does not reflect the intended movement. This swaps the direction meaning.
#define MANUAL_CONTROL_AXIS_0_INVERT  (false)
#define MANUAL_CONTROL_AXIS_1_INVERT  (false)
#define MANUAL_MIN_SPEED              133
#define MANUAL_MAX_SPEED              (40000)
#define MANUAL_UPDATE_INTERVAL        (UPDATE_TIMEOUT / 2)
// if the deviation from mid point is less than this we assume the joystick is centered
#define MANUAL_DEADBAND               30


bool manual_control = false;
bool tracking = false;
unsigned long last_manual_update_time = 0;

float targetSpeed_0 = 0;
unsigned long last_step_time_0 = 0;
unsigned long last_update_time_0 = 0;
unsigned long stepInterval_0 = 1000;

float targetSpeed_1 = 0;
unsigned long last_step_time_1 = 0;
unsigned long last_update_time_1 = 0;
unsigned long stepInterval_1 = 1000;

void update_direction_pins();
void update_step_intervals();

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

  pinMode(TRACKING_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(MANUAL_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(MANUAL_SPEED_PIN, INPUT);
  pinMode(MANUAL_CONTROL_AXIS_0_PIN, INPUT);
  pinMode(MANUAL_CONTROL_AXIS_1_PIN, INPUT);

  last_step_time_0 = micros();
  last_step_time_1 = micros();
  last_update_time_0 = micros();
  last_update_time_1 = micros();

  tracking = !digitalRead(TRACKING_TOGGLE_PIN);
  manual_control = !digitalRead(MANUAL_TOGGLE_PIN);
  last_manual_update_time = micros();
}

void loop()
{
  tracking = !digitalRead(TRACKING_TOGGLE_PIN);
  manual_control = !digitalRead(MANUAL_TOGGLE_PIN);

  unsigned long now = micros();
  if ((targetSpeed_0 != 0)
      && ((now - last_step_time_0) > stepInterval_0)
      && (manual_control || ((now - last_update_time_0) < UPDATE_TIMEOUT))) {
    last_step_time_0 = now;
    digitalWrite(AXIS_0_PIN_STEP, 1);
  }

  if ((targetSpeed_1 != 0)
      && ((now - last_step_time_1) > stepInterval_1)
      && (manual_control || ((now - last_update_time_1) < UPDATE_TIMEOUT))) {
    last_step_time_1 = now;
    digitalWrite(AXIS_1_PIN_STEP, 1);
  }

  delayMicroseconds(5);
  digitalWrite(AXIS_0_PIN_STEP, 0);
  digitalWrite(AXIS_1_PIN_STEP, 0);


  // Format is:
  // A+12345\n for channel 0
  // B+12345\n for channel 1
  if ((!manual_control) && Serial.available() >= 9) {
    bool changed = false;
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
      last_update_time_0 = micros();
      changed = true;
    }

    if (axis == 'B') {
      targetSpeed_1 = Serial.parseFloat();
      last_update_time_1 = micros();
      changed = true;
    }

    if (changed) {
      update_step_intervals();
      update_direction_pins();
    }

    digitalWrite(MONITOR_PIN, 0);
  }

  if (manual_control && ((now - last_manual_update_time) > MANUAL_UPDATE_INTERVAL)) {
    float manual_speed_max = map(analogRead(MANUAL_SPEED_PIN), 0, 1023, MANUAL_MIN_SPEED, MANUAL_MAX_SPEED);

    int axis_0_command = analogRead(MANUAL_CONTROL_AXIS_0_PIN);
    int axis_1_command = analogRead(MANUAL_CONTROL_AXIS_1_PIN);

    int axis_0_dev = abs(axis_0_command - 512);
    int axis_1_dev = abs(axis_1_command - 512);

    if (axis_0_dev > MANUAL_DEADBAND) {
      targetSpeed_0 = map(axis_0_dev, MANUAL_DEADBAND, 512, 0, manual_speed_max);
      if (axis_0_command < 512) {
        targetSpeed_0 = -1 * targetSpeed_0;
      }
      if (MANUAL_CONTROL_AXIS_0_INVERT) {
        targetSpeed_0 = -1 * targetSpeed_0;
      }
    } else {
      targetSpeed_0 = 0;
    }

    if (tracking) {
      targetSpeed_0 = targetSpeed_0 + TRACKING_SPEED;
    }

    if (axis_1_dev > MANUAL_DEADBAND) {
      targetSpeed_1 = map(axis_1_dev, MANUAL_DEADBAND, 512, 0, manual_speed_max);
      if (axis_1_command < 512) {
        targetSpeed_1 = -1 * targetSpeed_1;
      }
      if (MANUAL_CONTROL_AXIS_1_INVERT) {
        targetSpeed_1 = -1 * targetSpeed_1;
      }
    } else {
      targetSpeed_1 = 0;
    }

    update_step_intervals();
    update_direction_pins();

    last_manual_update_time = now;
  }
}


void update_step_intervals()
{
  if (targetSpeed_0 != 0) {
    stepInterval_0 = fabs(1000000.0 / targetSpeed_0);
  }
  if (targetSpeed_1 != 0) {
    stepInterval_1 = fabs(1000000.0 / targetSpeed_1);
  }
}


void update_direction_pins()
{
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
}
