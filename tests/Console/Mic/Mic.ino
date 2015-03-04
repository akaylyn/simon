// compile for Mega

// MSGEQ7 datasheet: https://www.sparkfun.com/datasheets/Components/General/MSGEQ7.pdf

#include <Metro.h>
#include <Streaming.h>

// pin locations
#define MSGEQ7_STROBE_PIN      24
#define MSGEQ7_RESET_PIN       22
#define MSGEQ7_ANALOG_PIN      A15

#define NUM_FREQUENCY_BANDS    7
int bandVolume[NUM_FREQUENCY_BANDS];
const int bandCenter[NUM_FREQUENCY_BANDS] = {
  63, 160, 400, 1000, 2500, 6250, 16000
}; // in Hz.

// see this discussion on what instruments appear in what band:
// http://homerecording.com/bbs/general-discussions/mixing-techniques/frequency-charts-50110/

void setup() {
  Serial.begin(115200);
  Serial << F("Mic: startup.") << endl;

  micStart();

  Serial << F("Mic startup complete.") << endl;
}

void loop() {
  // couple of options for testing

  // just show the entire frequency band.
//  micPrint();
//  delay(50);

  boolean isBeat = micIsBeat();

  if( isBeat ) {
    Serial << F("beat: ") << millis() << endl;
  }

}

void micPrint() {
  // gets everthing
  micReadAll();

  // save a copy and only print on "significant" delta
  static int lastBandVolume[NUM_FREQUENCY_BANDS];
  boolean showVolume = false;
  const int sigDelta = 25; // dunno
  for (int i = 0; i < NUM_FREQUENCY_BANDS; i++) {
    showVolume = showVolume || abs(lastBandVolume[i] - bandVolume[i]) >= sigDelta;
  }
  if ( showVolume ) {
    // save it
    memcpy(&lastBandVolume, &bandVolume, sizeof(bandVolume));

    Serial << millis() << F(":\t\t");
    for (int i = 0; i < NUM_FREQUENCY_BANDS; i++) {
      Serial << bandVolume[i] << F("\t\t");
    }
    Serial << endl;
  }

}

void micReadAll() {
  // This whole loop looks like it's ~504 us or 0.5 ms.  A clock instruction at 16 MHz takes
  // 0.0625 us, for reference.

  // Toggle the RESET pin of the MSGEQ7 to start reading from the lowest frequency band
  digitalWrite(MSGEQ7_RESET_PIN, HIGH); // HIGH for >= 100 nS; easy
  digitalWrite(MSGEQ7_RESET_PIN, LOW);

  // Read the volume in every frequency band from the MSGEQ7
  for (int i = 0; i < NUM_FREQUENCY_BANDS; i++) {

    digitalWrite(MSGEQ7_STROBE_PIN, LOW);
    delayMicroseconds(30); // Allow the output to settle, and get >=72 us btw LOW strobe-strobe

    bandVolume[i] = analogRead(MSGEQ7_ANALOG_PIN);    // LOW strobe-strobe delay needs to be >=72 us

    digitalWrite(MSGEQ7_STROBE_PIN, HIGH); // HIGH for >= 18 us.
    //    delayMicroseconds(15);

  }
}

boolean micIsBeat() {
  // take advantage of the likelihood that "beats" are all in the lower band.
  getLowEndVolume();
  
  static float avgVol=50; // dunno
  const float wt = 0.1;
  const float th = 1.5;
  
  boolean isBeat = bandVolume[0] > avgVol*th;
 
  avgVol = avgVol*(1.0-wt) + float(bandVolume[0])*wt;
  
//  Serial << avgVol << bandVolume[0] << endl;
  
  return(isBeat);
  
}

int getLowEndVolume() {
  // Toggle the RESET pin of the MSGEQ7 to start reading from the lowest frequency band
  digitalWrite(MSGEQ7_RESET_PIN, HIGH); // HIGH for >= 100 nS; easy
  digitalWrite(MSGEQ7_RESET_PIN, LOW);

  digitalWrite(MSGEQ7_STROBE_PIN, LOW);
  delayMicroseconds(30); // Allow the output to settle
  bandVolume[0] = analogRead(MSGEQ7_ANALOG_PIN);
  digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

  // only updating the low end.
}

void micStart() {
  // Set up the MSGEQ7 IC
  pinMode(MSGEQ7_ANALOG_PIN, INPUT);
  pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
  pinMode(MSGEQ7_RESET_PIN, OUTPUT);
  digitalWrite(MSGEQ7_RESET_PIN, LOW);
  digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
}


