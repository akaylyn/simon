// mp3 player: VS1053 include SPI, SD and VS1053 libaries libraries

#ifndef Music_h
#define Music_h

#include "Pinouts.h"

#include <Arduino.h>

#include <Streaming.h> // <<-style printing

#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// 
void musicStart();

//
void musicUnitTest();

// File listing helper
void printDirectory(File dir, int numTabs);

#endif
