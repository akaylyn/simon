// Compile for Arduino Mega 2560.

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <MPR121.h> // MPR121 capsense board
#include <Wire.h> // capsense is an I2C device
#include <Bounce.h> // with debounce routine.
#include <EasyTransfer.h> // used for sending message to the sound module
#include <wavTrigger.h> // sound board

#include "Common.h"
#include "Touch.h"
#include "Sound.h" // Sound subunit.  Responsible for UX (music) output.

void setup() {
  Serial.begin(115200);
  
  byte touchMapToColor[N_COLORS] = {I_RED, I_GRN, I_BLU, I_YEL};
  touch.begin(touchMapToColor);

  touch.recalibrate();
  
  sound.begin(); //
  sound.setLeveling(1, 0); // 1x tone and 1x track

  delay(1000);  
  Serial << "Touch Unit Test" << endl;

}

int baselineCount = 0;

void loop() {
  // put your main code here, to run repeatedly:
  int stuckCount = 0;
  
  if (touch.anyPressed()) {
    MPR121.updateAll();
    color button = touch.whatPressed();
    sound.playTone(button);
    Serial << "Got Press: " << button << " filter: " << MPR121.getFilteredData(button);
    
    while(touch.anyPressed()) {
      delay(100);
        Serial << "Touch detected: baseline: " << MPR121.getBaselineData(button) <<  " filter: " << MPR121.getFilteredData(button) << endl;
      //Serial << ".";
      stuckCount++;
      if (stuckCount > 50) {
        MPR121.updateAll();
        Serial << "Stuck detected: baseline: " << MPR121.getBaselineData(button) <<  " filter: " << MPR121.getFilteredData(button) << endl;
        touch.recalibrate();
      }
    }
    
    sound.stopTone(button);
    
    Serial << " Released" << endl;
  }
  
  baselineCount++;
  if (baselineCount == 200) {
    touch.printBaseline();
    baselineCount = 0;
    touch.recalibrate();
    
  }
  
  delay(1);
}
