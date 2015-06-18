// Compile for Uno

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

// LCD
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <EEPROM.h> // saving and loading radio and menu settings
#include <MENWIZ.h>

// this is where the lights and fire instructions from Console are placed
towerInstruction tInst;
// structure stores how this Tower acts on those towerInstructions
towerConfiguration tConfig;
// structure stores Console instructions
consoleInstruction cInst;
// structure stores Console configuration
consoleConfiguration cConfig;

// Need an instance of the Radio Module
RFM69 radio;
// store my NODEID
byte myNodeID;

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// menuing
menwiz menu;

// place to paste 3-digit byte
char buf[3];

// add global variables to bind to menu
boolean buttonPressed = false;
int towerLightIndex, towerFireIndex, towerFireMin, towerFireMax, towerFireCooldown;
int testLightLevel, testFireLevel;

// track which towers are assigned to which channels (for Each2Own)
byte towerChannels[N_TOWERS] = {I_RED, I_BLU, I_YEL, I_GRN};

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

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

void menuSetup() {

  /* Menu structure:

    Config
      Light Index: Individual, All
      Fire Index: Individual, All
      Fire Time Min: integer [min:10, max:1000, step: 10]
      Fire Time Max: integer [min:10, max:1000, step: 10]
      Send: call configSend
    Control:
      Index: Red, Green, Blue, Yellow, All
      Toggle: call controlSend
    Save: call saveSettings
  */

  Serial << F("Setting up menu...") << endl;

  _menu *r, *s1, *s11, *s12, *s13, *s14, *s15, *s16, *s17, *s2, *s21, *s22, *s23, *s24, *s25, *s3;

  menu.begin(&lcd, 16, 2); //declare lcd object and screen size to menwiz lib

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

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lcd.setBacklight(WHITE);
  lcd.blink(); // blinking cursor
  lcd.cursor(); // so we can see

  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print(F("LCD: startup"));
  delay(500);

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
  strcat(menu.sbuf, itoa(lightLevel, buf, 10));
  strcat(menu.sbuf, " F:"); // 1st row
  strcat(menu.sbuf, itoa(fireLevel, buf, 10));
  strcat(menu.sbuf, " M:"); // 1st row
  strcat(menu.sbuf, itoa(musicLevel, buf, 10));
  strcat(menu.sbuf, "\n"); // 2nd row
  strcat(menu.sbuf, "|!! |done");
  menu.drawUsrScreen(menu.sbuf);

  // always a good idea.
  instClear(tInst);

  // wait for escape to be pressed.
  uint8_t buttons = lcd.readButtons();
  while ( ! ( buttons & BUTTON_SELECT ) ) {
    buttons = lcd.readButtons();

    tInst.lightLevel[I_RED] = (buttons & BUTTON_RIGHT) ? lightLevel : 0;
    tInst.lightLevel[I_BLU] = (buttons & BUTTON_DOWN) ? lightLevel : 0;
    tInst.lightLevel[I_YEL] = (buttons & BUTTON_LEFT) ? lightLevel : 0;
    tInst.lightLevel[I_GRN] = (buttons & BUTTON_UP) ? lightLevel : 0;

    tInst.fireLevel[I_RED] = (buttons & BUTTON_RIGHT) ? fireLevel : 0;
    tInst.fireLevel[I_BLU] = (buttons & BUTTON_DOWN) ? fireLevel : 0;
    tInst.fireLevel[I_YEL] = (buttons & BUTTON_LEFT) ? fireLevel : 0;
    tInst.fireLevel[I_GRN] = (buttons & BUTTON_UP) ? fireLevel : 0;

    sendTowerInst(1, 99);
    delay(10);
  }
}
void sendTowerInst(int sendCount, byte tower) {
  //  static towerInstruction lastInst = tInst;
  byte address = tower == 99 ? 0 : towerNodeID[address];

  // careful.  Just send deltas.
  //  if ( memcmp((void*)(&tInst), (void*)(&lastInst), sizeof(tInst)) != 0 ) {
//  Serial << F("Sending tInst!") << endl;
  for ( int i = 0; i < sendCount; i++ ) {
    radio.send(address, (const void*)(&tInst), sizeof(tInst));
    while (!radio.canSend()); // wait for the transmission to complete
    delay(5);
  }
  //   lastInst = tInst; // save it.
  // }
}
void towerWhich() {
  Serial << F("towerWhich") << endl;

  for (byte ni = 0; ni < N_TOWERS; ni++ ) {

    strcpy(menu.sbuf, "T:"); // zero out
    strcat(menu.sbuf, itoa(ni, buf, 10));
    strcat(menu.sbuf, " set chan\n");
    strcat(menu.sbuf, "|!! |done");
    menu.drawUsrScreen(menu.sbuf);


    // wait for escape to be pressed.
    uint8_t buttons = lcd.readButtons();
    uint8_t lastButton = buttons;
    while ( ! ( buttons & BUTTON_SELECT ) ) {
      buttons = lcd.readButtons();

      // always a good idea.
      instClear(tInst);
      
      if( (buttons & BUTTON_RIGHT) ) {
        towerChannels[ni] = I_RED;
        tInst.lightLevel[I_RED] = 255;
      }
      if( (buttons & BUTTON_DOWN) ) {
        towerChannels[ni] = I_BLU;
        tInst.lightLevel[I_BLU] = 255;
      }
      if( (buttons & BUTTON_LEFT) ) {
        towerChannels[ni] = I_YEL;
        tInst.lightLevel[I_YEL] = 255;
      }
      if( (buttons & BUTTON_UP) ) {
        towerChannels[ni] = I_GRN;
        tInst.lightLevel[I_GRN] = 255;
      }

      sendTowerInst(1, ni);
      delay(10);

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
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_GRN], buf, 10));
  strcat(menu.sbuf, "G");
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_GRN], buf, 10));
  strcat(menu.sbuf, " ");
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_RED], buf, 10));
  strcat(menu.sbuf, "R");
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_RED], buf, 10));

  strcat(menu.sbuf, "\n");

  // 2nd row
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_YEL], buf, 10));
  strcat(menu.sbuf, "Y");
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_YEL], buf, 10));
  strcat(menu.sbuf, " ");
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_BLU], buf, 10));
  strcat(menu.sbuf, "B");
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_BLU], buf, 10));

  menu.drawUsrScreen(menu.sbuf);
}

void saveSettings() {
  Serial << F("Settings saved.") << endl;
  menu.writeEeprom();
}

// This is the "custom" navigation button function required for MENWIZ
int readButtons() {
  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    buttonPressed = true;
    if (buttons & BUTTON_UP) {
      return MW_BTU;
    }
    else if (buttons & BUTTON_DOWN) {
      return MW_BTD;
    }
    else if (buttons & BUTTON_LEFT) {
      return MW_BTE;
    }
    else if (buttons & BUTTON_RIGHT) {
      return MW_BTC;
    }
    else if (buttons & BUTTON_SELECT) {
      return MW_BTL; // escape
    } else {
      return MW_BTNULL;
    }
  }
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
      tConfig.lightListen[nc] = towerLightIndex == 0 ? ( towerChannels[ni] == nc ? true : false ) : true; // Each2Own
      tConfig.fireListen[nc] = towerFireIndex == 0 ? ( towerChannels[ni] == nc ? true : false ) : true; // AllIn
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




