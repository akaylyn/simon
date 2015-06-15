// Fire subunit.  Responsible for UX (fire) outboard on Towers.

#ifndef Fire_h
#define Fire_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// for send function
#include "Network.h"

// for fireEnable function
#include "Sensor.h" 

class Fire {
  public:
    // startup.  layout the towers.
    void begin();

    // set fire level, taking advantage of layout position
    void setFire(color position, byte flameDuration=minPropaneTime, flameEffect effect=veryRich);
    void setFire(color position, fireInstruction &inst);

    void clear(); // zaps everything

  private:
    
    byte checkFireEnabled(byte flameDuration);
};

extern Fire fire;

#endif
