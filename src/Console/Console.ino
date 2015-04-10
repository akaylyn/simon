// Compile for Arduino Mega 2560.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <MPR121.h> // MPR121 capsense board
#include <Wire.h> // capsense is an I2C device
#include <Bounce.h> // with debounce routine.
#include <FiniteStateMachine.h> // using a FSM to run the game
#include <LED.h> // led control abstraction
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module
#include <EasyTransfer.h> // used for sending message to the sound module
#include <wavTrigger.h> // sound board

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>
#include <SoundMessage.h> // contains the share message for EasyTransfer

//------ Input units.
#include "Touch.h" // Touch subunit. Responsible for UX input.
#include "Mic.h" // Microphone
#include "Sensor.h" // Sensor subunit.  Responsible for game and fire enable

//------ "This" units.
#include "Gameplay.h" // Game Play subunit.  Responsible for Simon game.
#include "Extern.h" // Extern subunit.  Responsible for interfacing with other projects via RFM12b.

//------ Output units.
#include "Light.h" // Light subunit.  Responsible for UX output local Console (light) and remote Towers (light/fire)
#include "Sound.h" // Sound subunit.  Responsible for UX (music) output.

// should Unit Tests be run if the startup routines return an error?
#define RUN_UNIT_ON_ERROR false

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(A5));

  // start each unit
  //------ Input units.
  touch.begin();
  sensor.begin();
  mic.begin();
  //------ Output units.
  light.begin(); // moved this up to the front, as synchronization with Light is apparently important.
  if ( !sound.begin() && RUN_UNIT_ON_ERROR || 0) sound.unitTest();
  //------ "This" units.
  gameplayStart(sound);
  externStart();

  Serial << F("STARTUP: complete.") << endl;
}

// main loop for the core.
void loop() {
  boolean fireMode = sensor.fireEnabled();
  boolean gamePlayMode = sensor.gameEnabled();

  if ( gamePlayMode ) {
    // play the Simon game; returns true if we're playing
    if ( ! gameplayUpdate() ) {
      // we're not playing, so check for Extern traffic; returns true if we have traffic.
      if ( ! externUpdate() ) {
        // no Extern traffic, so perform Tower maintenance and idle displays.
        light.update();
      }
    }
    //touchUnitTest(50UL);
  }
  else {
    // assume we're setting up the project on-site, so this is a good time to run unit tests, calibration activities, etc.
    // when a button is pressed, send the colors out and make some fire (drum machine mode?)
    setupMode();
  }

}

void setupMode() {
  if ( touch.anyChanged()) {
    // if anything's pressed, pack the instructions
    byte tones = 5;
    for ( byte i = 0; i < N_COLORS; i++ ) {
      if ( touch.pressed(i) ) {
        tones = i; // LIFO.  can't do multiple tones... yet.
        light.setLight(i, LIGHT_ON);
        light.setFire(i, LIGHT_ON);
      } else {
        light.setLight(i, LIGHT_OFF);
        light.setFire(i, LIGHT_OFF);
      }
    }
    // maybe a tone
    if ( tones < 5 ) {
      sound.playTone(tones);
    } else {
      sound.stopAllTracks();
    }
    // show
    light.show();
  } else {
    // maybe resend
    light.update();
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


