// This file should match src/Console/Touch.cpp

#include "Touch.h"

// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

#define MPR121_I2CADDR_DEFAULT 0x5A
#define DEBUG true

boolean touchStart() {
    // some reasonable defaults are set.
    // expect this to be called after startup
    // and again between games.
    Serial << F("Touch: startup.") << endl;

    // following Examples->BareConductive_MPR->SimpleTouch

    // 0x5C is the MPR121 I2C address on the Bare Touch Board
    if (!MPR121.begin(MPR121_I2CADDR_DEFAULT)) {
        Serial << F("Touch: error setting up MPR121");
        switch (MPR121.getError()) {
            case ADDRESS_UNKNOWN:
                Serial << F("Touch: incorrect address") << endl;
                return ( false );
                break;
            case READBACK_FAIL:
                Serial << F("readback failure") << endl;
                return ( false );
                break;
            case OVERCURRENT_FLAG:
                Serial << F("overcurrent on REXT pin") << endl;
                return ( false );
                break;
            case OUT_OF_RANGE:
                Serial << F("electrode out of range") << endl;
                return ( false );
                break;
            case NOT_INITED:
                Serial << F("not initialised") << endl;
                return ( false );
                break;
            default:
                Serial << F("unknown error") << endl;
                return ( false );
                break;
        }
    }
    else {
        Serial << F("Touch: no error") << endl;
        // set the interrupt handler.
        MPR121.setInterruptPin(TOUCH_IRQ);
        // initial data update
        touchCalibrate();
        MPR121.updateTouchData();
        return (true);
    }

}

// calibrates the Touch interface
void touchCalibrate() {}

// returns true if any of the buttons have switched states.
// avoid short-circuit eval so that each button gets an update
boolean touchAnyPressed() {
    return (
            touchPressed(I_RED) ||
            touchPressed(I_GRN) ||
            touchPressed(I_BLU) ||
            touchPressed(I_YEL)
           );
}

// returns true if any of the buttons are pressed.
boolean touchAnyChanged() {
    return (
            touchChanged(I_RED) ||
            touchChanged(I_GRN) ||
            touchChanged(I_BLU) ||
            touchChanged(I_YEL)
           );
}

// returns true WHILE a specific sensor IS PRESSED
// this function will be called after touchChanged() asserts a change.
boolean touchPressed(byte touchIndex) {
    if (touchIndex == I_ALL) return touchAnyPressed();

    MPR121.updateTouchData();
    return MPR121.getTouchData(touchIndex);
}

// Returns true if the state of a specific button has changed
// based on what it was previously.
boolean touchChanged(byte touchIndex) {
    if (touchIndex == I_ALL) return touchAnyChanged();

    MPR121.updateTouchData();
    static boolean previousState[N_COLORS];
    boolean currentState = MPR121.getTouchData(touchIndex);
    if (previousState[touchIndex] != currentState) {
        previousState[touchIndex] = currentState;
        return true;
    }
    return false;
}

