#include <Streaming.h>

#define RED_TONE 311   // Red 310 Hz D#4 (true pitch 311.127 Hz)
#define GRN_TONE 415   // Green 415 Hz G#4 (true pitch 415.305 Hz)
#define BLU_TONE 208   // Blue 209 Hz G#3 (true pitch 207.652 Hz)
#define YEL_TONE 248    // Yellow 252 Hz B3 (true pitch 247.942 Hz)
#define WRONG_TONE 42       // a losing tone of 42 Hz

// tones we want to trial
#define NTONES 5 // tone count
unsigned int tones[NTONES] = {WRONG_TONE, BLU_TONE, YEL_TONE, RED_TONE, GRN_TONE};
boolean doTones[NTONES] =    {true,       true,     true,     true,     true    };

#define PIN 3 // pin

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly: 
  for( int i=0; i<NTONES; i++ ) {
    if( doTones[i] ) {
      Serial << "Freq: " << tones[i] << endl;
      tone(PIN, tones[i]);
      delay(5000);
    }
  }
  noTone(PIN);
}
