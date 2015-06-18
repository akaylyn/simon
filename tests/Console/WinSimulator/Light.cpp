#include "Light.h"
#include <Adafruit_NeoPixel.h>

// Need an instance of the Radio Module
RFM12B radio;

// might as for resends, but only on this interval
Metro resendCounter(SEND_INTERVAL);

// time before we reconfig the network
Metro networkConfigUpdate(CONFIG_SEND_INTERVAL);

// button pins.  wire to Mega GPIO, bring LOW to indicate pressed.
//create object
EasyTransfer ET; 

//give a name to the group of data
LightET lightInst;

// system outputs
// LED lights
LED redLight(LED_RED);
LED grnLight(LED_GRN);
LED bluLight(LED_BLU);
LED yelLight(LED_YEL);

// try to use these nodes:
boolean listenAll[N_COLORS]={true, true, true, true};
boolean listenRed[N_COLORS]={true, false, false, false};
boolean listenGrn[N_COLORS]={false, true, false, false};
boolean listenBlu[N_COLORS]={false, false, true, false};
boolean listenYel[N_COLORS]={false, false, false, true};

// use the above configurations to layout the tower responses
// e.g. respond to everything
boolean *towerColor[N_TOWERS] = {listenAll, listenAll, listenAll, listenAll};
boolean *towerFire[N_TOWERS] = {listenAll, listenAll, listenAll, listenAll};
// e.g. respond to single colors
//boolean *towerColor[N_TOWERS] = {listenRed, listenGrn, listenBlu, listenYel};
//boolean *towerFire[N_TOWERS] = {listenRed, listenGrn, listenBlu, listenYel};

// minimum accumulator opening time.
unsigned long minFireTime = 50UL;
// maximum accumulator opening time.
unsigned long maxFireTime = 500UL;
// once the accumulator recloses, don't reopen for a time span which is the prior opening time
// divided by this number.
// e.g if the solenoid was just open for 50 ms, it won't open again for 50ms/flameCoolDownDivisor
unsigned int flameCoolDownDivisor = 2;

#define PIN 31
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

// might as for resends, but only on this interval
Metro timers[4] = {Metro(50),Metro(50),Metro(50),Metro(50)};

// startup
void Light::begin() {
  // moved this up front, as synchronization with Light apparently important.
  Serial << F("Light: Waiting for Light...") << endl;
  
  Serial << F("Light: startup.") << endl;

  led[I_RED] = &redLight;
  led[I_GRN] = &grnLight;
  led[I_BLU] = &bluLight;
  led[I_YEL] = &yelLight;

  timers[0] = Metro(50);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // once configured, start with all off.
  setAllOff();

  Serial << F("Light: startup complete.") << endl;

}

// set light level
void Light::setLight(byte index, byte level, boolean showNow, byte nodeID) {
  if ( index < I_RED || index > I_YEL ) {
    Serial << F("ERROR: Light::setLight.  Index out of range: ") << index << endl;
    while (1);
  }

  inst.lightLevel[index] = level;

  // maybe send
  if ( showNow ) show(nodeID);
}

void Light::setAllLight(byte level, boolean showNow, byte nodeID) {
  for ( int i = 0; i < N_COLORS; i++ )
    setLight(i, level, false); // don't send immediately

  // maybe send
  if ( showNow ) show(nodeID);
}

// set fire level
void Light::setFire(byte index, byte level, boolean showNow, byte nodeID) {
  if ( index < I_RED || index > I_YEL ) {
    Serial << F("ERROR: Light::setFire.  Index out of range: ") << index << endl;
    while (1);
  }

  // check the sensors for fire enable
  boolean fireAllowed = true;

  // set instructions
  inst.fireLevel[index] = fireAllowed ? level : 0;

  if ( showNow ) show(nodeID);
}

void Light::setAllFire(byte level, boolean showNow, byte nodeID) {
  for ( int i = 0; i < N_COLORS; i++ )
    setFire(i, level, false); // don't send immediately

  if ( showNow ) show(nodeID);
}

// set light and fire off
void Light::setAllOff(boolean showNow, byte nodeID) {
  setAllFire(LIGHT_OFF, false);
  setAllLight(LIGHT_OFF, false);

  if ( showNow ) show(nodeID);
}


unsigned long convLevelToTime(int fireLevel) {
    unsigned long flameTime = constrain( // constrain may be overkill, but map doesn't guarantee constraints
    map(fireLevel, 1, 255, 50UL, 500UL),
    50UL, 500UL);
    
    //Serial << "Fire Time: " << flameTime << endl;
    return flameTime;
}

// call this to latch the light and fire settings
void Light::show(byte nodeID) {
  if (nodeID == 0) {
    for(int i=2; i < 6; i++) {
        strip.setPixelColor((i-2)*2,strip.Color(inst.lightLevel[I_RED],inst.lightLevel[I_GRN],inst.lightLevel[I_BLU]));
        if (inst.fireLevel[I_RED] > 0) {
          strip.setPixelColor((i-2)*2+1,strip.Color(255,0,0));
          timers[i-2].interval(convLevelToTime(inst.fireLevel[I_RED]));
          timers[i-2].reset();
        }
    }
  } else {
    //Serial << "Sending to: " << ((nodeID-2)*2) << " color: " << inst.lightLevel[I_RED] << " " << inst.lightLevel[I_GRN] << " " << inst.lightLevel[I_BLU] << endl;
    strip.setPixelColor((nodeID-2)*2,strip.Color(inst.lightLevel[I_RED],inst.lightLevel[I_GRN],inst.lightLevel[I_BLU]));
    if (inst.fireLevel[I_RED] > 0) {
      strip.setPixelColor((nodeID-2)*2+1,strip.Color(255,0,0));
      timers[nodeID-2].interval(convLevelToTime(inst.fireLevel[I_RED]));
      timers[nodeID-2].reset();
    }
  }
  strip.show();
}

// call this to perform resend and network maintenace
void Light::update() {
  for(int i=0; i < 4; i++) {
    if (timers[i].check()) {
      strip.setPixelColor((i)*2+1,strip.Color(0,0,0)); 
      strip.show(); 
    }
  }
}


Light light;
