#ifndef Animations_h
#define Animations_h

#include <Streaming.h>
#include <Adafruit_Neopixel.h>
#include <Adafruit_NeoMatrix.h>
//#include "AnimateFunc.h"
#include "AnimationConfig.h"
#include "Light.h"

// Animations designed for the NeoPixel Strips surrounding the buttons
void laserWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void laserWipeEdge(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void colorWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);

// Animations designed for the NeoPixel Matrix wrapped around the inside of the console
void colorWipeMatrix(Adafruit_NeoMatrix &strip, int c);

// Position information needed for the LaserWipe animation
struct LaserWipePosition {
    int prev;
    int dir; // direction
};

#endif

