// This file should match src/Console/Touch.cpp

#include "Touch.h"

// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

Adafruit_MPR121 cap = Adafruit_MPR121();

boolean touchStart(uint8_t touchCount, uint8_t releaseCount) {
  // some reasonable defaults are set.
  // expect this to be called after startup
  // and again between games.
  Serial << F("Touch: startup.") << endl;

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial << F("Touch: MPR121 not found, check wiring?") << endl;
    return( false );
  }
  Serial << F("Touch:MPR121 found!") << endl;
  Serial << F("Touch: count threshold set: ") << touchCount << F(". Release count threshold set: ") << releaseCount << F(".") << endl;
  cap.setThreshholds(touchCount, releaseCount);
  
  return( true );
}

// calibrates the Touch interface
void touchCalibrate() {
  // when this is called, assume that nothing is touching the
  // sensors, and you've got time to restart the capsense hardware,
  // take some readings, and assure that the sensors are set up
  // correctly.
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
      break;
    case I_GRN:
      break;
    case I_BLU:
      break;
    case I_YEL:
      break;
    case I_ALL:
      return ( touchAnyPressed() );
      break;
    default:
      Serial << F("Touch: touchPressed Error, case=") << touchIndex << endl;
  }
}


void touchUnitTest(boolean details, unsigned long timeout) {

  Metro unitTestTimeout(timeout);

  // Keeps track of the last pins touched
  // so we know when buttons are 'released'
  uint16_t lasttouched = 0;
  uint16_t currtouched = 0;

  while (! unitTestTimeout.check() ) {

    // Get the currently touched pads
    currtouched = cap.touched();

    for (uint8_t i = 0; i < 12; i++) {
      // it if *is* touched and *wasnt* touched before, alert!
      if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(F(" touched"));
      }
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(F(" released"));
      }
    }

    // reset our state
    lasttouched = currtouched;

    // comment out this line for detailed data from the sensor!
    if ( !details) break;

    // debugging info, what
    Serial.print(F("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x")); Serial.println(cap.touched(), HEX);
    Serial.print(F("Filt: "));
    for (uint8_t i = 0; i < 12; i++) {
      Serial.print(cap.filteredData(i)); Serial.print(F("\t"));
    }
    Serial.println();
    Serial.print(F("Base: "));
    for (uint8_t i = 0; i < 12; i++) {
      Serial.print(cap.baselineData(i)); Serial.print(F("\t"));
    }
    Serial.println();

    // put a delay so it isn't overwhelming
    delay(100);
  }
}
