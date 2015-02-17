// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Arduino.h>
#include <Simon_Indexes.h> // sizes, indexing defines.
#include <Streaming.h> // <<-style printing
#include <EasyTransfer.h> // data transfer between arduinos
#include <SoundMessage.h> // sound message structure and consts
#include "Sound.h"

EasyTransfer easyTransfer;
Sound sound;

void setup() {

    Serial << "!! setup !!" << endl;

    // put your setup code here, to run once:
    // attach interrupt to pin - interrupt 1 is on pin 2 of the arduino (confusing I know)
    // attachInterrupt(0, dataAvailable, FALLING);
    Serial.begin(115200);

    // random seed.
    randomSeed(analogRead(0));
    if (!sound.begin()) {
        Serial << "ERROR: Sound.begin failed" << endl;
    }
}

// main loop for the core.
void loop() {
    Serial << "~~ loop ~~" << endl;
    // just to make sure not to overload the console
    delay(100);

    Serial << "Playing WIN" << endl;
    sound.playWin();
    delay(500);

    Serial << "Playing LOSE" << endl;
    sound.playLose();
    delay(500);

    Serial << "Playing BAFF" << endl;
    sound.playBaff();
    delay(500);

    Serial << "Playing ROCK" << endl;
    sound.playRock();
    delay(500);

    Serial << "Playing TONE: RED" << endl;
    sound.playTone(I_RED, 1000);
    delay(500);


    Serial << "Tests complete." << endl;
}

