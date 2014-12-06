#ifndef Gameplay_h
#define Gameplay_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <FiniteStateMachine.h>

#include "Touch.h"
#include "Music.h"
#include "Comms.h"

// when playing, a player can wait this interval between button presses before losing.
#define PLAYER_TIMEOUT 2000UL // 2 seconds

// enable this to press anything to match the last sequence
#define CHEATY_PANTS_MODE false

// define the minimum time between fanfares in kiosk mode  <- influenced by knob 1
#define KIOSK_FANFARE_MIN 30000UL  // 30 seconds
#define KIOSK_FANFARE_MAX 300000UL  // 5 minutes

// define the time between kiosk mode updates to light pattern
#define KIOSK_UPDATE_INTERVAL 20UL // ms

// get this number in a row, and you're super cheatypants.
#define MAX_SEQUENCE_LENGTH 25 // good luck with that.
// get this number or higher for a real song
#define LEVEL_1 6
// get this number of higher for awesome songs
#define LEVEL_2 12

// define the timing of the poofs
#define POOF_MIN 25UL
#define POOF_MODE 100UL
#define POOF_MAX 300UL
// pause between poofs
#define WAIT_MIN 150UL
#define WAIT_MAX 500UL
// define the number of poofs to throw 
#define MIN_POOF_COUNT 2
#define MAX_POOF_COUNT 4

// call this from loop()
void updateSimonFSM();

// stats for FSM:
void idleState();
void gameState();
void playerState();


#endif
