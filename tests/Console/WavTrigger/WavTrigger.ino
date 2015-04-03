// compile for Mega

#include <Streaming.h>
#include <Metro.h>

// include these in main .ino
#include "Sound.h" // Sound class.  Hoping for a drop-in replacement for existing .h/.cpp
#include <wavTrigger.h> // WAV board
#include <Simon_Common.h> // color defs

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    // startup
    sound.begin();

    // play some tones (will loop)
    int redTr = sound.playTone(I_RED);
    delay(1000);
    int grnTr = sound.playTone(I_GRN);
    delay(1000);
    int bluTr = sound.playTone(I_BLU);
    delay(1000);
    int yelTr = sound.playTone(I_YEL);
    delay(1000);
    int wrongTr = sound.playTone(I_RED, false);
    delay(1000);

    // finish with this, or get really annoying by the sound.
    sound.stopAllTracks();
}

void loop() {
    // put your main code here, to run repeatedly:

}
