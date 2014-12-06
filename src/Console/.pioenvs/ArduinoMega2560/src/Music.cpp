#include "Music.h"

// See: https://learn.adafruit.com/adafruit-vs1053-mp3-aac-ogg-midi-wav-play-and-record-codec-tutorial/simple-audio-player-wiring
// and player_interrupts example

Adafruit_VS1053_FilePlayer musicPlayer =
  Adafruit_VS1053_FilePlayer(MP3_RST, MP3_CS, MP3_XDCS, MP3_DREQ, MP3_SDCS);

void musicStart() {
  Serial << F("Startup Adafruit VS1053 breakout.") << endl;

  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
    Serial << F("Couldn't find VS1053, do you have the right pins defined?") << endl;
    while (1);
  }
  Serial << F("VS1053 found") << endl;

  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    Serial << F("DREQ pin is not an interrupt pin") << endl;


  if (!SD.begin(MP3_SDCS)) {
    Serial << F("SD failed, or not present") << endl;
    while (1);  // don't do anything more
  }
  Serial << F("SD OK!") << endl;

}

void musicUnitTest() {
  // list files
  printDirectory(SD.open("/"), 0);

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile("track001.mp3")) {
    Serial.println(F("Could not open file track001.mp3"));
    while (1);
  }
  Serial.println(F("Started playing"));

  while (musicPlayer.playingMusic) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)
    Serial.print(F("."));
    delay(1000);
  }
  Serial.println(F("Done playing music"));

}


// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       Serial.println(F("**nomorefiles**"));
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print(F("\t"));
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println(F("/"));
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print(F("\t\t"));
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}


