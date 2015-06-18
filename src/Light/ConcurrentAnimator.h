#ifndef ConcurrentAnimator_h
#define ConcurrentAnimator_h

#include <Adafruit_NeoPixel.h>
#include <Metro.h>
#include <Streaming.h>
#include <Arduino.h>
#include "AnimationConfig.h"

class ConcurrentAnimator {
  public:
    void animate(AnimateFunc animate, AnimationConfig &config);
    void calculateAnimation(AnimateFunc animate, AnimationConfig &config);
    void push(AnimationConfig &config);
};

#endif

