// Compile for Uno

// include the library code:
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <Simon_Indexes.h> // sizes, indexing and comms common to Towers and Console
#include <EEPROM.h> // saving and loading radio settings
#include <SPI.h> // radio board is a SPI device
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

// how many presses to take per second.
#define PRESS_PER_SECOND 3

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
  lcd.print(F("Wand: startup"));
  
  // zero out instruction.
  commsDefault(inst);

  myNodeID = commsStart(7); // we're node 7
  radio.setHighPower(); // for HW boards.
  radio.promiscuous(true); // so broadcasts are received.
  
  if ( myNodeID == 0 ) {
    Serial << F("Unable to recover RFM settings!") << endl;
    while (1);
  }
  Serial << F("Radio startup complete. Node: ") << myNodeID << endl;

  Serial << F("Wand: startup complete.") << endl;

  lcd.setCursor(0,1);
  lcd.print(F("Node: "));
  lcd.print(myNodeID);
  
  delay(500);
}

void loop() {
  // store which we're selecting
  const byte colorInd[N_COLORS] = {I_GRN, I_RED, I_YEL, I_BLU};
  static byte colorSel = 0;

  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(inst) ) {
      // save instruction for lights/flame
      inst = *(towerInstruction*)radio.DATA;
    }
  }

  // print instruction status on LCD, first row
  printInstruction();

  // move the cursor to show what we're currently setting
  byte cursorX = colorSel == 0 || colorSel == 2 ? 3 : 12;
  byte cursorY = colorSel < 2 ? 0 : 1;
  lcd.setCursor(cursorX, cursorY);

  // read buttons and toggle instructions
  uint8_t buttons = lcd.readButtons();
  
  // but only do so on an interval
  static Metro buttonInterval(1000UL/PRESS_PER_SECOND);
  
  if (buttons && buttonInterval.check()) {
    if (buttons & BUTTON_UP) {
      inst.lightLevel[colorInd[colorSel]]++; // will wrap
      inst.fireLevel[colorInd[colorSel]]++; // will wrap
    }
    if (buttons & BUTTON_DOWN) {
      inst.lightLevel[colorInd[colorSel]]--; // will wrap
      inst.fireLevel[colorInd[colorSel]]--; // will wrap
    }
    if (buttons & BUTTON_LEFT) {
      if ( colorSel > 0 ) colorSel--;
      else colorSel == N_COLORS - 1; // wrap
    }
    if (buttons & BUTTON_RIGHT) {
      if ( colorSel < (N_COLORS - 1) ) colorSel++;
      else colorSel == 0; // wrap
    }
    if (buttons & BUTTON_SELECT) {
      // send now
      commsSend(inst);
    }
    buttonInterval.reset();
  }
}

void printInstruction() {
  char row1[16] = "";
  char row2[16] = "";

  // formatted
  sprintf(row1, "Grn:%3d  Red:%3d", inst.lightLevel[I_GRN], inst.lightLevel[I_RED]);
  sprintf(row2, "Yel:%3d  Blu:%3d", inst.lightLevel[I_YEL], inst.lightLevel[I_BLU]);

  lcd.setCursor(0, 0);
  lcd.print(row1);
  lcd.setCursor(0, 1);
  lcd.print(row2);
}
