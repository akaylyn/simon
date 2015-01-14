// Compile for Arduino Mega.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

//------ Input units.

// Touch subunit. Responsible for UX input.
#include "Touch.h"
// capsense touch: soft capsense buttons
#include <MPR121.h> // MPR121 capsense board
#include <Wire.h> // capsense is an I2C device

// Button subunit.  Responsible for hard buttons on a PCB.
#include "Button.h"
#include <Bounce.h> // with debounce routine.

//------ "This" units.

// Game Play subunit.  Responsible for Simon game.
#include "Gameplay.h"
#include <FiniteStateMachine.h> // using a FSM to run the game

// Extern subunit.  Responsible for interfacing with other projects via RFM12b.
#include "Extern.h"

//------ Output units.

// Light subunit.  Responsible for UX (light) output local to the console.
#include "Light.h"
// Three sets of lights:
// WS2812 lights: lights wrapping the console rim and capsense touch buttons
// manual lights: hard LEDs on a PCB
#include <LED.h> // with LED abstraction

// Tower subunit.  Responsible for UX (light/fire) output at the Tower.
#include "Tower.h"
#include <Simon_Indexes.h> // sizes, indexing and
#include <Simon_Comms.h> // comms between Towers and Console
#include <RFM12B.h> // RFM12b radio transmitter module
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings

// Music subunit.  Responsible for UX (sound) output.
#include "Music.h"

// should Unit Tests be run if the startup routines return an error?
#define RUN_UNIT_ON_ERROR false

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(0));

  // start each unit
  //------ Input units.
  touchStart();
  if( !buttonStart() && RUN_UNIT_ON_ERROR || 0) buttonUnitTest();
  //------ Output units.
  lightStart();
  towerStart();
  if( !musicStart() && RUN_UNIT_ON_ERROR || 0) musicUnitTest();
  //------ "This" units.
  gameplayStart();
  externStart();

  Serial << F("STARTUP: complete.") << endl;
}

// main loop for the core.
void loop() {
  // play the Simon game; returns true if we're playing
  if( ! gameplayUpdate() ) {
    // we're not playing, so check for Extern traffic; returns true if we have traffic.
    if( ! externUpdate() ) {
      // no Extern traffic, so perform Tower maintenance and idle displays.
      towerUpdate();
    }
  }
  //touchUnitTest(50UL);
}

/* possible IRQ pins (for attachInterrupt):
  pin 2 (IRQ 0) taken by RFM12b
  pin 3 (IRQ 1) taken by VS1023
  pin 21 (IRQ 2) taken by MPR121 (Wire library!)
  pin 20 (IRQ 3) taken by MPR121 (Wire library!)
  pin 19 (IRQ 4) reserved for MPR121
  pin 18 (IRQ 5)
*/

