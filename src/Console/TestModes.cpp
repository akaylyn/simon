#include "TestModes.h"

//------ Input units.
#include "Touch.h" // Touch subunit. Responsible for UX input.
#include "Mic.h" // Microphone
#include "Sensor.h" // Sensor subunit.  Responsible for game and fire enable

//------ Output units.
#include "Light.h" // Light subunit.  Responsible for UX output local Console (light) and remote Towers (light/fire)
#include "Sound.h" // Sound subunit.  Responsible for UX (music) output.

// simply operate the Console in "bongoes" mode.  Will shoot fire
void TestModes::bongoModeLoop(bool performStartup) {

  if( performStartup ) {
    Serial << "Starting up bongoMode" << endl;
    sound.stopAll();
    sound.setLeveling(4, 0); // prep for 4x tones and no music.
  }

  if ( touch.anyChanged()) {
    sound.stopTones(); // stop tones
    // if anything's pressed, pack the instructions
    for ( byte i = 0; i < N_COLORS; i++ ) {    
      if ( touch.pressed(i) ) {
        sound.playTone(i);
        light.setLight(i, LIGHT_ON);
        light.setFire(i, LIGHT_ON);
      } 
      else {
        light.setLight(i, LIGHT_OFF);
        light.setFire(i, LIGHT_OFF);
      }
    }
    // show
    light.show();
  } 
  else {
    // maybe resend
    light.update();
  }
}

// uses the MPR121 device to adjust lights and sound based on Player 1's proximity to sensors
extern float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve); // from Touch.cpp
void TestModes::proximityModeLoop(bool performStartup) {

  static Metro restartTimer(25000UL); // tones are only 30 seconds long, so we need to restart
  static int gainMax=TONE_GAIN - 6;
  static int gainMin=gainMax - 40;
  static int trTone[N_COLORS];
  static byte lastDistance[N_COLORS];
  static byte distanceThreshold = 200;
  
  if( performStartup || restartTimer.check() ) {
    Serial << "Starting up proximityMode" << endl;
    sound.stopAll();
    sound.setLeveling(4, 0); // prep for 4x tones and no music.

    for( byte i = 0; i < N_COLORS; i++ ) {
      // start the tones up
      trTone[i] = sound.playTone(i);
      // and quietly
      sound.setVolume(trTone[i], gainMin);
    }

    restartTimer.reset();
  }

  boolean showLightsNow = false;
  for( byte i = 0; i < N_COLORS; i++ ) {
    // read the sensor distance
    byte dist = touch.distance(i);
    
    if( dist != lastDistance[i] ) {
      // adjust the volume based on the distance
      int gain = fscale(0, 255, gainMax, gainMin, dist, -10.0); // log10
      
      sound.setVolume(trTone[i], gain);  
      // save it
      lastDistance[i] = dist;
      
      // set lights
      light.setLight(i, dist < distanceThreshold ? LIGHT_ON : LIGHT_OFF );
      showLightsNow = true;
    }
  }
  
  if( showLightsNow ) light.show();
  // update to towers
  light.update();
    
}

void TestModes::lightsTestModeLoop(bool performStartup) {
  // TODO: Each prox button will cycle through light colors for the button/tower
}

void TestModes::fireTestModeLoop(bool performStartup) {
  // TODO: Each prox button will arm, then fire its tower.
}

void TestModes::proximityResetModeLoop(bool performStartup) {
  if(performStartup) {
    //Serial << "Resetting the touch sensors" << endl;
    touch.begin();
    //Serial << "Done resetting the touch sensors" << endl;
  }
}

TestModes testModes;
