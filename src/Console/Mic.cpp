// Mic
#include "Mic.h"

const int bandCenter[NUM_FREQUENCY_BANDS] = {
  63, 160, 400, 1000, 2500, 6250, 16000
}; // in Hz.

void Mic::begin(int resetPin, int stobePin, int outPin) {
  Serial << "Mic: startup." << endl;

  this->resetPin = resetPin;
  this->strobePin = strobePin;
  this->outPin = outPin;
  
  // Set up the MSGEQ7 IC
  pinMode(resetPin, OUTPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(outPin, INPUT);
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);

  // set threshold
  for ( int i = 0; i < NUM_FREQUENCY_BANDS; i++ ) {
    setThreshold(i, DEFAULT_THRESHOLD);
    setBeatMin(i, DEFAULT_MIN_BEAT);
  }
  
  // start sampling
  sampleIndex = 0;
  for ( int i = 0; i < NUM_SAMPLES; i++ ) update();

  Serial << F("Mic: startup complete.") << endl;
}

// show the volume levels
void Mic::print() {

  Serial << millis() << F(":\t");
  for (int i = 0; i < NUM_FREQUENCY_BANDS; i++) {
    Serial << bandCenter[i] << F(":") << bandVol[i][sampleIndex] << F("\t");
  }
  Serial << endl;

  delay(5); // just in case
}

void Mic::update() {
  // This whole loop looks like it's ~504 us or 0.5 ms.  A clock instruction at 16 MHz takes
  // 0.0625 us, for reference.

  // current values
  float currVol[NUM_FREQUENCY_BANDS];

  // Toggle the RESET pin of the MSGEQ7 to start reading from the lowest frequency band
  digitalWrite(resetPin, HIGH); // HIGH for >= 100 nS; easy
  digitalWrite(resetPin, LOW);

  // Read the volume in every frequency band from the MSGEQ7
  for (int i = 0; i < NUM_FREQUENCY_BANDS; i++) {

    digitalWrite(strobePin, LOW);
    delayMicroseconds(30); // Allow the output to settle, and get >=72 us btw LOW strobe-strobe

    currVol[i] = analogRead(outPin);    // LOW strobe-strobe delay needs to be >=72 us

    digitalWrite(strobePin, HIGH); // HIGH for >= 18 us.
    //    delayMicroseconds(15);

  }

  // ok, we have the data.

  // increment current counter
  sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

  // flag a beat in the band if currVol >= volAvg + threshold*volSD.
  for (int i = 0; i < NUM_FREQUENCY_BANDS; i++) {
    isBeat[i] = (currVol[i] > bandBeatMin[i]) & (currVol[i] >= bandAvg[i] + bandTh[i] * bandSD[i]);
//    if( isBeat[i] ) {
//      Serial << "band " << i << ": vol=" << currVol[i] << " avg=" << bandAvg[i] << " sd=" << bandSD[i] << " th=" << bandTh[i] << endl;
//      delay(5);
//    }
  }

  // recompute average and SD.
  for (int b = 0; b < NUM_FREQUENCY_BANDS; b++) {
    // store the new data
    bandVol[b][sampleIndex] = currVol[b];

    // sum
    float sum = 0;
    for (int s = 0; s < NUM_SAMPLES; s++) {
      sum += bandVol[b][s];
    }

    // average
    bandAvg[b] = sum / float(NUM_SAMPLES);

    // squared difference from avg
    float diff = 0;
    for (int s = 0; s < NUM_SAMPLES; s++) {
      diff += pow(bandVol[b][s] - bandAvg[b], 2.0);
    }

    // SD
    bandSD[b] = sqrt(diff / float(NUM_SAMPLES));
  }

}

int Mic::getVol(byte b) {
  return ( bandVol[b][sampleIndex] );
}

bool Mic::getBeat(byte b) {
  return ( isBeat[b] );
}

float Mic::getAvg(byte b) {
  return ( bandAvg[b] );
}

float Mic::getSD(byte b) {
  return ( bandSD[b] );
}

float Mic::getTh(byte b) {
  return ( bandTh[b] );
}

void Mic::setThreshold(byte b, float threshold) {
  bandTh[b] = threshold;
}

void Mic::setBeatMin(byte b, int val) {
  bandBeatMin[b] = val;
}

Mic listenWav, listenMic;



