// Responsible for sensor readings
#ifndef TestModes_h
#define TestModes_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include "Sound.h" // for sound module
 
//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class TestModes {
  public:
    void bongoModeLoop(bool performStartup);
    void proximityModeLoop(bool performStartup);
    void lightsTestModeLoop(bool performStartup);
    void fireTestModeLoop(bool performStartup);
};

extern TestModes testModes;

#endif
