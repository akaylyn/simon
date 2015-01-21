// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
// wiring connections
#include "Pinouts.h"
// capsense touch: MPR121 include I2C, MPR121 libraries.
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <Bounce.h> // manual buttons as well
#include <LED.h> // manual lights as well
#include <Simon_Comms.h>
#include "Touch.h"

void setup() {
  // put your setup code here, to run once:
  // attach interrupt to pin - interrupt 1 is on pin 2 of the arduino (confusing I know)
  attachInterrupt(0, dataAvailable, FALLING);
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));

  setupCapacitiveRegisters();
}

// main loop for the core.
void loop() {
    //touchUnitTest(false, 5000);
    readCapacitiveSensor();
}
