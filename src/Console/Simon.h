// Simon subunit.  Responsible for Simon implementation as a Finite State Machine.

#ifndef Simon_h
#define Simon_h

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
#include "Fanfare.h"
#include "Network.h"
#include "Tests.h"
#include "Sensor.h"

/*************************************

Finite State Machine (FSM) Map:

  test <-> idle -> game <-> player
              ^                |
              |                |
              v               /
           fanfare <---------

*************************************/

// Idle
void idleEnter(), idleUpdate(), idleExit();
// Game
void gameEnter(), gameUpdate(), gameExit();
// Player
void playerEnter(), playerUpdate(), playerExit();
// Fanfare
void fanfareEnter(), fanfareUpdate(), fanfareExit();
// Test
void testEnter(), testUpdate(), testExit();

// Helper functions
void waitAllReleased();
void waitDuration(unsigned long duration);

extern FSM simon;

#endif
