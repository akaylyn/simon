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
#include <EEPROM.h> // saving and loading radio settings
#include <SPI.h> // radio transmitter is a SPI device
#include <RFM12B.h> // RFM12b radio transmitter module
#include <Simon_Comms.h> // comms between Towers and Console

// Music subunit.  Responsible for UX (sound) output.
#include "Music.h"

// should Unit Tests be run if the startup routines return an error?
#define RUN_UNIT_ON_ERROR false

// remote control
#define GAME_ENABLE_PIN 45
#define FIRE_ENABLE_PIN 47
Bounce gameEnable = Bounce(GAME_ENABLE_PIN, BUTTON_DEBOUNCE_TIME);
Bounce fireEnable = Bounce(FIRE_ENABLE_PIN, BUTTON_DEBOUNCE_TIME);

extern towerConfiguration config[N_TOWERS];
extern byte towerFire[N_TOWERS];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(0));

  // remote control
  pinMode(GAME_ENABLE_PIN, INPUT_PULLUP);
  pinMode(FIRE_ENABLE_PIN, INPUT_PULLUP);

  // start each unit
  //------ Input units.
  if ( !touchStart() && RUN_UNIT_ON_ERROR || 0) touchUnitTest();
  if ( !buttonStart() && RUN_UNIT_ON_ERROR || 0) buttonUnitTest();
  //------ Output units.
  lightStart();
  towerStart();
  if ( !musicStart() && RUN_UNIT_ON_ERROR || 0) musicUnitTest();
  //------ "This" units.
  gameplayStart();
  externStart();

commsPrint(config[0], 2);
  Serial << F("STARTUP: complete.") << endl;

  

}

// main loop for the core.
void loop() {
  if( towerFire[0]!=I_ALL ) {
    Serial << "! towerFire[0] changed from I_ALL !" << endl;
    configureNetwork();
    while(1);
  }
  if( config[0].fireIndex!=I_ALL ) {
    
    Serial << "! config[0].fireIndex changed from I_ALL !" << endl;
    commsPrint(config[0], 2);
    commsPrint(config[1], 3);
    commsPrint(config[2], 4);
    commsPrint(config[3], 5);
    while(1);
  }

  // remote control.  There's a relay that will pull FIRE_ENABLE_PIN to LOW when pressed (enable fire).
  // goes to HIGH when pressed again (disable fire).
  // on the Towers, this same relay will physically prevent the accumulator solenoid from opening,
  // so this is really a "FYI" for the Console.  We'll use that to make noise over the FM transmitter
  // to let the Operator know what's up.
  static boolean fireMode = fireEnabled();
  if ( fireEnable.update() ) {
    // fire enable/disable state has changed.
    Serial << "Fire Enable pin change!" << endl;
    fireMode = fireEnabled();
    int freq;
    if ( fireMode ) {
      Serial << "Fire ENABLED!" << endl;
      // fire is ENABLED.  make three "klaxons"
      freq = 100; // boops
    } else {
      Serial << "Fire disabled!" << endl;
      // fire is disabled.  make three "cheeps"
      freq = 500; // cheeps

    }
    // this could be replaced by asking Music to play an mp3 file.  For now, we'll just use the tone system.
    for ( int i = 0; i < 3; i++ ) {
      tone( SPEAKER_WIRE, freq ); // 500 Hz tone
      delay( 250UL ); // wait 0.25 seconds
      noTone( SPEAKER_WIRE );
      delay( 100UL ); // wait 0.1 seconds
    }
  }

  // remote control.  There's a relay that will pull GAME_ENABLE_PIN to LOW when pressed (disable gameplay).
  // goes to HIGH when pressed again (enable gameplay).
  static boolean gamePlayMode = gameEnabled();
  if ( gameEnable.update() ) {
    Serial << "Game Enable pin change!" << endl;
    // system enable/disable state has changed.
    gamePlayMode = gameEnabled();
    if( gamePlayMode ) {
      Serial << "Game enabled!" << endl;
    } else {
      Serial << "Game DISABLED!" << endl;
    }
  }
  if ( gamePlayMode ) {
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
    // when a button is pressed, send the colors out and make some fire (drum machine mode?)  

    if( touchAnyChanged() || buttonAnyChanged() ) {

      // this is where the lights and fire instructions to Towers are placed
      extern towerInstruction inst;    
      // clear out instructions
      commsDefault(inst);
          
     // if anything's pressed, pack the instructions 
      byte index = I_NONE;
      if( touchPressed(I_RED) || buttonPressed(I_RED) ) index = I_RED;
      if( touchPressed(I_GRN) || buttonPressed(I_GRN) ) index = I_GRN;
      if( touchPressed(I_BLU) || buttonPressed(I_BLU) ) index = I_BLU;
      if( touchPressed(I_YEL) || buttonPressed(I_YEL) ) index = I_YEL;
        
      // Lights on Console
      // Sound on Console and Tower
      // Light on Towers
      // Fire on Towers
      if( index == I_NONE ) {
        Serial << F("\tReleased.") << endl;
        lightStart();
        musicStop();
        // no Tower instructions needed.  commsDefault zeros it out, but let's be pedantic

      } else  {
        Serial << F("Pressed: ") << index;
        lightSet(index, LIGHT_ON);  
        musicTone(index);
        inst.lightLevel[index] = 255;
        if( fireMode ) inst.fireLevel[index] = 255;
      }
      
      // send to Towers
      towerSend(0); // no resend

    } else {
      
      towerUpdate();
      
    }
  }
  

}

boolean fireEnabled() {
  // at system power up, relay is open, meaning pin will read HIGH.
  return ( fireEnable.read() == LOW );
}

boolean gameEnabled() {
  // at system power up, relay is open, meaning pin will read HIGH.
  return ( gameEnable.read() == HIGH );
}

/* possible IRQ pins (for attachInterrupt):
  pin 2 (IRQ 0) taken by RFM12b
  pin 3 (IRQ 1) taken by VS1023
  pin 21 (IRQ 2) taken by MPR121 (Wire library!)
  pin 20 (IRQ 3) taken by MPR121 (Wire library!)
  pin 19 (IRQ 4) reserved for MPR121
  pin 18 (IRQ 5)
*/

