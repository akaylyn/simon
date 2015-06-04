#ifndef ConcurrentAnimator_h
#define ConcurrentAnimator_h

#include <Adafruit_NeoPixel.h>
#include <Metro.h>
#include <Streaming.h>
#include <Arduino.h>

typedef void (*AnimateFunc)(Adafruit_NeoPixel&, int, int, int, void*);

struct RgbColor {
  int red;
  int green;
  int blue;
};

struct AnimationConfig {
  char* name;
  Adafruit_NeoPixel *strip;
  RgbColor color;
  void* position;
  bool ready;
  Metro timer;
  AnimationConfig() : timer(1){};
};

class ConcurrentAnimator {
  public:
    void animate(AnimateFunc animate, AnimationConfig &config);
    void calculateAnimation(AnimateFunc animate, AnimationConfig &config);
    void push(AnimationConfig &config);
};

#endif

