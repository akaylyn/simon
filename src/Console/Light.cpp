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
  showLocal(inst[position]);
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
  showLocal(inst[position]);
}
void Light::setLight(nodeID node, colorInstruction &inst) {
  setLight(node, inst.red, inst.green, inst.blue);
}

void Light::showLocal(colorInstruction &inst) {
  // show on console WS2812's via Light module
  ETinst = inst;
//  colorInstruction c = cMap[I_YEL];
//  ETinst = c;
  ET.sendData();
  
  // show on console hard lights
  if( inst.red>0 && inst.green> 0 ) {
    led[I_YEL]->setValue(((int)inst.red + (int)inst.green)/2);
    led[I_RED]->setValue(0);  
    led[I_GRN]->setValue(0);  
  } else {
    led[I_YEL]->setValue(0);  
    led[I_RED]->setValue(inst.red);
    led[I_GRN]->setValue(inst.green);
  }
  led[I_BLU]->setValue(inst.blue);
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
