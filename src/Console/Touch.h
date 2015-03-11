/*
 * Touch subunit. Responsible for UX input (buttons and touch sensors).
 *
 */

#ifndef Touch_h
#define Touch_h

#include <Arduino.h>

//----- capsense touch: soft capsense buttons
#include <MPR121.h> // MPR121 capsense board
// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

#include <Wire.h> // capsense is an I2C device
// Wire library requirements: http://arduino.cc/en/Reference/Wire
#define TOUCH_SCL 21 // Wire SCL
#define TOUCH_SDA 20 // Wire SDA
#define TOUCH_IRQ 19 // IRQ 4, but could move if we don't implement an interrupt
#define NUM_ELECTRODES 4
#define MPR121_I2CADDR_DEFAULT 0x5A

//----- manual buttons: hard buttons on a PCB.
#include <Bounce.h>
// debounce the hardware buttons on the console
#define BUTTON_DEBOUNCE_TIME 50UL // ms

// note that the buttons are wired to ground with a pullup resistor.
#define PRESSED_BUTTON LOW
// Manual Buttons
// button switches.  wire to Arduino (INPUT_PULLUP) and GND.
#define BUTTON_YEL 4 // can move, digital
#define BUTTON_GRN 5 // can move, digital
#define BUTTON_BLU 6 // can move, digital
#define BUTTON_RED 7 // can move, digital

#include <Streaming.h> // <<-style printing

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class Touch {
  public:
    // intialization; returns true if ok.
    boolean begin();

    // state change checks
    boolean changed(byte index); // returns true if state changed
    boolean anyChanged(); // convenience function; returns true if any index is changed

    // pressed state checks
    boolean pressed(byte index); // returns true if pressed
    boolean anyPressed(); // convenience function; returns true if any index is pressed

    // proximity/distance sensor
    // returns "distance" an object is to the sensor
    int distance(byte touchIndex = 12); // defaults to 13th "virtual" sensor.

    // unit test
    void unitTest();

  private:
    // update function; returns true if there's a state change
    boolean update();

    // hardware buttons
    Bounce *button[N_COLORS];  // messy, but I can't figure out how to declare without instantiation, which the compiler requires.

};

extern Touch touch;

// power transformation for nonlinear map() function
float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve);

#endif

