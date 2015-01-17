// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <Simon_Indexes.h> // indexing.
// capsense touch: MPR121 include I2C, MPR121 libraries from Bare Conductive.
#include <Wire.h>
#include <MPR121.h> // from: https://github.com/BareConductive/mpr121
#include <Bounce.h> // manual buttons as well
#include <LED.h> // manual lights as well
#include "TouchTests.h"
#include "Touch.h" // this

// should Unit Tests be run if the startup routines return an error?
#define RUN_UNIT_ON_ERROR true

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));

  //if( !touchStart() && RUN_UNIT_ON_ERROR || 0) touchUnitTest();
  touchStart();

}

// main loop for the core.
void loop() {
    TouchTests tests;

    Metro timeout(tests.TIMEOUT);
    while (!timeout.check()) {
        for (int i = 0; i < NUM_ELECTRODES; i++) {

            tests.testChanged(i);
            tests.testPressed(i);
        }
    }
}
