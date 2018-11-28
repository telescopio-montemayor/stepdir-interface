# Simple stepper motor drive.

This is a basic interface to command step/direction motor drivers.

## Hardware connections.

The default pinout is:

```
    AXIS_0_PIN_STEP             11
    AXIS_0_PIN_DIR              12
    AXIS_0_PIN_ENABLE           13

    AXIS_1_PIN_STEP             7
    AXIS_1_PIN_DIR              8
    AXIS_1_PIN_ENABLE           9
```

## Protocol.

The default serial speed is 57600 baud. 

The control message format is:

```
\n[AXIS][SPEED]\n
```

Where *AXIS* is one of A, B (uppercase) and speed is the desired steps per second for that axis.
Negative speeds toggle direction.

For example:

```
\nA12345\n
```

If the driver does not get an update from the host after 100 milliseconds that axis is stopped as a security measure.

## Compiling

We are using [PlatformIO](http://platformio.org/) but it works fine under the Arduino IDE too.
