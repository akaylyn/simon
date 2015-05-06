#ifndef ConcurrentAnimator_h
#define ConcurrentAnimator_h

#include <Adafruit_Neopixel.h>
#include <Metro.h>
#include <Arduino.h>

typedef void (*AnimateFunc)(Adafruit_NeoPixel&, int, int, int, int);

typedef struct  {
    int red;
    int green;
    int blue;
} RgbColor;

struct AnimationConfig {
    Adafruit_NeoPixel *strip;
    RgbColor color;
    int position;
    bool ready;
    Metro timer;
    AnimationConfig() : timer(1){};
};

class ConcurrentAnimator {
    public:
        void animate(AnimateFunc animate, AnimationConfig &config);

        void calculateAnimation(
            AnimateFunc animate,
            Adafruit_NeoPixel *strip, RgbColor color, int position, bool &ready
        );

        void push(Adafruit_NeoPixel *strip, Metro timer, bool &ready);
};

#endif

