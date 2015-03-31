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
#define WTSerial Serial1 // use Serial1 hardware

#include <Simon_Common.h> // for color defs.

// define track number for tones
#define N_TONES 5
const int trTones[N_TONES] = {1,2,3,4,5}; // red, grn, blu, yel, wrong

// define track ranges for other play types.  used to select random tracks of these types.
// must be in the range [1,999]
const int trWins[2] = {100, 101};
const int trLose[2] = {300, 300};
const int trRock[2] = {500, 502};
const int trBaff[2] = {700, 701};

// some defaults
// gains.  remember that polyphonic sounds get stacked, so clipping can easily occur.  gonna have to tune this.
#define DEFAULT_MASTER_GAIN 0 // -70 to +4 dB.  0 is nominal.
#define DEFAULT_TRACK_GAIN 0 // -70 to +10.  0 is nominal.
#define DEFAULT_TONE_GAIN 0 // -70 to +10. 0 is nominal.

// fade out time (extro)
#define DEFAULT_FADE_TIME 1000UL // ms it takes from current gain to stop.

#define RANDOM_TRACK 0 // use zero to mean "random", as zero isn't a valid track number.

// Real Keyboard Jockeys could probably write Sound as extending wavTrigger.
class Sound {
    public:
        // fire it up.
        bool begin();

        // Play a track.  
        // track is 1 to 999.
        // gain is track gain -70 to +10.  
        // repeat true signals looping of track.
        // -> returns track #.
        int playTrack(int track, int gain=DEFAULT_TRACK_GAIN, bool repeat=false);
        
        // note that the calling program should store the return value for later stopTrack
        // and fadeTrack usage.  
        
        // Play tracks by type.  track==0 means "random".  returns track #.
        int playWin(int track=RANDOM_TRACK, int gain=DEFAULT_TRACK_GAIN, bool repeat=false);
        int playLose(int track=RANDOM_TRACK, int gain=DEFAULT_TRACK_GAIN, bool repeat=false);
        int playBaff(int track=RANDOM_TRACK, int gain=DEFAULT_TRACK_GAIN, bool repeat=false);
        int playRock(int track=RANDOM_TRACK, int gain=DEFAULT_TRACK_GAIN, bool repeat=false);

        // convenience function for Fx board. returns track #.
        int playTone(byte colorIndex, boolean correctTone=true);

        // Stop a track
        void stopTrack(int track);
        // Stop all tracks
        void stopAllTracks();
        // Fade out track
        void fadeTrack(int track, unsigned long fadeTime=DEFAULT_FADE_TIME);
        
        // NOTE: it's tempting to use "pause" features, but there's only 14 voices available,
        // and "pause" doesn't release a voice slot.  I think it's better to restart.
        
        // unit test for Music
        void unitTest();
        
        // set master gain
        void setMasterGain(int gain=DEFAULT_MASTER_GAIN); // -70 to +4 dB

    private:
        // select a random track
        int randomTrack(const int (&range)[2]);

};

extern Sound sound;
extern wavTrigger wav;

#endif
