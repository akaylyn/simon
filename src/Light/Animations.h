#ifndef Animations_h
#define Animations_h

#include <Adafruit_Neopixel.h>
// Animations are defined within the Animations directory

typedef void (*AnimateFunc)(Adafruit_NeoPixel&, int, int, int, void*);

// Animations designed for the NeoPixel Strips surrounding the buttons
struct ButtonAnimations {
    AnimateFunc LaserWipe;
    AnimateFunc ColorWipe;
};

// Animations designed for the NeoPixel Matrix wrapped around the inside of the console
struct RimAnimations {
    AnimateFunc ColorWipe;
};

struct Animations {
    RimAnimations Rim;
    ButtonAnimations Btn;
};

#endif

