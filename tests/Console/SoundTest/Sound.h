/*
 * New music subunit.
 * Responsible for UX (sound) output.
 * Coordinates outboard Music.
 */

#ifndef Sound_h
#define Sound_h

// Speaker
#define SPEAKER_WIRE 3 // can move, PWM needed

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

        void playTone(byte colorIndex, unsigned long duration=0);

        // Stop playing sounds
        void stop();

    private:
        // Send request to play a sound (using EasyTransfer)
        void sendData();

};

// unit test for Music
void soundTest();

#endif

