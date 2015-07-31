// Responsible for keeping score and displaying the scoreboard
//
#ifndef SimonScoreboard_h
#define SimonScoreboard_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing
#include <Simon_Common.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>


class SimonScoreboard {
  public:
    void begin();
    void resetCurrScore();
    void saveHighScore();
    void saveCurrScore(int playerCurrent);
  private:
    uint32_t highScore;
    uint32_t currScore;
    static const byte EEPROM_ADDR = 77; // layoutpos + sizeof layout data; 69 + 8
    void displayCurrScore();
    void displayHighScore();
};

extern SimonScoreboard scoreboard;

#endif

