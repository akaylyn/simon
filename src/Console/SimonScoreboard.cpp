#include "SimonScoreboard.h"
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// MGD See: https://www.arduino.cc/en/Reference/PROGMEM

// backer messages
                      //   "01234567890123456789" -> 20 characters in width, max
const char s00[] PROGMEM = "Allah on high";   // "String 0" etc are strings to store - change to suit.
const char s01[] PROGMEM = "Thanks, Mom!";
const char s02[] PROGMEM = "Ralph Baer R.I.P.";
const char s03[] PROGMEM = "Loves to SimonCrew";
// Then set up a table to refer to your strings.
const char* const backerMessages[] PROGMEM = {s00, s01, s02, s03};

void SimonScoreboard::begin() {

  // LCD
  lcd.begin(20,4); // initialize the lcd
  lcd.backlight(); // backlight, always
  lcd.home();                   // go home
  lcd.print("    Simon v2 LCD    ");

  highScore = EEPROM.read(EEPROM_ADDR);
  displayHighScore();

}

void SimonScoreboard::saveHighScore() {
  if (currScore > highScore) {
    highScore = currScore;
    EEPROM.write(EEPROM_ADDR, highScore);
  }
  displayHighScore();
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

void SimonScoreboard::displayCurrScore() {
  lcd.setCursor(0, 1);
  lcd.print(currScore);
}

void SimonScoreboard::displayHighScore() {
  lcd.setCursor(10, 1);
  lcd.print(highScore);
}

void SimonScoreboard::showBackerMessages() {
  char buffer[20], buffer2[20]; // 20 characters.
  static char thx[] = "THX! to our Backers:";
  static Metro cycleInterval(3000);
  static int nMessages = sizeof(backerMessages)/2;
  static int i=random(0, nMessages); // start somewhere new at the beginning.
  
  if( cycleInterval.check() ) {
    Serial << "i=" << i << endl;
    Serial << "nM=" << nMessages << endl;
    Serial << "thx=" << thx << endl;
    strcpy_P(buffer, (char*)pgm_read_word(&(backerMessages[i]))); // Necessary casts and dereferencing, just copy.
    Serial << "m=" << buffer << endl;
    sprintf(buffer2, "%20s", buffer); // sprintf incurs a 1K memory cost.  It's awful, and I just need blank padding.
    Serial << "m=" << buffer2 << endl;
        
    lcd.setCursor(0, 2);
    lcd.print(thx);
    lcd.setCursor(0, 3);
    lcd.print(buffer2);
    
    cycleInterval.reset();
    (++i) %= nMessages; // increment with wrap
  }
}



SimonScoreboard scoreboard;

