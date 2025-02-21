// Compile for Uno.

// MGD: adding a touch here to confirm my Git-fu is not weak.

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
//#define HARD_SET_NODE_ID_TO BROADCAST
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
#define IDLE_PERIOD_MIN 30UL // ms
#define IDLE_PERIOD_MAX 1000UL // ms

// fire the sparker on some interval
#define RELIGHT_PERIOD_MIN 5UL * 60000UL // ms
#define RELIGHT_PERIOD_MAX 15UL * 60000UL // ms

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
  fire.doSparky(); // good idea at startup.
  
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
  static systemState lastState, newState;

  // if we're idle and we haven't received anything, cycle the lights.
  static Metro idleUpdate(IDLE_PERIOD_MAX);

  // check for radio traffic instructions
  if( instruction.update(newState) )
    // reset idle
    idleUpdate.reset();

  // execute any new instructions
  if ( memcmp((void*)(&newState), (void*)(&lastState), sizeof(systemState)) != 0 ) {
    Serial << F("New state instruction.") << endl;
    // change the lights
    light.perform(newState);
    // change the fire
    fire.perform(newState);
    // cache
    lastState = newState;   
  }
  if ( newState.mode != lastState.mode ) {
    // change the mode
    modeChange(newState.mode);
  }
  
  // Go to idle cycle, unless we're in the lights test.  
  // This lets us stay on the same color indefinitely for testing.
  if ( idleUpdate.check() && newState.mode != LIGHTS) {
    idleTestPattern();
    // and take a moment to check heap+stack remaining
    Serial << F("Tower: free RAM: ") << freeRam() << endl;
    // and add some entropy
    idleUpdate.interval(random(IDLE_PERIOD_MIN, IDLE_PERIOD_MAX));
    idleUpdate.reset();
  }

  // check for relight
  static Metro relight(RELIGHT_PERIOD_MIN);
  if( relight.check() ) {
    Serial << F("Relight....") << endl;
    fire.doSparky();
    // and add some entropy
    relight.interval(random(RELIGHT_PERIOD_MIN, RELIGHT_PERIOD_MAX));  
    relight.reset(); 
  }

}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void modeChange(byte mode) {

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
  static systemState st;
  for(byte i=0; i<N_COLORS; i++) st.light[i] = color;
  light.perform(st);
  
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

void idleTestPattern() { 
  const colorInstruction colors[N_COLORS] = {cRed, cBlue, cYellowConsole, cGreen}; // go clockwise around the simon console
  
  static systemState st;

  // where are we?
  static byte c = 0; // all the nodes will start one off from each other.
  
  st.light[I_RED] = colors[(0+c) % N_COLORS];
  st.light[I_BLU] = colors[(1+c) % N_COLORS];
  st.light[I_YEL] = colors[(2+c) % N_COLORS];
  st.light[I_GRN] = colors[(3+c) % N_COLORS];
    
  Serial << endl << F("Idle: color offset ") << c << F(" of ") << N_COLORS << F(".")  << endl;
  light.perform(st);

  c += 1;
  c = c % N_COLORS;
}
