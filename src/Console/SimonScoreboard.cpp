#include "SimonScoreboard.h"
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void SimonScoreboard::begin() {

  // LCD
  lcd.begin(20,4); // initialize the lcd
  lcd.backlight(); // backlight, always
  lcd.home();                   // go home
  lcd.print("    Simon v2 LCD    ");

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

void SimonScoreboard::incrementCurrScore() {
  currScore += 1;
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

SimonScoreboard scoreboard;

