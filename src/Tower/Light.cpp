#include "Light.h"

void Light::begin(byte redPin, byte greenPin, byte bluePin, byte floodPin) {
  Serial << F("Light::begin") << endl;
  
  // constructors
  this->tank = new LED(redPin, greenPin, bluePin);
  this->flood = new Flood();
  this->flood->begin(floodPin);
  // tank effect
  this->effect(SOLID);
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
  flood->setColor(inst); 
  tank->writeRGB(rgb);
}

void Light::update() {
  // run the update functions
  flood->update(); 
  tank->update();
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
  
  // save parameters
  this->sendInterval = sendInterval;
  this->sendCount = sendCount;
  // assume we're at the dimmest setting at startup
  this->currentBright = 0;
  
  // turn it on
  this->on(); 
  
  // set to white
  colorInstruction color = cWhite;
  this->setColor(color);
}


void Flood::on() {
  this->queCode(K24_ON);
  this->queCode(K24_ON); // make sure
  this->isOn = true;
}
void Flood::off() {
  this->queCode(K24_OFF);
  this->queCode(K24_OFF); // make sure
//  send(K24_SMOOTH);
  this->isOn = false;
}

void Flood::setBright(byte level) {
  if( level > N_BRIGHT_STEPS ) level = N_BRIGHT_STEPS;
  
  Serial << F("Flood: bright: ") << level << endl;
  if( level > currentBright ) {
    for( byte sends=level-currentBright; sends > 0; sends--) this->queCode(K24_UP);
  } else if( level < currentBright) {
    for( byte sends=currentBright-level; sends > 0; sends--) this->queCode(K24_DOWN);
  }
  // track
  this->currentBright = level;
}

void Flood::sendCode(unsigned long data) {
  Serial << F("Flood: send: ") << _HEX(data) << endl;
  ir->sendNEC(data, 32);
}

void Flood::queCode(unsigned long data) {
  for( byte i=0; i<sendCount; i++)
    que.push(data);
}

void Flood::update() {
  // send on an interval
  static Metro canSend(this->sendInterval);
  
  // check the que and the timer
  if( canSend.check() & !que.isEmpty() ) {
    sendCode(que.pop());
    canSend.reset();
  }
}

void Flood::dropQue() {
  while( !que.isEmpty() ) que.pop();
}

void Flood::setColor(colorInstruction &color) {  
  
  // kill the que.
  dropQue();
  
  unsigned long code;
  byte bright;
  if( color.red > 0 && color.green > 0 && color.blue > 0 ) {
    // white
    queCode(K24_WHT);
    setBright(intensityToBright(avgIntensity(color.red, color.green, color.blue)));
  } else if( color.red > 0 && color.green > 0 ) {
    // yellow
    queCode(K24_YEL);
    setBright(intensityToBright(avgIntensity(color.red, color.green)));
  } else if( color.red > 0 ) {
    // red
    queCode(K24_RED);
    setBright(intensityToBright(color.red));
  } else if( color.green > 0 ) {
    // green
    queCode(K24_GRN);
    setBright(intensityToBright(color.green));
  } else if( color.blue > 0 ) {
    // blue
    queCode(K24_BLU);
    setBright(intensityToBright(color.blue));
  } else {
    // note-quite-off
//    queCode(K24_WHT);
//    setBright(intensityToBright(0));
    // too much buffering
    queCode(K24_SMOOTH);
    setBright(intensityToBright(255));
  }
  
  // track
  currentColor = color;

}

// helper functions
byte Flood::avgIntensity(unsigned long c1, unsigned long c2, unsigned long c3) {
  unsigned long sum = c1 + c2 + c3;
  return( sum/3UL );
}
byte Flood::avgIntensity(unsigned long c1, unsigned long c2) {
  unsigned long sum = c1 + c2;
  return( sum/2UL );
}
byte Flood::intensityToBright(byte intensity) {
  // serious "magic number" time
  // we only have N_BRIGHT_STEPS=5 color steps into which we map 0-255
  const byte cutoff[N_BRIGHT_STEPS] = {0, 8, 26, 82, 255};
  
  for( byte b=0; b<N_BRIGHT_STEPS; b++ ) {
    if( intensity <= cutoff[b] ) return( b );
  }
}
