#include "Light.h"

// startup
void Light::begin(nodeID layout[N_COLORS], Stream *lightModuleSerial) {
  // moved this up front, as synchronization with Light apparently important.
  Serial << F("Light: startup.") << endl;
  
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc. 
  ET.begin(details(this->ETinst), lightModuleSerial);
  Serial << F("Light: serial comms with Light module.") << endl;
  
  this->led[I_RED] = new LED(LED_RED);
  this->led[I_GRN] = new LED(LED_GRN);
  this->led[I_BLU] = new LED(LED_BLU);
  this->led[I_YEL] = new LED(LED_YEL);
  Serial << F("Light: console hardwired buttons configured.") << endl;
 
  // store it
  for( byte i=0; i<N_COLORS; i++ )
    this->layout[i] = layout[i];
 
  for( byte i=0; i<N_COLORS; i++ ) {
    Serial << F(" Color ") << i << (" assigned to Tower ") << layout[i] << endl;
  }

  Serial << F("Light: startup complete.") << endl;

}

// set light level, taking advantage of layout position
void Light::setLight(color position, byte red, byte green, byte blue) {
  inst[position].red = red;
  inst[position].green = green;
  inst[position].blue = blue;
  
//  Serial << F("setLight: r:") << inst[position].red << F(" g:") << inst[position].green << F(" b:") << inst[position].blue << endl;
  
  // show on the network
  network.send(inst[position], layout[position]);
  // show locally
  showLocal(position, inst[position]);
}
void Light::setLight(color position, colorInstruction &inst) {
  setLight(position, inst.red, inst.green, inst.blue);
}
// set light level, ignoring tower positions (good luck with that)
void Light::setLight(nodeID node,  byte red, byte green, byte blue) {
  // we'll stash the instruction by tower, not color
  color position = (color)(node-TOWER1);
  
  inst[position].red = red;
  inst[position].green = green;
  inst[position].blue = blue;
  
  // show on the network
  network.send(inst[position], node);

  // show locally
  showLocal(position, inst[position]);
}
void Light::setLight(nodeID node, colorInstruction &inst) {
  setLight(node, inst.red, inst.green, inst.blue);
}

void Light::showLocal(color position, colorInstruction &inst) {
  // show on console WS2812's via Light module
  ETinst.color[position] = inst;
  ETinst.anim[position] = SOLID;
  ET.sendData();
  
  // show on console hard lights
  switch( position ) {
    case I_RED: led[I_RED]->setValue(inst.red); break;
    case I_GRN: led[I_GRN]->setValue(inst.green); break;
    case I_BLU: led[I_BLU]->setValue(inst.blue); break;
    case I_YEL: led[I_YEL]->setValue(((int)inst.red + (int)inst.green)/2); break;
  }
}

void Light::clear() {
  // clear it
  for( byte i=0; i<N_COLORS; i++) {
    memset(&inst[i], 0, sizeof(inst[i]));
    // show it
    setLight((color)i, inst[i]);
  }
}

Light light;
