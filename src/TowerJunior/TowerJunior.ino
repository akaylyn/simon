// Compile for Uno.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Bounce.h> // remote relays
#include <Metro.h> // countdown timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// handle incoming instructions and idle patterns
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module
#include <EEPROM.h> // saving and loading radio settings
// boostrap the tower nodeID to this value (2,3,4,5), overwriting EEPROM.
// set "BROADCAST" to read EEPROM value
#define HARD_SET_NODE_ID_TO JUNIOR
#include "Instruction.h"

// perform lighting
#include <DmxSimple.h>

#include "Light.h"
// pin locations for outputs
#define PIN_DMX 3 // the pin that drives the DMX system
#define PIN_LED 9 // the pin connected to the onboard led

// perform fire
#include <Timer.h> // interval timers
#include "Fire.h"

// instantiate
Instruction instruction;
Light light;
Fire fire;

// without comms for this duration, run a lighting test pattern
#define IDLE_PERIOD 10000UL // ms

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial << F("Setup: begin") << endl;

  Serial << F("Setup: pausing after boot 1 sec...") << endl;
  delay(1000UL);

  // startup
  instruction.begin(HARD_SET_NODE_ID_TO);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_DMX, OUTPUT);
  DmxSimple.usePin(PIN_DMX);

  light.begin();
  fire.begin();
  
  // random seed.
  randomSeed(analogRead(A3)); // or some other unconected pin

  Serial << F("Setup: free RAM: ") << freeRam() << endl;

  Serial << F("Setup: complete") << endl;
}

void loop() {
  // SAFETY: do not move this code after any other code.
  // check to see if we need to mess with the fire
  fire.update();
  // check to see if we need to mess with the lights.
  light.update();

  // a place to store instructions
  static colorInstruction lastColorInst, newColorInst;
  static fireInstruction lastFireInst, newFireInst;
  static systemMode lastMode, newMode;

  // if we're idle and we haven't received anything, cycle the lights.
  static Metro idleUpdate(IDLE_PERIOD);

  // check for radio traffic instructions
  if( instruction.update(newColorInst, newFireInst, newMode) )
    // reset idle
    idleUpdate.reset();

  // execute any new instructions
  if ( memcmp((void*)(&newColorInst), (void*)(&lastColorInst), sizeof(colorInstruction)) != 0 ) {
    Serial << F("New color instruction. R:") << newColorInst.red << F(" G:") << newColorInst.green << F(" B:") << newColorInst.blue << endl;
    // change the lights
    light.perform(newColorInst);
    // cache
    lastColorInst = newColorInst;   
  }
  if ( memcmp((void*)(&newFireInst), (void*)(&lastFireInst), sizeof(fireInstruction)) != 0 ) {
    Serial << F("New fire instruction. D:") << newFireInst.duration << F(" E:") << newFireInst.effect  << endl;
    // change the lights
    fire.perform(newFireInst);
    // cache
    lastFireInst = newFireInst;
  }
  if ( newMode != lastMode ) {
    // change the mode
    modeChange(newMode);
    // cache
    lastMode = newMode;
  }
  
  // Go to idle cycle, unless we're in the lights test.  
  // This lets us stay on the same color indefinitely for testing.
  if ( idleUpdate.check() && newMode != LIGHTS) {
    idleTestPattern(newColorInst);
    // and take a moment to check heap+stack remaining
    Serial << F("Tower: free RAM: ") << freeRam() << endl;
  }

}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void modeChange(systemMode &mode) {

  Serial << F("Mode state change.  Going to mode: ") << mode << endl;

  // If we've gone to one of the test modes, display a color for 1.5 seconds.
  // This should be the same amount of time that the console is playing a
  // sound, so the delay won't get us out of sync
  colorInstruction color;
  switch( mode ) {
    case 1: color=cRed; break;
    case 2: color=cGreen; break;
    case 3: color=cRed; break;
    case 4: color=cYellow; break;
    default: color=cWhite; break;
  }
  light.perform(color);
  
  // run a delay, paying attention to solenoid timers during.
  static Metro delayTime(1500UL);
  delayTime.reset();
  while ( !delayTime.check() ) {
    // check to see if we need to mess with the fire
    fire.update();
    // check to see if we need to mess with the lights.
    light.update();
  }

}

void idleTestPattern(colorInstruction &inst) { 
  const byte colorOrder[N_COLORS]={I_RED, I_BLU, I_YEL, I_GRN}; // go clockwise around the simon console
  
  // where are we?
  static byte c = instruction.getNodeID(); // all the nodes will start one off from each other.
  c += 1;
  c = c % N_COLORS;
  
  inst = cMap[colorOrder[c]];
  Serial << endl << F("Idle: color ") << c+1 << F(" of ") << N_COLORS << F(". R:") << inst.red << F(" G:") << inst.green << F(" B:") << inst.blue << endl;
}
