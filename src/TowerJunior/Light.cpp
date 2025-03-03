#include "Light.h"

#define FLAME_CHANS 2 // DMX in box should read "001"; flame is channel 1; sparker is channel 2

#define LAMP_CHANS 5 // DMX lamps should read "d003", "d008", "d013", "d018"
// Red:d003, Blue: d008, Yellow: d013, Green:d018 
#define LAMP_N 4

int lampIndex(byte lamp, byte channel) {
  int ret = (int)FLAME_CHANS+(int)LAMP_CHANS*(int)lamp+(int)channel;
//  Serial << ret << endl;
  return(ret);
}
void master(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,1), level); }
void red(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,2), level); }
void green(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,3), level); }
void blue(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,4), level); }
void white(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,5), level); }
void all(byte lamp, byte level) { red(lamp, level); green(lamp, level); blue(lamp, level); white(lamp, level); }

#define FOR_ALL_LAMPS for(byte l=0;l<LAMP_N;l++)

void Light::begin() {
  Serial << F("Light::begin") << endl;

  // max channels
  DmxSimple.maxChannel(FLAME_CHANS+LAMP_N*LAMP_CHANS);

  // For no reason that's obvious to me, we need to initialize to one past the number of lamps we have.  worrisome.
  for(byte i=0;i<=LAMP_N;i++) { master(i,255); red(i,0); green(i,0); blue(i,0); white(i,0); }
}

void Light::perform(systemState &system) {

  // red 
  red(0, system.light[I_RED].red);
  green(0, system.light[I_RED].green);
  blue(0, system.light[I_RED].blue);
  white(0, 0);
  
  // blue 
  red(1, system.light[I_BLU].red);
  green(1, system.light[I_BLU].green);
  blue(1, system.light[I_BLU].blue);
  white(1, 0);
  
  // yellow 
  red(2, system.light[I_YEL].red);
  green(2, system.light[I_YEL].green);
  blue(2, system.light[I_YEL].blue);
  white(2, 0);
  
  // green 
  red(3, system.light[I_GRN].red);
  green(3, system.light[I_GRN].green);
  blue(3, system.light[I_GRN].blue);
  white(3, 0);
  
}

void Light::update() {

}

