#include "Sound.h"


bool Sound::begin() {

  Serial << "Sound::begin: startup." << endl;

  // set up comms at 57600 baud
  // WTSerial.begin(57600);
  // if ( !WTSerial ) {
  //   Serial << F("Sound::begin: error setting up WAV board serial.") << endl;
  //   return ( false );
  // }

  // start the wav board
  wav.start();
  delay(10);

  // set master gain
  setMasterGain();

  // set leveling
  setLeveling();

  // quiet
  stopAll();
  wav.samplerateOffset(0);

  currDrumSet = 0;

  // enable debugging
  wav.setReporting(true);

    // Allow time for the WAV Trigger to respond with the version string and
  //  number of tracks.
  delay(100); 

  int  gNumTracks;              // Number of tracks on SD card
  char gWTrigVersion[VERSION_STRING_LEN];    // WAV Trigger version string
  
  // If bi-directional communication is wired up, then we should by now be able
  //  to fetch the version string and number of tracks on the SD card.
  if (wav.getVersion(gWTrigVersion, VERSION_STRING_LEN)) {
    Serial << gWTrigVersion << endl;
    Serial << "Sound: Track Count: " << wav.getNumTracks() << endl;
  
  }
  else
      Serial.print("WAV Trigger response not available");

  Serial << endl << "Sound::begin: complete." << endl;
  return ( true );
}

void Sound::setMasterGain(int gain) {
  int g = constrain(gain, -70, 4);
  wav.masterGain(g);
  Serial << F("Sound:setMasterGain: gain=") << g << endl;
}

// set how we want volume levels to be applied so that simultaneous playback does not clip.
void Sound::setLeveling(int nTones, int nTracks) {
  // see: https://www.noisemeters.com/apps/db-calculator.asp for the gnarly logarithm mess
  // basically, we're calibrating the tone gain based on the number of tones and tracks currently playing,
  // relative to the total gain that we want out of the system
  this->toneGain = TONE_GAIN - floor( 10.0*log10(float(nTones) + float(nTracks)*pow(10.0, float(TRACK_GAIN_RELATIVE_TO_TONE)/10.0)) );
  this->trackGain = this->toneGain + TRACK_GAIN_RELATIVE_TO_TONE;

  Serial << F("Sound::setLeveling: nTones=") << nTones << F(" with gain=") << this->toneGain;
  Serial << F(" nTracks=") << nTracks << F(" with gain=") << this->trackGain << endl;
}

int Sound::playWins(int track) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trWins) : track, this->trackGain) );
}
int Sound::playLose(int track) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trLose) : track, this->trackGain) );
}
int Sound::playBaff(int track) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trBaff) : track, this->trackGain) );
}
int Sound::playRock(int track) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trRock) : track, this->trackGain) );
}
int Sound::randomTrack(const int (&range)[2]) {
  return ( random(range[0], range[1] + 1) );
}

int Sound::playTrack(int track) {
  return ( playTrack(track, this->trackGain) );
}

int Sound::playTrack(int track, int gain) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  int ga = constrain(gain, -70, 10);

  // set volume
  wav.trackGain(tr, ga);
  // play in polyphonic mode
  wav.trackPlaySolo(tr);

  Serial << F("Sound::playTrack: track=") << tr << F(" gain=") << ga << endl;

  return ( tr );
}

// Stop a track
void Sound::stopTrack(int track) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  // stop
  wav.trackStop(tr);

  Serial << F("Sound::stopTrack track=") << tr << endl;
}

// Fade out track
void Sound::fadeTrack(int track, unsigned long fadeTime) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  // fade, with Stop at the end.  Stop is important, so voices can be freed up.
  wav.trackFade(tr, -70, fadeTime, true);

  Serial << F("Sound::fadeTrack track=") << tr << F(" in(ms)=") << fadeTime << endl;
}


// Fade out track and into another
void Sound::crossFadeTrack(int extro, int intro, unsigned long fadeTime) {
  // enforce limits
  int ex = constrain(extro, 1, 999);
  int in = constrain(intro, 1, 999);

  // fade, with Stop at the end.  Stop is important, so voices can be freed up.
  wav.trackFade(ex, in, this->trackGain, fadeTime);

  Serial << F("Sound::crossFadeTrack track=") << ex << F(" into=") << in << F(" in(ms)=") << fadeTime << endl;
}

// convenience function for Fx board. returns track #.
int Sound::playTone(byte colorIndex) {
  int tr;
  switch ( colorIndex ) {
    case I_RED: tr = trTones[0]; break;
    case I_GRN: tr = trTones[1]; break;
    case I_BLU: tr = trTones[2]; break;
    case I_YEL: tr = trTones[3]; break;
  }

  return (playTrack(tr, this->toneGain));
}

void Sound::stopTone(byte colorIndex) {
  int tr;
  switch ( colorIndex ) {
    case I_RED: tr = trTones[0]; break;
    case I_GRN: tr = trTones[1]; break;
    case I_BLU: tr = trTones[2]; break;
    case I_YEL: tr = trTones[3]; break;
  }

  return (stopTrack(tr));
}

int Sound::playFailTone() {
  return (playTrack(trTones[4], this->toneGain));
}
void Sound::stopFailTone() {
  return (stopTrack(trTones[4]));
}


// Stop all tones track
void Sound::stopTones() {
  // get the array of currently playing tracks
  // int tr[14];
  // wav.getPlayingTracks(tr);

  // // loop across and issue a stop command
  // for( byte i=0; i<14; i++ ) {
  //   if( tr[i] > 0 && tr[i] <= N_TONES) wav.trackStop(tr[i]);
  // }

  wav.stopAllTracks();

//  for( int ti=0; ti<N_TONES; ti++ ) {
//    // stop
//    wav.trackStop(trTones[ti]);
//  }

//  Serial << F("Sound::stopTones") << endl;
}



// Stop all track
void Sound::stopAll() {
  // get the array of currently playing tracks
  // int tr[14];
  // wav.getPlayingTracks(tr);

  // // loop across and issue a stop command
  // for( byte i=0; i<14; i++ ) {
  //   if( tr[i] > 0 ) wav.trackStop(tr[i]);
  // }

  // and, be damned sure
  // stop
  wav.stopAllTracks();
  wav.flush();

//  Serial << F("Sound::stopAll") << endl;
}



// Adjust volume on playing track
void Sound::setVolume(int track, int gain) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  int ga = constrain(gain, -70, 10);

  // set volume
  wav.trackGain(tr, ga);

//  Serial << F("Sound: volume for track:") << tr << F(" =") << ga << endl;
}

/*
// Relevel volume on playing tracks to summed 0dB gain prevent clipping
void Sound::relevelVol() {
  // playing track indices.
  int tr[14];
  // get the array of currently playing tracks
  wav.getPlayingTracks(tr);

  // figure out what we've got in the way of total tracks, tone and music tracks
  byte nTotal = 0;
  byte nTones =0;
  byte nTracks = 0;
  for( byte i=0;i<14;i++ ) {
    if( tr[i] > 0 ) {
//      Serial << "TR: " << tr[i] << endl;
      nTotal++;
      if( tr[i] <= 100 ) nTones++; // any track number less than 100 is a tone
      else nTracks++;
    }
  }

  // with that, execute the volume adjustments per track
  // figure out what we've got in the way of total tracks, tone and music tracks
  for( byte i=0;i<14;i++ ) {
    if( tr[i] > 0 ) {
      if( tr[i] <= 100 ) setVol(tr[i], toneGain); // any track number less than 100 is a tone
      else setVol(tr[i], trackGain);
    }
  }

  // QED
}
*/

// unit test for Music
void Sound::unitTest() {

  // quiet
  this->stopAll();

  // try out leveling to confirm 4x tones and 0x tracks don't clip;
  this->setLeveling(4, 0);
  this->playTone(I_RED);
  delay(1000);
  this->playTone(I_GRN);
  delay(1000);
  this->playTone(I_BLU);
  delay(1000);
  this->playTone(I_YEL);
  delay(1000);

  delay(5000);
  this->stopTone(I_RED);
  delay(5000);

  this->stopTones();

  // try out leveling to confirm 1x tones and 1x tracks don't clip;
  this->setLeveling(1, 1);

  this->playTone(I_RED);
  delay(1000);
  int tr = this->playWins(101);
  delay(1000);
  this->stopTones();
  delay(5000);

  this->stopAll();

  // try out leveling to confirm zero tones and 1x tracks don't clip;
  this->setLeveling(0, 1);

  tr = this->playWins(101);
  delay(5000);

  this->stopAll();

}

int Sound::playDrumSound(byte colorIndex) {
  return playTrack(trDrum[0] + currDrumSet + colorIndex, this->toneGain);
}

int Sound::nextDrumSet() {
  Serial << "Set: " << currDrumSet << endl;
  currDrumSet += N_COLORS;
  if (trDrum[0] + currDrumSet >= trDrum[1]) {
    currDrumSet = 0;
  }
  return currDrumSet;
}

int Sound::prevDrumSet() {
  Serial << "Set: " << currDrumSet << endl;
  if (trDrum[0] + currDrumSet <= trDrum[0]) {
    currDrumSet = trDrum[1] - trDrum[0] - N_COLORS;
  } else {
    currDrumSet -= N_COLORS;
  }
  return currDrumSet;
}

char* drumKitLabels[] = {
  "Disco 1",
  "Disco 2",
  "Beep Boop",
  "Tribal",
};

char* Sound::getLabel(int drumSet) {
  int id = drumSet + trDrum[0];
  if (id == 710)
    return drumKitLabels[0];
  if (id == 714)
    return drumKitLabels[1];
  if (id == 718)
    return drumKitLabels[2];
  if (id == 722)
    return drumKitLabels[3];
}

char* Sound::getCurrLabel() {
  return getLabel(currDrumSet);
}


// instantiate
Sound sound;
wavTrigger wav;

