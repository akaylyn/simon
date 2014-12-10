// capsense touch: MPR121 include I2C and MPR121 libraries.

#ifndef Touch_h
#define Touch_h

#include "Pinouts.h"
#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <Bounce.h> // button debounce
#include <LED.h> // lights
#include <Wire.h>
#include <Adafruit_MPR121.h>

#include <Simon_Comms.h>

// debounce the hardware buttons on the console (NOT the touch sensors)
#define BUTTON_DEBOUNCE_TIME 10UL // ms

// note that the buttons are wired to ground with a pullup resistor.
#define PRESSED_BUTTON LOW 

// 
void touchStart(uint8_t touchCount, uint8_t releaseCount);
void touchStart();

// unit test for touch sensor
void touchUnitTest(boolean details, unsigned long timeout=10000);

// configures buttons at startup
void configureManualButtons();
// configures lights for buttons at startup
void configureManualLights();
// configures speaker at startup

// manual light handlers
void manualLightSet(byte lightIndex, byte lightLevel);

// manual button handler
boolean buttonChanged(byte lightIndex);
boolean buttonPressed(byte lightIndex);

#endif
