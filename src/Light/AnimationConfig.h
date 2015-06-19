#ifndef AnimationConfig_h
#define AnimationConfig_h

#include <Metro.h>
#include "AnimateFunc.h"

struct RgbColor {
  int red;
  int green;
  int blue;
};

struct AnimationConfig {
  char* name;
  Adafruit_NeoPixel *strip;
  Adafruit_NeoMatrix *matrix;
  RgbColor color;
  void* position;
  bool ready;
  Metro timer;
};

#endif

