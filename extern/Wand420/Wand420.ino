// Compile for ATMega1284P (Mighty1284p 16MHz using Optiboot).  

// from: http://forums.adafruit.com/viewtopic.php?f=47&t=45258

// include the library code:
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

// the glue
#include <Simon_Common.h> // sizes, indexing and comms common to Towers and Console

// radio
#include <SPI.h> // radio board is a SPI device
#include <avr/eeprom.h> // eeprom block r/w
#include <RFM69.h> // RFM12b radio transmitter module
// Need an instance of the Radio Module
RFM69 radio;
// store my NODEID
byte myNodeID;

// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
#include <EEPROM.h>

// menuing
#include <MENWIZ.h>
menwiz menu;

// this is where the lights and fire instructions from Console are placed
towerInstruction tInst;
// structure stores how this Tower acts on those towerInstructions
towerConfiguration tConfig;
// structure stores Console instructions
consoleInstruction cInst;
// structure stores Console configuration
consoleConfiguration cConfig;

// place to paste 3-digit byte
char buf[3];

// add global variables to bind to menu
boolean buttonPressed = false;
int towerLightIndex, towerFireIndex, towerFireMin, towerFireMax, towerFireCooldown;
int testLightLevel, testFireLevel;

// track which towers are assigned to which channels (for Each2Own)
byte towerChannels[N_TOWERS] = {
  I_RED, I_BLU, I_YEL, I_GRN};

#define FIRE_MIN 10
#define FIRE_MAX 1000
#define FIRE_STEP 10

#define LEVEL_MIN 0
#define LEVEL_MAX 10
#define LEVEL_STEP 1

#define CD_MIN 0
#define CD_MAX 10

#define INTERACTIVE_TIMEOUT 10000UL

// force overwrite of EEPROM.  useful for bootstrapping new Moteinos.
#define WRITE_EEPROM_NOW false

// radio pins
#define D_CS_PIN 10 // default SS pin for RFM module
// might need to be D4.

#define B_RED 22 
#define B_GRN 21
#define B_BLU 23
#define B_YEL 20
#define B_STK 19
#define J_X A0 // D24, A0
#define J_Y A1 // D25, A1
// add wire from +3.3V to "5V" on shield.

void menuSetup() {
  Serial << F("Setting up menu...") << endl;

  _menu *r, *s1, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s2, *s21, *s22, *s23, *s24, *s25, *s3;

  menu.begin(&lcd, 20, 4); //declare lcd object and screen size to menwiz lib

  r = menu.addMenu(MW_ROOT, NULL, F("Main Menu"));

  s1 = menu.addMenu(MW_SUBMENU, r, F("Config"));
  s11 = menu.addMenu(MW_VAR, s1, F("TowerLights"));
  s11->addVar(MW_LIST, &towerLightIndex);
  s11->addItem(MW_LIST, F("Each2Own"));
  s11->addItem(MW_LIST, F("AllIn"));
  s12 = menu.addMenu(MW_VAR, s1, F("TowerFire"));
  s12->addVar(MW_LIST, &towerFireIndex);
  s12->addItem(MW_LIST, F("Each2Own"));
  s12->addItem(MW_LIST, F("AllIn"));
  s17 = menu.addMenu(MW_VAR, s1, F("Assign Colors"));
  s17->addVar(MW_ACTION, towerWhich);
  s17->setBehaviour(MW_ACTION_CONFIRM, false); // just do it.
  s14 = menu.addMenu(MW_VAR, s1, F("FireTimeMin"));
  s14->addVar(MW_AUTO_INT, &towerFireMin, FIRE_MIN, FIRE_MAX, FIRE_STEP);
  s13 = menu.addMenu(MW_VAR, s1, F("FireTimeMax"));
  s13->addVar(MW_AUTO_INT, &towerFireMax, FIRE_MIN, FIRE_MAX, FIRE_STEP * 10);
  s16 = menu.addMenu(MW_VAR, s1, F("FireCooldownX"));
  s16->addVar(MW_AUTO_INT, &towerFireCooldown, CD_MIN, CD_MAX, 1);
  s15 = menu.addMenu(MW_VAR, s1, F("!!!Send!!!"));
  s15->addVar(MW_ACTION, towerConfig);

  s2 = menu.addMenu(MW_SUBMENU, r, F("Control"));
  s24 = menu.addMenu(MW_VAR, s2, F("LightSet"));
  s24->addVar(MW_AUTO_INT, &testLightLevel, LEVEL_MIN, LEVEL_MAX, LEVEL_STEP);
  s25 = menu.addMenu(MW_VAR, s2, F("FireSet"));
  s25->addVar(MW_AUTO_INT, &testFireLevel, LEVEL_MIN, LEVEL_MAX, LEVEL_STEP);
  s21 = menu.addMenu(MW_VAR, s2, F("Light Test"));
  s21->addVar(MW_ACTION, lightTest);
  s21->setBehaviour(MW_ACTION_CONFIRM, false); // just do it.
  s23 = menu.addMenu(MW_VAR, s2, F("Fire Test"));
  s23->addVar(MW_ACTION, fireTest);
  s23->setBehaviour(MW_ACTION_CONFIRM, true);  // yeah.
  s22 = menu.addMenu(MW_VAR, s2, F("Sound Test"));
  s22->addVar(MW_ACTION, soundTest);
  s22->setBehaviour(MW_ACTION_CONFIRM, false); // just do it.

  s3 = menu.addMenu(MW_VAR, r, F("Save Settings"));
  s3->addVar(MW_ACTION, saveSettings);

  // Use the navigation button from the Adafruit I2C shield
  menu.addUsrNav(readButtons, 4);

  // show a splash screen
  char splash[] = " Simon v2 Wand\n      -Magister";
  menu.addSplash(splash, 1000);

  // After a inactivity, show custom screen with real-time information
  menu.addUsrScreen(printInstruction, INTERACTIVE_TIMEOUT);

  // load settings
  menu.readEeprom();
}

void setup() {
  // Debugging output
  Serial.begin(115200);
  Serial << F("Wand: startup.") << endl;

  // buttons
  pinMode(B_RED, INPUT); 
  pinMode(B_GRN, INPUT); 
  pinMode(B_BLU, INPUT); 
  pinMode(B_YEL, INPUT); 
  pinMode(B_STK, INPUT); 

  // analog
  pinMode(J_X, INPUT);
  pinMode(J_Y, INPUT);

  // set up menus
  menuSetup();

  // zero out instruction.
  myNodeID = 7;
  myNodeID = networkStart(WRITE_EEPROM_NOW); // assume that EEPROM has been used to correctly set values
  radio.setHighPower(); // for HW boards.
  radio.promiscuous(true); // so broadcasts are received.
  Serial << F("Radio startup complete. Node: ") << myNodeID << endl;

  Serial << F("Wand: startup complete.") << endl;

}

void loop() {
  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(tInst) ) {
      // save instruction for lights/flame
      Serial << sizeof(radio.DATA) << endl;
      tInst = *(towerInstruction*)radio.DATA;
    }
  }

  // do the menus
  menu.draw();

  // check for errors
  int err = menu.getErrorMessage(false);
  if ( err > 0 ) {
    err = menu.getErrorMessage(true); // printing
    delay(50);
  }
}

void lightTest() {
  Serial << F("Light Test!") << endl;
  sendTest( map(testLightLevel, 0, 10, 0, 255), 0, 0);
}
void fireTest() {
  Serial << F("Fire Test!") << endl;
  sendTest( map(testLightLevel, 0, 10, 0, 255), map(testFireLevel, 0, 10, 0, 255), 0);
}
void soundTest() {
  Serial << F("Sound Test!") << endl;
  sendTest( map(testLightLevel, 0, 10, 0, 255), 0, 0);
}
void sendTest(byte lightLevel, byte fireLevel, byte musicLevel) {
  strcpy(menu.sbuf, ""); // zero out

  strcat(menu.sbuf, "L:"); // 1st row
  strcat(menu.sbuf, pad(lightLevel));
  strcat(menu.sbuf, " F:"); // 1st row
  strcat(menu.sbuf, pad(fireLevel));
  strcat(menu.sbuf, " M:"); // 1st row
  strcat(menu.sbuf, pad(musicLevel));
  strcat(menu.sbuf, "\n"); // 2nd row
  strcat(menu.sbuf, "|!! |done");
  menu.drawUsrScreen(menu.sbuf);

  // always a good idea.
  instClear(tInst);
  // wait for escape to be pressed.
  boolean esc = digitalRead(B_STK)==LOW;
  while ( ! esc ) {
    byte buttons = readColorButtons();

    tInst.lightLevel[I_RED] = bitRead(buttons, I_RED) ? lightLevel : 0;
    tInst.lightLevel[I_BLU] = bitRead(buttons, I_BLU) ? lightLevel : 0;
    tInst.lightLevel[I_YEL] = bitRead(buttons, I_YEL) ? lightLevel : 0;
    tInst.lightLevel[I_GRN] = bitRead(buttons, I_GRN) ? lightLevel : 0;

    tInst.fireLevel[I_RED] = bitRead(buttons, I_RED) ? fireLevel : 0;
    tInst.fireLevel[I_BLU] = bitRead(buttons, I_BLU) ? fireLevel : 0;
    tInst.fireLevel[I_YEL] = bitRead(buttons, I_YEL) ? fireLevel : 0;
    tInst.fireLevel[I_GRN] = bitRead(buttons, I_GRN) ? fireLevel : 0;

    sendTowerInst(1, 99);
    printInstruction();
    delay(10);

    esc = digitalRead(B_STK)==LOW;
  }

}

void sendTowerInst(int sendCount, byte tower) {
  byte address = (tower == 99) ? 0 : towerNodeID[tower];

  // careful.  Just send deltas.
  static towerInstruction lastInst = tInst;
  if ( memcmp((void*)(&tInst), (void*)(&lastInst), sizeof(tInst)) != 0 ) {
    //  Serial << F("Sending tInst!") << endl;
    for ( int i = 0; i < sendCount; i++ ) {
      radio.send(address, (const void*)(&tInst), sizeof(tInst));
      while (!radio.canSend()); // wait for the transmission to complete
      delay(5);
    }
    lastInst = tInst; // save it.
  }
}
void towerWhich() {
  Serial << F("towerWhich") << endl;

  for (byte ni = 0; ni < N_TOWERS; ni++ ) {

    strcpy(menu.sbuf, "Tower:"); // zero out
    strcat(menu.sbuf, pad(ni));
    strcat(menu.sbuf, " set chan\n\n\n");
    strcat(menu.sbuf, " |assign      done| ");
    menu.drawUsrScreen(menu.sbuf);

    // wait for escape to be pressed.
    boolean esc = digitalRead(B_STK)==LOW;
    while ( ! esc ) {
      byte buttons = readColorButtons();

      // always a good idea.
      instClear(tInst);

      if( bitRead(buttons, I_RED) ) {
        towerChannels[ni] = I_RED;
        tInst.lightLevel[I_RED] = 255;
      }
      if( bitRead(buttons, I_GRN) ) {
        towerChannels[ni] = I_GRN;
        tInst.lightLevel[I_GRN] = 255;
      }
      if( bitRead(buttons, I_BLU) ) {
        towerChannels[ni] = I_BLU;
        tInst.lightLevel[I_BLU] = 255;
      }
      if( bitRead(buttons, I_YEL) ) {
        towerChannels[ni] = I_YEL;
        tInst.lightLevel[I_YEL] = 255;
      }

      sendTowerInst(1, ni);
      delay(10);

      esc = digitalRead(B_STK)==LOW;
      while( digitalRead(B_STK)==LOW ); // wait for release.
    }

    Serial << F("Tower ") << ni << F(" assigned ") << towerChannels[ni] << endl;
  }

}
void controlSend() {
  Serial << F("Send control.") << endl;
}


void printInstruction() {
  strcpy(menu.sbuf, ""); // clear out buffer this way; direct assignment fails...

  // 1st row
  strcat(menu.sbuf, pad(tInst.lightLevel[I_GRN]));
  strcat(menu.sbuf, "G");
  strcat(menu.sbuf, pad(tInst.fireLevel[I_GRN]));
  strcat(menu.sbuf, " ");
  strcat(menu.sbuf, pad(tInst.lightLevel[I_RED]));
  strcat(menu.sbuf, "R");
  strcat(menu.sbuf, pad(tInst.fireLevel[I_RED]));

  strcat(menu.sbuf, "\n");

  // 2nd row
  strcat(menu.sbuf, pad(tInst.lightLevel[I_YEL]));
  strcat(menu.sbuf, "Y");
  strcat(menu.sbuf, pad(tInst.fireLevel[I_YEL]));
  strcat(menu.sbuf, " ");
  strcat(menu.sbuf, pad(tInst.lightLevel[I_BLU]));
  strcat(menu.sbuf, "B");
  strcat(menu.sbuf, pad(tInst.fireLevel[I_BLU]));

  strcat(menu.sbuf, "\n");
  strcat(menu.sbuf, "\n");

  menu.drawUsrScreen(menu.sbuf);
}

void saveSettings() {
  Serial << F("Settings saved.") << endl;
  menu.writeEeprom();
}

// This is the "custom" navigation button function required for MENWIZ
int readButtons() {
  // need to define UP, DOWN, ESCAPE, CONFIRM
  int x=analogRead(J_X);
  int y=analogRead(J_Y);

  static int lastState, currentState;
  // offset from extremes to score as "yes"
  const int offSet = 100;

  // do want to allow for change-when-held, but only after a delay
  static unsigned long lastChange = millis();
  const unsigned long pressAgainInterval = 200UL;

  if( x>=(1023-offSet) && y>=(1023-offSet) ) currentState = MW_BTC; // right/confirm: +x, +y
  else if( x<=(0+offSet) && y>=(1023-offSet) ) currentState = MW_BTD; // down: -x, +y
  else if( x<=(0+offSet) && y<=(0+offSet) ) currentState = MW_BTE; // left/escape: -x, -y
  else if( x>=(1023-offSet) && y<=(0+offSet) ) currentState = MW_BTU; // up: +x, -y
  else currentState = MW_BTNULL; // nada

  if( currentState == lastState && (millis() - lastChange) <= pressAgainInterval) {
    // nothing new
    return( MW_BTNULL );
  } 
  else {
    lastState = currentState;
    lastChange = millis();
    return( currentState );
  }
}

byte readColorButtons() {
  byte ret = 0;
  if( digitalRead(B_RED)==LOW ) ret |= 1 << I_RED;
  if( digitalRead(B_GRN)==LOW ) ret |= 1 << I_GRN;
  if( digitalRead(B_BLU)==LOW ) ret |= 1 << I_BLU;
  if( digitalRead(B_YEL)==LOW ) ret |= 1 << I_YEL;
  return( ret );
}

char * pad(byte v) {
  const char padding[] = "   "; // could zero pad
  strcpy(buf, padding);

  byte ind = v>=10 ? (v>=100 ? 0 : 1) : 
  2;
  //  Serial << F("buf: '") << buf << F("'") << F("   ind") << ind << endl;

  // copy at the right starting character.  
  itoa(v, buf+ind, 10);

  return( buf );
}

/*
**** copied straight from Tower.ino
 */


// starts the radio
byte networkStart(boolean overWriteEEPROM) {

  Serial << F("Tower: Reading radio settings from EEPROM.") << endl;

  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;

  // try to recover settings from EEPROM
  byte offset = 0;
  byte nodeID = EEPROM.read(radioConfigLocation + (offset++));
  byte groupID = EEPROM.read(radioConfigLocation + (offset++));
  byte band = EEPROM.read(radioConfigLocation + (offset++));
  byte csPin = EEPROM.read(radioConfigLocation + (offset++));

  if ( groupID != D_GROUP_ID || overWriteEEPROM ) {
    Serial << F("Tower: EEPROM not configured.  Doing so.") << endl;
    // then EEPROM isn't configured correctly.
    offset = 0;
    EEPROM.write(radioConfigLocation + (offset++), myNodeID);
    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF69_915MHZ);
    EEPROM.write(radioConfigLocation + (offset++), D_CS_PIN);

    return ( networkStart(false) ); // go again after EEPROM save

  }
  else {
    Serial << F("Tower: Startup RFM12b radio module. ");
    Serial << F(" NodeID: ") << nodeID;
    Serial << F(" GroupID: ") << groupID;
    Serial << F(" Band: ") << band;
    Serial << F(" csPin: ") << csPin;
    Serial << endl;

    radio.initialize(band, nodeID, groupID);
    radio.setHighPower(); // for HW boards.
    radio.promiscuous(true); // so broadcasts are received.

    Serial << F("Tower: RFM69HW radio module startup complete. ");
    return (nodeID);
  }

}

// saves configuration to EEPROM
void configSave(towerConfiguration &config) {
  Serial << F("Writing towerConfiguration to EEPROM.") << endl;
  eeprom_write_block((const void*)&config, (void*)towerConfigLocation, sizeof(config));
}
// reads configuration from EEPROM
void configLoad(towerConfiguration &config) {
  Serial << F("Reading towerConfiguration from EEPROM.") << endl;
  eeprom_read_block((void*)&config, (void*)towerConfigLocation, sizeof(config));
}
// and prints current configuration
void configPrint(towerConfiguration &config, byte nodeID) {
  Serial << F("Tower (") << nodeID << F(") config: ");
  Serial << F("Color(");
  for (byte i = 0; i < N_COLORS; i++) Serial << config.lightListen[i] << F(" ");
  Serial << F(") Fire(");
  for (byte i = 0; i < N_COLORS; i++) Serial << config.fireListen[i] << F(" ");
  Serial << F(") ");
  Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").");
  Serial << F("Flame cooldown divisor: ") << config.flameCoolDownDivisor << endl;
}

// configure network
void towerConfig() {
  Serial << F("Tower: configuring network.") << endl;

  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    Serial << F("Tower ") << ni << F(" assigned ") << towerChannels[ni] << endl;
    // set tower listening channels.
    for ( byte nc = 0; nc < N_COLORS; nc++ ) {
      tConfig.lightListen[nc] = towerLightIndex == 0 ? ( towerChannels[ni] == nc ? true : false ) : 
      true; // Each2Own
      tConfig.fireListen[nc] = towerFireIndex == 0 ? ( towerChannels[ni] == nc ? true : false ) : 
      true; // AllIn
    }
    // open time mapping
    tConfig.minFireTime = towerFireMin;
    tConfig.maxFireTime = towerFireMax;
    tConfig.flameCoolDownDivisor = towerFireCooldown;

    sendTowerConfig(tConfig, towerNodeID[ni]);
  }
}

// configure a single tower
void sendTowerConfig(towerConfiguration & config, byte nodeID) {
  radio.send(nodeID, (const void*)(&config), sizeof(config));

  Serial << F("Tower: sending config: ");
  Serial << F("Tower (") << nodeID << F(") config: ");
  Serial << F("Color(");
  for (byte i = 0; i < N_COLORS; i++) Serial << config.lightListen[i] << F(" ");
  Serial << F(") Fire(");
  for (byte i = 0; i < N_COLORS; i++) Serial << config.fireListen[i] << F(" ");
  Serial << F(") ");
  Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").");
  Serial << F(" Flame cooldown divisor: ") << config.flameCoolDownDivisor << endl;

  while (!radio.canSend()); // wait for the transmission to complete
}

// clears all of the instructions
void instClear(towerInstruction & inst) {
  for (int i = 0; i < N_COLORS; i++ ) {
    inst.lightLevel[i] = 0;
    inst.fireLevel[i] = 0;
  }
}








