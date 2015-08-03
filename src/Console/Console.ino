// Compile for Arduino Mega 2560.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <MPR121.h> // MPR121 capsense board
#include <Wire.h> // capsense is an I2C device
#include <Bounce.h> // with debounce routine.
#include <EEPROM.h> // saving
#include <FiniteStateMachine.h> // using a FSM to run the game
#include <LED.h> // led control abstraction
#include <SPI.h> // radio transmitter is a SPI device
#include <RFM12B.h> // RFM12b radio transmitter module
#include <EEPROM.h> // saving settings
#include <EasyTransfer.h> // used for sending message to the sound module
#include <wavTrigger.h> // sound board
#include <LiquidCrystal_I2C.h> // LCD
#include <avr/pgmspace.h> // PROGMEM
#include <phi_super_font.h> // for really big characters

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

//------ Communications units
#include "Network.h" // runs the network

//------ Input units.
#include "Touch.h" // Touch subunit. Responsible for UX input.
#include "Mic.h" // Microphone
#include "Sensor.h" // Sensor subunit.  Responsible for game and fire enable

//------ "This" units.
#include "Simon.h" // Game Play subunit.  Responsible for Simon game.
#include "Tests.h"

//------ Output units.
#include "Fire.h" // Fire subunit.  Responsible for UX output on remote Towers (fire)
#include "Light.h" // Light subunit.  Responsible for UX output local Console (light) and remote Towers (light/fire)
#include "Sound.h" // Sound subunit.  Responsible for UX (music) output.

//------ LCD Scoreboard
#include "SimonScoreboard.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

//  Serial << F("SETUP: pausing 500ms after REBOOT") << endl;
//  delay(500);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(A5));

  // start each unit

  //------ Input units.
  byte touchMapToColor[N_COLORS] = {I_RED, I_GRN, I_BLU, I_YEL};
  touch.begin(touchMapToColor);
  sensor.begin();
  mic.begin();

  //------ Network
  network.begin();

  //------ Output units.
  fire.begin(); //
  light.begin(); //
  sound.begin(); //
  scoreboard.begin();

  Serial << F("Network: free RAM: ") << freeRam() << endl;

  Serial << F("STARTUP: complete.") << endl;
}

// main loop for the core.
void loop() {
  // calls the FSM to handle the state of the system
  simon.update();

  // perform Tower resends; you should do this always if you want meaningful synchronization with Towers
  network.update();

  // MGD new buttons
  if( touch.startPressed() ) Serial << F("Touch: start pressed") << endl;
  if( touch.leftPressed() ) Serial << F("Touch: left pressed") << endl;
  if( touch.rightPressed() ) Serial << F("Touch: right pressed") << endl;

}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

