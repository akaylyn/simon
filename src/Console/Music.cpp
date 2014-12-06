#include "Music.h"


void configureManualSound() {
  pinMode(SPEAKER_WIRE, OUTPUT);
  
  const unsigned long playDuration = 250;
  playTone(RED_TONE, playDuration);
  delay(playDuration);
  playTone(GRN_TONE, playDuration);
  delay(playDuration);
  playTone(BLU_TONE, playDuration);
  delay(playDuration);
  playTone(YEL_TONE, playDuration);
}

// plays a tone
void playTone(unsigned int freq, unsigned long duration) {
  if( duration > 0 ) {
    tone(SPEAKER_WIRE, freq, duration); // not blocking
  } else {
    tone(SPEAKER_WIRE, freq); // forever
  }
}

