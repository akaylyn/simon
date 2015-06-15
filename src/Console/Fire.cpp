#include "Fire.h"

// startup.  layout the towers.
void Fire::begin() {
  Serial << "Fire: begin" << endl;
  
  this->clear();
}

// set fire level, taking advantage of layout position
void Fire::setFire(color position, byte flameDuration, flameEffect effect) {
  fireInstruction inst;
  inst.duration = checkFireEnabled(flameDuration);
  inst.effect = (byte)effect;
  
  this->setFire(position, inst);
}

void Fire::clear() {
  fireInstruction inst;
  inst.duration = 0;
  inst.effect = (byte)veryRich;
    
  // clear it
  for( byte i=0; i<N_COLORS; i++) {
    // show it
    this->setFire((color)i, inst);
  }

}

void Fire::setFire(color position, fireInstruction &inst) {
  // show on Towers and Light Module
  network.send(position, inst);
}

byte Fire::checkFireEnabled(byte flameDuration) {
  // check the sensors for fire enable
  boolean fireAllowed = sensor.fireEnabled();
  return( fireAllowed ? flameDuration : 0 );
}

Fire fire;
