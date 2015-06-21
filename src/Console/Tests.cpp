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

  // yes, we could accomplish this with an array of function pointers, if we were real software engineers...
  switch( currentMode ) {
    case GAMEPLAY:
      modeChange = true; // when we return from gameplay, we'll need to start up again.
      return( true );
      break;
    case WHITEOUT:
      whiteoutModeLoop(performStartup);
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

/*
  This mode is to be used for sudden whiteouts, when we need safety lighting, and we need to dump the propane.
   
  Press any button and hold it to dump propane.  When the button is not pressed, bright white light is on.
*/
void TestModes::whiteoutModeLoop(boolean performStartup) {
  static Metro nextStepTimer(400UL);
  
  static Metro fireOnMetro(500UL);   // 0.5 seconds
  static Metro fireOffMetro(1100UL); // 1.1 seconds, enough for the coolDown
  static int step = 0;
  static boolean fireIsOn = false;
  
  if( performStartup ) {
    Serial << "Starting up whiteout mode!" << endl;
    sound.stopAll();
    sound.setLeveling(1, 0); // prep for 1x tones and no music.
    
    step = 0;
    
    fire.clear();
    
    // Turn all the towers on for visibility
    colorInstruction cInst = cWhite;
    for(int index = 0; index < N_COLORS; index++) 
      light.setLight((color)index, cInst);      
  }
  
  if(!touch.anyPressed()) {
    // No button is being pressed, so go back to bright white light
    step = 0;
    
    // make sure we're not still shooting fire
    fire.clear();
    
    // Turn the colors back to white
    colorInstruction cInst = cWhite;
    for(int index = 0; index < N_COLORS; index++) 
      light.setLight((color)index, cInst);
  }
  else {  
    // The user is holding down a button.  Decide what to do about that.
    
    if(step >= N_COLORS + 1) {
      // The user has held the button down long enough.  Starting dumping propane.
      // To do this, we alternate between turning the fire on for 500 milliseconds, and then letting it cool down for 11000 milliseconds.
      Serial << "Turn and burn!" << endl;
      
      if(!fireIsOn && fireOffMetro.check()) {
        // The fire is currently off, and we've waited long enough.  Turn the fire on!
        fireIsOn = true;
        
        // Turn all the fire on!
        for(int index = 0; index < N_COLORS; index++) {
          fire.setFire((color)index, 50, veryRich);
        }              
        
        // Start the timer for switching to cooldown mode
        fireOffMetro.reset();
      }

      if(fireIsOn && fireOffMetro.check()) {
        // The fire is on, and it's time to cool down
        // it's also important we tell the towers to turn the fire off, as they will ignore duplicated "on" instructions
        fire.clear();
        
        fireIsOn = false;
        
        // Reset the timer for turning hte fire back on
        fireOnMetro.reset();
      }
      
    }
    else if(nextStepTimer.check()) {
      // The user hasn held the button down long enough to go to the next step of the count down
      
      Serial << "next step: " << step << endl;      
      
      step++;
      
      sound.playTrack(BOOP_TRACK);
      
      // Turn on red lights in accordance with the count down
      for(int index = 0; index < min(step, N_COLORS); index++) {
        colorInstruction cInst = cRed;
        light.setLight((color)index, cInst);
      }
      
      // wait for the *next* next step
      nextStepTimer.reset();
    }
  }
}

// assign towers to locations around the Simon bezel
void TestModes::layoutModeLoop(boolean performStartup) {

  static boolean showNow = true;
  static color layout[N_COLORS];

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

    int addr = 69;
    for ( byte i = 0; i < N_COLORS; i++ ) {
      layout[i] = (color)EEPROM.read(addr+i);
    }
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
    
    //turn all of the lights off to start out with
    fire.clear();
    light.clear();
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

/*
  This mode lets the user step each tower through the primary colors, white, and off.  It's usefull for debugging issues with the LED strands
  Pressing any button advances the associated tower to the next color in the sequence.
*/
void TestModes::lightsTestModeLoop(boolean performStartup) {
  
  colorInstruction colorSequence[] = { cOff, cRed, cGreen, cBlue, cWhite };
  
  static int towerSpotInSequence [N_COLORS];
  
  if(performStartup)
  {
    // Start all of the towers at the start of the sequence (off)
    for(int index = 0; index < N_COLORS; index++)
      towerSpotInSequence[index] = 0;
    
    // Turn the lights off;
    light.clear();
  }
  
  if ( touch.anyChanged() ) { 
    if ( touch.anyPressed()) {  
      sound.playTrack(BOOP_TRACK);
      
      color whatPressed = touch.whatPressed();
      
      // advance the tower pressed into the next step of the sequence, wrap around.
      towerSpotInSequence[whatPressed] ++;
      towerSpotInSequence[whatPressed] %= 5;
      
      // send the correct color to that tower
      colorInstruction c = colorSequence[towerSpotInSequence[whatPressed]];
      light.setLight(whatPressed, c);
    }
  }   
}

/*
  This mode lets the user test the solenoids/fire of each tower individually.
  Pressing a button "arms" that tower.  Pressing the button again while the tower is armed will fire maximum fire out of the tower.
  Towers automatically disarm after two seconds.
  Only one tower can be armed at one.
*/

#define FIRE_TEST_ARMED_TIMEOUT_MILLIS 2 * 1000
void TestModes::fireTestModeLoop(boolean performStartup) {
    
  static Metro armedTimer(FIRE_TEST_ARMED_TIMEOUT_MILLIS);
  static bool armed[N_COLORS];
  
  // I was having trouble with some timing (the touch panels are not debounced), so I keep a frame count for debugging.
  static int frame = 0;
  frame++;
   
  if(performStartup) {
    
    // Turn off the fire and lights
    fire.clear();
    light.clear();
    
    // Disarm each tower.
    for(int index = 0; index < N_COLORS; index++) {
      armed[index] = false;
    }
  }

  // Check to see if any of the towers are armed
  bool anyArmed = false;  
  for(int index = 0; index < N_COLORS; index++) {
    if(armed[index]) {
      anyArmed = true;
    }
  }
  
  // If our armed timer has timed out and anything is armed, disarm every tower
  if(anyArmed && armedTimer.check()) {
    Serial << "Disarmed timer fired.  Disarm everything!";
    
    // Disarm all the towers
    for(int index = 0; index < N_COLORS; index++) {
      armed[index] = false;     
    }

    fire.clear(); // turn the fire off
    light.clear(); // turn the lights off
    
    // Let the user know
    sound.playTrack(DISARMED_TRACK);
  }

  //look for button presses 
  if (touch.anyChanged() && touch.anyPressed()) {  
    color whatPressed = touch.whatPressed();
      
    if(!sensor.fireEnabled()) {
      // If the fire solenoid is off, pressing the buttons doesn't do anything.  Warn the user
      sound.playTrack(DISARMED_TRACK);
    }
    else {
      if(armed[whatPressed]) {
        // The button has been pressed on an armed tower, fire it!
        
        Serial << "Firing on tower " << whatPressed << " on frame " << frame << endl;          
        
        // Make fire go now!
        fire.setFire(whatPressed, 255, kickMiddle);
        
        //  Play a sound.  This is helpful for debugging, to now we *tried* to fire, even if a solenoid failed.
        sound.playTrack(BOOP_TRACK);
        
        // Update the network, then wait until the send has gone out.
        network.update();
        delay(100);

        // Disarm us
        armed[whatPressed] = false;
        
        // turn our color and fire back off
        fire.clear(); // turn the fire off
        light.clear(); // turn the lights off
      }        
      else if(!anyArmed) {
        // A button was pressed, and now towers are armed.  Armed the tower that was pressed
        Serial << "Arming on tower " << whatPressed << " on frame " << frame << endl;

        // Light the tower up
        colorInstruction cInst = cRed;
        light.setLight(whatPressed, cInst);

        // Arm the tower
        armed[whatPressed] = true;
        
        // Tell the user
        sound.playTrack(ARMED_TRACK);

        // Reset the automatic disarm timer.
        armedTimer.reset();
      }
    }
  }
}

TestModes testModes;

