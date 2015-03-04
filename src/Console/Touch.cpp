// This file should match src/Console/Touch.cpp

#include "Touch.h"

// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

#define MPR121_I2CADDR_DEFAULT 0x5A
#define DEBUG true

boolean touchStart() {
  // some reasonable defaults are set.
  // expect this to be called after startup
  // and again between games.
  Serial << F("Touch: startup.") << endl;

  // following Examples->BareConductive_MPR->SimpleTouch

  // 0x5C is the MPR121 I2C address on the Bare Touch Board
  if (!MPR121.begin(MPR121_I2CADDR_DEFAULT)) {
    Serial << F("Touch: error setting up MPR121");
    switch (MPR121.getError()) {
      case ADDRESS_UNKNOWN:
        Serial << F("Touch: incorrect address") << endl;
        return ( false );
        break;
      case READBACK_FAIL:
        Serial << F("readback failure") << endl;
        return ( false );
        break;
      case OVERCURRENT_FLAG:
        Serial << F("overcurrent on REXT pin") << endl;
        return ( false );
        break;
      case OUT_OF_RANGE:
        Serial << F("electrode out of range") << endl;
        return ( false );
        break;
      case NOT_INITED:
        Serial << F("not initialised") << endl;
        return ( false );
        break;
      default:
        Serial << F("unknown error") << endl;
        return ( false );
        break;
    }
  }
  else {
    Serial << F("Touch: no error") << endl;
    // set the interrupt handler.
    MPR121.setInterruptPin(TOUCH_IRQ);
    
    // enable 13-th virtual proximity electrode, tying electrodes 0..3 together.
    MPR121.setProxMode(PROX0_3);
    
    // initial data update
    touchCalibrate();
    MPR121.updateTouchData();

    return (true);
  }

}

// calibrates the Touch interface
void touchCalibrate() {
  // also gets baseline, proxity, etc.
  MPR121.updateAll();

  Serial << "Touch: calibrate." << endl;
  Serial << "Red:";
  Serial << " data(" << MPR121.getTouchData(I_RED) << ")";
  Serial << " touch(" << MPR121.getTouchThreshold(I_RED) << ")";
  Serial << " release(" << MPR121.getReleaseThreshold(I_RED) << ")";
  Serial << " filtered(" << MPR121.getFilteredData(I_RED) << ")";
  Serial << " base(" << MPR121.getBaselineData(I_RED) << ")";
  Serial << endl;
}

// returns true if any of the buttons have switched states.
// avoid short-circuit eval so that each button gets an update
boolean touchAnyPressed() {
  return (
           touchPressed(I_RED) ||
           touchPressed(I_GRN) ||
           touchPressed(I_BLU) ||
           touchPressed(I_YEL)
         );
}

// returns true if any of the buttons are pressed.
boolean touchAnyChanged() {
  return (
           touchChanged(I_RED) ||
           touchChanged(I_GRN) ||
           touchChanged(I_BLU) ||
           touchChanged(I_YEL)
         );
}

// returns true WHILE a specific sensor IS PRESSED
// this function will be called after touchChanged() asserts a change.
boolean touchPressed(byte touchIndex) {
  if (touchIndex == I_ALL) return touchAnyPressed();

  MPR121.updateTouchData();
  return MPR121.getTouchData(touchIndex);
}

// Returns true if the state of a specific button has changed
// based on what it was previously.
boolean touchChanged(byte touchIndex) {
  if (touchIndex == I_ALL) return touchAnyChanged();

  MPR121.updateTouchData();
  static boolean previousState[N_COLORS];
  boolean currentState = MPR121.getTouchData(touchIndex);
  if (previousState[touchIndex] != currentState) {
    previousState[touchIndex] = currentState;
    return true;
  }
  return false;
}

// runs a unit test on Touch
// hold all buttons down to exit
void touchUnitTest() {
  static boolean keepRunning = true;
  while ( keepRunning ) {

    // also gets baseline, proxity, etc.
    MPR121.updateAll();

    boolean red = touchPressed(I_RED);
    boolean grn = touchPressed(I_GRN);
    boolean blu = touchPressed(I_BLU);
    boolean yel = touchPressed(I_YEL);

    Serial << "R: " << red;
    Serial << " G: " << grn;
    Serial << " B: " << blu;
    Serial << " Y: " << yel;
    Serial << endl;

    delay(100);
    keepRunning = !(red && grn && blu && yel);
  }
}

// for distance/proximity, see http://cache.freescale.com/files/sensors/doc/app_note/AN3893.pdf

// returns "distance" an object is to the sensor, scaled [0, 32767] 
// realistically, we have 10-bit resolution?
int touchDistance(byte touchIndex) {
  
  // for this, we need the baseline and filtered data
  boolean foo = MPR121.updateBaselineData();
  boolean bar = MPR121.updateFilteredData();

  // save the maximum delta we note
  static int maxDelta[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
  for(int i=0; i<13; i++ ) {
    maxDelta[i] = max(maxDelta[i], MPR121.getBaselineData(i) - MPR121.getFilteredData(i));
  }
  
  // which sensor are we interested in?
  byte sensorIndex = touchIndex == I_ALL ? 12 : touchIndex;
  // the larger the delta, the closer the object
  int sensorDelta = MPR121.getBaselineData(sensorIndex) - MPR121.getFilteredData(sensorIndex);

  // delta is zero at infinite distance, and very large (approaching baseline data) at zero distance, so distance has an inverse relationship with delta
  // like many fields in a 3-D volume, the radiated energy decreses with the distance squared from the source.
  // so, we'll take a stab at distance = 1/(delta^2)
  const int maxInt = 32767;
  const float calibrant = -2.0;

  float distance = fscale(0, maxDelta[sensorIndex], maxInt, 0, sensorDelta, calibrant);

  return( int(distance) );

}

// snagged this from https://github.com/BareConductive/midi_theremin/blob/public/midi_theremin/midi_theremin.ino
// http://playground.arduino.cc/Main/Fscale
float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;

  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  /*
   Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution  
   Serial.println();
   */

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin){
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  /*
  Serial.print(OriginalRange, DEC);  
   Serial.print("   ");  
   Serial.print(NewRange, DEC);  
   Serial.print("   ");  
   Serial.println(zeroRefCurVal, DEC);  
   Serial.println();  
   */

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0){
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {  
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
