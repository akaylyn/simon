#include "TouchTests.h"

extern Touch touch;

/*
 * Touched should return true when each sensor state changes
 */
void TouchTests::testChanged(int electrode) {
  if (touch.changed(electrode)) {
    Serial << "elecrode " << electrode << " changed state" << endl;
  }
}

/*
 * Pressed should return true when a sensor has been touched and then released
 */
void TouchTests::testPressed(int electrode) {
  if (touch.pressed(electrode)) {
    Serial << "elecrode " << electrode << " is pressed" << endl;
  }
}

void TouchTests::testAnyColorPressed() {
  Serial << "anyColorPressed: " << touch.anyColorPressed() << endl;
}

void TouchTests::testAnyButtonPressed() {
  Serial << "anyButtonPressed: " << touch.anyButtonPressed() << endl;
}

void TouchTests::testWhatPressed() {
  Serial << "whatPressed: " << touch.whatPressed() << endl;
}

void TouchTests::testBongoMode() {
  boolean DEBUG = true;
  Metro unitTestTimeout(TIMEOUT);
  while (! unitTestTimeout.check() ) {
    if (touch.anyChanged()) {
      if (touch.anyButtonPressed()) {
        color pressed = touch.whatPressed();
        Serial << "pressed: " << pressed << endl;
      }
    }
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
          touch.pressed(i);
        } else if (MPR121.isNewRelease(i)) {
          if (DEBUG) {
            Serial.print("electrode ");
            Serial.print(i, DEC);
            Serial.println(" was just released");
          }
          touch.changed(i);
        }
      }
    }
  }
}
