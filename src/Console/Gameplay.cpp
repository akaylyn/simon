#include "Gameplay.h"
#include "Sound.h"

// during idle, do a fanfare of light, music and fire
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
//  Serial << F("Current State: Idle") << endl;
  if ( currentLength > 0 ) {
    kioskTimer.reset();
  }
  // reset everything.
  correctLength = 0;
  currentLength = 0;
  correctSequence[0] = '\0';

  // check buttons for game play start
  if ( touch.anyPressed() ) {
    Serial << F("Gameplay: Idle->Game") << endl;
    quiet();
    /* MGD removed this en lieu of anypresed.  The logic, below, doesn't exit with an abberant press.
     while (1) {
     if (buttonAnyChanged())
     break;
     if (touchAnyChanged())
     break;
     }*/
    while ( touch.anyPressed()) ;
    // let's play a game
    simon.transitionTo(game);
  } 
  else if ( kioskTimer.check() ) {
    // let's do some light, music and fire
    idleFanfare();
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
    // wait
    sound.playBaff();
    Metro delayNow(2000);
    while (! delayNow.check() ) {
      light.update();
    }
    quiet();
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
    while ( touch.anyPressed() ) {
      light.update();
    }

    // done
    quiet();

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
      Metro delayNow(500);
      while (! delayNow.check() ) {
        light.update();
      }

      quiet();
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
    Serial << "Inside exiting state.  correct: " << correctLength << endl;
    // if so, show the correct next button
    //play(correctSequence[correctLength], false);
    byte fanfareLevel = getLevel(currentLength-1);
    if (fanfareLevel == FANFARE_NONE) {
      Serial << "Animating failure";
      animateFailure();
      // wait
      Metro delayNow(3000);
      while (! delayNow.check() ) {
        light.update();
      }
    } else {
        playerFanfare(fanfareLevel); 
    }
    quiet();
    
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
  sound.playTone(colorIndex, correctTone);
  
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
  sound.setVolume(0);
  sound.playLose();
  for (int i = 0; i < 6; i++) {
    light.setAllLight(LIGHT_ON, true);
    delay(300);
    light.setAllOff();
    delay(100);
  }

  quiet();
}

// turn off lights and music
void quiet() {
  // Lights and Fire on Tower
  // Lights on Console
  light.setAllOff();

  // Sound on Console and Tower
  sound.stop();

}

// playback a character array of the current correct sequence
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
    // lights and music
    play(correctSequence[i], true);
    // wait
    pauseDuring.reset();
    while ( !pauseDuring.check() ) {
      light.update(); // for resend, really.
    }

    // done
    quiet();

    // pause between next
    // wait
    pauseAfter.reset();
    while ( !pauseAfter.check() ) {
      light.update(); // for resend, really.
    }
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
  } else {
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

void idleFanfare() {
  Serial << F("Gameplay: Idle Fanfare !!") << endl;

  // pick a song
  int minSong = 22;
  int maxSong = 121;
  int song = random(minSong, maxSong + 1);
  Serial << F(" song: ") << song << endl;
  //  musicPlayer.play_P(song_table[song], 0);  // tunez and fire.  yeah.

  Serial << F("Gameplay: Idle fanfare ended") << endl;

  // reset the timer
  delay(300); // be still, my electrical heart
  unsigned long kioskTimerInterval = random(KIOSK_FANFARE_MIN, KIOSK_FANFARE_MAX);
  kioskTimer.interval(kioskTimerInterval);
  Serial << F("Gameplay: idle Fanfare interval reset to ") << kioskTimerInterval << endl;
  kioskTimer.reset();
}

void playerFanfare(byte level) {
  Serial << "***Inside playerFanFare: level: " << level << "\n";
  
  if (!FANFARE_ENABLED) {
    Serial.println("Fanfare disabled");
    return;
  }

  Serial << F("Gameplay: Player fanfare, duration: ") << FANFARE_DURATION_PER_CORRECT * currentLength << endl;

  Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

  // turn up the music
  byte volume = MUSIC_DEFAULT_VOL;
  sound.setVolume(volume);
/*
  // make sweet fire/light/music.
  sound.playWin();
  
  while ( ! fanfareDuration.check() ) {
    // should calculate Light and Fire on Towers here.

    // resend tower commands
    light.update();
  }
*/
    if (level >= FANFARE_1) {
      Serial << "Playing win level 0";
      for(byte n=0; n < 8; n++) {
        for ( byte i = 0; i < N_COLORS; i++ ) {
          if (n % 2 == 0) {
            light.setLight(I_BLU, LIGHT_ON);
            light.setFire(i, LIGHT_ON);
          } else {
            light.setLight(I_BLU, LIGHT_OFF);
            light.setFire(i, LIGHT_OFF);
          }
        }
        if (n % 2 == 0) {
          light.show();
          Metro delayNow(50);
          while (! delayNow.check() ) {
            light.update();
          }
        } else {
          light.show();
          Metro delayNow(400);
          while (! delayNow.check() ) {
            light.update();
          }
        }
      }
    }
    
    light.setAllLight(LIGHT_OFF);
    light.setAllFire(LIGHT_OFF);

  // ramp down the volume to exit the music playing cleanly.
  Metro volumeRampTime(MUSIC_RAMP_DOWN_TIME / volume);
  while ( volume > 0 ) {
    if ( volumeRampTime.check() ) {
      volumeRampTime.reset();
      volume--;
      sound.decVolume();
    }

    // resend tower commands
    light.update();
  }

  // end display
  quiet();

  Serial << F("Gameplay: Player fanfare ended") << endl;
}

// Get the fanfare level to play based on # correct.  Returns 0-3
byte getLevel(int correct) {
  Serial << "Get Level: correct: " << correct << "\n";
  if (correct < FANFARE_LEVEL1) {
    return FANFARE_NONE;
  } else if (correct <= FANFARE_LEVEL2) {
    return FANFARE_1;
  } else if (correct <= FANFARE_LEVEL3) {
    return FANFARE_2;
  } else {
    return FANFARE_3;
  }
}





