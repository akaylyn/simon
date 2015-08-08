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
#define TOUCH_IRQ 3 // int.1, D3; but could move if we don't implement an interrupt
#define NUM_ELECTRODES 7
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
    boolean begin(byte sensorIndex[N_COLORS]);

    // state change checks
    boolean changed(color index); // returns true if state changed
    boolean anyChanged(); // convenience function; returns true if any index is changed

    // pressed state checks
    boolean pressed(color index); // returns true if pressed
    boolean anyColorPressed(); // convenience function; returns true if any index is pressed
    boolean anyButtonPressed();
    color whatPressed(); // returns the first pressed button found

    // returns "distance" an object is to the sensor
    byte distance(color index); // roughly speaking, the distance an object is away from the sensor
    byte proximity(); // 13th "virtual" sensor, which is the sum of all active sensors

    // debugging tool: prints sensor baseline and electrode information
    void printElectrodeAndBaselineData();
    
    // MGD new buttons
    boolean startPressed();
    boolean leftPressed();
    boolean rightPressed();

  private:
    // update function; returns true if there's a state change
    boolean update();

    // maps color index to MPR121 sensor index
    byte sensorIndex[N_COLORS];

    // hardware buttons
    Bounce *button[N_COLORS];  // messy, but I can't figure out how to declare without instantiation, which the compiler requires.

    // does the heavy lifting
    byte distance(byte index);

};

extern Touch touch;

// power transformation for nonlinear map() function
float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve);

#endif

