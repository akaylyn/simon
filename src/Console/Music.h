// Music subunit.  Responsible for UX (sound) output.  Coordinates outboard Music.

#ifndef Music_h
#define Music_h

#include "Pinouts.h"
#include <Arduino.h>
#include <Simon_Comms.h> // sizes, indexing defines.

#include <Streaming.h> // <<-style printing

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
#define Music Serial1
#define MUSIC_COMMS_RATE 115200

// default sound level on Music module [0,9]
#define MUSIC_MAX_VOL 9
#define MUSIC_DEFAULT_VOL 7
#define MUSIC_MIN_VOL 0

// sets up sound at startup
void musicStart();

// plays a tone
void musicTone(byte colorIndex, unsigned long duration=0);

// stop sound on speaker and Music module
void musicQuiet();

// set the volume on the Music module.  We don't have any in-software control over speaker.
void musicVolume(byte level);

// return true if the Music module is playing a track
boolean musicIsPlaying();

// play a random track from "WINS" subdirectory on Music module
void musicWins();
// play a random track from "LOSE" subdirectory on Music module
void musicLose();
// play a random track from "BAFF" subdirectory on Music module
void musicBaff();
// play a random track from "ROCK" subdirectory on Music module
void musicRock();

#endif
