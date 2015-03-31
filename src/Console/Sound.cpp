#include "Sound.h"

// instantiate
Sound sound;
wavTrigger wav;

bool Sound::begin() {

    // set up comms at 57600 baud
    WTSerial.begin(57600);
    if( !WTSerial ) {
        Serial << F("Sound: error setting up WAV board serial.") << endl;
        return( false );
    }

    // start the wav board
    wav.start(&WTSerial);

    // set master gain
    setMasterGain();

    return( true );
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

    return( tr );
}

int Sound::playWin(int track, int gain, bool repeat) {
    return( playTrack(track==RANDOM_TRACK ? randomTrack(trWins) : track, gain, repeat) );
}
int Sound::playLose(int track, int gain, bool repeat) {
    return( playTrack(track==RANDOM_TRACK ? randomTrack(trLose) : track, gain, repeat) );
}
int Sound::playBaff(int track, int gain, bool repeat) {
    return( playTrack(track==RANDOM_TRACK ? randomTrack(trBaff) : track, gain, repeat) );
}
int Sound::playRock(int track, int gain, bool repeat) {
    return( playTrack(track==RANDOM_TRACK ? randomTrack(trRock) : track, gain, repeat) );
}
int Sound::randomTrack(const int (&range)[2]) {
    return( random(range[0], range[1]+1) );
}

// convenience function for Fx board. returns track #.
int Sound::playTone(byte colorIndex, boolean correctTone) {
    int tr;
    if ( correctTone ) {
        switch ( colorIndex ) {
            case I_RED: tr=trTones[0]; break;
            case I_GRN: tr=trTones[1]; break;
            case I_BLU: tr=trTones[2]; break;
            case I_YEL: tr=trTones[3]; break;
        }
    } else {
        tr=trTones[4];
    }

    // looping enabled!
    return( playTrack(tr, DEFAULT_TONE_GAIN, true) );
}



// Stop a track
void Sound::stopTrack(int track) {
    // enforce limits
    int tr = constrain(track, 1, 999);
    // stop
    wav.trackStop(tr);

    Serial << F("Sound: stopping track:") << tr << endl;
}

// Stop all track
void Sound::stopAllTracks() {
    // stop
    wav.stopAllTracks();

    Serial << F("Sound: stopping all tracks:") << endl;
}

// Fade out track
void Sound::fadeTrack(int track, unsigned long fadeTime) {
    // enforce limits
    int tr = constrain(track, 1, 999);
    // fade, with Stop at the end.  Stop is important, so voices can be freed up.
    wav.trackFade(tr, -70, fadeTime, true);

    Serial << F("Sound: fading track:") << tr << F(" stopped in:") << fadeTime << endl;
}

