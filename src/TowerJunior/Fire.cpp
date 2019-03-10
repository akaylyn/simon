#include "Fire.h"

// lamp 1 is the flame effect
void lamp1On() { DmxSimple.write(1, 255); }
void lamp1Off() { DmxSimple.write(1, 0); }
// lamp 2 is the spark igniter
void lamp2On() { DmxSimple.write(2, 255); }
void lamp2Off() { DmxSimple.write(2, 0); }

Metro flameDuration(maxPropaneTime);

void Fire::begin() {
  Serial << F("Fire::begin") << endl;
  
  stop();
}

void Fire::update() {
  // just turn off
  if( flameDuration.check() ) stop();
}

void Fire::perform(systemState &system) {
  // track if we're running something
  static boolean isLockedOut = false;
  static Metro lockoutTimer;
  if( lockoutTimer.check() ) isLockedOut = false;
  
  // special cases
  // we're already running a flame effect.  bug out.
  if( isLockedOut ) { 
    Serial << ("Fire: locked out") << endl;
    return;
  }
  
  // unpack flame instruction
  unsigned long maxDuration = 0;
  for( byte i=0; i<N_COLORS; i++ ) 
    if( maxDuration < system.fire[i].duration ) maxDuration = system.fire[i].duration;
    
  // we're not being asked for a flame effect
  if( maxDuration == 0 ) return;
  
  // unpack flame instruction
  unsigned long flameTime = constrain(
    maxDuration * 10UL, // requested time in ms
    minPropaneTime, maxPropaneTime
  );
  // set the lockout appropriately
  unsigned long lockoutInterval = flameTime*(1UL+propaneClosedMultiplier);
  lockoutTimer.interval(lockoutInterval);
  lockoutTimer.reset();
  
  // reset timers
  flameDuration.interval(flameTime);
  flameDuration.reset();
  
  // fire it up.
  lamp1On();
  
/* 
   
  // air effects are more complicated
  switch ( inst.effect ) {
    case veryRich: // just st// save a pointer to the instatiated Fire class
Fire *thisHack;
raight propane (DEFAULT) "very rich"
      break; // well, that was easy.

    case kickStart:  // toss in some air at the beginning; can't start for 50ms
      for ( unsigned long i = delayAirTime; i < (flameTime / 3UL); i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;
    case kickMiddle:  // toss in some air in the middle
      for ( unsigned long i = max(delayAirTime, flameTime / 3UL); i < (flameTime * 2UL / 3UL); i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;
    case kickEnd:  // toss in some air at the end
      for ( unsigned long i = max(delayAirTime, flameTime * 2UL / 3UL); i < flameTime; i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;

    case gatlingGun: // short bursts of air throughout
      for ( unsigned long i = delayAirTime; i < flameTime; i += airPulseTime * 3UL)
        solenoids.after(i, airBurst); // load timers
      break;
    case randomly:  // toss in some air in a random pattern
      for ( unsigned long i = delayAirTime; i < flameTime; i += random(airPulseTime * 3UL, airPulseTime * 10UL))
        solenoids.after(i, airBurst); // load timers
      break;
    case veryLean: // as much air as as we can before getting "too lean"
      for ( unsigned long i = delayAirTime; i < flameTime; i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;
  }
*/
  Serial << F("Fire: effect duration ") << flameTime << F(" ms. Effect IGNORED. Lockout ") << lockoutInterval << endl;
  
  // set fireInstruction back to idle
//  inst.duration = 0;
//  inst.effect = veryRich;
}


void Fire::stop() {
  lamp1Off();
}
