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

// add global variables to bind to menu
boolean buttonPressed = false;
int towerLightIndex, towerFireIndex, towerFireMin, towerFireMax;
int testLightLevel, testFireLevel;

#define FIRE_MIN 10
#define FIRE_MAX 1000
#define FIRE_STEP 10

#define LEVEL_MIN 0
#define LEVEL_MAX 10
#define LEVEL_STEP 1

#define INTERACTIVE_TIMEOUT 3000

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

  _menu *r, *s1, *s11, *s12, *s13, *s14, *s15, *s2, *s21, *s22, *s23, *s24, *s25, *s3;

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
    s13 = menu.addMenu(MW_VAR, s1, F("FireTimeMax"));
      s13->addVar(MW_AUTO_INT, &towerFireMin, FIRE_MIN, FIRE_MAX, FIRE_STEP);
    s14 = menu.addMenu(MW_VAR, s1, F("FireTimeMin"));
      s14->addVar(MW_AUTO_INT, &towerFireMax, FIRE_MIN, FIRE_MAX, FIRE_STEP*10);
    s15 = menu.addMenu(MW_VAR, s1, F("!!!Send!!!"));
      s15->addVar(MW_ACTION, configSend);

  s2 = menu.addMenu(MW_SUBMENU, r, F("Control"));
    s24 = menu.addMenu(MW_VAR, s2, F("LightSet"));
      s24->addVar(MW_AUTO_INT, &testLightLevel, LEVEL_MIN, LEVEL_MAX, LEVEL_STEP);
    s25 = menu.addMenu(MW_VAR, s2, F("FireSet"));
      s25->addVar(MW_AUTO_INT, &testFireLevel, LEVEL_MIN, LEVEL_MAX, LEVEL_STEP);
    s21 = menu.addMenu(MW_VAR, s2, F("Light Test"));
      s21->addVar(MW_ACTION, lightTest);
      s21->setBehaviour(MW_ACTION_CONFIRM, false); // just do it.
    s22 = menu.addMenu(MW_VAR, s2, F("Sound Test"));
      s22->addVar(MW_ACTION, soundTest);
      s22->setBehaviour(MW_ACTION_CONFIRM, false); // just do it.
    s23 = menu.addMenu(MW_VAR, s2, F("Fire Test"));
      s23->addVar(MW_ACTION, fireTest);
      s23->setBehaviour(MW_ACTION_CONFIRM, true);  // yeah.
       
  s3 = menu.addMenu(MW_VAR, r, F("Save Settings"));
    s3->addVar(MW_ACTION, saveSettings);

  // Use the navigation button from the Adafruit I2C shield
  menu.addUsrNav(readButtons, 4);

  // show a splash screen
  char splash[] = " Simon v2 Wand\n      -Magister";
  menu.addSplash(splash, 1000);

  // After a inactivity, show custom screen with real-time information
//  menu.addUsrScreen(printInstruction, INTERACTIVE_TIMEOUT);

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
}

void printInstruction() {
  static char buf[3];

  strcpy(menu.sbuf, "G");
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_GRN], buf, 10));
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_GRN], buf, 10));
  strcat(menu.sbuf, " R");  // 1st row
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_RED], buf, 10));
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_RED], buf, 10));

  strcat(menu.sbuf, "\n"); // 2nd row

  strcat(menu.sbuf, "Y");
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_YEL], buf, 10));
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_YEL], buf, 10));
  strcat(menu.sbuf, " B");
  strcat(menu.sbuf, itoa(tInst.lightLevel[I_BLU], buf, 10));
  strcat(menu.sbuf, itoa(tInst.fireLevel[I_BLU], buf, 10));

  menu.drawUsrScreen(menu.sbuf);
}

void lightTest() {
  Serial << F("Light Test!") << endl;
  byte lightLevel = map(testLightLevel, 0, 10, 0, 255);

  static char buf[3];
  strcpy(menu.sbuf, "Light:"); // 1st row
  strcat(menu.sbuf, itoa(lightLevel, buf, 10));
  strcat(menu.sbuf, "\n"); // 2nd row
  strcat(menu.sbuf, "|!! |esc");
  menu.drawUsrScreen(menu.sbuf);

  // always a good idea.
  instClear(tInst);

  // wait for escape to be pressed.  
  uint8_t buttons = lcd.readButtons();
  while( ! ( buttons & BUTTON_SELECT ) ) {
      buttons = lcd.readButtons();

      tInst.lightLevel[I_RED] = (buttons & BUTTON_RIGHT) ? lightLevel : 0;
      tInst.lightLevel[I_BLU] = (buttons & BUTTON_DOWN) ? lightLevel : 0;
      tInst.lightLevel[I_YEL] = (buttons & BUTTON_LEFT) ? lightLevel : 0;
      tInst.lightLevel[I_GRN] = (buttons & BUTTON_UP) ? lightLevel : 0;
      
      sendTowerInst(3);
  }
  
  Serial << lightLevel << endl;    
}
void soundTest() {
  Serial << F("Sound Test!") << endl;
}
void fireTest() {
  Serial << F("Fire Test!") << endl;
}
void sendTowerInst(int sendCount) {
  static towerInstruction lastInst=tInst;

  // careful.  Just send deltas.
  if( memcmp((void*)(&tInst), (void*)(&lastInst), sizeof(tInst)) != 0 ) {
    Serial << F("Sending iInst!") << endl;
    for( int i=0; i<sendCount; i++ ) {
      radio.send(0, (const void*)(&tInst), sizeof(tInst));
      delay(5);
    }
    lastInst = tInst; // save it.
  }
}

void configSend() {
  Serial << F("Send configuration.") << endl;
}

void controlSend() {
  Serial << F("Send control.") << endl;
  
  static boolean isOn = false;
  byte val;
  
  // toggle full on and full off
  if( isOn ) {
    val = 0;
  } else {
    val = 255;
  }
  
  // toggle light and fire
  switch(1) {
    case 0: // red
      tInst.lightLevel[I_RED] = val;
      tInst.fireLevel[I_RED] = val;
      break;
    case 1: // green
      tInst.lightLevel[I_GRN] = val;
      tInst.fireLevel[I_GRN] = val;
      break;
    case 2: // blue
      tInst.lightLevel[I_BLU] = val;
      tInst.fireLevel[I_BLU] = val;
      break;
    case 3: // yellow
      tInst.lightLevel[I_YEL] = val;
      tInst.fireLevel[I_YEL] = val;
      break;
    case 4: // all
      memset((void*)(&tInst), val, sizeof(tInst));
      break;
  }
  
  // flip for next call
  isOn = !isOn;  
  
  // send it
//  commsSend(inst);
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
  for(byte i=0; i<N_COLORS; i++) Serial << config.lightListen[i] << F(" ");
  Serial << F(") Fire(");
  for(byte i=0; i<N_COLORS; i++) Serial << config.fireListen[i] << F(" ");
  Serial << F(") ");
  Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").");
  Serial << F("Flame cooldown divisor: ") << config.flameCoolDownDivisor << endl;
}
// clears all of the instructions 
void instClear(towerInstruction & inst) {
  for (int i = 0; i < N_COLORS; i++ ) {
    inst.lightLevel[i] = 0;
    inst.fireLevel[i] = 0;
  }
}




