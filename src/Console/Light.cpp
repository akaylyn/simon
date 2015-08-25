#include "Light.h"

// startup
void Light::begin() {
  // moved this up front, as synchronization with Light apparently important.
  Serial << F("Light: startup.") << endl;

  this->led[I_RED] = new LED(LED_RED);
  this->led[I_GRN] = new LED(LED_GRN);
  this->led[I_BLU] = new LED(LED_BLU);
  this->led[I_YEL] = new LED(LED_YEL);
  Serial << F("Light: console hardwired buttons configured.") << endl;

  this->clear();

  Serial << F("Light: startup complete.") << endl;
}

// set light level, taking advantage of layout position
void Light::setLight(color position, byte red, byte green, byte blue) {
  colorInstruction inst;
  inst.red = red;
  inst.green = green;
  inst.blue = blue;

  this->setLight(position, inst);
}

void Light::clear() {
  network.update();
  animate(A_NoRim);
  network.update();
  clearButtons();
  animate(A_None);
  network.update();
}

void Light::clearButtons() {
  colorInstruction inst;
  inst.red = 0;
  inst.green = 0;
  inst.blue = 0;

  // clear it
  for( byte i=0; i<N_COLORS; i++) {
    // show it
    this->setLight((color)i, inst);
  }
}

void Light::setLight(color position, colorInstruction &inst) {
  // show on Towers and Light Module
  network.send(position, inst);

  // show on hard buttons
  switch( position ) {
    case I_RED: this->led[I_RED]->setValue(inst.red); break;
    case I_GRN: this->led[I_GRN]->setValue(inst.green); break;
    case I_BLU: this->led[I_BLU]->setValue(inst.blue); break;
    case I_YEL: this->led[I_YEL]->setValue(int(inst.red+inst.green)/2); break;
  }
}

void Light::animate(animationInstruction animation) {
    network.send(animation);
}

Light light;
