#include "Light.h"

void Light::begin(byte redPin, byte greenPin, byte bluePin) {
  Serial << F("Light::begin") << endl;
  
  // constructors
  this->tank = new LED(redPin, greenPin, bluePin);
  // tank effect
  this->effect(Solid);
}

void Light::effect(lightEffect_t effect, uint16_t onTime, uint16_t offTime) {
  tank->setMode(effect);
  tank->setBlink(onTime, offTime);
}

void Light::perform(colorInstruction &inst) {
  // copy out the colors
  static RGB rgb; // could take advantage of the aligned memory structure and memcpy, but...
  rgb.red = inst.red;
  rgb.green = inst.green;
  rgb.blue = inst.blue;
  
  // apply
  tank->writeRGB(rgb);
}

void Light::update() {
  // run the update functions
  tank->update();
}

