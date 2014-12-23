// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <Simon_Indexes.h> // indexing.
// capsense touch: MPR121 include I2C, MPR121 libraries from Adafruit.
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <Bounce.h> // manual buttons as well
#include <LED.h> // manual lights as well
#include "Touch.h" // this

// should Unit Tests be run if the startup routines return an error?
#define RUN_UNIT_ON_ERROR true

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));
  
  if( !touchStart() && RUN_UNIT_ON_ERROR || 0) touchUnitTest(false, 5000);

}

// main loop for the core.
void loop() {
    touchUnitTest(false, 5000);
}
