// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Arduino.h>
#include <Simon_Common.h> // sizes, indexing defines.
#include <Streaming.h> // <<-style printing
#include <EasyTransfer.h> // data transfer between arduinos
#include <SoundMessage.h> // sound message structure and consts
#include "Sound.h"

#define DELAY_TIME 5000

EasyTransfer easyTransfer;

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
    sound.setVolume(0);

    Serial << "Playing WIN" << endl;
    sound.playWin(2);
    delay(DELAY_TIME);

    Serial << "Playing LOSE" << endl;
    sound.playLose();
    delay(DELAY_TIME);

    Serial << "Playing BAFF" << endl;
    sound.playBaff();
    delay(DELAY_TIME);

    Serial << "Playing ROCK" << endl;
    sound.playRock();
    delay(DELAY_TIME);

    Serial << "Tests complete." << endl;
}

