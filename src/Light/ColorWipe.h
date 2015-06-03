#ifndef ColorWipe_h
#define ColorWipe_h

#include <Streaming.h>
#include "Animations.h"

void colorWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void colorWipeMatrix(Adafruit_NeoMatrix &strip, int r, int g, int b, void *posData);

#endif

