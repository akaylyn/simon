// Touch subunit. Responsible for UX input.

#ifndef Touch_h
#define Touch_h

#include "Pinouts.h"
#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <Simon_Comms.h> // sizes, indexing defines.

//----- capsense touch: soft capsense buttons
#include <Adafruit_MPR121.h> // MPR121 capsense board
#include <Wire.h> // capsense is an I2C device

// restarts the Touch interface
void touchStart();

// calibrates the Touch interface
void touchCalibrate();

// returns true if any of the buttons have switched states.
boolean touchAnyChanged();

// returns true if any of the buttons are pressed.
boolean touchAnyPressed();

// returns true if a specific button has changed
boolean touchChanged(byte buttonIndex);

// returns true if a specific button is pressed
boolean touchPressed(byte buttonIndex);
  
// unit test for touch sensor
void touchUnitTest(boolean details, unsigned long timeout=10000);

#endif
