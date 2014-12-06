// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // <<-style printing
#include <RFM12B.h> // RFM12b radio transmitter module
#include <EEPROM.h> // saving and loading radio settings
#include <SPI.h> // mp3 player: VS1053 include SPI, SD and VS1053 libaries libraries
#include <SD.h>
#include <Adafruit_VS1053.h>
// capsense touch: MPR121 include I2C and MPR121 libraries.
#include <Wire.h>
#define MPR121_h // adafruit's MPR121 library fails to wrap with ifndef, so we do this here.
#include <Adafruit_MPR121.h>
#include <Simon.h> // sizes, indexing and comms common to Towers and Console

// wiring connections
#include "Pinouts.h"
// capsense touch: MPR121 
#include "Touch.h"
// mp3 player: VS1053 
#include "Music.h"

// store my NODEID
byte myNodeID;
// store an in-network state
boolean inNetwork = false;

// wait for system to power up before initiating network configuration
#define D_CONFIG_DELAY 5000
// set a treshold for comms quality, as a fraction of packets that get through
#define D_ACCEPT_QUAL 90

// this is where the lights and fire instructions from Console are placed
towerInstruction inst;
// structure stores how Towers acts on those towerInstructions
towerConfiguration config[N_TOWERS];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  // start MPR121 touch
  touchStart();
  touchUnitTest(false); // no details

  // start RFM12b radio
  myNodeID = commsStart();
  if ( myNodeID == 0 ) {
    Serial << F("Unable to recover RFM settings!") << endl;
    while (1);
  }
  
  // start VS2053 mp3
  musicStart();
  musicUnitTest();
    
}

void loop() {
  // put your main code here, to run repeatedly:
  
   if( ! inNetwork ) {
    inNetwork = configureNetwork();
  } else {
    commsUnitTestConsole(config, inst); 
  }
  

}

boolean configureNetwork() {
  Serial << F("Configuring network....") << endl;
  // wait for other systems to power up
  delay(D_CONFIG_DELAY);
  
  // track quality 
  boolean networkQualityOK = true;
  
  // check ping quality
  for( byte ni=0; ni<N_TOWERS; ni ++ ) {
    Serial << F("Ping quality to node: ") << towerNodeID[ni] << F(" =");
    byte qual = commsQuality(towerNodeID[ni]);
    Serial << qual << F("%") << endl;
    // check for acceptable quality
    if( qual < D_ACCEPT_QUAL ) networkQualityOK = networkQualityOK && false;
  }  
  
  if( ! networkQualityOK ) {
    Serial << F("Missing one or more towers.  Defaulting to TOWER_ALL layout.") << endl;
    boolean toss = commsConfigTowers(config, TOWER_ALL, TOWER_ALL);
  } else {
    Serial << F("Comms quality OK.  Setting Tower configurations TOWER_ONE layout.") << endl;
    networkQualityOK = commsConfigTowers(config, TOWER_ONE, TOWER_ONE);
  }
  
  return( networkQualityOK );
}
