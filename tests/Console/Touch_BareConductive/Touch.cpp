// This file should match src/Console/Touch.cpp

#include "Touch.h"

// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

#define MPR121_I2CADDR_DEFAULT 0x5A

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
void touchCalibrate() {
    // when this is called, assume that nothing is touching the
    // sensors, and you've got time to restart the capsense hardware,
    // take some readings, and assure that the sensors are set up
    // correctly.
    /*
    // debug code to make sure auto calibration was finishing successfully
    // the error detection code catches this
    Serial.println(MPR121.getRegister(OORS1), BIN);
    Serial.println(MPR121.getRegister(OORS2), BIN);
    */

    /* MPR121.h exposese autocalibration, but doesn't use them.  :(
       ACCR0(0x00),
       ACCR1(0x00),
       USL(0x00),
       LSL(0x00),
       TL(0x00)

       MPR121.applySettings() is probably what we want to call to set these correctly.  setRegister would do
       the trick, too.
       */

}

// returns true if any of the buttons have switched states.
boolean touchAnyChanged() {
    // avoid short-circuit eval so that each button gets an update
    boolean redC = touchChanged(I_RED);
    boolean grnC = touchChanged(I_GRN);
    boolean bluC = touchChanged(I_BLU);
    boolean yelC = touchChanged(I_YEL);
    return ( redC || grnC || bluC || yelC );
}

// returns true if any of the buttons are pressed.
boolean touchAnyPressed() {
    return (
            touchPressed(I_RED) ||
            touchPressed(I_GRN) ||
            touchPressed(I_BLU) ||
            touchPressed(I_YEL)
           );
}

// returns true if a specific button has changed
boolean touchChanged(byte touchIndex) {
    /* this function will be called very frequently, so this is where the capsense
       calls should be made and the debouncing routines should reside
       */

    switch ( touchIndex ) {
        case I_RED:
            break;
        case I_GRN:
            break;
        case I_BLU:
            break;
        case I_YEL:
            break;
        case I_ALL:
            return ( touchAnyChanged() );
            break;
        default:
            Serial << F("Touch: touchChanged Error, case=") << touchIndex << endl;
    }
}

// returns true if a specific sensor is pressed
boolean touchPressed(byte touchIndex) {
    // this function will be called after touchChanged() asserts a change.

    switch ( touchIndex ) {
        case I_RED:
            Serial.println("RED!");
            break;
        case I_GRN:
            Serial.println("GREEN!");
            break;
        case I_BLU:
            Serial.println("BLUE!");
            break;
        case I_YEL:
            Serial.println("YELLOW!");
            break;
        case I_ALL:
            return ( touchAnyPressed() );
            break;
        default:
            Serial << F("Touch: touchPressed Error, case=") << touchIndex << endl;
    }
}


void touchUnitTest(unsigned long timeout) {
    boolean DEBUG = false;

    Metro unitTestTimeout(timeout);
    while (! unitTestTimeout.check() ) {

        if (MPR121.touchStatusChanged()) {
            MPR121.updateTouchData();
            for (int i = 0; i < numElectrodes; i++) {
                if (MPR121.isNewTouch(i)) {
                    if (DEBUG) {
                        Serial.print("electrode ");
                        Serial.print(i, DEC);
                        Serial.println(" was just touched");
                    }
                    touchPressed(i);
                } else if (MPR121.isNewRelease(i)) {
                    if (DEBUG) {
                        Serial.print("electrode ");
                        Serial.print(i, DEC);
                        Serial.println(" was just released");
                    }
                    touchChanged(i);
                }
            }
        }
    }
}
