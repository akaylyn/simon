#include "Light.h"

void Light::begin(byte redPin, byte greenPin, byte bluePin, byte floodPin) {
  Serial << F("Light::begin") << endl;
  
  // constructors
  this->tank = new LED(redPin, greenPin, bluePin);
  this->flood = new Flood();
  this->flood->begin(floodPin);
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
//  flood->setColor(inst); 
}

void Light::update() {
  // run the update functions
  tank->update();
  flood->update(); 
}

void Flood::begin(byte floodPin, unsigned long sendInterval, byte sendCount) {
  Serial << F("Flood::begin") << endl;

  if( floodPin != 3 ) {
    Serial << F("Light: ERROR.  floodPin must be 3.  Halting.") << endl;
    while(1);
  }
  pinMode(3, OUTPUT); // just making sure you understand you can't change this.
  
  // instantiate
  this->ir = new IRsend();

  // timing and resends    
  this->sendInterval = sendInterval;
  this->sendCount = sendCount;
  
  // turn it on
  this->on(); 
  
  // set to white
  colorInstruction color = cWhite;
  this->setColor(color);
}


void Flood::on() {
  this->sendCode(K24_ON);
  this->sendCode(K24_ON);
  this->sendCode(K24_UP);
  this->sendCode(K24_UP);
  this->sendCode(K24_UP);
  this->sendCode(K24_UP);
  this->sendCode(K24_UP);
  this->sendCode(K24_UP);
}
void Flood::off() {
 this->sendCode(K24_OFF);
 //  send(K24_SMOOTH);
}

void Flood::sendCode(unsigned long data) {
  Serial << F("Flood: send: ") << _HEX(data) << endl;
  for( byte i=0; i < this->sendCount; i++ ) {
    ir->sendNEC(data, 32);
//    delay( this->sendInterval );
  }
}

void Flood::update() {
}

void Flood::setColor(colorInstruction &color) {  
  
  unsigned long code;
  byte bright;
  if( color.red > 0 && color.green > 0 && color.blue > 0 ) {
    // white
    code = K24_WHT;
  } else if( color.red > 0 && color.green > 0 ) {
    // yellow
    code = K24_YEL;
  } else if( color.red > 0 ) {
    // red
    code = K24_RED;
  } else if( color.green > 0 ) {
    // green
    code = K24_GRN;
  } else if( color.blue > 0 ) {
    // blue
    code = K24_BLU;
  } else {
    code = K24_SMOOTH;
  }
  
  if( code != currentCode ) this->sendCode(code);
  // track
  currentCode = code;

}

