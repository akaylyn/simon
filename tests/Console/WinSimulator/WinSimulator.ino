#include "Light.h"
#include "Mic.h"
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <Adafruit_NeoPixel.h>

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

#include <EasyTransfer.h>
#include <LightMessage.h> // common message definition

// LED abstracting
#include <LED.h> 

// radio
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module


const int bandCenter[NUM_FREQUENCY_BANDS] = {
  63, 160, 400, 1000, 2500, 6250, 16000
}; // in Hz.

int tower = 2;
int band = 1;
int band2 = 6;
boolean hearBeat = false;
byte active = 0;
int tower2 = 0;  

void setup() {
    // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(A5));
  
  mic.begin();
  light.begin();
  
  mic.setThreshold(band,5);
  mic.setThreshold(band2,5);

}

void showBeats() {
  mic.update();
  //mic.print();
  boolean found = false;
//  for(byte i = 0; i<NUM_FREQUENCY_BANDS; i++) {
  for(byte i = 0; i<3; i++) {
     if( mic.getBeat(i) ) {
       Serial << bandCenter[i] << ":!\t";
       found = true;
     }
     else Serial << bandCenter[i] << ": \t";
   }
   Serial << endl;
  delay(25);
}

// 1000 * 60 / bpm = ms / beat   ~375ms 
void loop() {
  mic.update();

  // clear packet  
  light.setAllLight(0,false,0);
  light.setAllFire(0,false,0);
  
  for(byte i = 0; i<NUM_FREQUENCY_BANDS; i++) {
     if( mic.getBeat(i) ) {
       active++;
     }
  }
  
  if (mic.getBeat(band) && hearBeat == false) {
    tower++;
    if (tower > 5) tower = 2;
    //light.setLight(I_BLU,255,false,tower);
    light.setFire(I_RED,(int)((mic.getAvg(band)/1024.0)*255),false,tower);
    light.show(tower);
    hearBeat = true;
  } else if (!mic.getBeat(band)) {
    light.setFire(I_RED,0,false,tower);
    light.show();
    hearBeat = false;
  }
    // clear packet  
  light.setAllLight(0,false,0);
  light.setAllFire(0,false,0);

if (active > 0) {
   switch(active) {
     case 0:
       light.setAllLight(0,false,0);
       break;
     case 1:
     case 2:
       light.setLight(I_RED,255,false,0);
       break;
     case 3:
     case 4:
       light.setLight(I_GRN,255,false,0);
       break;
     case 5:
     case 6:
       light.setLight(I_BLU,255,false,0);
       break;
     default:
       light.setAllLight(255,false,0);
       active = 0;
       break;
   }
       
   light.show(tower2);
   if (random(10) > 2) {
      tower2++;
   }
   if (tower2 > 6) tower2 = 0;
}
    wait(1); 
    
 /* 
  wait(500);
  light.setLight(I_BLU,0,false,tower);
  light.setFire(I_RED,0,false,tower);
  light.show(tower);
  wait(500);
  tower++;
  if (tower > 5) tower = 2;
  */
}

void wait(unsigned long time) {
  Metro delayNow(time);
  
  while(!delayNow.check()) {
    light.update();
  }
}

