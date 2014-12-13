#include "Touch.h"

// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

Adafruit_MPR121 cap = Adafruit_MPR121();

// restarts the Touch interface
void touchStart() {
   // some reasonable defaults are set.
   // expect this to be called after startup
   // and again between games.
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
   // for the debounce code.
   boolean redC = touchChanged(I_RED);
   boolean grnC = touchChanged(I_GRN);
   boolean bluC = touchChanged(I_BLU);
   boolean yelC = touchChanged(I_YEL);
   return( redC || grnC || bluC || yelC ); 
}

// returns true if any of the buttons are pressed.
boolean touchAnyPressed() {
   return( 
      touchPressed(I_RED) || 
      touchPressed(I_GRN) || 
      touchPressed(I_BLU) || 
      touchPressed(I_YEL) 
   );
}

// returns true if a specific button has changed
boolean touchChanged(byte buttonIndex) {
   /* this function will be called very frequently, so this is where the capsense
      calls should be made and the debouncing routines should reside
   */
   
   // for now
   return( false );
}

// returns true if a specific button is pressed
boolean touchPressed(byte buttonIndex) {
   // this function will be called after touchChanged() asserts a change.
   
   // for now
   return( false );
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

