#include "Light.h"

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


// startup
void Light::begin() {
  // moved this up front, as synchronization with Light apparently important.
  Serial << F("Light: Waiting for Light...") << endl;
  
  LightComms.begin(LIGHT_COMMS_RATE);
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc. 
  ET.begin(details(lightInst), &LightComms);
  
  // send instructions until we get a response.
  byte handShake = 'h';
  while( LightComms.read() != handShake) {
    Serial << F(".");
    ET.sendData();
    LightComms.flush(); // wait for xmit to complete.
    delay(25);
  }
  Serial << endl;

  Serial << F("Light: Light checked in.  Proceeding...") << endl;

  Serial << F("Tower: startup.") << endl;

  // start radio
  networkStart();
  // ping the network
  networkPing();
  // configure network
  networkConfig();

  Serial << F("Light: startup.") << endl;

  led[I_RED] = &redLight;
  led[I_GRN] = &grnLight;
  led[I_BLU] = &bluLight;
  led[I_YEL] = &yelLight;
  
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
void Light::setFire(byte index, byte level, flameEffect_t effect, boolean showNow, byte nodeID) {
  if ( index < I_RED || index > I_YEL ) {
    Serial << F("ERROR: Light::setFire.  Index out of range: ") << index << endl;
    while (1);
  }

  // check the sensors for fire enable
  extern Sensor sensor;
  boolean fireAllowed = sensor.fireEnabled();

  // set instructions
  inst.fireLevel[index] = fireAllowed ? level : 0;
  inst.flameEffect = effect;

  if ( showNow ) show(nodeID);
}

void Light::setAllFire(byte level, flameEffect_t effect, boolean showNow, byte nodeID) {
  for ( int i = 0; i < N_COLORS; i++ )
    setFire(i, level, effect, false); // don't send immediately

  if ( showNow ) show(nodeID);
}

// set light and fire off
void Light::setAllOff(boolean showNow, byte nodeID) {
  setAllFire(LIGHT_OFF, FE_veryRich, false);
  setAllLight(LIGHT_OFF, false);

  if ( showNow ) show(nodeID);
}

// call this to latch the light and fire settings
void Light::show(byte nodeID) {
  // outboard Tower lighting and fire
  radio.Send(nodeID, (const void*)(&inst), sizeof(inst));

  // update the local LEDs 
  for ( int i = 0; i < N_COLORS; i++ ) {
    // local LEDs
    led[i]->setValue(inst.lightLevel[i]);
  }
  // Light module WS2812's
  lightInst.red = inst.lightLevel[I_RED] >= PIXEL_THRESHOLD ? true : false;
  lightInst.grn = inst.lightLevel[I_GRN] >= PIXEL_THRESHOLD ? true : false;
  lightInst.blu = inst.lightLevel[I_BLU] >= PIXEL_THRESHOLD ? true : false;
  lightInst.yel = inst.lightLevel[I_YEL] >= PIXEL_THRESHOLD ? true : false;
  
  //send the data
  ET.sendData();
}

void Light::sendInstruction(towerInstruction &externInst, int nodeID) {
  inst = externInst;
  show(nodeID);
}

// call this to perform resend and network maintenace
void Light::update() {

  // then we process outbound traffic
  if ( resendCount > 0 && resendCounter.check() ) { // we need to send the instructions
    resendCount--; // one less time
    resendCounter.reset();

    // NOTE: it is assumed retries are going out to all Towers.
    radio.Send(0, (const void*)(&inst), sizeof(inst));

  } 

}

// call this to perform network maintenance
void Light::updateNetwork() {
  // update network
  if ( networkConfigUpdate.check() ) { 
    networkConfig();
  }
}


// instantiates radio communications
byte Light::networkStart() {

  // check that the tower payloads aren't the same size.
  int size1 = sizeof(towerInstruction);
  int size2 = sizeof(towerConfiguration);
  int size3 = sizeof(modeSwitchInstruction);
  if ( size1 == size2 || size1 == size3 || size2 == size3) {
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
void Light::networkPing(int count) {
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
void Light::networkConfig() {
  Serial << F("Tower: configuring network.") << endl;

  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    memcpy(config[ni].lightListen, towerColor[ni], sizeof(config[ni].lightListen));
    memcpy(config[ni].fireListen, towerFire[ni], sizeof(config[ni].fireListen));
    config[ni].minFireTime = minFireTime;
    config[ni].maxFireTime = maxFireTime;
    config[ni].flameCoolDownDivisor = flameCoolDownDivisor;
    towerConfig(config[ni], towerNodeID[ni]);
  }

  // take a breather
  networkConfigUpdate.reset();

}

// configure a single tower
void Light::towerConfig(towerConfiguration & config, byte nodeID) {
  radio.Send(nodeID, (const void*)(&config), sizeof(config));
  
  Serial << F("Tower: sending config: ");
  Serial << F("Tower (") << nodeID << F(") config: ");
  Serial << F("Color(");
  for(byte i=0; i<N_COLORS; i++) Serial << config.lightListen[i] << F(" ");
  Serial << F(") Fire(");
  for(byte i=0; i<N_COLORS; i++) Serial << config.fireListen[i] << F(" ");
  Serial << F(") ");
  Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").");
  Serial << F(" Flame cooldown divisor: ") << config.flameCoolDownDivisor << endl;
  
  radio.SendWait(); // wait for the transmission to complete
}

void Light::sendModeSwitchInstruction(int mode) {
  modeSwitchInstruction modeSwitchInstr;
  modeSwitchInstr.currentMode = mode;
  radio.Send(0, (const void*)(&modeSwitchInstr), sizeof(modeSwitchInstr));
}

Light light;
