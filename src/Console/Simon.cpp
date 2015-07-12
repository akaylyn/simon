#include "Simon.h"

// see this for the game details: http://www.waitingforfriday.com/index.php/Reverse_engineering_an_MB_Electronic_Simon_game

/*************************************

Finite State Machine (FSM) Map:

   test <-> idle -> game <-> player
              ^                |
              |                |
              v               /
           fanfare <---------

*************************************/

#define CHEATY_PANTS_MODE false

// implemented Simon as a finite state machine
// this is the definitions of the states that our program uses
// each state has an entry (Enter) and exit (Exit) function that get called once
// and an update (Update) function that is called every time

// Idle
State idle = State(idleEnter, idleUpdate, idleExit);
Metro idleBeforeFanfare(10UL * 60000UL); // if we're idle for this long, do a fanfare

// Game
State game = State(gameEnter, gameUpdate, gameExit);
const int gameMaxSequenceLength = 31;
int gameCurrent, playerCurrent;
color gameSequence[gameMaxSequenceLength];
int rockTrack; // stores track at startup.

// Player
State player = State(playerEnter, playerUpdate, playerExit);
// during play, the player can pause between button presses for this long before losing
Metro playerTimeout(3000UL);

// Fanfare
State fanfare = State(fanfareEnter, fanfareUpdate, fanfareExit);
fanfare_t fanfareLevel;
//int fanfareCorrectMapping[N_LEVELS] = { 8, 14, 20, 31 }; // stock simon numbers
//int fanfareCorrectMapping[N_LEVELS] = { 4, 7, 12, 18 }; // easier
int fanfareCorrectMapping[N_LEVELS] = { 2, 3, 4, 5 }; // test easy

// Tests
State test = State(testEnter, testUpdate, testExit);

// the state machine controls which of the states get attention and execution time
FSM simon = FSM(test); //initialize state machine, start in state: test

//***** Idle
void idleEnter() {
  Serial << F("Simon: ->idle") << endl;

  // reset the game.
  gameCurrent = 0;

  light.clear();
  fire.clear();
  network.update();

  sound.setMasterGain();
  sound.setLeveling();
  sound.stopAll();

  idleBeforeFanfare.reset();

}
void idleUpdate() {
  light.animate(A_LaserWipe);
  // check buttons for game play start
  if ( touch.anyPressed() ) {
    // going to start a game
    Serial << F("Simon: idle->game") << endl;

    sound.stopAll();
    sound.setLeveling(1, 1); // 1x tone and 1x track
    rockTrack = sound.playRock(501);

    // let's play a game
    simon.transitionTo(game);
  } else if ( idleBeforeFanfare.check() ) {

    // do some fanfare
    fanfareLevel = IDLE;
    simon.transitionTo(fanfare);
  } else if ( sensor.modeChange() ) {
    // run tests
    simon.transitionTo(test);
  }
}
void idleExit() {
}

//***** Game
void gameEnter() {
  Serial << F("Simon: ->game") << endl;

  // there's a button held, so wait for a release.
  waitAllReleased();

  // clear
  light.clear();
  fire.clear();

  sound.stopAll();

  // delay after a player's last move
  waitDuration(800UL);
}
void gameUpdate() {
  // check to see if we've maxed out
  if ( gameCurrent + 1 == gameMaxSequenceLength ) {
    // holy crap.  someone's good with a pen and paper.
    fanfareLevel = MAXOUT;
    simon.transitionTo(fanfare);
    return;
  }

  // add to the sequence
  gameSequence[gameCurrent++] = (color)random(N_COLORS);

  // how long between each
  unsigned long pauseDuration = 100; // ms

  // how long to light and tone
  unsigned long playDuration = 420;
  if ( gameCurrent >= 6 ) playDuration = 320; // gets faster as you progress
  if ( gameCurrent >= 14) playDuration = 220;

  for (int i = 0; i < gameCurrent; i++) {
    colorInstruction c = cMap[gameSequence[i]];
    light.setLight(gameSequence[i], c);

    // sound
    sound.playTone(gameSequence[i]);

    // wait
    waitDuration(playDuration);

    // done
    light.clear();
    sound.stopTones();

    // wait
    waitDuration(pauseDuration);
  }

  simon.transitionTo(player);
}
void gameExit() {
}

//***** Player
void playerEnter() {
  Serial << F("Simon: ->player") << endl;

  // clear
  light.clear();
  fire.clear();

  // reset the timeout
  playerTimeout.reset();
  // reset player position to start of sequence
  playerCurrent = 0;
}
void playerUpdate() {
  // assume we're correct unless proved otherwise
  boolean correct = true;

  // wait for button press.
  if ( touch.anyPressed() ) {
    // you could, in theory, press all the buttons simultaneously to get it right...
    // but humans aren't that fast, so this is an alien/Ninja/godling detector.
    color button = touch.whatPressed();
    correct = (button == gameSequence[playerCurrent]) || CHEATY_PANTS_MODE; // note total cheat check.

    // light the correct button
    colorInstruction c = cMap[gameSequence[playerCurrent]];
    light.setLight(gameSequence[playerCurrent], c);

    // sound
    if ( correct ) {
      // correct tone
      sound.playTone(gameSequence[playerCurrent]);
      // got one more
      playerCurrent++;
    } else {
      // also light the (wrong) button they pressed
      colorInstruction c = cMap[button];
      light.setLight(button, c);
      // wrong tone
      sound.playFailTone();
    }

    // hold it while we're mashing
    waitAllReleased();

    // done
    sound.stopTones();
    light.clear();

    // reset timeout
    playerTimeout.reset();
  }

  // exit to fanfare conditions:
  boolean hasTimedOut = playerTimeout.check();
  boolean hasWrongMove = !correct;
  if ( hasTimedOut ||  hasWrongMove ) {
    Serial << "Done.  current is: " << playerCurrent << " gamecurrent: " << gameCurrent << endl;

    if ( gameCurrent > fanfareCorrectMapping[LEVEL4] ) {
      fanfareLevel = LEVEL4;
    } else if ( gameCurrent > fanfareCorrectMapping[LEVEL3] ) {
      fanfareLevel = LEVEL3;
    } else if ( gameCurrent > fanfareCorrectMapping[LEVEL2] ) {
      fanfareLevel = LEVEL2;
    } else if ( gameCurrent > fanfareCorrectMapping[LEVEL1] ) {
      fanfareLevel = LEVEL1;
    } else {
      fanfareLevel = CONSOLATION;
    }

    simon.transitionTo(fanfare);
  }

  // exit to game conditions:
  boolean hasCompletedSequence = playerCurrent == gameCurrent;
  if ( hasCompletedSequence ) {
    simon.transitionTo(game);
  }

  // otherwise, we'll come back to playerUpdate to complete the sequence
}
void playerExit() {
}

//***** Fanfare
void fanfareEnter() {
  Serial << F("Simon: ->fanfare") << endl;

  // clear everthing
  sound.stopAll();
  light.clear();
  fire.clear();

  // defined in Fanfare.h/.cpp
  playerFanfare(fanfareLevel);

  simon.transitionTo(idle);
}
void fanfareUpdate() {
}
void fanfareExit() {
}

//***** Test
void testEnter() {
  Serial << F("Simon: ->test") << endl;
}
void testUpdate() {
  // run the test modes until they're done
  if( testModes.update() ) simon.transitionTo(idle);
}
void testExit() {
}

void waitDuration(unsigned long duration) {
  Metro wait(duration);
  wait.reset();
  while ( !wait.check() ) network.update();
}

void waitAllReleased() {
  while ( touch.anyPressed() ) network.update();
}

// Not used, currently, but Mike would like to retain this code:

// generate a random number on the interval [xmin, xmax] with mode xmode.
// takes about 0.22 ms to generate a number.  random(min,max) takes 0.14 ms.
unsigned long trandom(unsigned long xmin, unsigned long xmode, unsigned long xmax) {
  // using a triangular pdf
  // http://en.wikipedia.org/wiki/Triangular_distribution#Generating_Triangular-distributed_random_variates

  unsigned long modeMinusMin = xmode - xmin;
  unsigned long maxMinusMin = xmax - xmin;
  float cut = modeMinusMin / maxMinusMin;
  // not using random() here: too slow.
  // we'll use the timekeeping function to give a number [0,1] with period 10 ms.
  float u = micros() % 10001 / 10000.;

  if ( u < cut ) {
    return ( xmin + sqrt( u * maxMinusMin * modeMinusMin ) );
  }
  else {
    unsigned long maxMinusMode = xmax - xmode;
    return ( xmax - sqrt( (1 - u) * maxMinusMin * maxMinusMode ) );
  }
}

void waitForButtonsReleased() {
  // wait for all of the buttons to be released.
  while ( touch.anyPressed() ) {
    network.update();
  }
}

void waitForTimer(unsigned long t) {
  Metro delayNow(t);
  delayNow.reset();
  while (! delayNow.check() ) {
    network.update();
  }
}

