// Gameplay subunit.  Responsible for Simon implementation as a Finite State Machine.

#ifndef Gameplay_h
#define Gameplay_h

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

// when playing, a player can wait this interval between button presses before losing.
#define PLAYER_TIMEOUT 2000UL // 2 seconds

// enable this to press anything to match the last sequence
#define CHEATY_PANTS_MODE false

// disable fanfare - originally used for speeding up debugging
#define FANFARE_ENABLED true

// get this number in a row, and you're super cheatypants.
#define MAX_SEQUENCE_LENGTH 25 // good luck with that.

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

// after the game ends, music/light/fire duration is proportional to the number of correct button presses.
#define FANFARE_DURATION_PER_CORRECT 3000UL // fanfare duration = "this" * correct button presses; ms

// when we're done playing music, ramp the volume down to zero over this time
#define MUSIC_RAMP_DOWN_TIME 2000UL // ms

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

// startup the Simon game
void gameplayStart(Sound &currSound);

// call this from to upate the FSM.  returns true if state in [game, player], false if [idle].
boolean gameplayUpdate();

// states for FSM
// game is not being played
void idleState();
// game is being played, and the uC is displaing the correct sequence. BLOCKING CODE.
void gameState();
// game is being played, and the player is trying to presse the correct sequence.  BLOCKING CODE.
void playerState();
// game has ended and the uC is displaying an ending animation.  BLOCKING CODE.
void exitingState();

// helper function to tie together Tower, Light, Sound
void setSoundLights(byte colorIndex, boolean correctTone);

// decode color character and setSoundLights
void play(char color, boolean correctTone);

// play failure animation
void animateFailure();

// playback a character array of the current correct sequence
void playSequence();

// generate a random number on the interval [a, b] with mode c.
unsigned long trandom(int a, int c, int b);

// generate a character to append to the current correct sequence
char nextMove();

// update lighting on an interval when there's nobody playing
void idleUpdate();

// do a little ditty on an interval as an attractant.
void idleFanfare();


// wait for all of the buttons to be released.
void waitForButtonsReleased();
// wait for a timer to elapse
void waitForTimer(unsigned long t);

#endif
