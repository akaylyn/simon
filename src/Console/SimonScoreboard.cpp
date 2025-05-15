#include "SimonScoreboard.h"
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// MGD See: https://www.arduino.cc/en/Reference/PROGMEM

// Backer messages
                      //   "01234567890123456789" -> 20 characters in width, max
const char b00[] PROGMEM = "Simon )'( Michelle";   // "String 0" etc are strings to store - change to suit.
const char b01[] PROGMEM = "TrogdorTheBurninator";
const char b02[] PROGMEM = "DN <3 DS";
const char b03[] PROGMEM = "Play nice and often!";

// All backer names
const char b04[] PROGMEM = "lauren.desteno";
const char b05[] PROGMEM = "beaverlakestables";
const char b06[] PROGMEM = "chozan";
const char b07[] PROGMEM = "Trevor Schrock";
const char b08[] PROGMEM = "John Grant";
const char b09[] PROGMEM = "Patrick Gleason";
const char b10[] PROGMEM = "Brian Artiaco";
const char b11[] PROGMEM = "Michelle Clemens";
const char b12[] PROGMEM = "Alina Morris";
const char b13[] PROGMEM = "Anonymous";
const char b14[] PROGMEM = "Grace Jones";
const char b15[] PROGMEM = "r0beka";
const char b16[] PROGMEM = "Joel Henderson";
const char b17[] PROGMEM = "Bribe & Lucy";
const char b18[] PROGMEM = "Heidi Pickman";
const char b19[] PROGMEM = "Chase Childers";
const char b20[] PROGMEM = "Scott Cohen";
const char b21[] PROGMEM = "joeblakewestseattle";
const char b22[] PROGMEM = "madeline.barch";
const char b23[] PROGMEM = "kilgore5612";
const char b24[] PROGMEM = "Anonymous";
const char b25[] PROGMEM = "tommyhinman";
const char b26[] PROGMEM = "Preston";
const char b27[] PROGMEM = "James Sosan";
const char b28[] PROGMEM = "Michael Shaver";
const char b29[] PROGMEM = "Colleen Himes";
const char b30[] PROGMEM = "Sean Thayer";
const char b31[] PROGMEM = "Keith Salender";
const char b32[] PROGMEM = "Anonymous";
const char b33[] PROGMEM = "Anonymous";
const char b34[] PROGMEM = "David Betz-Zall";
const char b35[] PROGMEM = "ewp123";
const char b36[] PROGMEM = "lauren.desteno";
const char b37[] PROGMEM = "Anonymous";
const char b38[] PROGMEM = "Anonymous";

// Then set up a table to refer to your strings.
const char* const backerMessages[] PROGMEM = {b00, b01, b02, b03, b04, b06, b06, b07, b08, b09,
                                              b10, b11, b12, b13, b14, b16, b16, b17, b18, b19,
                                              b20, b21, b22, b23, b24, b26, b26, b27, b28, b29,
                                              b30, b31, b32, b33, b34, b36, b36, b37, b38};

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

void SimonScoreboard::showMessagePersist(char* msg) {
  lcd.setCursor(0, 0);
  lcd.print(" ");
  lcd.print(msg);
}
// doesn't work, remove
void SimonScoreboard::showMessagePersist2(char* msg) {
  lcd.setCursor(0, 1);
  // clear previous message on line 2, don't clear line 0
  lcd.print("                    "); 
  lcd.print(msg);
}


SimonScoreboard scoreboard;

