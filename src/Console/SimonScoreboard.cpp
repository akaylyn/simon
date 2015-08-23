#include "SimonScoreboard.h"
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// MGD See: https://www.arduino.cc/en/Reference/PROGMEM

// backer messages
                      //   "01234567890123456789" -> 20 characters in width, max
const char b00[] PROGMEM = "Simon )'( Michelle";   // "String 0" etc are strings to store - change to suit.
const char b01[] PROGMEM = "TrogdorTheBurninator";
const char b02[] PROGMEM = "DN <3 DS";
const char b03[] PROGMEM = "Play nice and often!";
// Then set up a table to refer to your strings.
const char* const backerMessages[] PROGMEM = {b00, b01, b02, b03};

// Simon team
                      //   "01234567890123456789" -> 20 characters in width, max
const char t00[] PROGMEM = "Mike Dodds";   // "String 0" etc are strings to store - change to suit.
const char t01[] PROGMEM = "Kyle Miller";
const char t02[] PROGMEM = "Andrea Pollitt";
const char t03[] PROGMEM = "Shane Iler";
const char t04[] PROGMEM = "Chris Galvin";
const char t05[] PROGMEM = "Alan Hudson (Giles)";
const char t06[] PROGMEM = "Brice Reinhardt";
const char t07[] PROGMEM = "Angelina Allen";
const char t08[] PROGMEM = "Aaron St.John";
// Then set up a table to refer to your strings.
const char* const simonTeam[] PROGMEM = {t00, t01, t02, t03, t04, t05, t06, t07, t08};

void SimonScoreboard::begin() {

  // LCD
  lcd.begin(20,4); // initialize the lcd
  lcd.backlight(); // backlight, always

  Serial << F("Setup: setting up super fonts....") << endl;

  init_super_font(&lcd); // big big numbers

  Serial << F("Setup: LCD initialized.") << endl;

  highScore = EEPROM.read(EEPROM_ADDR);

}

void SimonScoreboard::clear() {
  lcd.clear(); // takes a while, iirc.
}

void SimonScoreboard::resetCurrScore() {
  currScore = 0;
  displayCurrScore();
}

void SimonScoreboard::saveCurrScore(int playerCurrent) {
  if (playerCurrent > currScore) {
    currScore = playerCurrent;
  }
  displayCurrScore();
}

void SimonScoreboard::resetHighScore() {
  highScore = 0;
  EEPROM.write(EEPROM_ADDR, 0);
}

void SimonScoreboard::saveHighScore() {
  if (currScore > highScore) {
    highScore = currScore;
    EEPROM.write(EEPROM_ADDR, highScore);
  }
}

/*
  01234567890123456789

0:PLAYER1: ***** *****
1:             *     *
2:             *     *
3:HIGH:00      *     *

*/

void SimonScoreboard::displayCurrScore() {
  static char p1[] = "PLAYER1:";
  lcd.setCursor(0, 0);
  lcd.print(p1);
  char b[3]; // zero terminated strings, so you need one more allocated than expected.
  sprintf(b, "%02d", currScore); // using a buffer to get padding
  render_super_msg(b, 9, 0);

  static char p2[] = "HIGH:";
  lcd.setCursor(0, 3);
  lcd.print(p2);
  sprintf(b, "%02d", highScore); // using a buffer to get padding
  lcd.print(b);
}

void SimonScoreboard::showBackerMessages() {
  char buffer[20], buffer2[20]; // 20 characters.
  static char thx[] = "THX! to our Backers:";
  static Metro cycleInterval(3000);
  static int nMessages = sizeof(backerMessages)/2;
  static int i=random(0, nMessages); // start somewhere new at the beginning.

  if( cycleInterval.check() ) {
//    Serial << "i=" << i << endl;
//    Serial << "nM=" << nMessages << endl;
//    Serial << "thx=" << thx << endl;
    strcpy_P(buffer, (char*)pgm_read_word(&(backerMessages[i]))); // Necessary casts and dereferencing, just copy.
//    Serial << "m=" << buffer << endl;
    sprintf(buffer2, "%20s", buffer); // sprintf incurs a 1K memory cost.  It's awful, and I just need blank padding.
//    Serial << "m=" << buffer2 << endl;

    lcd.setCursor(0, 2);
    lcd.print(thx);
    lcd.setCursor(0, 3);
    lcd.print(buffer2);

    cycleInterval.interval(random(3000,4000));
    cycleInterval.reset();
    (++i) %= nMessages; // increment with wrap
  }
}
void SimonScoreboard::showSimonTeam() {
  char buffer[20], buffer2[20]; // 20 characters.
  static char thx[] = "*** Simon v2, by ***";
  static Metro cycleInterval(3000);
  static int nMessages = sizeof(simonTeam)/2;
  static int i=random(0, nMessages); // start somewhere new at the beginning.

  if( cycleInterval.check() ) {
//    Serial << "i=" << i << endl;
//    Serial << "nM=" << nMessages << endl;
//    Serial << "thx=" << thx << endl;
    strcpy_P(buffer, (char*)pgm_read_word(&(simonTeam[i]))); // Necessary casts and dereferencing, just copy.
//    Serial << "m=" << buffer << endl;
    sprintf(buffer2, "%20s", buffer); // sprintf incurs a 1K memory cost.  It's awful, and I just need blank padding.
//    Serial << "m=" << buffer2 << endl;

    lcd.setCursor(0, 0);
    lcd.print(thx);
    lcd.setCursor(0, 1);
    lcd.print(buffer2);

    cycleInterval.interval(random(3000,4000));
    cycleInterval.reset();
    (++i) %= nMessages; // increment with wrap
  }
}

void SimonScoreboard::showMessage(char * msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg);
}

void SimonScoreboard::showMessage2(char * msg) {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(msg);
}


SimonScoreboard scoreboard;

