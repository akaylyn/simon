#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

// See: https://learn.adafruit.com/adafruit-vs1053-mp3-aac-ogg-midi-wav-play-and-record-codec-tutorial/simple-audio-player-wiring
// and player_interrupts example

// VS1053 mp3:
// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define MP3_CLK 52 // SPI CLK
#define MP3_MISO 50 // SPI MISO
#define MP3_MOSI 51 // SPI MOSI
#define MP3_CS 40 // can move; set to 10 in example; VS1053 chip select pin (output); called BREAKOUT_CS in the example
#define MP3_RST 41 // can move; set to 9 in the example; VS1053 reset pin (output); called BREAKOUT_RESET in the example
#define MP3_XDCS 42 // can move; set to 8 in the example; VS1053 Data/command select pin (output); called BREAKOUT_DCS in the example
#define MP3_SDCS 43 // can move; set to 4 in the example;  Card chip select pin; called CARDCS in the example
#define MP3_DREQ 3 // IRQ 1; set to 3 in the example; VS1053 Data request, ideally an Interrupt pin; called DREQ in the example


Adafruit_VS1053_FilePlayer musicPlayer =
//  Adafruit_VS1053_FilePlayer(MP3_RST, MP3_CS, MP3_XDCS, MP3_DREQ, MP3_SDCS);
  Adafruit_VS1053_FilePlayer( // use hardware SPI with these commented out: MP3_MOSI, MP3_MISO, MP3_CLK, 
                              MP3_RST, MP3_CS, MP3_XDCS, MP3_DREQ, 
                              MP3_SDCS);


void setup() {
  Serial.begin(115200);
  
  // put your setup code here, to run once:
  musicStart();
  musicUnitTest();

}

void loop() {

  // check Serial for commands.
  if( Serial.available() ) {
    byte cmd = Serial.read();
    switch( cmd ) {
      case 't': // unit test
        musicUnitTest();
        break;
      case 'p': // toggle play/pause
        musicPlayer.pausePlaying(!musicPlayer.paused());
        Serial << F("Toggling play/pause.") << endl;
        break;
      case 's': // stop
        musicPlayer.stopPlaying();
        Serial << F("Stopping playback.") << endl;
      default:
        Serial << F("Unknown command.") << endl;
    }   
  }
  
  // store the playing state
  static boolean isPlaying = false;
  // check it, and report if we've stopped
  if( isPlaying && !musicPlayer.playingMusic) { 
    // we've just stopped
    Serial << F("-") << endl;
  }
  // store current state
  isPlaying = musicPlayer.playingMusic;
  
  // put your main code here, to run repeatedly:
  // check if music is running.
  static Metro isPlayingTick(500);
  if (musicIsPlaying() && isPlayingTick.check()) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)
    Serial << F(".");
    isPlayingTick.reset();
  }

}


void musicStart() {
  Serial << F("Startup Adafruit VS1053 breakout.") << endl;

  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
    Serial << F("Couldn't find VS1053, do you have the right pins defined?") << endl;
    while (1);
  }
  Serial << F("VS1053 found") << endl;

  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    Serial << F("DREQ pin is not an interrupt pin.") << endl;

//  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT))
//    Serial << F("Can't use Timer0.") << endl;

  if (!SD.begin(MP3_SDCS)) {
    Serial << F("SD failed, or not present") << endl;
    while (1);  // don't do anything more
  }
  Serial << F("SD OK!") << endl;

}

// returns true if music is running in the background.
boolean musicIsPlaying() {
  return( musicPlayer.playingMusic );
}

void musicUnitTest() {
  // list files
  printDirectory(SD.open("/"), 0);

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(0, 0);

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile("track001.mp3")) {
    Serial.println(F("Could not open file track001.mp3"));
    while (1);
  }
  Serial.println(F("Started playing"));

 /*
 while (musicPlayer.playingMusic) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)
    Serial.print(F("."));
    delay(1000);
  }
  Serial.println(F("Done playing music"));
*/
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


