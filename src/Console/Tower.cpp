#include "Tower.h"

// Need an instance of the Radio Module
RFM12B radio;

// but wait an interval of time before resending
Metro resendCounter(SEND_INTERVAL);
// and countdown the remaining sends
int sendCount = 0;

// structure stores how Towers acts on those towerInstructions
towerConfiguration config[N_TOWERS];

// time before we reconfig the network
#define CONFIG_SEND_INTERVAL 30000UL // ms. 
Metro networkConfigUpdate(CONFIG_SEND_INTERVAL);

// try to use these nodes:
//    to respond to a color (I_RED, I_GRN, I_BLU, I_YEL)
//    to respond to all colors (I_ALL)
//    to responde to nothing (ie. not used) (I_NONE)
byte towerColor[N_TOWERS] = {I_ALL, I_ALL, I_ALL, I_ALL};
byte towerFire[N_TOWERS] = {I_ALL, I_ALL, I_ALL, I_ALL};

// minimum accumulator opening time.
unsigned long minFireTime = 50UL;
// maximum accumulator opening time.
unsigned long maxFireTime = 500UL;
// won't re-open the accumulator within this interval.
unsigned long flameCoolDownTime = 1000UL;

// this is where the lights and fire instructions to Towers are placed
towerInstruction inst;
// from Simon_Comms.h:
// typedef struct {
//	byte lightLevel[N_COLORS]; // 0..255.  PWM light level is map(lightLevel, 0, 255, 0, 255)
//	byte fireLevel[N_COLORS]; // 0..255.  accumulator open time is map(fireLevel, 0, 255, minFireTime, maxFireTime)
// } towerInstruction;

// startup Tower communications.
void towerStart() {
  Serial << F("Tower: startup.") << endl;

  // start radio
  networkStart();
  // ping the network
  networkPing();
  // configure network
  networkConfig();
  // clear instructions
  towerClearInstructions();
}

// instantiates radio communications
byte networkStart() {

  // check that the tower payloads aren't the same size.
  int size1 = sizeof(towerInstruction);
  int size2 = sizeof(towerConfiguration);
  if ( size1 == size2 ) {
    Serial << F("Tower: radio instructions are indistinguishable by size!") << endl;
    while (1); // halt
  }

  Serial << F("Tower: Reading radio settings from EEPROM.") << endl;

  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;

  // try to recover settings from EEPROM
  byte offset = 0;
  byte nodeID = EEPROM.read(radioConfigLocation + (offset++));
  byte groupID = EEPROM.read(radioConfigLocation + (offset++));
  byte band = EEPROM.read(radioConfigLocation + (offset++));
  byte csPin = EEPROM.read(radioConfigLocation + (offset++));

  if ( groupID != D_GROUP_ID ) {
    Serial << F("Tower: EEPROM not configured.  Doing so.") << endl;
    // then EEPROM isn't configured correctly.
    offset = 0;
    EEPROM.write(radioConfigLocation + (offset++), consoleNodeID);
    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF12_915MHZ);
    EEPROM.write(radioConfigLocation + (offset++), D_CS_PIN);

    return ( networkStart() ); // go again after EEPROM save
    
  } else {
    Serial << F("Tower: Startup RFM12b radio module. ");
    Serial << F(" NodeID: ") << nodeID;
    Serial << F(" GroupID: ") << groupID;
    Serial << F(" Band: ") << band;
    Serial << F(" csPin: ") << csPin;
    Serial << endl;

    radio.Initialize(nodeID, band, groupID);
    Serial << F("Tower: RFM12b radio module startup complete. ");
    return (radio.nodeID);
  }

}

// ping network for quality
void networkPing(int count) {
  Serial << F("Tower: ping network....") << endl;

  // use the instruction payload size+1.  It's contents are irrelevant.
  byte payload[sizeof(towerInstruction) + 1];

  // check ping quality and use that to figure out how many Towers can be used.
  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    Serial << F("Tower: ping node: ") << towerNodeID[ni]  << endl;

    // ping count times
    for ( int i = 0; i < count; i++ ) {
      radio.Send(towerNodeID[ni], (const void*)(&payload), sizeof(payload));
      Serial << F("+");
    }
    Serial << endl;
  }
}

// configure network
void networkConfig() {
  Serial << F("Tower: configuring network.") << endl;

  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    config[ni].colorIndex = towerColor[ni];
    config[ni].fireIndex = towerFire[ni];
    config[ni].minFireTime = minFireTime;
    config[ni].maxFireTime = maxFireTime;
    config[ni].flameCoolDownTime = flameCoolDownTime;
    towerConfig(config[ni], towerNodeID[ni]);
  }

  // take a breather
  networkConfigUpdate.reset();

}

// configure a single tower
void towerConfig(towerConfiguration & config, byte nodeID) {
  radio.Send(nodeID, (const void*)(&config), sizeof(config));
  Serial << F("Tower: sending config: ");
  Serial << F("Tower (") << nodeID << F(") config: ");
  Serial << F("Color(") << config.colorIndex << F(") Fire(") << config.fireIndex << F(") ");
  Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").") << endl;
}

// set lights on Tower with optional immediate send.  
void towerLightSet(byte colorIndex, byte level, boolean sendNow, byte nodeID) {
  // set instructions
  if( colorIndex == I_ALL ) {
    inst.lightLevel[I_RED] = level;
    inst.lightLevel[I_GRN] = level;
    inst.lightLevel[I_BLU] = level;
    inst.lightLevel[I_YEL] = level;
  } else if( colorIndex >= I_RED && colorIndex <= I_YEL ) {
    inst.lightLevel[colorIndex] = level;
  }
  // send it?
  if( sendNow ) {
    if( nodeID == 0 ) {
      towerSendAll();
    } else {
      towerSendOne(nodeID);
    }
  }
}

// set fire on Tower with optional immediate send.  
void towerFireSet(byte fireIndex, byte level, boolean fireAllowed, boolean sendNow, byte nodeID) {
  // set instructions
  inst.fireLevel[fireIndex] = fireAllowed ? level : 0;
  if( fireIndex == I_ALL ) {
    inst.fireLevel[I_RED] = level;
    inst.fireLevel[I_GRN] = level;
    inst.fireLevel[I_BLU] = level;
    inst.fireLevel[I_YEL] = level;
  } else if( fireIndex >= I_RED && fireIndex <= I_YEL ) {
    inst.fireLevel[fireIndex] = level;
  }
  // send it?
  if( sendNow ) {
    if( nodeID == 0 ) {
      towerSendAll();
    } else {
      towerSendOne(nodeID);
    }
  }
}

// clears all of the instructions 
void towerClearInstructions() {
  for (int i = 0; i < N_COLORS; i++ ) {
    inst.lightLevel[i] = 0;
    inst.fireLevel[i] = 0;
  }
}

// turns off light and fire on Towers with immediate send.
void towerQuiet(int sendN) {
  towerClearInstructions();
  // send it.
  towerSendAll(sendN);
}

// sends the current value of inst to the Towers, and tries again in an interval by towerComms.
void towerSendAll(int sendN) {
  // send now, broadcase mode (all Towers)
  radio.Send(0, (const void*)(&inst), sizeof(inst));
  // maybe try again in an interval, but only if we're in bread
  sendCount = sendN - 1;
  resendCounter.reset();
}

// sends the current value of inst to a specific Tower, with no retries.
void towerSendOne(byte nodeID) {
  // send now
  radio.Send(nodeID, (const void*)(&inst), sizeof(inst));
  sendCount = 0;
}

// check for inbound comms, resend instructions (if needed) periodically, send configuration periodically.
void towerUpdate() {

  // important that we check the buffer before sending, or we'll overwrite stuff.
  // while we don't (currently) do anything with inbound Tower traffic, the radio FSM needs a ReceiveComplete check frequently to advance states.
  if ( radio.ReceiveComplete() && radio.CRCPass() ) {
    // we have radio comms

    // check cases

    // we don't do anything with inbound traffic from the towers.
  }

  // then we process outbound traffic
  if ( sendCount > 0 && resendCounter.check() ) { // we need to send the instructions
    sendCount--; // one less time
    resendCounter.reset();

    // NOTE: it is assumed retries are going out to all Towers.
    radio.Send(0, (const void*)(&inst), sizeof(inst));

  } else if ( networkConfigUpdate.check() ) { // update network
    networkConfig();
  }
}



