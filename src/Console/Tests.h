// Responsible for sensor readings
#ifndef Tests_h
#define Tests_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include "Sound.h" // for sound module
#include "Network.h" // for mode switch sends and Tower comms
#include "Light.h" // for lights
#include "Fire.h" // for fire

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
