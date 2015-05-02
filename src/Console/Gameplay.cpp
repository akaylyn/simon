#include "Gameplay.h"
#include "Sound.h"

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

// implemented Simon as a finite state machine
// this is the definitions of the states that our program uses
State idle = State(idleState);
State game = State(gameState);
State player = State(playerState);
State exiting = State(exitingState);

// the state machine controls which of the states get attention and execution time
FSM simon = FSM(idle); //initialize state machine, start in state: idle

// startup
void gameplayStart(Sound &currSound) {
  Serial << F("Gameplay: startup.") << endl;
  sound = currSound;
}

// call this from loop()
boolean gameplayUpdate() {

  // update FSM
  simon.update();

  // return true if playing
  return ( !simon.isInState( idle ) );

}


// when there's nobody playing, we're in this state.
void idleState() {

  // reset everything.
  correctLength = 0;
  currentLength = 0;
  correctSequence[0] = '\0';

  // check buttons for game play start
  if ( touch.anyPressed() ) {
    // going to start a game
    Serial << F("Gameplay: Idle->Game") << endl;
    // bring up the lights
    light.setAllOff();

    sound.stopAll();
    sound.setLeveling(1, 1); // 1x tone and 1x track
    sound.playRock(101);

    // wait for them to let go of the button
    waitForButtonsReleased();

    // turn off the lights; immediate send
    light.setAllOff(); 

    // let's play a game
    simon.transitionTo(game);
  } 

  /*
  // stubbing in microphone pickup activity.
   // doesn't do anything useful; just makes a tone that should be synchronized with the beat.
   mic.update();
   static Metro clearInterval(50);
   byte level=0;
   for(byte i = 0; i<NUM_FREQUENCY_BANDS; i++) {
   if( mic.getBeat(i) ) bitSet(level, 7-i);
   }
   if( level>0 ) { // got some kind of beat
   //    byte b1 = map(mic.getVol(0), byte(mic.getAvg(0)), byte(mic.getAvg(0)+mic.getSD(0)*mic.getTh(0)), LIGHT_OFF, LIGHT_ON);
   //    byte b2 = map(mic.getVol(1), byte(mic.getAvg(1)), byte(mic.getAvg(1)+mic.getSD(1)*mic.getTh(1)), LIGHT_OFF, LIGHT_ON);
   //    byte b3 = map(mic.getVol(2), byte(mic.getAvg(2)), byte(mic.getAvg(2)+mic.getSD(2)*mic.getTh(2)), LIGHT_OFF, LIGHT_ON);
   //    Serial << rl << endl; delay(25);
   //    light.setLight(I_RED, b3);
   //    light.setLight(I_GRN, b2);
   //    light.setLight(I_BLU, b1);
   
   
   light.setAllLight(level);
   light.setAllFire(bitRead(level,6) ? 10 : 0); // just the 160 Hz band, and only a little fire
   light.show();
   delay(10); // need to let the transmission happen
   
   clearInterval.reset();
   } 
   
   if( clearInterval.check() ) {
   light.setAllOff(); // clear colors
   clearInterval.reset();
   }
   */

}

// when the player has played, or at the start of the game, we're in this state
void gameState() {
  Serial << F("Current State: Game") << endl;
  // generate the next in sequence and add that to the sequence
  correctSequence[currentLength] = nextMove();
  correctSequence[currentLength + 1] = '\0'; // and null terminate it for printing purposes.
  // increment
  currentLength++;
  // play back sequence
  Serial << F("Gameplay: Game currentLength=") << currentLength << F(". sequence=") << correctSequence << endl;
  playSequence();
  // debug

  // reset timeout
  playerTimeout.reset();

  // give it back to the player
  Serial << F("Gameplay: Game->Player") << endl;
  simon.transitionTo(player);
}

// waiting for the player to press a button.
void playerState() {
  //Serial << F("Current State: Player.  correctLength: ") << correctLength << endl;
  // check if they waited too long
  if ( playerTimeout.check() ) {
    Serial << F("Gameplay: Player timeout.  currentLength = ") << currentLength << endl;

    // if so, show the correct next button
    play(correctSequence[correctLength], false);
    sound.playFailTone();

    // wait for them to let go of the button
    waitForButtonsReleased();      

    // and exit game play
    Serial << F("Gameplay: Player->Exiting") << endl;
    simon.transitionTo(exiting);
  }

  // otherwise, wait for button press.
  if ( touch.anyPressed() ) {
    boolean correct =
      (touch.pressed(I_GRN) && correctSequence[correctLength] == 'G') ||
      (touch.pressed(I_RED) && correctSequence[correctLength] == 'R') ||
      (touch.pressed(I_BLU) && correctSequence[correctLength] == 'B') ||
      (touch.pressed(I_YEL) && correctSequence[correctLength] == 'Y');

    // there's a total cheat.
    correct = correct || CHEATY_PANTS_MODE;

    // light and music
    play( correctSequence[correctLength], correct );
    
    // hold it while we're mashing
    waitForButtonsReleased();

    // done
    sound.stopTones();
    light.setAllOff();

    if ( correct ) {
      correctLength++;
      Serial << "incCorrect: " << correctLength << "\n";
      // reset timeout
      playerTimeout.reset();
      // keep going
    }
    else {
      Serial << F("Gameplay: Player incorrect.  currentLength = ") << currentLength << " correct: " << correctLength << " expecting: " << correctSequence[correctLength] << endl;

      // if so, show the correct next button
      play(correctSequence[correctLength], false);

      // wait
      waitForTimer(500UL);

      sound.stopTones();
      
      simon.transitionTo(exiting);
    }
  }

  /// check if they've got the sequence complete, or if there's just awesome
  if ( correctLength == currentLength || correctLength == (MAX_SEQUENCE_LENGTH - 1) ) {
    // nice.  pass it back to game
    Serial << F("Gameplay: Player correct.  currentLength = ") << currentLength << endl;
    Serial << F("Gameplay: Player->Game") << endl;

    correctLength = 0;
    simon.transitionTo(game);
  }
}

void exitingState() {

  // safer not to assume anything about the lights and sound coming into these states
  // shut down lights and sound
  light.setAllOff();
  sound.stopAll(); // note that this is stopping ROCK
   
  Serial << "Inside exiting state.  correct: " << correctLength << endl;
  // if so, show the correct next button
  //play(correctSequence[correctLength], false);
  byte fanfareLevel = getLevel(currentLength-1);
  if (fanfareLevel == FANFARE_NONE) {
    Serial << "Animating failure";
    animateFailure();
    
  } 
  else {
    playerFanfare(fanfareLevel);
  }
  
  // shut down lights and sound
  light.setAllOff();
  sound.stopAll();
  
  // reset correct length
  correctLength = 0;

  // exit game play
  Serial << F("Gameplay: Player->Idle") << endl;
  simon.transitionTo(idle);
}

// generate a character to append to the current correct sequence
char nextMove() {
  int move = random(1, 4 + 1);
  // 1=GRN, 2=RED, 3=BLU, 4=YEL
  switch ( move ) {
  case 1:
    return ('G');
  case 2:
    return ('R');
  case 3:
    return ('B');
  case 4:
    return ('Y');
  }
}


// helper function to tie together Tower, Light, Sound
void setSoundLights(byte colorIndex, boolean correctTone) {
  // order is important: sound is slower than lights

  // Sound on Console and Tower
  if (correctTone) {
    sound.playTone(colorIndex);
  } 
  else {
    sound.playFailTone();
  }

  // Lights on Tower
  light.setLight(colorIndex, LIGHT_ON, true);

}

// decode color character and set lights and music
void play(char color, boolean correctTone) {
  switch ( color ) {
  case 'G':
    setSoundLights(I_GRN, correctTone);
    break;
  case 'R':
    setSoundLights(I_RED, correctTone);
    break;
  case 'B':
    setSoundLights(I_BLU, correctTone);
    break;
  case 'Y':
    setSoundLights(I_YEL, correctTone);
    break;
  }
}

void animateFailure()
{
  Serial << "AnimateFailure!" << endl;
  
  sound.stopAll(); // remember to stop ROCK before playing other tracks.
  sound.playLose();
  
  for (int i = 0; i < 6; i++) {
    light.setAllLight(LIGHT_ON, true);
    delay(300);
    light.setAllOff();
    delay(100);
  }

  light.setAllOff();
  sound.stopAll();
}

// playback a character array of the current correct sequence
void playSequence() {
  // how long between each
  unsigned long pauseDuration = 50; // ms

  // how long to light and tone
  unsigned long playDuration = 420;
  if ( currentLength >= 6 ) playDuration = 320;
  if ( currentLength >= 14) playDuration = 220;

  // delay after a player's last move
  delay(playDuration);

  for (int i = 0; i < currentLength; i++) {
    // lights and music
    play(correctSequence[i], true);
    // wait
    waitForTimer(playDuration);

    // done
    sound.stopTones();
    light.setAllOff();
    // pause between next
    waitForTimer(pauseDuration);
  }
}

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
    return ( xmax - sqrt( (1-u) * maxMinusMin * maxMinusMode ) );
  }
}

//
void flameOn() {
}
void flameOff() {
}

void towerPoof(int poofCount) {
  Serial << F("! Poofs. N:") << poofCount << endl;
  for (int i = 0; i < poofCount; i++) {
    flameOn();
    delay(trandom(POOF_MIN, POOF_MODE, POOF_MAX));
    flameOff();
    delay(random(WAIT_MIN, WAIT_MAX));
  }
}

void playerFanfare(byte level) {
  Serial << "***Inside playerFanFare: level: " << level << "\n";

  if (!FANFARE_ENABLED) {
    Serial.println("Fanfare disabled");
    return;
  }

  Serial << F("Gameplay: Player fanfare, duration: ") << FANFARE_DURATION_PER_CORRECT * currentLength << endl;

  Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

  // make sweet fire/light/music.
  int winTrack = sound.playWins();

  if (level >= FANFARE_1) {
    Serial << "Playing win level 0";
    for(byte n=0; n < 8; n++) {
      for ( byte i = 0; i < N_COLORS; i++ ) {
        if (n % 2 == 0) {
          light.setLight(I_BLU, LIGHT_ON);
          light.setFire(i, LIGHT_ON);
        } 
        else {
          light.setLight(I_BLU, LIGHT_OFF);
          light.setFire(i, LIGHT_OFF);
        }
      }
      if (n % 2 == 0) {
        light.show();
        waitForTimer(50UL);
      } 
      else {
        waitForTimer(400UL);
      }
    }
  }

  // use this, as it does an immediate send.
  light.setAllOff();

  // ramp down the volume to exit the music playing cleanly.
  sound.fadeTrack(winTrack);

  Serial << F("Gameplay: Player fanfare ended") << endl;
}

// Get the fanfare level to play based on # correct.  Returns 0-3
byte getLevel(int correct) {
  Serial << "Get Level: correct: " << correct << "\n";
  if (correct < FANFARE_LEVEL1) {
    return FANFARE_NONE;
  } 
  else if (correct <= FANFARE_LEVEL2) {
    return FANFARE_1;
  } 
  else if (correct <= FANFARE_LEVEL3) {
    return FANFARE_2;
  } 
  else {
    return FANFARE_3;
  }
}

void waitForButtonsReleased() {
  // wait for all of the buttons to be released.
  while ( touch.anyPressed() ) {
    light.update();
  }
}

void waitForTimer(unsigned long t) {
  Metro delayNow(t);
  while (! delayNow.check() ) {
    light.update();
  }
}

