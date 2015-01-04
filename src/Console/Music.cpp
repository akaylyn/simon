#include "Music.h"

// track volume setting.
byte volumeSetting = MUSIC_DEFAULT_VOL;

// sets up sound at startup
boolean musicStart() {
  Serial << F("Music: startup.") << endl;

  // setup speaker
  pinMode(SPEAKER_WIRE, OUTPUT);
  
  // setup comms to Music
  Music.begin(MUSIC_COMMS_RATE);

  // set the default volume
  musicVolumeSet(volumeSetting);
  
  // stop it
  musicStop();
  
  return( Music ); // if there was an error with Serial connection to Music, return false.
}
  
// plays a tone
void musicTone(byte colorIndex, unsigned long duration) {
  unsigned int freq = WRONG_TONE;
  
    // this function will be called after buttonChanged() asserts a change.
  switch ( colorIndex ) {
    case I_RED:
      freq = RED_TONE;
      break;
    case I_GRN:
      freq = GRN_TONE;
      break;
    case I_BLU:
      freq = BLU_TONE;
      break;
    case I_YEL:
      freq = YEL_TONE;
      break;
    case I_NONE:
      freq = WRONG_TONE;
      break;
  }

  if( duration > 0 ) {
    tone(SPEAKER_WIRE, freq, duration); // not blocking
  } else {
    tone(SPEAKER_WIRE, freq); // forever
  }
}

void musicSend(char send) {
  Music.write(send);
  Serial << F("Music: sending ") << send << endl;
}

// stop music on speaker and Music module
void musicStop() {
  // turn off the speaker
  noTone(SPEAKER_WIRE);

  // stop Music
  musicSend('s');
}

// set the volume on the Music module.  We don't have any in-software control over speaker.
void musicVolumeSet(byte level) {
  // constrain
  char send = '0' + constrain(level, MUSIC_MIN_VOL, MUSIC_MAX_VOL);
  // send
  musicSend(send);
  // track volume setting
  volumeSetting = send - '0';  
}
void musicVolumeUp() {
  musicVolumeSet(++volumeSetting);
}
void musicVolumeDown() {
  musicVolumeSet(--volumeSetting);
}

// play a random track from "WINS" subdirectory on Music module
void musicWins() {
  musicSend('w');
}
// play a random track from "LOSE" subdirectory on Music module
void musicLose() {
  musicSend('l');
}
// play a random track from "BAFF" subdirectory on Music module
void musicBaff() {
  musicSend('b');
}
// play a random track from "ROCK" subdirectory on Music module
void musicRock() {
  musicSend('r');
}

// unit test for Music
void musicUnitTest() {
  Serial << F("Music: tone test...") << endl;
  
  musicTone(I_RED);
  delay(500);
  
  musicTone(I_GRN);
  delay(500);

  musicTone(I_BLU);
  delay(500);

  musicTone(I_YEL);
  delay(500);
  
  // send STOP
  musicStop();
  
  // up volume
  musicVolumeSet(MUSIC_MAX_VOL);

  // start a unit test.  
  musicSend('w');
  
  Serial << F("from Music: ");
  while( volumeSetting > 0 ) {
    while( Music.available() > 0) {
      Serial.print( char(Music.read()) );
    }
    delay(1000);
    musicVolumeDown();
  }
  Serial << endl;
  
  musicStop();
  
  Serial << F("Music: unit test stopped playback.") << endl;
}

