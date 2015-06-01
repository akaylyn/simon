#include "Fire.h"

Fire::Fire(byte firePin, byte airPin) {
  this->firePin = firePin;
  this->airPin = airPin;
  // order is important.  set then output.
  stop();
  // order is important.  set then output.
  pinMode(firePin, OUTPUT); 
  pinMode(airPin, OUTPUT); 
}

void Fire::update() {
  // run through timer events
  solenoids.update();
}

void Fire::perform(towerInstruction &inst) {
  // track if we're running something
  static Metro lockoutTimer;
  
  // special cases
  // we're already running a flame effect.  bug out.
  if( !lockoutTimer.check() ) return;
  // we're not being asked for a flame effect
  if( inst.flame == 0 ) return;
  
  // unpack flame instruction
  unsigned long flameTime = constrain(
    (unsigned long)inst.flame * 10UL, // requested time in ms
    minPropaneTime, maxPropaneTime
  );
  // set the lockout appropriately
  unsigned long lockoutInterval = flameTime*(1UL+propaneClosedMultiplier);
  lockoutTimer.interval(lockoutInterval);
  lockoutTimer.reset();
  
  // reset timers
  stop();
  
  // fire it up.
  solenoids.pulseImmediate(firePin, flameTime, ON);
  
  // unpack the air effect instruction
  flameEffect_t effect = inst.effect;
  
  // air effects are more complicated
  switch ( effect ) {
    case FE_veryRich: // just straight propane (DEFAULT) "very rich"
      break; // well, that was easy.

    case FE_kickStart:  // toss in some air at the beginning; can't start for 50ms
      for ( unsigned long i = delayAirTime; i < (flameTime / 3UL); i += airPulseTime * 2UL)
        solenoids.after(i, this->airBurst); // load timers
      break;
    case FE_kickMiddle:  // toss in some air in the middle
      for ( unsigned long i = max(delayAirTime, flameTime / 3UL); i < (flameTime * 2UL / 3UL); i += airPulseTime * 2UL)
        solenoids.after(i, this->airBurst); // load timers
      break;
    case FE_kickEnd:  // toss in some air at the end
      for ( unsigned long i = max(delayAirTime, flameTime * 2UL / 3UL); i < flameTime; i += airPulseTime * 2UL)
        solenoids.after(i, this->airBurst); // load timers
      break;

    case FE_gatlingGun: // short bursts of air throughout
      for ( unsigned long i = delayAirTime; i < flameTime; i += airPulseTime * 3UL)
        solenoids.after(i, this->airBurst); // load timers
      break;
    case FE_randomly:  // toss in some air in a random pattern
      for ( unsigned long i = delayAirTime; i < flameTime; i += random(airPulseTime * 3UL, airPulseTime * 10UL))
        solenoids.after(i, this->airBurst); // load timers
      break;
    case FE_veryLean: // as much air as as we can before getting "too lean"
      for ( unsigned long i = delayAirTime; i < flameTime; i += airPulseTime * 2UL)
        solenoids.after(i, this->airBurst); // load timers
      break;
  }

  Serial << F("Fire: effect duration ") << flameTime << F(" ms. Effect ") << effect << F(". Lockout ") << lockoutInterval << endl;
}

// pulse the air on 
void Fire::airBurst() {
  //  Serial << "Air.   short air:" << millis() << endl;
  solenoids.pulseImmediate(airPin, airPulseTime, ON);
}

void Fire::stop() {
  digitalWrite(firePin, OFF);
  digitalWrite(airPin, OFF);

  // clear out the timers
  for ( uint8_t i = 0; i < MAX_NUMBER_OF_EVENTS; i++) solenoids.stop(i);
}
