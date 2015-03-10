#ifndef Mic_h
#define Mic_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing

// MSGEQ7 datasheet: https://www.sparkfun.com/datasheets/Components/General/MSGEQ7.pdf
// pin locations

#define MSGEQ7_STROBE_PIN      24
#define MSGEQ7_RESET_PIN       22
#define MSGEQ7_ANALOG_PIN      A15

#define NUM_FREQUENCY_BANDS    7

// see this discussion on what instruments appear in what band:
// http://homerecording.com/bbs/general-discussions/mixing-techniques/frequency-charts-50110/

void micStart();
void micPrint();
void micReadAll();
byte micIsBeat(float threshold); // volumeNow/volumeAvg required to call it a "beat"
void getLowEndVolume();


#endif
