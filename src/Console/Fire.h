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
extern Network network;

// for fireEnable function
#include "Sensor.h" 
extern Sensor sensor;

class Fire {
  public:
    // startup.  layout the towers.
    void begin(nodeID layout[N_COLORS]);

    // set fire level, taking advantage of layout position
    void setFire(color position, byte flameDuration=minPropaneTime, flameEffect effect=veryRich);
    void setFire(color position, fireInstruction &inst);
    // set fire level, ignoring tower positions (good luck with that)
    void setFire(nodeID node, byte flameDuration=minPropaneTime, flameEffect effect=veryRich);
    void setFire(nodeID node, fireInstruction &inst);

    void clear(); // zaps everything

  private:
    // storage for light and fire levels.
    fireInstruction inst[N_COLORS];

    // stores which towers should be sent color commands
    nodeID layout[N_COLORS];
    
    byte checkFireEnabled(byte flameDuration);
};

extern Fire fire;

#endif
