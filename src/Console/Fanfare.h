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
#include "Light.h"
#include "Sound.h"
#include "Mic.h"

// Number of correct <= num that achieves the level
#define FANFARE_NONE 0
#define FANFARE_1 1
#define FANFARE_2 2
#define FANFARE_3 3
#define FANFARE_4 4

/* // Real levels
#define FANFARE_LEVEL0 8
#define FANFARE_LEVEL1 12
#define FANFARE_LEVEL2 16
#define FANFARE_LEVEL3 20
*/
// Testing levels
#define FANFARE_LEVEL1 2
#define FANFARE_LEVEL2 3
#define FANFARE_LEVEL3 4
#define FANFARE_LEVEL4 5

// gratz Player 1, do some Music, Light, Sound, Fire.
// level 0-4.  0 is low, 4 is amazing
void playerFanfare(byte level);

// Get the fanfare level to play.  Returns 0-4
byte getLevel(int correct);


#endif
