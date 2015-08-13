// Responsible for keeping score and displaying the scoreboard
//
#ifndef SimonScoreboard_h
#define SimonScoreboard_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing
#include <Simon_Common.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <avr/pgmspace.h> // PROGMEM
#include <Metro.h>
#include <phi_super_font.h> // for really big characters

class SimonScoreboard {
  public:
    void begin();
    void clear(); // really slow function
    void resetCurrScore();
    void saveCurrScore(int playerCurrent);
    void resetHighScore(); // accessed from within LayoutMode.  Likely, we want to reset this once every project setup, so it's a convenient place to do it.
    void saveHighScore();

    void showBackerMessages();
    void showSimonTeam();

    void displayCurrScore();

    void showMessage(char * msg);
    void showMessage2(char * msg);

  private:
    uint32_t highScore;
    uint32_t currScore;
    static const byte EEPROM_ADDR = 77; // layoutpos + sizeof layout data; 69 + 8
};

extern SimonScoreboard scoreboard;

#endif

