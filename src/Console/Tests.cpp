#include "Tests.h"

#define MODE_TRACK_OFFSET 699

// called from the main loop.  return true if we want to head back to playing Simon. 
boolean TestModes::update() {
  static int currentMode = N_systemMode-1;
  static boolean performStartup, modeChange = true;

  if( sensor.modeChange() || modeChange ) {
    (++currentMode) %= N_systemMode; // wrap

//    Serial << "CURRENT MODE: " << currentMode << endl;
    // Tell the tower's we're in a new mode
    network.send((systemMode)currentMode);
    
    // Play the sound to let the use know what mode we're in
    sound.stopAll();
    sound.setLeveling(1, 0); // Level for one track, no music
    sound.playTrack(MODE_TRACK_OFFSET + currentMode);

    Metro delayFor(1500UL);
    delayFor.reset();
    while( !delayFor.check() ) network.update(); // better.

    performStartup = true;
    modeChange = false;
  } 

  // yes, we could accomplish this with an array of function pointers...
  switch( currentMode ) {
  case GAMEPLAY:
    modeChange = true; // when we return from gameplay, we'll need to start up again.
    return( true );
    break;
  case BONGO: 
    bongoModeLoop(performStartup);
    break;
  case PROXIMITY:
    proximityModeLoop(performStartup);
    break;
  case FIRE:
    fireTestModeLoop(performStartup);
    break;
  case LIGHTS:
    lightsTestModeLoop(performStartup);
    break;
  case LAYOUT:
    layoutModeLoop(performStartup);
    break;
  case EXTERN:
    break;
  }

  // for our next visit
  performStartup = false;

  return(false);
}

// assign towers to locations around the Simon bezel
void TestModes::layoutModeLoop(boolean performStartup) {

  static boolean showNow = true;
  static color layout[N_COLORS] = {
                      I_RED, // Tower 1, RED, upper right.  
                      I_GRN, // Tower 2, GREEN, upper left. 
                      I_BLU, // Tower 3, BLUE, lower right.  
                      I_YEL  // Tower 4, YELLOW, lower left.  
  };
  // track idle for saving
  static Metro writeSettingsNow(5000UL);
  
  if( performStartup ) {
    Serial << "Starting up layoutMode" << endl;
    sound.stopAll();
    sound.setLeveling(4, 0); // prep for 4x tones and no music.
    
    fire.clear();
    light.clear();
    Metro delayFor(100UL);
    delayFor.reset();
    while( ! delayFor.check() ) network.update();
    
    showNow = true;
    writeSettingsNow.reset();
  }

  if ( touch.anyChanged() ) { 
    if ( touch.anyPressed() ) {
      // increment this tower's color assignment
      byte tower = (byte)touch.whatPressed();
      
      byte newLayout = (byte)layout[tower] +1;
      layout[tower] = (color)newLayout;
      if( layout[tower] > N_COLORS ) layout[tower]=I_RED; // N_COLORS is valid; means "All color channels"
  
      showNow = true;      
      writeSettingsNow.reset();
    } 
  }

  if( showNow ) {
    for( byte i=I_RED; i<N_COLORS; i++ ){
      colorInstruction c;
      if( layout[i] == N_COLORS ) c = cWhite;
      else c = cMap[layout[i]];

      light.setLight((color)i, c);
    }
    showNow = false;
  }  
  
  if( writeSettingsNow.check() ) {
    light.clear();
    Metro delayFor(500UL);
    delayFor.reset();
    while( ! delayFor.check() ) network.update();
    showNow = true;
    writeSettingsNow.reset();
    
    // do the deed.
    network.layout(layout, layout);
  }
}

// simply operate the Console in "bongoes" mode.  Will shoot fire
void TestModes::bongoModeLoop(boolean performStartup) {

  if( performStartup ) {
    Serial << "Starting up bongoMode" << endl;
    sound.stopAll();
    sound.setLeveling(4, 0); // prep for 4x tones and no music.
  }

  // track the last time we fired
  static unsigned long lastFireTime = millis();

  if ( touch.anyChanged() ) { 
    if ( touch.anyPressed()) {
      // if anything's pressed, pack the instructions
      color pressed = touch.whatPressed();

      sound.playTone(pressed);

      colorInstruction c = cMap[pressed];
      light.setLight(pressed, c);

      // only allow full-on every 10s.
      byte fireLevel = map(millis() - lastFireTime, 0UL, 10000UL, 50UL, 500UL) / 10;
      fire.setFire(pressed, fireLevel, gatlingGun);
      lastFireTime = millis();
    } else {
      light.clear(); // clear lights
      fire.clear(); // clear fire
      sound.stopTones(); // stop tones
    }
  }


}

// uses the MPR121 device to adjust lights and sound based on Player 1's proximity to sensors
extern float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve); // from Touch.cpp
void TestModes::proximityModeLoop(boolean performStartup) {

  static Metro restartTimer(25000UL); // tones are only 30 seconds long, so we need to restart
  static int gainMax=TONE_GAIN - 6;
  static int gainMin=gainMax - 40;
  static int trTone[N_COLORS];
  static byte lastDistance[N_COLORS];
  static byte distanceThreshold = 200;

  // track the last time we fired
  static unsigned long lastFireTime = millis();

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
    byte dist = touch.distance((color)i);

    if( dist != lastDistance[i] ) {
      // adjust the volume based on the distance
      int gain = fscale(0, 255, gainMax, gainMin, dist, -10.0); // log10

      sound.setVolume(trTone[i], gain);  

      // set lights
      colorInstruction c = cMap[i];
      c.red -= c.red > 0 ? dist : 0;
      c.green -= c.green > 0 ? dist : 0;
      c.blue -= c.blue > 0 ? dist : 0;
      light.setLight((color)i, c);

      if( dist<10 ) {
        // only allow full-on every 10s.
        byte fireLevel = map(millis() - lastFireTime, 0UL, 10000UL, 5, 50);
        fire.setFire((color)i, fireLevel, gatlingGun);
        lastFireTime = millis();        
      } else {
        fire.setFire((color)i, 0, gatlingGun);        
      }

      // save it
      lastDistance[i] = dist;
    }
  }

}

void TestModes::lightsTestModeLoop(boolean performStartup) {
  static Metro switchInterval(100UL);
  static byte intensity=255;
  static byte currentColor=0;
  static colorInstruction c; 
  
  if(performStartup) {
    Serial << "Starting up light mode" << endl;

    sound.stopAll();
    sound.setLeveling(1, 0);

    light.clear();
    fire.clear();
  }
  
  if( switchInterval.check() ) {
  
    intensity+=10;

    if( intensity<=10 ) {
      currentColor++;
      if( currentColor==N_COLORS ) {
        // white
        c = cWhite;
        currentColor = 255; // will wrap
      } else {
        c = cMap[currentColor];
      }
      intensity = 1;
    }
    c.red = c.red>0 ? intensity : 0;
    c.green = c.green>0 ? intensity : 0;    
    c.blue = c.blue>0 ? intensity : 0;
    
    light.setLight(I_RED, c);
    light.setLight(I_GRN, c);
    light.setLight(I_BLU, c);
    light.setLight(I_YEL, c);

  }
  
  /*
  for(int index = 0; index < N_TOWERS; index++)
   {    
   if(touch.changed(index) && touch.pressed(index)) {
   sound.playTrack(BOOP_TRACK);
   
   lightColorIndex[index]++;
   lightColorIndex[index] %= N_COLORS;
   
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
   */
}

#define FIRE_TEST_ARMED_TIMEOUT_MILLIS 5 * 1000
void TestModes::fireTestModeLoop(boolean performStartup) {
  /*
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
   
   light.setFire(0, 0, FE_veryRich, false);
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
   light.setFire(I_RED, 255, FE_veryRich, true, towerNodeID[index]);
   
   // Disarm us
   armed[index] = false;
   
   // Turn the lights off
   light.setFire(0, 0, FE_veryRich, false);
   light.setAllLight(0, true);
   }        
   else if(!anyArmed) {
   Serial << "Arming on tower " << index << endl;
   
   // Light the tower up
   light.setFire(0, 0, FE_veryRich, false);
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
   */
}

TestModes testModes;
