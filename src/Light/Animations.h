#ifndef Animations_h
#define Animations_h

#include <Streaming.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include "AnimationConfig.h"
#include "Strip.h"
#include "ConcurrentAnimator.h"

// Utilities
void setStripColor(Adafruit_NeoPixel &strip, int r, int g, int b);
void setStripColor(Adafruit_NeoPixel &strip, uint32_t c);
void setStripColor(Adafruit_NeoPixel &strip, colorInstruction &inst);
void setStripColor(Adafruit_NeoMatrix &matrix, uint32_t c);
void gameplayFillFromMiddle(Adafruit_NeoMatrix &matrix, int center, int prev, uint16_t color);

uint32_t Wheel(Adafruit_NeoPixel &strip, byte WheelPos);

// Animations designed for the NeoPixel Strips surrounding the buttons
void laserWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void laserWipeEdge(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void colorWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void rainbowGlow(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void twinkleRand(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);

// Tron Light Cycles Animation
// Uses the matrix as a strip
void tronLightCycles(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
// all state altered in these methods must be passed
//void serialPrint();
boolean isCycle(TronCycles &cycles, int x, int y);
void addCycle(Adafruit_NeoPixel &strip, TronCycles &cycles, uint32_t x, uint32_t y, uint32_t color);
void fadeCycles(Adafruit_NeoPixel &strip);
void moveCycles(TronCycles &cycles);
void moveThisCycle(Adafruit_NeoPixel &strip, TronCycles &cycles, byte c);
uint32_t getPixelN(uint32_t x, uint32_t y);

// Animations designed or the NeoPixel Matrix wrapped around the inside of the console
void colorWipeMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void proximityPulseMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void idleMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void gameplayMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void gameplayDecayMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);

// Position information needed for the LaserWipe animation
struct LaserWipePosition {
  int prev;
  int dir; // direction
};

struct ProxPulsePosition {
  int magnitude;
  int prev;
  int tailLength;
};

struct GameplayPosition {
  int prev;
  int yellow;
  GameplayPosition* decayPos;
};

struct TronCycles {
  boolean live; // live or dead
  uint32_t x,y; // location
  uint32_t color; // color of the cycle
  byte movePref; // CW or CCW preference
};

#endif

