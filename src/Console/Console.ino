// Compile for Arduino Mega.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

//----- Wiring connections.  
// Could be pulled out to individual subunit .h files, which would add clarity
// to the subunit sections, at the cost of not having a concise list of pins in one place.  
#include "Pinouts.h"

//------ Input units.

// Touch subunit. Responsible for UX input.
#include "Touch.h"
// capsense touch: soft capsense buttons
#include <Adafruit_MPR121.h> // MPR121 capsense board
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
#include <Simon_Comms.h> // sizes, indexing and comms between Towers and Console
#include <RFM12B.h> // RFM12b radio transmitter module
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings

// Music subunit.  Responsible for UX (sound) output.
#include "Music.h" 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(0));

  // start each unit
  //------ Input units.
  touchStart();
  buttonStart();
  //------ Output units.
  lightStart();
  towerStart();
  musicStart();
  //------ "This" units.
  gameplayStart();
  externStart();

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
}


