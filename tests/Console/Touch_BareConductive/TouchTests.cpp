#include "TouchTests.h"

/*
 * Touched should return true when each sensor state changes
 */
void TouchTests::testChanged(int electrode) {
    if (touchChanged(electrode)) {
        Serial << "elecrode " << electrode << " changed state" << endl;
    }
}

/*
 * Pressed should return true when a sensor has been touched and then released
 */
void TouchTests::testPressed(int electrode) {
    if (touchPressed(electrode)) {
        Serial << "elecrode " << electrode << " is pressed" << endl;
    }
}

void TouchTests::touchUnitTest(unsigned long timeout) {
    boolean DEBUG = true;
    Metro unitTestTimeout(timeout);
    while (! unitTestTimeout.check() ) {

        if (MPR121.touchStatusChanged()) {
            MPR121.updateTouchData();
            for (int i = 0; i < NUM_ELECTRODES; i++) {
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
