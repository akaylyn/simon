#ifndef ConcurrentAnimator_h
#define ConcurrentAnimator_h

#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Metro.h>
#include <Streaming.h>
#include <Arduino.h>
#include "AnimationConfig.h"

class ConcurrentAnimator {
  public:
    // Strip
    void animate(AnimateFunc animate, AnimationConfig &config);
    void calculateAnimation(AnimateFunc animate, AnimationConfig &config);

    // Matrix
    void animate(AnimateMatrixFunc animate, AnimationConfig &config);
    void calculateAnimation(AnimateMatrixFunc animate, AnimationConfig &config);

    void push(AnimationConfig &config);
};

#endif

