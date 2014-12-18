// Touch subunit. Responsible for UX input.

#ifndef Touch_h
#define Touch_h

#include <Arduino.h>

// MPR121 touch:
/* AdaFruit MPR121 capsense touch:
  See: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
  
  Vin -> 5V
  3Vo -> (unused, but a regulated 3.3 source if we need it)
  GND -> GND
  SCL -> Uno A5 (Mega 21)
  SDA -> Uno A4 (Mega 20)
  IRQ -> (unused, but could be added to generate an interrupt on touch and released
  ADDR -> 

    ADDR not connected: 0x5A
    ADDR tied to 3V: 0x5B
    ADDR tied to SDA: 0x5C
    ADDR tied to SCL: 0x5D

*/
// Wire library requirements: http://arduino.cc/en/Reference/Wire
#define TOUCH_SCL 21 // Wire SCL
#define TOUCH_SDA 20 // Wire SDA
#define TOUCH_IRQ 19 // IRQ 4, but could move if we don't implement an interrupt

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
