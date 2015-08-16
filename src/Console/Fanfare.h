// Fanfare subunit.  Responsible for playing animations for winning and losing gameplay conditions.
// Animations elements:
//      4 towers:  
//        0,1,2) 50m RGB LED Strip.  3 channels, 0-255 intensity
//        3) Flame effect, 0-255 = solenoid opening time.  50-500ms.  cooldown of time / 2.  Defined in Light.cpp
//        4) Air effect.  0-255 = solenoid opening time.  50-500ms.  cooldown of time / 2.  Defined in Light.cpp 
//        5,6,7) LED Flood Light.  3 channels, 0-255 intensity 
//        Speaker.  Use Sound interface.
//
// Design:  Provide timings per channel as in channel,(ontime,intensity),offtime,...
//          Well convert ontime to solenoid level to make it easier to sync events.
//        
//          0,100
//
//  Sequencing
//  250K program storage or SD card?
//  8 channels, 50ms resolution, 1 byte per sample, 30s track.  1000/50 * 8 = 4,800 bytes.

// Design programs:  vixen or HLS

#ifndef Fanfare_h
#define Fanfare_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <FiniteStateMachine.h>

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

// makes heavy use of the modules.
#include "Touch.h"
#include "Network.h"
#include "Light.h"
#include "Fire.h"
#include "Sound.h"
#include "Mic.h"
#include "Simon.h"

// fanfare mapping
enum fanfare_t {
  LEVEL1=0, // won, a little
  LEVEL2, // won more
  LEVEL3,  
  LEVEL4, // etc.
  
  N_LEVELS, // 4
  
  NONE, // nada
  IDLE, // system was idle long enough to trigger a fanfare
  CONSOLATION, // player didn't get very far
  MAXOUT // must of had a pen and paper, because they max'd at 32 correct
};

void playerFanfare(fanfare_t level);

color incColor(color val);
color randColor();
color oppTower(color val);
  
void saveFireBudgetFactor(float factor);
float loadFireBudgetFactor();
static const int budgetEepromAddr = 108;

#endif
