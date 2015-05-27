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
        
    //Serial << "Resetting the touch sensors" << endl;
    touch.begin();
    //Serial << "Done resetting the touch sensors" << endl;
    
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
 static int lightColorIndex[N_COLORS];
 towerInstruction instruction;
  
  if(performStartup) {
      Serial << "Starting up light mode" << endl;
      
    sound.stopAll();
    sound.setLeveling(1, 0);
  }
  
  for(int index = 0; index < N_TOWERS; index++)
  {    
    if(touch.changed(index) && touch.pressed(index)) {
      sound.playTrack(BOOP_TRACK);
      
      lightColorIndex[index]++;
      lightColorIndex[index] %= 6;
      
      Serial << "Setting light " << index << " to color " << lightColorIndex[index] << endl;
      
      // Send an instruction to go black.  This will (among other things) turn off the console lights off
      for(int index = 0; index < N_COLORS; index++) {        
        instruction.lightLevel[index] = 0;
        instruction.fireLevel[index] = 0; // Oh, and no fire in this mode!
      }
      light.sendInstruction(instruction, towerNodeID[index]);
      
      // Figure out which new color to send
      // (In the case of 0, stay black)
      if(lightColorIndex[index] == 1) {
        Serial << "Red" << endl;
        instruction.lightLevel[I_RED] = 255;
        instruction.lightLevel[I_GRN] = 0;
        instruction.lightLevel[I_BLU] = 0;
        instruction.lightLevel[I_YEL] = 0;
      }
      else if(lightColorIndex[index] == 2) {
        Serial << "Green" << endl;
        instruction.lightLevel[I_RED] = 0;
        instruction.lightLevel[I_GRN] = 255;
        instruction.lightLevel[I_BLU] = 0;
        instruction.lightLevel[I_YEL] = 0;
      }
      else if(lightColorIndex[index] == 3) {
        Serial << "Blue" << endl;
        instruction.lightLevel[I_RED] = 0;
        instruction.lightLevel[I_GRN] = 0;
        instruction.lightLevel[I_BLU] = 255;
        instruction.lightLevel[I_YEL] = 0;
      }
      else if(lightColorIndex[index] == 4) {
        Serial << "Yellow" << endl;
        instruction.lightLevel[I_RED] = 0;
        instruction.lightLevel[I_GRN] = 0;
        instruction.lightLevel[I_BLU] = 0;
        instruction.lightLevel[I_YEL] = 255;
      }
      else if(lightColorIndex[index] == 5) {
        Serial << "White" << endl;
        instruction.lightLevel[I_RED] = 128;
        instruction.lightLevel[I_GRN] = 128;
        instruction.lightLevel[I_BLU] = 128;
        instruction.lightLevel[I_YEL] = 0;
      }
      
      // Send the new color
      light.sendInstruction(instruction, towerNodeID[index]);
    }
  }
}

#define FIRE_TEST_ARMED_TIMEOUT_MILLIS 5 * 1000
void TestModes::fireTestModeLoop(bool performStartup) {
  static Metro armedTimer(FIRE_TEST_ARMED_TIMEOUT_MILLIS);
  static bool armed[N_TOWERS];
  
  if(performStartup) {
    for(int index = 0; index < N_TOWERS; index++) {
      armed[index] = false;
      light.setAllLight(0, true); // Turn all the lights off
    }
  }
  
  // If our armed timer has timed out and anything is armed, disarm things
  bool anyArmed = false;
  for(int index = 0; index < N_TOWERS; index++)
    if(armed[index])
        anyArmed = true;
      
  
  if(anyArmed && armedTimer.check()) {
    for(int index = 0; index < N_TOWERS; index++) {
      armed[index] = false;     
    }

    light.setFire(0, 0, false);
    light.setAllLight(0, true); // Turn all the lights off
    sound.playTrack(DISARMED_TRACK);
  }
  
  //look for button presses 
  for(int index = 0; index < N_TOWERS; index++)
  {
    if(touch.changed(index) && touch.pressed(index)) {
      if(sensor.fireEnabled()) {
        if(armed[index]) {
          // Fire!
          Serial << "Firing on tower " << index << endl;
          light.setFire(I_RED, 255, true, towerNodeID[index]);
          
          // Disarm us
          armed[index] = false;
          
          // Turn the lights off
          light.setFire(0, 0, false);
          light.setAllLight(0, true);
        }        
        else if(!anyArmed) {
          Serial << "Arming on tower " << index << endl;
          
          // Light the tower up
          light.setFire(0, 0, false);
          light.setLight(index, 255, true, towerNodeID[index]);
          
          armed[index] = true;
          sound.playTrack(ARMED_TRACK);
                  
          armedTimer.reset();
        }
        // else, ignore it, so we can only arm one tower at once.
      }
      else {
        sound.playTrack(DISARMED_TRACK);
      }
    }
  }
}

TestModes testModes;