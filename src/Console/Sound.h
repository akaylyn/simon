/*
 * New music subunit.
 * Responsible for UX (sound) output.
 * Coordinates outboard Music.
 */

#ifndef Sound_h
#define Sound_h
#include <Arduino.h>
#include <Streaming.h>

// communications with WAV module via Serial port
#include <wavTrigger.h>
#define WTSerial Serial2

#include <Simon_Common.h> // for color defs.

// define track number for tones
#define N_TONES 5
const int trTones[N_TONES] = {1, 2, 3, 4, 5}; // red, grn, blu, yel, wrong

// define track ranges for other play types.  used to select random tracks of these types.
// must be in the range [1,999]
const int trWins[2] = {102, 102}; // store WINS in 100-299
const int trLose[2] = {300, 300}; // store LOSE in 300-499
const int trRock[2] = {501, 501}; // store ROCK in 500-699
const int trBaff[2] = {700, 701}; // store BAFF in 700-999

#define BOOP_TRACK 100
#define ARMED_TRACK 900
#define DISARMED_TRACK 901

// some defaults
// gains.  remember that polyphonic sounds get stacked, so clipping can easily occur.  gonna have to tune this.
#define MASTER_GAIN -3 // -70 to +4 dB.  0 is nominal.
#define TONE_GAIN_RELATIVE_TO_MASTER 0 // we want the tones to be as loud as possible
#define TRACK_GAIN_RELATIVE_TO_TONE -12 // other tracks (music) should be slighly quieter

#define TONE_GAIN MASTER_GAIN + TONE_GAIN_RELATIVE_TO_MASTER // proportionality is additive in log-space
#define TRACK_GAIN TONE_GAIN + TRACK_GAIN_RELATIVE_TO_TONE // proportionality is additive in log-space

// fade out time (extro)
#define FADE_TIME 1000UL // ms it takes from current gain to stop.

#define RANDOM_TRACK 0 // use zero to mean "random", as zero isn't a valid track number.

// Real Keyboard Jockeys could probably write Sound as extending wavTrigger.
class Sound {
  public:
    // fire it up.
    bool begin();

    // set master gain
    void setMasterGain(int gain = MASTER_GAIN); // -70 to +4 dB

    // set how we want volume levels to be applied so that simultaneous playback does not clip.
    void setLeveling(int nTones=N_TONES-1, int nTracks=1);
    // e.g. 
    //  bongo: setLeveling(4, 0) -> reduce individual track gains so that 4x tones and zero tracks can play simultaneously
    //  gameplay: setLeveling(1, 1) -> reduce individual track gains so that 1x tones and 1x track (ROCK) can play simultaneously
    //  win/lose: setLevel(0, 1) -> reduce individual track gains so that zero tones and 1x track (WINS/LOSE) can play simultaneously
    
    // note that the calling program should store the return value for later stopTrack
    // and fadeTrack usage.

    // Play tracks by type.  track==0 means "random".  returns track #.
    int playWins(int track = RANDOM_TRACK);
    int playLose(int track = RANDOM_TRACK);
    int playBaff(int track = RANDOM_TRACK);
    int playRock(int track = RANDOM_TRACK);
    
    // Play a specific track by number
    int playTrack(int track);
    
    // Stop a track
    void stopTrack(int track);
    // Fade out track
    void fadeTrack(int exitingTrack, unsigned long fadeTime = FADE_TIME);
    // Crossfade from a track to a track
    void crossFadeTrack(int exitingTrack, int enteringTrack, unsigned long fadeTime = FADE_TIME);

    // convenience function for Fx board. returns track #.
    int playTone(byte colorIndex);
    void stopTone(byte colorIndex);

    int playFailTone();
    void stopFailTone();

    // Stop tones
    void stopTones();
    // Stop tones and tracks
    void stopAll();
      
    // unit test for Music
    void unitTest();
    
    // set volume manually.  THIS IS VERY LIKELY TO CREATE CLIPPING UNLESS YOU KNOW WHAT YOU'RE DOING
    void setVolume(int track, int gain);

  private:
    // Play a track.  INTENTIONALLY private to preserve gain settings
    // track is 1 to 999.
    // gain is track gain -70 to +10.
    // -> returns track #.
    int playTrack(int track, int gain);

    // select a random track
    int randomTrack(const int (&range)[2]);
    
    // store tone and volume levels
    int toneGain, trackGain;

};

extern Sound sound;
extern wavTrigger wav;

#endif
