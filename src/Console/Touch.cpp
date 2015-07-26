#include "Touch.h"

// MPR121 object instantiated in the library.

boolean Touch::begin(byte sensorIndex[N_COLORS]) {

  Serial << F("Touch: startup.") << endl;

  // following Examples->BareConductive_MPR->SimpleTouch

  // store it
  for( byte i=0; i<N_COLORS; i++ )
    this->sensorIndex[i] = sensorIndex[i];

  // 0x5A is the MPR121 I2C address on the Bare Touch Board
  Wire.begin();

  Serial << F("Touch: Wire begin.") << endl;

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
    }
    else {
      Serial << F("Touch: MPR121: initialized.") << endl;
      /*
      // WARNING: MPR121.reset() blows the whole thing up.  Probably need to resend configuration after doing so?
      MPR121.reset();
      //      Serial << F("Touch: MPR121: reset.") << endl;
      // NOT USING interrupt handler.  Polling mode only.
      MPR121.setInterruptPin(TOUCH_IRQ);
      */

      // Alan removed
      // enable 13-th virtual proximity electrode, tying electrodes 0..3 together.
      //MPR121.setProxMode(PROX0_3);
      Serial << F("Touch: MPR121 proximity enabled.") << endl;

      // initial data update
      MPR121.updateAll();
      Serial << F("Touch: MPR121 data update.") << endl;

      Serial << F("Touch: MPR121 initialization complete.") << endl;
      mprError = false;
    }
  }

  Serial << F("Touch: setting up hard buttons.") << endl;
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GRN, INPUT_PULLUP);
  pinMode(BUTTON_BLU, INPUT_PULLUP);
  pinMode(BUTTON_YEL, INPUT_PULLUP);

  // buttons
  button[I_RED] = new Bounce(BUTTON_RED, BUTTON_DEBOUNCE_TIME);
  button[I_GRN] = new Bounce(BUTTON_GRN, BUTTON_DEBOUNCE_TIME);
  button[I_BLU] = new Bounce(BUTTON_BLU, BUTTON_DEBOUNCE_TIME);
  button[I_YEL] = new Bounce(BUTTON_YEL, BUTTON_DEBOUNCE_TIME);

  Serial << F("Button: startup complete.") << endl;

  return ( true );
}

// Returns true if the state of a specific button has changed
// based on what it was previously.
boolean Touch::changed(color index) {
  boolean ret = false;

  // capsense
  static boolean previousState[N_COLORS];

  MPR121.updateTouchData();
  boolean currentState = MPR121.getTouchData(sensorIndex[index]);

  if (previousState[index] != currentState) {
    previousState[index] = currentState;
    ret |= true;
  }

  // hard buttons
  //  ret |= button[sensorIndex[index]]->update();

  // return
  return ( ret );

}
// returns true if any of the buttons are pressed.
boolean Touch::anyChanged() {
  return ( changed(I_RED) || changed(I_GRN) || changed(I_BLU) || changed(I_YEL) );
}


// returns true WHILE a specific sensor IS PRESSED
// this function will be called after touchChanged() asserts a change.
boolean Touch::pressed(color index) {
  boolean ret = false;

  // capsense
  MPR121.updateTouchData();
  ret |= MPR121.getTouchData(sensorIndex[index]);

  // hard buttons
  // call the updater for debouncing first.
  //  boolean toss = button[sensorIndex[index]]->update();

  //  ret |= button[sensorIndex[index]]->read() == PRESSED_BUTTON;

  // return
  return ( ret );
}

// returns true if any of the buttons have switched states.
boolean Touch::anyPressed() {
  return ( pressed(I_RED) || pressed(I_GRN) || pressed(I_BLU) || pressed(I_YEL) );
}

// returns the first pressed button found
color Touch::whatPressed() {
  if( pressed(I_RED) ) return (I_RED);
  if( pressed(I_GRN) ) return (I_GRN);
  if( pressed(I_BLU) ) return (I_BLU);
  if( pressed(I_YEL) ) return (I_YEL);
  // otherwise, signal "Boo!"
  return(N_COLORS);
}

// returns "distance" an object is to the sensor, scaled [0, 255]
// realistically, we see distance readings in [0,50], so scaling to a byte is reasonable
// for distance/proximity, see http://cache.freescale.com/files/sensors/doc/app_note/AN3893.pdf
byte Touch::distance(byte index) {

  // for this, we need the filtered data
  int sensorRead = 0;
  for( int i=0; i<10; i++ ) {
    // average ten readings
    boolean bar = MPR121.updateFilteredData();
    sensorRead += MPR121.getFilteredData(index);
  }
  sensorRead /= 10;

  // track sensor returns for 12 sensors and the virtual 13th.
  static int minRead[13] = { 350,350,350,350,350,350,350,350,350,350,350,350,350 }; // 300 seems to be the normal low end, but let's leave some room for drift
  minRead[index] = min(minRead[index], sensorRead);

  int delta = sensorRead - minRead[index];

  // track deltas 12 sensors and the virtual 13th.
  static int maxDelta[13] = { 0,0,0,0,0,0,0,0,0,0,0,0,0 };
  maxDelta[index] = max(maxDelta[index], delta);

  // nonlinear transform to get higher sensitivity at larger distances
  byte distance = fscale(0, maxDelta[index], 0, 255, delta, -3.0);
  //  Serial << F("Proximity: distance=") << distance << F(" delta=") << delta << F(" curr=") << sensorRead << F(" minR=") << minRead[sensorIndex] << F(" maxD=") << maxDelta[sensorIndex] << endl;

  return( distance );

}

byte Touch::distance(color index) {
  // reference color
  return( distance(sensorIndex[index]) );
}

byte Touch::proximity() {
  return( distance(12) );
}

void Touch::printElectrodeAndBaselineData() {
  // baseline values
  uint16_t base0 = ((uint16_t)MPR121.getRegister(0x1E))<<2;
  uint16_t base1 = ((uint16_t)MPR121.getRegister(0x1F))<<2;
  uint16_t base2 = ((uint16_t)MPR121.getRegister(0x20))<<2;
  uint16_t base3 = ((uint16_t)MPR121.getRegister(0x21))<<2;

  // sensor readings
  uint16_t data0 = (((uint16_t)MPR121.getRegister(0x05))<<8) | MPR121.getRegister(0x04);
  uint16_t data1 = (((uint16_t)MPR121.getRegister(0x07))<<8) | MPR121.getRegister(0x06);
  uint16_t data2 = (((uint16_t)MPR121.getRegister(0x09))<<8) | MPR121.getRegister(0x08);
  uint16_t data3 = (((uint16_t)MPR121.getRegister(0x0B))<<8) | MPR121.getRegister(0x0A);

  Serial << "0, " << base0 << ", " << data0 << endl;
  Serial << "1, " << base1 << ", " << data1 << endl;
  Serial << "2, " << base2 << ", " << data2 << endl;
  Serial << "3, " << base3 << ", " << data3 << endl;
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


