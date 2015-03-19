/*
 * New music subunit.
 * Responsible for UX (sound) output.
 * Coordinates outboard Music.
 */

#ifndef Sound_h
#define Sound_h

// Speaker
//#define SPEAKER_WIRE 3 // can move, PWM needed

#include <Arduino.h>
#include <Simon_Common.h> // sizes, indexing defines.
#include <Streaming.h> // <<-style printing
#include <EasyTransfer.h> // data transfer between arduinos
#include <SoundMessage.h> // sound message structure and consts

// tone frequencies for each note
#define NOTE_DS4 311
#define RED_TONE NOTE_DS4   // Red 310 Hz D#4 (true pitch 311.127 Hz)
#define NOTE_GS4 415
#define GRN_TONE NOTE_GS4   // Green 415 Hz G#4 (true pitch 415.305 Hz)
#define NOTE_GS3 208
#define BLU_TONE NOTE_GS3   // Blue 209 Hz G#3 (true pitch 207.652 Hz)
#define NOTE_B3  248
#define YEL_TONE NOTE_B3    // Yellow 252 Hz B3 (true pitch 247.942 Hz)
#define WRONG_TONE 42       // a losing tone of 42 Hz

#define N_TRIGGER 11
enum Trigger {
  TR_BAFF=0,
  TR_RED=1, TR_GRN=2, TR_BLU=3, TR_YEL=4, TR_WRONG=5,
  // unused, currently, from here on out
  TR_U1=6, TR_U2=7, TR_U3=8, TR_U4=9, TR_U5=10
};
const int pin[N_TRIGGER] = {
  30,
  31,32,33,34,35,
  36,37,38,39,40
};

#define FX_RESET 41
#define FX_VOL_UP 42
#define FX_VOL_DOWN 43 
#define FX_PRESS_DELAY 1UL

// communications with Music module via Serial port
#define Music Serial2
#define MUSIC_COMMS_RATE 19200

// default sound level on Music module [0,9]
#define MUSIC_MAX_VOL 0
#define MUSIC_DEFAULT_VOL 100
#define MUSIC_MIN_VOL 255

class Sound {
    byte volume;
    SoundMessage message;
    EasyTransfer easyTransfer;

    public:
        bool begin();

        // Play tracks by type
        void playWin(int playCount=1);
        void playLose();
        void playBaff();
        void playRock(int playCount=1);

        // Control volume level
        void setVolume(int level);
        void incVolume();
        void decVolume();

        // Control Fx board:
        void fxReset();
        void fxOn(Trigger t);
        void fxOff(Trigger t);
        void fxAllOff();
        void fxVolUp();
        void fxVolDown();
        void fxVolMax();
        void fxVolMin();
        // convience function for Fx board
        void playTone(byte colorIndex, boolean correctTone=true);

        // Stop playing sounds
        void stop();
        
        // unit test for Music
        void unitTest();

    private:
        // Send request to play a sound (using EasyTransfer)
        void sendData();

};

extern Sound sound;

#endif

