// Fire subunit.  Controls propane and air solenoids

#ifndef Fire_h
#define Fire_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers.
#include <Timer.h> // interval timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class Fire {
  public:
    Fire(byte firePin, byte airPin);
    void update();
    void perform(towerInstruction &inst);

    void stop();


  protected:
    // pin control
    byte firePin, airPin;
 
    void airBurst();
    // Timer control for solenoids
    Timer solenoids;
  
};


// pin state definitions
#define OFF HIGH
#define ON LOW

#endif

