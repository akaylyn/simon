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

// Animations designed or the NeoPixel Matrix wrapped around the inside of the console
void colorWipeMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void proximityPulseMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void idleMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void gameplayMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData);
void gameplayDecayMatrix(Adafruit_NeoMatrix &matrix, void *posData, bool r, bool g, bool b, bool y);
int getDecayedOffset(int offset);
int getFilledOffset(int offset);

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
  int redOffset;
  int greenOffset;
  int blueOffset;
  int yellowOffset;
  int yellow;
};

// ****************************************************************************
// Tron Light Cycles Animation
// Uses the matrix as a strip

void tronLightCycles(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);

struct TronCycles {
  boolean live; // live or dead
  uint32_t x,y; // location
  uint32_t color; // color of the cycle
  byte movePref; // CW or CCW preference
};

#define CYCLE_DICK_MOVE_PERCENT 15 // chance that a cycle will throw some "zigs" in it's path around the rim
#define CYCLE_TRAIL_LENGTH BLU_X-RED_X // a light cycle leaves a trail as long as 1/4 of the rim circumference
#define MAX_CYCLES 50
struct TronPosition {
  TronCycles* cycles;
  uint32_t x,y; // location
  bool addCycle;
};

// all state altered in these methods must be passed
//void serialPrint();
boolean isCycle(TronCycles *cycles, int x, int y);
void addCycle(Adafruit_NeoPixel &strip, TronCycles *cycles, uint32_t x, uint32_t y, uint32_t color);
void fadeCycles(Adafruit_NeoPixel &strip);
void moveCycles(Adafruit_NeoPixel &strip, TronCycles *cycles);
void moveThisCycle(Adafruit_NeoPixel &strip, TronCycles *cycles, byte c);
uint32_t getPixelN(uint32_t x, uint32_t y);

#endif

