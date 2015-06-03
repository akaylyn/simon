#include "Fire.h"
// startup.  layout the towers.
void Fire::begin(nodeID layout[N_COLORS]) {
  Serial << "Fire: begin" << endl;
  
  // store it
  for( byte i=0; i<N_COLORS; i++ )
    this->layout[i] = layout[i];
  
  for( byte i=0; i<N_COLORS; i++ ) {
    Serial << F(" Color ") << i << (" assigned to Tower ") << layout[N_COLORS] << endl;
  }
}

// set fire level, taking advantage of layout position
void Fire::setFire(color position, byte flameDuration, flameEffect effect) {
  inst[position].flame = checkFireEnabled(flameDuration);
  inst[position].effect = effect;
  
  network.send(inst[position], layout[position]);
}
void Fire::setFire(color position, fireInstruction &inst) {
  setFire(position, inst.flame, inst.effect);
}
// set fire level, ignoring tower positions (good luck with that)
void Fire::setFire(nodeID node, byte flameDuration, flameEffect effect) {
  // we'll stash the instruction by tower, not color
  color position = (color)(node-TOWER1);
  
  inst[position].flame = checkFireEnabled(flameDuration);
  inst[position].effect = effect;
  
  network.send(inst[position], node);

}
void Fire::setFire(nodeID node, fireInstruction &inst) {
  setFire(node, inst.flame, inst.effect);
}

byte Fire::checkFireEnabled(byte flameDuration) {
  // check the sensors for fire enable
  boolean fireAllowed = sensor.fireEnabled();
  return( fireAllowed ? flameDuration : 0 );
}

void Fire::clear() {
  for( byte i=0; i<N_COLORS; i++)
    memset(&inst[i], 0, sizeof(inst[i]));
    
  // nothing to show.  the towers wind fire down automagically.  
}

Fire fire;
