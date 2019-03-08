// Fire subunit.  Controls propane and air solenoids

#ifndef Fire_h
#define Fire_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers.
#include <Timer.h> // interval timers
#include <DmxSimple.h>

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class Fire {
  public:
    void begin();
    void update();
    void perform(fireInstruction &inst);

    void stop();

  private:
  
};

#endif

