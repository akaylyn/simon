// Compile for Uno.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Timer.h> // control air injector
#include <Metro.h> // timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

#define AIR 13 // relay for air solenoid
#define FLAME 12 // relay for fire solenoid

// Timer control for air solenoid
Timer air;
const unsigned long shortAirTime = 50UL;
const unsigned long longAirTime = 100UL;

// track how long we need to have the flame on for.
Metro flameOnTime(100UL);
// but only fire this often.
Metro flameCoolDownTime(1000UL);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial << F("Configuring output pins.") << endl;
  flameOff(); 
  pinMode(FLAME, OUTPUT); // order is important.  set then output.
  airOff(); 
  pinMode(AIR, OUTPUT); // order is important.  set then output.
}

void loop() {
  // SAFETY: do not move this code after any other code.
  if ( flameOnTime.check() ) { // time to turn the flame off
    flameOff();
  }
  
  // check to see if we need to mess with the air.
  air.update();
  
  // execute on the longest requested flame length.
  static boolean runNow = true;
  if( runNow ) {
    flameOn(255, FE_randomly);
    runNow=false;
  }

}

void flameOn(int fireLevel, flameEffect_t effect) {

  // special case for fireLevel==0 i.e. "none"
  if ( fireLevel == 0 ) return;

  // only if the cooldown timer is ready
  if ( flameCoolDownTime.check() ) return;
  
  unsigned long flameTime = constrain( // constrain may be overkill, but map doesn't guarantee constraints
          map(fireLevel, 1, 255, 50UL, 1000UL),
          50UL, 1000UL
  );

  // SAFETY: never, ever, ever set this pin high without resetting the flameOnTime timer, unless you
  // have some other way of turning it back off.

  // To put a Fine Point on it: this simple line will unleash 100,000 BTU.  Better take care.
  digitalWrite(FLAME, LOW); // This line should appear exactly once in a sketch, and be swaddled in all manner of caution.
  flameOnTime.interval(flameTime);
  flameOnTime.reset();

  // set flame cooldown from config
  unsigned long cooldown = flameTime/1UL;
  flameCoolDownTime.interval(cooldown);

  // start cooldown counter
  flameCoolDownTime.reset();

  Serial << F("Flame on! ") << fireLevel << F(" mapped [") << 50UL << F(",") << 1000UL << F("]");
  Serial << F(" -> ") << flameTime << F("ms.  Cooldown: ") << cooldown << F("ms.") << endl;
  
  // just making sure
  airOff();
  Serial << "Air.  start time:" << millis() << endl;
  
  switch( effect ) {
    case FE_veryRich: // just straight propane (DEFAULT) "very rich"
      break; // well, that was easy. 

    case FE_kickStart:  // toss in some air at the beginning; can't start for 50ms
      for( unsigned long i = shortAirTime; i<(flameTime/3UL); i+=longAirTime)
        air.after(i, shortAirBurst); // load timers
      break;
    case FE_kickMiddle:  // toss in some air in the middle
      for( unsigned long i = max(shortAirTime,flameTime/3UL); i<(flameTime*2UL/3UL); i+=longAirTime)
        air.after(i, shortAirBurst); // load timers
      break;
    case FE_kickEnd:  // toss in some air at the end
      for( unsigned long i = max(shortAirTime,flameTime*2UL/3UL); i<flameTime; i+=longAirTime)
        air.after(i, shortAirBurst); // load timers
      break;

    case FE_gatlingGun: // short bursts of air throughout
      air.every(longAirTime+shortAirTime, shortAirBurst);
      break;
    case FE_randomly:  // toss in some air in a random pattern
      for( unsigned long i = shortAirTime; i<flameTime; i+=random(longAirTime,longAirTime*5UL))
        air.after(i, shortAirBurst); // load timers
      break;
    case FE_veryLean: // as much air as as we can before getting "too lean"
      air.every(shortAirTime+shortAirTime, shortAirBurst); 
      break;

  }
}

// pulse the air on for shortAirTime
void shortAirBurst() {
  Serial << "Air.   short air:" << millis() << endl;
  air.pulseImmediate(AIR, shortAirTime, LOW);
}

void flameOff() {
  digitalWrite(FLAME, HIGH);
  airOff();

  Serial << F("Flame off! ") << endl;
}

void airOff() {
  digitalWrite(AIR, HIGH);
  // clean out the event timers.
  for( uint8_t i=0; i<MAX_NUMBER_OF_EVENTS; i++) air.stop(i);
}


