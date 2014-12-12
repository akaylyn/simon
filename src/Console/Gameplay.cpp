#include "Gameplay.h"

// during idle, do a fanfare of light, sound and fire
Metro kioskTimer(KIOSK_FANFARE_MAX);

// during play, the player can pause between button presses for this long before losing
Metro playerTimeout(PLAYER_TIMEOUT);

// track the game states
//
// the sequence length, [0, MAX_SEQUENCE_LENGTH-1]
int currentLength = 0;
// track the sequence.
char correctSequence[MAX_SEQUENCE_LENGTH];
// the count of corrent matches that the player has made.  <= currentLength
int correctLength = 0;

// this is where the lights and fire instructions to Towers are placed
extern towerInstruction inst;

// implemented Simon as a finite state machine
// this is the definitions of the states that our program uses
State idle = State(idleState);
State game = State(gameState);
State player = State(playerState);

// the state machine controls which of the states get attention and execution time
FSM simon = FSM(idle); //initialize state machine, start in state: idle

// call this from loop()
void updateSimonFSM() {

  // update FSM
  simon.update();

}

void play(char color, boolean correctTone) {
  // reset instructions
  commsDefault(inst);

  // lights and sound
  switch ( color ) {
    case 'G':
      inst.lightLevel[I_GRN] = LED_ON;
      towerSend();
      manualLightSet(I_GRN, LED_ON);
      digitalWrite(PIXELS_GRN, LOW);
      playTone(correctTone ? GRN_TONE : WRONG_TONE);
      break;
    case 'R':
      inst.lightLevel[I_RED] = LED_ON;
      towerSend();
      manualLightSet(I_RED, LED_ON);
      digitalWrite(PIXELS_RED, LOW);
      playTone(correctTone ? RED_TONE : WRONG_TONE);
      break;
    case 'B':
      inst.lightLevel[I_BLU] = LED_ON;
      towerSend();
      manualLightSet(I_BLU, LED_ON);
      digitalWrite(PIXELS_BLU, LOW);
      playTone(correctTone ? BLU_TONE : WRONG_TONE);
      break;
    case 'Y':
      inst.lightLevel[I_YEL] = LED_ON;
      towerSend();
      manualLightSet(I_YEL, LED_ON);
      digitalWrite(PIXELS_YEL, LOW);
      playTone(correctTone ? YEL_TONE : WRONG_TONE);
      break;
  }
}

void quiet() {
  // reset instructions
  commsDefault(inst);

  noTone(SPEAKER_WIRE);
  manualLightSet(I_ALL, LED_OFF);
  digitalWrite(PIXELS_RED, HIGH);
  digitalWrite(PIXELS_YEL, HIGH);
  digitalWrite(PIXELS_BLU, HIGH);
  digitalWrite(PIXELS_GRN, HIGH);

  towerSend();
  towerComms();

}

void playSequence() {
  // how long between each
  unsigned long pauseDuration = 50; // ms
  Metro pauseAfter(pauseDuration);

  // how long to light and tone
  unsigned long playDuration = 420;
  if ( currentLength >= 6 ) playDuration = 320;
  if ( currentLength >= 14) playDuration = 220;
  Metro pauseDuring(playDuration);

  // delay after a player's last move
  delay(playDuration);

  for (int i = 0; i < currentLength; i++) {
    // lights and sound
    play(correctSequence[i], true);
    // wait
    pauseDuring.reset();
    while ( !pauseDuring.check() ) {
      towerComms();
    }

    // done
    quiet();

    // pause between next
    // wait
    pauseAfter.reset();
    while ( !pauseAfter.check() ) {
      towerComms();
    }
  }
}

// update lighting on an interval when there's nobody playing
void doKioskBored() {

  // only update on an interval, or we're spamming the comms and using up a lot of CPU power with float operations
  static Metro boredUpdateInterval(KIOSK_UPDATE_INTERVAL);
  if ( !boredUpdateInterval.check() ) return;
  boredUpdateInterval.reset(); // for next loop

  // use a breathing function.
  float ts = millis() / 1000.0 / 2.0 * PI ;
  // put the colors out of phase
  byte redVal = (exp(sin(ts + 0.00 * 2.0 * PI)) - 0.36787944) * 108.0;
  byte grnVal = (exp(sin(ts + 0.25 * 2.0 * PI)) - 0.36787944) * 108.0;
  byte bluVal = (exp(sin(ts + 0.50 * 2.0 * PI)) - 0.36787944) * 108.0;
  byte yelVal = (exp(sin(ts + 0.75 * 2.0 * PI)) - 0.36787944) * 108.0;

  //  Serial << redVal << " " << grnVal << " " << bluVal << " " << yelVal << endl;

  // patch in the comms
  if ( inst.lightLevel[I_RED] != byte(redVal) ) {
    inst.lightLevel[I_RED] = redVal;
    manualLightSet(I_RED, redVal);
    //    if( redVal > 200 ) { inst.fireLevel[I_RED]= 100; }
    //    else { inst.fireLevel[I_RED]= 0; }
    //    commsPrint(inst);
    towerSend();
  }
  if ( inst.lightLevel[I_GRN] != byte(grnVal) ) {
    inst.lightLevel[I_GRN] = grnVal;
    manualLightSet(I_GRN, grnVal);
    towerSend();
  }
  if ( inst.lightLevel[I_BLU] != byte(bluVal) ) {
    inst.lightLevel[I_BLU] = bluVal;
    manualLightSet(I_BLU, bluVal);
    towerSend();
  }
  if ( inst.lightLevel[I_YEL] != byte(yelVal) ) {
    inst.lightLevel[I_YEL] = yelVal;
    manualLightSet(I_YEL, yelVal);
    towerSend();
  }

}

unsigned long flameOnTime(int a, int c, int b) {
  // using a triangular pdf
  // http://en.wikipedia.org/wiki/Triangular_distribution#Generating_Triangular-distributed_random_variates

  float cut = (c - a) / (b - a);

  float u = random(0, 101) / 100;

  if ( u < cut ) return ( a + sqrt( u * (b - a) * (c - a) ) );
  else return ( b - sqrt( (1 - u) * (b - a) * (b - c) ) );

}

void flameOn() {
}
void flameOff() {
}

void doPoofs(int poofCount) {
  Serial << F("! Poofs. N:") << poofCount << endl;
  for (int i = 0; i < poofCount; i++) {
    flameOn();
    delay(flameOnTime(POOF_MIN, POOF_MODE, POOF_MAX));
    flameOff();
    delay(random(WAIT_MIN, WAIT_MAX));
  }
}

void doKioskFanfare() {
  Serial << F("! Kiosk Fanfare") << endl;

  // pick a song
  int minSong = 22;
  int maxSong = 121;
  int song = random(minSong, maxSong + 1);
  Serial << F(" song: ") << song << endl;
  //  musicPlayer.play_P(song_table[song], 0);  // tunez and fire.  yeah.

  // reset the timer
  delay(300); // be still, my electrical heart
  unsigned long kioskTimerInterval = random(KIOSK_FANFARE_MIN, KIOSK_FANFARE_MAX);
  kioskTimer.interval(kioskTimerInterval);
  Serial << F("Kiosk timer reset: ") << kioskTimerInterval << endl;
  kioskTimer.reset();
}

void doWinnerFanfare() {
  Serial << F("! Winner. Fanfare.") << endl;

  // pick a song
  // 0-21 <5 sec
  // 22-121 <17 sec
  // 122-138 <45 sec
  int minSong = 0;
  int maxSong = 21;

  if ( currentLength >= LEVEL_1 ) {
    Serial << F("! > LEVEL_1") << endl;
    // start with a bang
    manualLightSet(I_ALL, LED_ON);
    doPoofs(map(currentLength, 1, MAX_SEQUENCE_LENGTH, MIN_POOF_COUNT, MAX_POOF_COUNT + 1));
    manualLightSet(I_ALL, LED_OFF);

    minSong = 22;
    maxSong = 121;
  }
  if ( currentLength >= LEVEL_2 ) {
    Serial << F("! > LEVEL_2") << endl;
    minSong = 122;
    maxSong = NUM_SONGS - 1;
  }

  int song = random(minSong, maxSong + 1);

  // check correctLength relative to MAX_SEQUENCE_LENGTH, and pick a song in the list
  //  int max_song = (NUM_SONGS - 1) * currentLength / MAX_SEQUENCE_LENGTH;
  // pick a song in a band around, assuming songs further in the sequence are longer = more fire
  //  int song = constrain(random(max(0, max_song - 20), (max_song + 1)), 0, (NUM_SONGS - 1));
  Serial << F("! Winner. currentLength: ") << currentLength << F(" song: [") << minSong << F("-") << maxSong << F("]. song: ") << song << endl;
  //  musicPlayer.play_P(song_table[song], 0);  // tunez and fire.  yeah.

  // special case: correctLength == (MAX_SEQUENCE_LENGTH-1)
  // in this case, spend the accumulator in a column of fire.  well done, Player 1.
  if ( currentLength == (MAX_SEQUENCE_LENGTH - 1) ) {
    Serial << F("! Winner: special case.  Opening the FLAME!") << endl;
    manualLightSet(I_ALL, LED_ON);
    flameOn();
    delay(1000);
    flameOff();
    manualLightSet(I_ALL, LED_OFF);
    Serial << F("! Winner. special case.  Closing the FLAME!") << endl;
  }
}



char nextMove() {
  int move = random(1, 4 + 1);
  // 1=GRN, 2=RED, 3=BLU, 4=YEL
  switch ( move ) {
    case 1: return ('G');
    case 2: return ('R');
    case 3: return ('B');
    case 4: return ('Y');
  }
}


// when there's nobody playing, we're in this state.
void idleState() {
  if ( currentLength > 0 ) {
    // somebody played, so let's give them a show
    doWinnerFanfare();
    // reset timers
    kioskTimer.reset();
  }
  // reset everything.
  correctLength = 0;
  currentLength = 0;
  correctSequence[0] = '\0';

  if ( buttonPressed(I_ALL) ) {
    Serial << F("Idle.  going to Game.") << endl;
    quiet();
    while ( ! buttonChanged(I_ALL) );
    // let's play a game
    simon.transitionTo(game);
  } else if ( kioskTimer.check() ) {
    // let's do some light, sound and fire
    doKioskFanfare();
  } else {
    // let's rotate the lighting slightly
    doKioskBored();
  }

}

// when the player has played, or at the start of the game, we're in this state
void gameState() {
  // generate the next in sequence and add that to the sequence
  correctSequence[currentLength] = nextMove();
  correctSequence[currentLength + 1] = '\0'; // and null terminate it for printing purposes.
  // increment
  currentLength++;
  // play back sequence
  Serial << F("Game: currentLength=") << currentLength << F(". sequence=") << correctSequence << F(". Back to Player.") << endl;
  playSequence();
  // debug

  // reset timeout
  playerTimeout.reset();
  // reset correct length
  correctLength = 0;
  // give it back to the player
  simon.transitionTo(player);
}

// waiting for the player to press a button.
void playerState() {
  // check if they waited too long
  if ( playerTimeout.check() ) {
    Serial << F("Player: timeout.  currentLength = ") << currentLength << F(".  Back to Idle.") << endl;
    // if so, show the correct next button
    play(correctSequence[correctLength], false);
    // wait
    Metro delayNow(2000);
    while (! delayNow.check() ) {
      towerComms();
    }
    quiet();
    // and exit game play
    simon.transitionTo(idle);
  }

  // otherwise, wait for button press.
  if ( buttonPressed(I_ALL) ) {
    boolean correct = (buttonPressed(I_GRN) && correctSequence[correctLength] == 'G') ||
                      (buttonPressed(I_RED) && correctSequence[correctLength] == 'R') ||
                      (buttonPressed(I_BLU) && correctSequence[correctLength] == 'B') ||
                      (buttonPressed(I_YEL) && correctSequence[correctLength] == 'Y');

    // there's a total cheat.
    correct = correct || CHEATY_PANTS_MODE;

    // light and sound
    play( correctSequence[correctLength], correct );
    // hold it while we're mashing
    while ( buttonPressed(I_ALL) ) {
      towerComms();
    }

    // done
    quiet();

    if ( correct ) {
      correctLength++;
      // reset timeout
      playerTimeout.reset();
      // keep going
    } else {
      Serial << F("Player: incorrect.  currentLength = ") << currentLength << F(".  Back to Idle.") << endl;
      // if so, show the correct next button
      play(correctSequence[correctLength], false);
      // wait
      Metro delayNow(3000);
      while (! delayNow.check() ) {
        towerComms();
      }
      quiet();
      // and exit game play
      // exit game play
      simon.transitionTo(idle);
    }
  }

  // check if they've got the sequence complete, or if there's just awesome
  if ( correctLength == currentLength || correctLength == (MAX_SEQUENCE_LENGTH - 1) ) {
    // nice.  pass it back to game
    Serial << F("Player: correct.  currentLength = ") << currentLength << F(".  Back to Game.") << endl;
    simon.transitionTo(game);
  }
}




