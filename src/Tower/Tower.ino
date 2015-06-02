// Compile for Uno.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Bounce.h> // remote relays

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// handle incoming instructions and idle patterns
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module
#include <EEPROM.h> // saving and loading radio settings
#include "Instruction.h"

// perform lighting
#include <RGBlink.h> // control LEDs
#include <IRremote.h> // control IR Rx lighting
#include <QueueArray.h> // queing for IR transmissions
#include <Metro.h> // countdown timers
#include "Light.h"
// pin locations for outputs
#define PIN_R 6 // the PWM pin which drives the red LED
#define PIN_G 5 // the PWM pin which drives the green LED
#define PIN_B 9 // the PWM pin which drives the blue LED
#define PIN_IR 3 // the PWM pin which drives the IR floodlight

// perform fire
#include <Metro.h> // countdown timers
#include <Timer.h> // interval timers
#include "Fire.h"
// pin locations for outputs
#define PIN_FLAME 7 // relay for flame effect solenoid
#define PIN_AIR 8 // relay for air solenoid

// instantiate
// boostrap the tower nodeID to this value (2,3,4,5), overwriting EEPROM.
// set "0" to read EEPROM value
#define HARD_SET_NODE_ID_TO 0
Instruction instruction(HARD_SET_NODE_ID_TO);
Light light(PIN_R, PIN_G, PIN_B, PIN_IR);
Fire fire(PIN_FLAME, PIN_AIR);

// remote control
#define RESET_PIN A0
#define MODE_SWITCH_PIN A1
#define DEBOUNCE_TIME 100UL // need a long debounce b/c electrical noise from solenoid.
Bounce systemReset = Bounce(RESET_PIN, DEBOUNCE_TIME);
Bounce modeSwitch = Bounce(MODE_SWITCH_PIN, DEBOUNCE_TIME);

// spend this much time on a cycle of light updates when idle
#define TEST_PATTERN_PERIOD 10000UL // ms

// color definitions
RGB c_red={255,0,0}; 
RGB c_grn={0,255,0}; 
RGB c_blu={0,0,255}; 
RGB c_yel={255,255,0}; 
RGB c_wht={255,255,255}; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial << F("Setup: begin") << endl;

  Serial << F("Setup: pausing after boot 1 sec...") << endl;
  delay(1000UL);

  // random seed.
  randomSeed(analogRead(A3)); // or some other unconected pin

  // see: http://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/
  bitSet(TCCR1B, WGM12); // puts Timer1 in Fast PWM mode to match Timer0.

  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

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
  static towerInstruction inst;
  boolean newInstructions = false;
 
  // check for reset condition, and set the tanks to blink during reset
  if ( systemReset.update() ) { // reset state change
    Serial << F("Reset state change.  State: ");
    if (systemReset.read() == LOW) {
      Serial << F("reset.") << endl;
      
      setInst(inst, c_red); 
      newInstructions = true;
      light.effect(BLINK);
    } else{
      Serial << F("normal.") << endl;
      light.effect(SOLID);
    }
  }
  
  // check for radio traffic instructions
  newInstructions |= instruction.update(inst);
  
  // if we're idle and we haven't received anything, cycle the lights.
  static Metro idleUpdate(TEST_PATTERN_PERIOD);
  if( !newInstructions && idleUpdate.check()) {
    newInstructions |= idleTestPattern(inst);
    Serial << F("Tower: free RAM: ") << freeRam() << endl;
  }

  // check for new instructions
  if ( newInstructions ) {
    // change the lights
    light.perform(inst);
    // change the fire
    fire.perform(inst);
  }
  
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setInst(towerInstruction &inst, RGB color) {
  inst.red = color.red;
  inst.green = color.green;
  inst.blue = color.blue;
  inst.flame = 0; // just to be sure
}

boolean idleTestPattern(towerInstruction &inst) {
  // how many colors to cycle through
  const RGB colors[N_COLORS] = {c_red, c_blu, c_yel, c_grn};
  
  // where are we?
  static byte c = 0;
  c = (c==N_COLORS) ? 0 : c+1;
  
  setInst(inst, colors[c]);
  
  return( true );
}




