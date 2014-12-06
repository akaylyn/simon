// mp3 player: VS1053 include SPI, SD and VS1053 libaries libraries

#ifndef Music_h
#define Music_h

#include "Pinouts.h"

#include <Arduino.h>

#include <Streaming.h> // <<-style printing

// tone frequencies for each note
#define NOTE_DS4 311
#define RED_TONE NOTE_DS4   // Red 310 Hz D#4 (true pitch 311.127 Hz)
#define NOTE_GS4 415
#define GRN_TONE NOTE_GS4   // Green 415 Hz G#4 (true pitch 415.305 Hz)
#define NOTE_GS3 208
#define BLU_TONE NOTE_GS3   // Blue 209 Hz G#3 (true pitch 207.652 Hz) 
#define NOTE_B3  247
#define YEL_TONE NOTE_B3    // Yellow 252 Hz B3 (true pitch 247.942 Hz)
#define WRONG_TONE 42       // a losing tone of 42 Hz 

#define NUM_SONGS 139

// sets up speaker directly connected to Mega.
void configureManualSound();

// plays a tone
void playTone(unsigned int freq, unsigned long duration=0);

#endif
