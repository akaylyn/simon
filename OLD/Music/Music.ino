// Compile for Uno Pro or Uno Pro Mini

#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <EasyTransfer.h> // data transfer between arduinos
#include <SoundMessage.h> // message structure (used with EasyTransfer)
#include "Music.h"

// track playing status
byte playingWhat = 0;
// instantiate
Adafruit_VS1053_FilePlayer musicPlayer =
//  Adafruit_VS1053_FilePlayer(MP3_RST, MP3_CS, MP3_XDCS, MP3_DREQ, MP3_SDCS);
Adafruit_VS1053_FilePlayer( // use hardware SPI with these commented out: MP3_MOSI, MP3_MISO, MP3_CLK,
        MP3_RST, MP3_CS, MP3_XDCS, MP3_DREQ,
        MP3_SDCS);

// holds the location of the tracks
const char dirBaff[] = "BAFF";
const char dirWins[] = "WINS";
const char dirLose[] = "LOSE";
const char dirRock[] = "ROCK";

const int NUM_DIRECTORIES = 4;

EasyTransfer easyTransfer;
SoundMessage message;
DirInfo musicDir[NUM_DIRECTORIES];

void setup() {
    Serial.begin(SERIAL_SPEED);

    // put your setup code here, to run once:
    musicStart();

    // set random seed from analog noise
    randomSeed(analogRead(A0));
    easyTransfer.begin(details(message), &Serial);

    // setup LED pin
    pinMode(LED_PIN, OUTPUT);
}

void toggleLED() {
    static boolean state = false;

    state = !state;

    digitalWrite(LED_PIN, state);
}

void serialEvent() {
    if (easyTransfer.receiveData()) {

        playRandomTrack(musicDir[message.type]);
    }
}

void loop() {

    // check playing state, and report if we've stopped
    if (message.type > TYPE_STOP && !musicPlayer.playingMusic) {
        Serial << endl; // it adds carriage return after a string of dots, see loop below
        musicPlayer.setVolume(message.volume, message.volume);
        playRandomTrack(musicDir[message.type]);
    } else if ( message.type == TYPE_STOP && musicPlayer.playingMusic ) {
        musicPlayer.stopPlaying();
    } else if ( message.type == TYPE_VOLUME ) {
        musicPlayer.setVolume(message.volume, message.volume);
    }

    // put your main code here, to run repeatedly:
    // check if music is running.
    static Metro isPlayingTick(500);
    if (musicPlayer.playingMusic && isPlayingTick.check()) {
        // file is now playing in the 'background' so now's a good time
        // to do something else like handling LEDs or buttons :)
        Serial << F(".");
        toggleLED();
        isPlayingTick.reset();
    }
}

void playRandomTrack(DirInfo &dirInfo) {
    // soft reset.  hate to have this here, but when I flip btw codec types, I get static w/o this.
    musicPlayer.stopPlaying();
    musicPlayer.sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);

    File dir = SD.open(dirInfo.loc);
    if ( ! dir.isDirectory() ) {
        Serial << F("Error! not a directory: ") << dirInfo.loc << endl;
    }
    dir.rewindDirectory(); // can't seem to cleanly get the SD card file handlers to start at the top of the dir!!!!

    // pick a track
    int track = random(0, dirInfo.count);

    // seek file
    File entry = dir.openNextFile(); // have to open a file simply to get it's name!!
    entry.close(); // not actually opening them.  just getting name.

    //  Serial << entry.name() << F("\t");
    for ( int i = 0; i < track; i++ ) {
        entry = dir.openNextFile(dir);
        entry.close();
        //   Serial << entry.name() << F("\t");z
    }
    //  Serial << endl;

    dir.close();

    Serial << endl << F(" -> ") << dir.name() << F("/") << entry.name() << F(" [") << track + 1 << F("/") << dirInfo.count << F("]") << endl;

    // make sure we've stopped.
    while (! musicPlayer.stopped() );

    // Start playing a file, then we can do stuff while waiting for it to finish
    if (! musicPlayer.startPlayingFile(strcat(strcat(dir.name(), "/"), entry.name()))) {
        Serial.println(F("Could not open file."));
        while (1);
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
    //   Serial << F("Can't use Timer0.") << endl;

    if (!SD.begin(MP3_SDCS)) {
        Serial << F("SD failed, or not present") << endl;
        while (1);  // don't do anything more
    }
    Serial << F("SD OK!") << endl;

    Serial << F("Track counts:") << endl;
    configMusicDir(musicDir[TYPE_BAFF], dirBaff);
    configMusicDir(musicDir[TYPE_WIN], dirWins);
    configMusicDir(musicDir[TYPE_LOSE], dirLose);
    configMusicDir(musicDir[TYPE_ROCK], dirRock);

    // show counts

    // Set volume for left, right channels. lower numbers == louder volume!
    musicPlayer.setVolume(0, 0);

    // wait for it.
    Serial.flush();
}

void configMusicDir(DirInfo &dirInfo, const char *loc) {
    dirInfo.count = countTracks(loc);
    dirInfo.loc = loc;
}

void musicUnitTest() {
    // soft reset.  hate to have this here, but when I flip btw codec types, I get static w/o this.
    musicPlayer.stopPlaying();
    musicPlayer.sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);

    // make sure we've stopped.
    while (! musicPlayer.stopped() );

    // Start playing a file, then we can do stuff while waiting for it to finish
    if (! musicPlayer.startPlayingFile("UnitTest.mp3")) {
        Serial.println(F("Could not open file UnitTest.mp3"));
        while (1);
    }
    Serial.println(F("Started playing UnitTest.mp3"));
}

// File listing helper
int countTracks(const char *loc) {
    int count = 0; // track number

    File dir = SD.open(loc);

    while (true) {

        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files
            //      Serial << dir.name() << F(" track count: ") << count << endl;
            //     dir.rewindDirectory(); // key!
            break;
        }
        if (!entry.isDirectory()) {
            count ++; // track
        }
        entry.close();
    }

    dir.close();
    Serial << loc << F(": ") << count << endl;
    return (count);
}


