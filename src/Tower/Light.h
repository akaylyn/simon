// Lights subunit.  Controls IR blaster and accumulator lighting

#ifndef Light_h
#define Light_h

#include <Arduino.h>

#include <RGBlink.h> // control LEDs

#include <Streaming.h> // <<-style printing

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// different lighting modes available.
enum lightEffect_t {
  Solid = 0, // always on
  Blink = 1, // blinking with intervals
  Fade = 2 // soft fading
};

class Light {
  public:

  void begin(byte redPin, byte greenPin, byte bluePin);
  void update();
  void perform(colorInstruction &inst);
  void effect(lightEffect_t effect = Solid, uint16_t onTime = 1000UL, uint16_t offTime = 100UL);

  private:
  
  // RGB lighting tied together on tank
  LED *tank;
};

#endif

