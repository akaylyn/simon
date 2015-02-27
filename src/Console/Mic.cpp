// Mic
#include "Mic.h"

int bandVolume[NUM_FREQUENCY_BANDS];
const int bandCenter[NUM_FREQUENCY_BANDS] = {
  63, 160, 400, 1000, 2500, 6250, 16000
}; // in Hz.

void micStart() {
  // Set up the MSGEQ7 IC
  pinMode(MSGEQ7_ANALOG_PIN, INPUT);
  pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
  pinMode(MSGEQ7_RESET_PIN, OUTPUT);
  digitalWrite(MSGEQ7_RESET_PIN, LOW);
  digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
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

// this function will likely return false positives when it's initially called after startup
// TODO: place avgVol in EEPROM for quicker restart.
// it needs to be continually called to trace the volume level.
boolean micIsBeat() {
  // stores the running average of volume.
  static unsigned int avgVol=150; // dunno.  
  
  // take advantage of the likelihood that "beats" are all in the lower band.
  getLowEndVolume(); // volume data now in bandVolume[0]

  const unsigned int th = 150; // if the current reading exceeds the average by this percent, we've found a beat.
  boolean isBeat = bandVolume[0] > (avgVol*th)/100;

  const unsigned int wt = 10; // new volume levels are averaged with historical with this weighting.
  avgVol = (avgVol*wt + bandVolume[0])/(wt+1);

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

