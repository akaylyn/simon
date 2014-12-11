// tone test
#define SPEAKER_WIRE 6 // can move, PWM needed
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




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  configureManualSound();

}

void loop() {
  // put your main code here, to run repeatedly:
    delay(1000);
    configureManualSound();
    Serial.print(".");
}


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
