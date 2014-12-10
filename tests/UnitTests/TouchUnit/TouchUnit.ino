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
#include "Touch.h"
/*
#include "Music.h"
// communications: RFM12b
// sizes, indexing and comms between Towers and Console
#include <EEPROM.h> // saving and loading radio settings
#include <SPI.h>
#include <RFM12B.h> // RFM12b radio transmitter module
#include <Simon_Comms.h>
#include "Comms.h"
// game play
#include <FiniteStateMachine.h>
#include "Gameplay.h"
*/

// this is where the lights and fire instructions to Towers are placed
//towerInstruction inst;
// from Simon_Comms.h:
// typedef struct {
//	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
//	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
// } towerInstruction;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));
}

// main loop for the core.
void loop() {
    touchUnitTest(false, 5000);
}
