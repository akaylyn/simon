// Compile for Arduino Mega 2560.

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <MPR121.h> // MPR121 capsense board
#include <Wire.h> // capsense is an I2C device
#include <Bounce.h> // with debounce routine.

#include "Common.h"
#include "Touch.h"

void setup() {
  Serial.begin(115200);
  
  byte touchMapToColor[N_COLORS] = {I_RED, I_GRN, I_BLU, I_YEL};
  touch.begin(touchMapToColor);

  touch.recalibrate();
  
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
    
    Serial << " Released" << endl;
  }
  
  baselineCount++;
  if (baselineCount == 100) {
    touch.printBaseline();
    baselineCount = 0;
  }
  
  delay(1);
}
