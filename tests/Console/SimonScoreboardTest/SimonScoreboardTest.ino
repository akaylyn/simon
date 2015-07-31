#include <Arduino.h>
#include <Streaming.h> // <<-style printing
#include <Simon_Common.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#include "SimonScoreboard.h"

void setup() {

  Serial << "!! setup !!" << endl;

  // put your setup code here, to run once:
  // attach interrupt to pin - interrupt 1 is on pin 2 of the arduino (confusing I know)
  // attachInterrupt(0, dataAvailable, FALLING);
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));

  // start
  scoreboard.begin();
}

// main loop for the core.
void loop() {

  // increment current score
  //
  scoreboard.resetCurrScore();
  scoreboard.incrementCurrScore();
  delay(500);
  scoreboard.incrementCurrScore();
  delay(500);
  // save high score
  scoreboard.saveHighScore();

}

