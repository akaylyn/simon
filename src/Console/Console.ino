// Compile for Arduino Megai 2560.

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
#include <EEPROM.h> // saving and loading radio settings
#include <SPI.h> // radio transmitter is a SPI device
#include <RFM12B.h> // RFM12b radio transmitter module
#include <Simon_Comms.h> // comms between Towers and Console

// Music subunit.  Responsible for UX (sound) output.
#include "Music.h"

// should Unit Tests be run if the startup routines return an error?
#define RUN_UNIT_ON_ERROR false

// remote control
#define SYSTEM_ENABLE_PIN 24
#define FIRE_ENABLE_PIN 26
Bounce systemEnable = Bounce(SYSTEM_ENABLE_PIN, BUTTON_DEBOUNCE_TIME);
Bounce fireEnable = Bounce(FIRE_ENABLE_PIN, BUTTON_DEBOUNCE_TIME);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(0));

  // remote control
  pinMode(SYSTEM_ENABLE_PIN, INPUT_PULLUP);
  pinMode(FIRE_ENABLE_PIN, INPUT_PULLUP);

  // start each unit
  //------ Input units.
  touchStart();
  if ( !buttonStart() && RUN_UNIT_ON_ERROR || 0) buttonUnitTest();
  //------ Output units.
  lightStart();
  towerStart();
  if ( !musicStart() && RUN_UNIT_ON_ERROR || 0) musicUnitTest();
  //------ "This" units.
  gameplayStart();
  externStart();

  Serial << F("STARTUP: complete.") << endl;
}

// main loop for the core.
void loop() {
  // remote control.  There's a relay that will pull FIRE_ENABLE_PIN to LOW when pressed (enable fire).
  // goes to HIGH when pressed again (disable fire).
  // on the Towers, this same relay will physically prevent the accumulator solenoid from opening,
  // so this is really a "FYI" for the Console.  We'll use that to make noise over the FM transmitter
  // to let the Operator know what's up.
  if ( fireEnable.update() ) {
    Serial << "Fire status change" << endl;
    // fire enable/disable state has changed.
    int freq;
    if ( fireEnable.read() == HIGH ) {
      // fire is disabled.  make three "cheeps"
      freq = 1000; // cheeps
      Serial << "Fire Disabled" << endl;
    } else {
      // fire is ENABLED.  make three "klaxons"
      freq = 100; // boops
      Serial << "Fire Enabled" << endl;
    }
    // this could be replaced by asking Music to play an mp3 file.  For now, we'll just use the tone system.
    for ( int i = 0; i < 3; i++ ) {
      tone( SPEAKER_WIRE, freq ); // 500 Hz tone
      delay( 250UL ); // wait 0.25 seconds
      noTone( SPEAKER_WIRE );
      delay( 100UL ); // wait 0.1 seconds
    }
  }

  // remote control.  There's a relay that will pull SYSTEM_ENABLE_PIN to LOW when pressed (enable gameplay).
  // goes to HIGH when pressed again (disable gameplay).
  static boolean systemNormalMode = true;
  if( systemEnable.update() ) {
    // system enable/disable state has changed.
    // TODO: change this to LOW when it is properly wired up
    systemNormalMode = systemEnable.read() == HIGH;
  }
  if( systemNormalMode ) {
    // play the Simon game; returns true if we're playing
    if ( ! gameplayUpdate() ) {
      // we're not playing, so check for Extern traffic; returns true if we have traffic.
      if ( ! externUpdate() ) {
        // no Extern traffic, so perform Tower maintenance and idle displays.
        towerUpdate();
      }
    }
    //touchUnitTest(50UL);
  } else {
    // assume we're setting up the project on-site, so this is a good time to run unit tests, calibration activities, etc.
    // maybe cycle the lights with commsSend(inst).
    // maybe when a button is pressed, send the colors out and make some fire (drum machine mode?)
    // maybe when buttons are pressed in a certain way, change the play mode and/or difficulty "level"?
    // maybe when buttons are pressed in a certain way, change what towers respond to what indexes?
  }
}



/* possible IRQ pins (for attachInterrupt):
  pin 2 (IRQ 0) taken by RFM12b
  pin 3 (IRQ 1) taken by VS1023
  pin 21 (IRQ 2) taken by MPR121 (Wire library!)
  pin 20 (IRQ 3) taken by MPR121 (Wire library!)
  pin 19 (IRQ 4) reserved for MPR121
  pin 18 (IRQ 5)
*/

