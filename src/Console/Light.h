// Light subunit.  Responsible for UX (light) output local to the console.  Coordinates outboard Light.

#ifndef Light_h
#define Light_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// for send function
#include "Network.h"

// LED abstracting
#include <LED.h>

// Manual button lights, panels, under console.  wire to N-channel MOSFET + and any GND.
#define LED_YEL 8 // can move, PWM
#define LED_GRN 9 // can move, PWM
#define LED_BLU 10 // can move, PWM
#define LED_RED 11 // can move, PWM

class Light {
  public:
    // startup.  layout the towers.
    void begin();

    // set fire level, taking advantage of layout position
    void setLight(color position, byte red, byte green, byte blue);
    void setLight(color position, colorInstruction &inst);
    void animate(animationInstruction animation);
    void stopAnimation();

    void clear(); // clear

  private:

    // hardware LED
    LED *led[N_COLORS];
};

extern Light light;

#endif
