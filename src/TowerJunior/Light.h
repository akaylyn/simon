// Lights subunit.  Controls IR blaster and accumulator lighting

#ifndef Light_h
#define Light_h

#include <Arduino.h>

#include <DmxSimple.h>

#include <Streaming.h> // <<-style printing

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class Light {
  public:

  void begin();
  void update();
  void perform(colorInstruction &inst);

  private:  
};

#endif

