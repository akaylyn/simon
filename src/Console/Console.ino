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
//    setupMode();

    // when hands are near the project, act like a theramin
    proximityMode();
    delay(100); // slow the Serial spew
  }

}

// uses the MPR121 device to adjust lights and sound based on Player 1's proximity to sensors
void proximityMode() {
  // save maximum observed distance data, for calibration
  static int maxDist = touch.distance();
  // store the track number we're playing.
  static int trackNumber = -1;
    
  // some kind of proxy for distance
  int currDist = touch.distance();  // note that we're using the default "13th" sensor, which is the multiplexed aggregate of all of the sensors
                                    // but, you could ask for distances for each sensor, individually [0..3].
  
  // take a look
  Serial << F("Proximity: curr=") << currDist << F(" max=") << maxDist << endl;
  
  // define a treshold for distance. closer: make lights and sound; further: turn off lights and sound
  const int thresholdDist = 100; 
  
  if( currDist >= thresholdDist ) {
    // lights off.  immediate send.
    light.setAllLight(LIGHT_OFF, true);
    // quiet
    if( trackNumber > 0 ) sound.fadeTrack(trackNumber);
    trackNumber = -1; // clear.
  } else {
    // map distance to [0,255] for lights
    byte lightLevel = map(currDist, 0, thresholdDist, 255, 0); // closer is brighter
    // map distance to [-70,0] for sound
    int soundLevel = map(currDist, 0, thresholdDist, 0, -70); // closed is louder
    Serial << F("Proximity: light=") << lightLevel << F(" sound=") << soundLevel << endl;
  
    // lights on.  immediate send
    light.setAllLight(lightLevel, true);
    // sound
    if( trackNumber > 0 ) sound.setVol(trackNumber, soundLevel); // adjust volume, if we're already playing
    else trackNumber = sound.playFailTone(soundLevel); // if we're not playing, start playing
  }

}

// simply operate the Console in "bongoes" mode.  Will shoot fire.
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


