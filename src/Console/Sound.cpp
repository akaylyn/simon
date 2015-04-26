#include "Sound.h"

// instantiate
Sound sound;
wavTrigger wav;

bool Sound::begin() {

  Serial << "Sound: startup." << endl;

  // set up comms at 57600 baud
  WTSerial.begin(57600);
  if ( !WTSerial ) {
    Serial << F("Sound: error setting up WAV board serial.") << endl;
    return ( false );
  }

  // start the wav board
  wav.start(&WTSerial);

  // set master gain
  setMasterGain();

  Serial << "Sound: startup complete." << endl;
  return ( true );
}

void Sound::setMasterGain(int gain) {
  wav.masterGain(constrain(gain, -70, 4));
}

int Sound::playTrack(int track, int gain, bool repeat) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  int ga = constrain(gain, -70, 10);

  // set volume
  wav.trackGain(tr, ga);
  // play in polyphonic mode
  wav.trackPlayPoly(tr);
  // set looping
  wav.trackLoop(tr, repeat);

  Serial << F("Sound: starting track:") << tr << F(" gain:") << ga << F(" repeat:") << repeat << endl;

  // adjust volume based on any playing tracks
  relevelVol();
  
  return ( tr );
}

int Sound::playWin(int track, int gain, bool repeat) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trWins) : track, gain, repeat) );
}
int Sound::playLose(int track, int gain, bool repeat) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trLose) : track, gain, repeat) );
}
int Sound::playBaff(int track, int gain, bool repeat) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trBaff) : track, gain, repeat) );
}
int Sound::playRock(int track, int gain, bool repeat) {
  return ( playTrack(track == RANDOM_TRACK ? randomTrack(trRock) : track, gain, repeat) );
}
int Sound::randomTrack(const int (&range)[2]) {
  return ( random(range[0], range[1] + 1) );
}

// convenience function for Fx board. returns track #.
int Sound::playTone(byte colorIndex, int gain) {
  int tr;
  switch ( colorIndex ) {
    case I_RED: tr = trTones[0]; break;
    case I_GRN: tr = trTones[1]; break;
    case I_BLU: tr = trTones[2]; break;
    case I_YEL: tr = trTones[3]; break;
  }
  // looping enabled!
  return (playTrack(tr, gain, true));
}

int Sound::playFailTone(int gain) {
  return (playTrack(trTones[4], gain, true));
}


// Stop a track
void Sound::stopTrack(int track) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  // stop
  wav.trackLoop(tr, false); // looping seems to pooch voice control.
  wav.trackStop(tr);

  Serial << F("Sound: stopping track:") << tr << endl;
}

// Stop all track
void Sound::stopAllTracks() {
  // dammit.  tracks set to loop don't reliably stop.  set to not loop, then turn off
  // playing track indices.
  int tr[14];
  // get the array of currently playing tracks
  wav.getPlayingTracks(tr);
  for( byte i=0;i<14;i++ ) {
    if( tr[i] > 0 ) {
//      Serial << F("Sound: stopping track ") << tr[i] << endl;
      wav.trackLoop(tr[i], false);
    }
  }  
  // stop
  wav.stopAllTracks();

  Serial << F("Sound: stopping all tracks:") << endl;
}

// Fade out track
void Sound::fadeTrack(int track, unsigned long fadeTime) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  // fade, with Stop at the end.  Stop is important, so voices can be freed up.
  wav.trackLoop(tr, false); // looping seems to pooch voice control.
  wav.trackFade(tr, -70, fadeTime, true);

  Serial << F("Sound: fading track:") << tr << F(" stopped in:") << fadeTime << endl;
}

// Adjust volume on playing track
void Sound::setVol(int track, int gain) {
  // enforce limits
  int tr = constrain(track, 1, 999);
  int ga = constrain(gain, -70, 10);

  // set volume
  wav.trackGain(tr, ga);
  
  Serial << F("Sound: volume for track:") << tr << F(" =") << ga << endl;

}

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
  
  // see: https://www.noisemeters.com/apps/db-calculator.asp for the gnarly logarithm mess
  // basically, we're calibrating the tone gain based on the number of tones and tracks currently playing,
  // relative to the total gain that we want out of the system
  int toneGain = TONE_GAIN - floor( 10.0*log10(float(nTones) + float(nTracks)*pow(10.0, float(TRACK_GAIN_RELATIVE_TO_TONE)/10.0)) );
  int trackGain = toneGain + TRACK_GAIN_RELATIVE_TO_TONE;
  
  Serial << "Gain: nTotal=" << nTotal << " nTones=" << nTones << " nTracks=" << nTracks;
  Serial << " tone gain=" << toneGain << " track gain=" << trackGain << endl;
  
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
