#include "Touch.h"

// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf 

Adafruit_MPR121 cap = Adafruit_MPR121();

void touchStart(uint8_t touchCount, uint8_t releaseCount) {
  Serial << F("Startup Adafruit MPR121 Capacitive Touch sensor.") << endl;

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial << F("MPR121 not found, check wiring?") << endl;
    while (1);
  }
  Serial << F("MPR121 found!") << endl;
  Serial << F("Touch count threshold set: ") << touchCount << F(". Release count threshold set: ") << releaseCount << F(".") << endl;
  cap.setThreshholds(touchCount, releaseCount);
}
void touchStart() {
  touchStart(12, 6); // take defaults from AdaFruit example
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
