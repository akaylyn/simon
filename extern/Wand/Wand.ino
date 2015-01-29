// Compile for Uno

// from: http://forums.adafruit.com/viewtopic.php?f=47&t=45258

// include the library code:
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <EEPROM.h> // saving and loading radio and menu settings
#include <SPI.h> // radio board is a SPI device
// LCD
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
//#include <buttons.h>
#include <MENWIZ.h>
// radio
#include <Simon_Indexes.h> // sizes, indexing and comms common to Towers and Console
#include <RFM69.h> // RFM12b radio transmitter module
#include <Simon_Comms.h>  // Wand<=>Console and Console=>Tower sniffing

// this is where the lights and fire instructions to Console are placed
towerInstruction inst;
// from Simon_Comms.h:
// typedef struct {
//	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
//	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
// } towerInstruction;

// Need an instance of the Radio Module
RFM69 radio;
// store my NODEID
byte myNodeID;

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// menuing
menwiz menu;

// add global variables to bind to menu
boolean buttonPressed = false;
int towerLightIndex, towerFireIndex, towerFireMin, towerFireMax, controlIndex;

#define FIRE_MIN 10
#define FIRE_MAX 1000
#define FIRE_STEP 10

#define INTERACTIVE_TIMEOUT 3000

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

  _menu *r, *s1, *s11, *s12, *s13, *s14, *s15, *s2, *s21, *s22, *s3;

  menu.begin(&lcd, 16, 2); //declare lcd object and screen size to menwiz lib

  r = menu.addMenu(MW_ROOT, NULL, F("Main Menu"));
  s1 = menu.addMenu(MW_SUBMENU, r, F("Config"));
    s11 = menu.addMenu(MW_VAR, s1, F("Light Index"));
      s11->addVar(MW_LIST, &towerLightIndex);
      s11->addItem(MW_LIST, F("Individual"));
      s11->addItem(MW_LIST, F("All"));
      s11->setBehaviour(MW_SCROLL_HORIZONTAL, true);
  
    s12 = menu.addMenu(MW_VAR, s1, F("Fire Index"));
      s12->addVar(MW_LIST, &towerFireIndex);
      s12->addItem(MW_LIST, F("Individual"));
      s12->addItem(MW_LIST, F("All"));
      s12->setBehaviour(MW_SCROLL_HORIZONTAL, true);
    s13 = menu.addMenu(MW_VAR, s1, F("Fire Time Min"));
      s13->addVar(MW_AUTO_INT, &towerFireMin, FIRE_MIN, FIRE_MAX, FIRE_STEP);
    s14 = menu.addMenu(MW_VAR, s1, F("Fire Time Min"));
      s14->addVar(MW_AUTO_INT, &towerFireMax, FIRE_MIN, FIRE_MAX, FIRE_STEP);
    s15 = menu.addMenu(MW_VAR, s1, F("Send Configuration"));
      s15->addVar(MW_ACTION, configSend);

  s2 = menu.addMenu(MW_SUBMENU, r, F("Control"));
    s21 = menu.addMenu(MW_VAR, s2, F("Control Index"));
      s21->addVar(MW_LIST, &controlIndex);
      s21->addItem(MW_LIST, F("Red"));
      s21->addItem(MW_LIST, F("Green"));
      s21->addItem(MW_LIST, F("Blue"));
      s21->addItem(MW_LIST, F("Yellow"));
      s21->addItem(MW_LIST, F("All"));
      s21->setBehaviour(MW_SCROLL_HORIZONTAL, true);

    s22 = menu.addMenu(MW_VAR, s2, F("Send Control"));
      s22->addVar(MW_ACTION, controlSend);
 
  s3 = menu.addMenu(MW_VAR, r, F("Save Settings"));
    s3->addVar(MW_ACTION, saveSettings);

  // Use the navigation button from the Adafruit I2C shield
  menu.addUsrNav(readButtons, 4);

  // show a splash screen
  char splash[] = "Simon v2 Wand\n(by Mike)";
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
  delay(100);
  
  // set up menus
  menuSetup();

  // zero out instruction.
  commsDefault(inst);

  myNodeID = commsStart(7); // we're node 7
  radio.setHighPower(); // for HW boards.
  radio.promiscuous(true); // so broadcasts are received.
  Serial << F("Radio startup complete. Node: ") << myNodeID << endl;

  Serial << F("Wand: startup complete.") << endl;

}

void loop() {
  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(inst) ) {
      // save instruction for lights/flame
      inst = *(towerInstruction*)radio.DATA;
    }
  }

  // do the menus
  menu.draw();
}

void printInstruction() {
  static char buf[3];

  menu.sbuf = ""; // 1st row
  strcat(menu.sbuf, "R");
  strcat(menu.sbuf, itoa(inst.lightLevel[I_RED], buf, 10));
  strcat(menu.sbuf, itoa(inst.fireLevel[I_RED], buf, 10));
  strcat(menu.sbuf, " G");
  strcat(menu.sbuf, itoa(inst.lightLevel[I_GRN], buf, 10));
  strcat(menu.sbuf, itoa(inst.fireLevel[I_GRN], buf, 10));

  strcat(menu.sbuf, "\n"); // 2nd row

  strcat(menu.sbuf, "B");
  strcat(menu.sbuf, itoa(inst.lightLevel[I_BLU], buf, 10));
  strcat(menu.sbuf, itoa(inst.fireLevel[I_BLU], buf, 10));
  strcat(menu.sbuf, " Y");
  strcat(menu.sbuf, itoa(inst.lightLevel[I_YEL], buf, 10));
  strcat(menu.sbuf, itoa(inst.fireLevel[I_YEL], buf, 10));

  menu.drawUsrScreen(menu.sbuf);
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
  switch(controlIndex) {
    case 0: // red
      inst.lightLevel[I_RED] = val;
      inst.fireLevel[I_RED] = val;
      break;
    case 1: // green
      inst.lightLevel[I_GRN] = val;
      inst.fireLevel[I_GRN] = val;
      break;
    case 2: // blue
      inst.lightLevel[I_BLU] = val;
      inst.fireLevel[I_BLU] = val;
      break;
    case 3: // yellow
      inst.lightLevel[I_YEL] = val;
      inst.fireLevel[I_YEL] = val;
      break;
    case 4: // all
      memset((void*)(&inst), val, sizeof(inst));
      break;
  }
  
  // flip for next call
  isOn = !isOn;  
  
  // send it
  commsSend(inst);
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
    else {
      return MW_BTNULL;
    }
  }
}

