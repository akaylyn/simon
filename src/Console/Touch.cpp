// This file should match src/Console/Touch.cpp

#include "Touch.h"

// buttons
Bounce redButton = Bounce(BUTTON_RED, BUTTON_DEBOUNCE_TIME);
Bounce grnButton = Bounce(BUTTON_GRN, BUTTON_DEBOUNCE_TIME);
Bounce bluButton = Bounce(BUTTON_BLU, BUTTON_DEBOUNCE_TIME);
Bounce yelButton = Bounce(BUTTON_YEL, BUTTON_DEBOUNCE_TIME);

// MPR121 object instantiated in the library.

boolean Touch::begin() {

  Serial << F("Touch: startup.") << endl;

  // following Examples->BareConductive_MPR->SimpleTouch

  // 0x5A is the MPR121 I2C address on the Bare Touch Board
  Wire.begin();

  boolean mprError = true;
  while( mprError ) {
    if (!MPR121.begin(MPR121_I2CADDR_DEFAULT)) {
      Serial << F("Touch: error setting up MPR121: ");
      switch (MPR121.getError()) {
        case ADDRESS_UNKNOWN:
          Serial << F("MPR121: did not respond at address") << endl;
          break;
        case READBACK_FAIL:
          Serial << F("MPR121: readback failure") << endl;
          break;
        case OVERCURRENT_FLAG:
          Serial << F("MPR121: overcurrent on REXT pin") << endl;
          break;
        case OUT_OF_RANGE:
          Serial << F("MPR121: electrode out of range") << endl;
          break;
        case NOT_INITED:
          Serial << F("MPR121: not initialised") << endl;
          break;
        default:
          Serial << F("MPR121: unknown error") << endl;
          break;
      }
      delay(1000);
    } 
    else {
      Serial << F("Touch: MPR121: initialized.") << endl;
        
      MPR121.reset();
      Serial << F("Touch: MPR121: reset.") << endl;
      // set the interrupt handler.
      MPR121.setInterruptPin(TOUCH_IRQ);

      // enable 13-th virtual proximity electrode, tying electrodes 0..3 together.
      MPR121.setProxMode(PROX0_3);
      Serial << F("Touch: MPR121 proximity enabled.") << endl;

      // initial data update
      MPR121.updateAll();
      Serial << F("Touch: MPR121 data updaet.") << endl;

      Serial << F("Touch: MPR121 initialization complete.") << endl;
      mprError = false;
    }
  }
  
  Serial << F("Touch: setting up hard buttons.") << endl;
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GRN, INPUT_PULLUP);
  pinMode(BUTTON_BLU, INPUT_PULLUP);
  pinMode(BUTTON_YEL, INPUT_PULLUP);

  button[I_RED] = &redButton;
  button[I_GRN] = &grnButton;
  button[I_BLU] = &bluButton;
  button[I_YEL] = &yelButton;

  Serial << F("Button: startup complete.") << endl;

  return ( true );
}

// Returns true if the state of a specific button has changed
// based on what it was previously.
boolean Touch::changed(byte index) {
  boolean ret = false;

  // capsense
  static boolean previousState[N_COLORS];

  MPR121.updateTouchData();
  boolean currentState = MPR121.getTouchData(index);

  if (previousState[index] != currentState) {
    previousState[index] = currentState;
    ret |= true;
  }

  // hard buttons
  ret |= button[index]->update();

  // return
  return ( ret );

}
// returns true if any of the buttons are pressed.
boolean Touch::anyChanged() {
  return (
  changed(I_RED) ||
    changed(I_GRN) ||
    changed(I_BLU) ||
    changed(I_YEL)
    );
}


// returns true WHILE a specific sensor IS PRESSED
// this function will be called after touchChanged() asserts a change.
boolean Touch::pressed(byte index) {
  boolean ret = false;

  // capsense
  MPR121.updateTouchData();
  ret |= MPR121.getTouchData(index);

  // hard buttons
  // call the updater for debouncing first.
  boolean toss = button[index]->update();

  ret |= button[index]->read() == PRESSED_BUTTON;

  // return
  return ( ret );
}

// returns true if any of the buttons have switched states.
// avoid short-circuit eval so that each button gets an update
boolean Touch::anyPressed() {
  return (
  pressed(I_RED) ||
    pressed(I_GRN) ||
    pressed(I_BLU) ||
    pressed(I_YEL)
    );
}
// for distance/proximity, see http://cache.freescale.com/files/sensors/doc/app_note/AN3893.pdf

// returns "distance" an object is to the sensor, scaled [0, 32767]
// realistically, we have 10-bit resolution?
int Touch::distance(byte sensorIndex) {

  // for this, we need the baseline and filtered data
  boolean foo = MPR121.updateBaselineData();
  boolean bar = MPR121.updateFilteredData();

  // save the maximum delta we note
  static int maxDelta[13] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0    };
  for (int i = 0; i < 13; i++ ) {
    maxDelta[i] = max(maxDelta[i], MPR121.getBaselineData(i) - MPR121.getFilteredData(i));
  }

  // the larger the delta, the closer the object
  int sensorDelta = MPR121.getBaselineData(sensorIndex) - MPR121.getFilteredData(sensorIndex);

  // delta is zero at infinite distance, and very large (approaching baseline data) at zero distance, so distance has an inverse relationship with delta
  // like many fields in a 3-D volume, the radiated energy decreses with the distance squared from the source.
  // so, we'll take a stab at distance = 1/(delta^2)
  const int maxInt = 32767;
  const float calibrant = -2.0;

  float distance = fscale(0, maxDelta[sensorIndex], maxInt, 0, sensorDelta, calibrant);

  return ( int(distance) );

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

  if (newEnd > newBegin) {
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

  if (invFlag == 0) {
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}

Touch touch;


