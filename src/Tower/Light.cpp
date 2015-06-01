#include "Light.h"

void Light::Light(byte redPin, byte greenPin, byte bluePin, byte floodPin) {
  // constructors
  tank(redPin, greenPin, bluePin);
  flood(floodPin);
  // tank effect
  tank.effect(SOLID);
}

void Light::effect(lightEffect_t effect, uint16_t onTime, uint16_t offTime) {
  tank.setMode(effect);
  tank.setBlink(onTime, offTime);
}

void Light::perform(towerInstruction &inst) {
  // copy out the colors
  static RGB rgb;
  rgb.red = inst.red;
  rgb.green = inst.green;
  rgb.blue = inst.blue;
  
  // apply
  flood.writeRGB(rgb); 
  tank.writeRGB(rgb);
}

void Light::update() {
  // run the update functions
  flood.update(); 
  tank.update();
}

void Flood::Flood(byte floodPin) {
  if( floodPin != 3 ) {
    Serial << F("Light: ERROR.  floodPin must be 3.  Halting.") << endl;
    while(1);
  }
  pinMode(3, OUTPUT);
  
  // turn it on
  this->on(); 
  
  // assume we're at the dimmest setting at startup
  this->currentBright = 0;

  // set to white
  RGB rgb;
  rgb.red = rgb.green = rgb.blue = 255;
  flood.writeRGB(RGB);
}


void Flood::on() {
  queCode(K24_ON);
  this->isOn = true;
}
void Flood::off() {
  queCode(K24_OFF);
//  send(K24_SMOOTH);
  this->isOn = false;
}

void Flood::setBright(byte level) {
  if( level > N_BRIGHT_STEPS ) level = N_BRIGHT_STEPS;
  
  Serial << F("Flood: bright: ") << level << endl;
  if( level > currentBright ) {
    for( byte sends=level-currentBright); sends > 0; sends--) queCode(K24_UP);
  } else if( level < currentBright) {
    for( byte sends=currentBright-level); sends > 0; sends--) queCode(K24_DOWN);
  }
  // track
  currentBright = level;
}

void Flood::sendCode(unsigned long data) {
  Serial << F("Flood: send: ") << _HEX(data) << endl;
  // simple wrapper
  irsend.sendNEC(data, 32);
}

void Flood::queCode(unsigned long data) {
  for( byte i=0; i<sendCount; i++)
    que.push(data);
}

void Flood::update() {
  // send on an interval
  static Metro canSend(sendInterval);
  
  // check the que and the timer
  if( canSend.check() & !que.isEmpty() ) {
    sendCode(que.pop());
    canSend.reset();
  }
}


void Flood::writeRGB(RGB color) {  
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
    queCode(K24_WHT);
    setBright(intensityToBright(0));
  }
  
  // track
  currentColor = color;

}

// helper functions
byte avgIntensity(unsigned long c1, unsigned long c2, unsigned long c3) {
  unsigned long sum = c1 + c2 + c3;
  return( sum/3UL );
}
byte avgIntensity(unsigned long c1, unsigned long c2) {
  unsigned long sum = c1 + c2;
  return( sum/2UL );
}
byte intensityToBrightness(byte intensity) {
  // serious "magic number" time
  // we only have N_BRIGHT_STEPS=5 color steps into which we map 0-255
  const byte cutoff[N_BRIGHT_STEPS] = {0, 8, 26, 82, 255};
  
  for( b=0; b<N_BRIGHT_STEPS; b++ ) {
    if( intensity <= cutoff[b] ) return( b );
  }
}
