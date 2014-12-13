#include "Music.h"

// at the end of music playback, this character will be sent.
char stopCharacter;

// sets up sound at startup
void musicStart() {
  Serial << F("Sound: startup.") << endl;

  // setup speaker
  pinMode(SPEAKER_WIRE, OUTPUT);
  
  // setup comms to Music
  Music.begin(MUSIC_COMMS_RATE);
  
  // turn off music, should return a Serial stream terminated with the stop character
  musicQuiet(); 
  while( Music.available() ) stopCharacter = Music.read();
  
  Serial << F("Sound: stop character is: ") << stopCharacter << endl;
  
  // set the default volume
  musicVolume(MUSIC_DEFAULT_VOL);
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

// stop music on speaker and Music module
void musicQuiet() {
  // turn off the speaker
  noTone(SPEAKER_WIRE);
  // stop Music
  const char send[5] = "STOP";
  Music.write(send);
}

// return true if the Music module is playing a track
boolean musicIsPlaying() {
  // track what we get from Music, as it only sends on an interval.
  static char recvFromMusic = stopCharacter;

  // fast-forward through Serial stream to the last byte.
  while( Music.available() ) recvFromMusic = Music.read();

  // check against stop character
  if( recvFromMusic == stopCharacter ) return(false);
  return(true);
}

// set the volume on the Music module.  We don't have any in-software control over speaker.
void musicVolume(byte level) {
  char send[5] = "VOL9";
  if( level > 9 ) {
    send[3] = '9';
  } else {
    send[3] = '0' + level;
  }
  Music.write(send);
}

// play a random track from "WINS" subdirectory on Music module
void musicWins() {
  const char send[5] = "WINS";
  Music.write(send);
}
// play a random track from "LOSE" subdirectory on Music module
void musicLose() {
  const char send[5] = "LOSE";
  Music.write(send);
}
// play a random track from "BAFF" subdirectory on Music module
void musicBaff() {
  const char send[5] = "BAFF";
  Music.write(send);
}
// play a random track from "ROCK" subdirectory on Music module
void musicRock() {
  const char send[5] = "ROCK";
  Music.write(send);
}

