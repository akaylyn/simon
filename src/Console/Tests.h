// Responsible for sensor readings
#ifndef Tests_h
#define Tests_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <EEPROM.h> // layout retrieval
#include "Sound.h" // for sound module
#include "Touch.h" // for touch module
#include "Network.h" // for mode switch sends and Tower comms
#include "Light.h" // for lights
#include "Fire.h" // for fire
#include "SimonScoreboard.h" // to enabled LCD/scoreboard use
#include "Mic.h"
#include "Simon.h"

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class TestModes {
  public:
    // returns true if we want to return to playing Simon
    boolean update(); 
    
  private:
    void whiteoutModeLoop(boolean performStartup);
    void bongoModeLoop(boolean performStartup);
    void proximityModeLoop(boolean performStartup);
    void lightsTestModeLoop(boolean performStartup);
    void fireTestModeLoop(boolean performStartup);
    void layoutModeLoop(boolean performStartup);
    void externModeLoop(boolean performStartup);
};

extern TestModes testModes;

#endif
