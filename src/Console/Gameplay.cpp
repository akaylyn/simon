#include "Gameplay.h"

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

// the state machine controls which of the states get attention and execution time
FSM simon = FSM(idle); //initialize state machine, start in state: idle

// startup
void gameplayStart() {
    Serial << F("Gameplay: startup.") << endl;
}

// call this from loop()
boolean gameplayUpdate() {

    // update FSM
    simon.update();

    // return true if playing
    return( !simon.isInState( idle ) );

}


// when there's nobody playing, we're in this state.
void idleState() {
    //Serial << F("Current State: Idle") << endl;
    if ( currentLength > 0 ) {
        // somebody played, so let's give them a show
        playerFanfare();
        // reset timers
        kioskTimer.reset();
    }
    // reset everything.
    correctLength = 0;
    currentLength = 0;
    correctSequence[0] = '\0';

    // check buttons for game play start
    if ( buttonAnyPressed() || touchAnyPressed() ) {
        Serial << F("Gameplay: Idle->Game") << endl;
        quiet();
        while (1) {
            if (buttonAnyChanged())
                break;
            if (touchAnyChanged())
                break;
        }
        // let's play a game
        simon.transitionTo(game);
    } else if ( kioskTimer.check() ) {
        // let's do some light, music and fire
        idleFanfare();
    }
    
    // stubbing in microphone pickup activity.  
    // doesn't do anything useful; just makes a tone that should be synchronized with the beat.
    if( micIsBeat() ) {
      tone( SPEAKER_WIRE, 63 ); // 63 Hz tone, which is the center of the bin used to detect beats.
    } else {
      noTone( SPEAKER_WIRE );
    }
    
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
    // reset correct length
    correctLength = 0;
    // give it back to the player
    Serial << F("Gameplay: Game->Player") << endl;
    simon.transitionTo(player);
}

boolean pressed(byte i) {
    return buttonPressed(i) || touchPressed(i);
}

// waiting for the player to press a button.
void playerState() {
    //Serial << F("Current State: Player") << endl;
    // check if they waited too long
    if ( playerTimeout.check() ) {
        Serial << F("Gameplay: Player timeout.  currentLength = ") << currentLength << endl;
        // if so, show the correct next button
        play(correctSequence[correctLength], false);
        // wait
        Metro delayNow(2000);
        while (! delayNow.check() ) {
            towerUpdate();
        }
        quiet();
        // and exit game play
        Serial << F("Gameplay: Player->Idle") << endl;
        simon.transitionTo(idle);
    }

    // otherwise, wait for button press.
    if ( pressed(I_ALL) ) {
        boolean correct =
            (pressed(I_GRN) && correctSequence[correctLength] == 'G') ||
            (pressed(I_RED) && correctSequence[correctLength] == 'R') ||
            (pressed(I_BLU) && correctSequence[correctLength] == 'B') ||
            (pressed(I_YEL) && correctSequence[correctLength] == 'Y');

        // there's a total cheat.
        correct = correct || CHEATY_PANTS_MODE;

        // light and music
        play( correctSequence[correctLength], correct );
        // hold it while we're mashing
        while ( pressed(I_ALL) ) {
            towerUpdate();
        }

        // done
        quiet();

        if ( correct ) {
            correctLength++;
            // reset timeout
            playerTimeout.reset();
            // keep going
        } else {
            Serial << F("Gameplay: Player incorrect.  currentLength = ") << currentLength << endl;
            // if so, show the correct next button
            play(correctSequence[correctLength], false);
            // wait
            Metro delayNow(3000);
            while (! delayNow.check() ) {
                towerUpdate();
            }
            quiet();
            // exit game play
            Serial << F("Gameplay: Player->Idle") << endl;
            simon.transitionTo(idle);
        }
    }

    // check if they've got the sequence complete, or if there's just awesome
    if ( correctLength == currentLength || correctLength == (MAX_SEQUENCE_LENGTH - 1) ) {
        // nice.  pass it back to game
        Serial << F("Gameplay: Player correct.  currentLength = ") << currentLength << endl;
        Serial << F("Gameplay: Player->Game") << endl;
        simon.transitionTo(game);
    }
}

// generate a character to append to the current correct sequence
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



// helper function to tie together Tower, Light, Sound
void setSoundLights(byte colorIndex, boolean correctTone) {
    // Lights on Tower
    towerLightSet(colorIndex, LIGHT_ON);

    // Sound on Console and Tower
    musicTone(correctTone? colorIndex : I_NONE);

    // Lights on Console
    lightSet(colorIndex, LIGHT_ON);
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

// turn off lights and music
void quiet() {
    // Lights and Fire on Tower
    towerQuiet();

    // Sound on Console and Tower
    musicStop();

    // Lights on Console
    lightSet(I_ALL, LIGHT_OFF);
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
            towerUpdate(); // for resend, really.
        }

        // done
        quiet();

        // pause between next
        // wait
        pauseAfter.reset();
        while ( !pauseAfter.check() ) {
            towerUpdate(); // for resend, really.
        }
    }
}

// update lighting on an interval when there's nobody playing
void idleUpdate() {
    // as long as we send heartbeats, Towers take care of the update pattern
    towerUpdate();
}

// generate a random number on the interval [a, b] with mode c.
unsigned long trandom(int a, int c, int b) {
    // using a triangular pdf
    // http://en.wikipedia.org/wiki/Triangular_distribution#Generating_Triangular-distributed_random_variates

    float cut = (c - a) / (b - a);

    float u = random(0, 101) / 100;

    if ( u < cut ) return ( a + sqrt( u * (b - a) * (c - a) ) );
    else return ( b - sqrt( (1 - u) * (b - a) * (b - c) ) );

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

void playerFanfare() {
    if (!FANFARE_ENABLED) {
        Serial.println("Fanfare disabled");
        return;
    }

    Serial << F("Gameplay: Player fanfare, duration: ") << FANFARE_DURATION_PER_CORRECT * currentLength << endl;

    Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

    // turn up the music
    byte volume = MUSIC_DEFAULT_VOL;
    musicVolumeSet(volume);

    // make sweet fire/light/music.
    musicWins();
    while( ! fanfareDuration.check() ) {
        // should calculate Light and Fire on Towers here.

        // resend tower commands
        towerUpdate();
    }

    // ramp down the volume to exit the music playing cleanly.
    Metro volumeRampTime(MUSIC_RAMP_DOWN_TIME / volume);
    while( volume > 0 ) {
        if( volumeRampTime.check() ) {
            volumeRampTime.reset();
            volume--;
            musicVolumeDown();
        }

        // resend tower commands
        towerUpdate();
    }

    // end display
    quiet();

    Serial << F("Gameplay: Player fanfare ended") << endl;
}





