#include "Touch.h"

// MPR121 object instantiated in the library.

boolean Touch::begin(byte sensorIndex[N_BUTTONS]) {

  Serial << F("Touch: startup.") << endl;

  // following Examples->BareConductive_MPR->SimpleTouch

  // store it
  for( byte i=0; i<N_BUTTONS; i++ )
  {
    this->sensorIndex[i] = sensorIndex[i];
    Serial << "touch init index: " << i << endl;
    Serial << "sensor index: " << sensorIndex[i] << endl;
  }

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
      //Serial << F("Touch: MPR121 proximity enabled.") << endl;

      // custom threshold for the start button
      MPR121.setTouchThreshold(I_START, 4);
      MPR121.setReleaseThreshold(I_START, 2);

      // custom threshold for the little left button; still need to get these numbers, I think?
      MPR121.setTouchThreshold(I_LEFT, 10);
      MPR121.setReleaseThreshold(I_LEFT, 5);

      // custom threshold for the little right button; still need to get these numbers, I think?
      MPR121.setTouchThreshold(I_RIGHT, 10);
      MPR121.setReleaseThreshold(I_RIGHT, 5);

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
boolean Touch::changed(byte index) {
  boolean ret = false;

  // capsense
  static boolean previousState[N_BUTTONS];

  MPR121.updateTouchData();
  boolean currentState = MPR121.getTouchData(sensorIndex[index]);

  if (previousState[index] != currentState) {
    previousState[index] = currentState;
    ret |= true;
  }

  // hard buttons
  //  ret |= button[sensorIndex[index]]->update();

  // TODO: put debugging code here
  // return
  return ( ret );

}
// returns true if any of the buttons are pressed.
boolean Touch::anyChanged() {
  return ( changed(I_RED) || changed(I_GRN) || changed(I_BLU) || changed(I_YEL)
      || changed(I_START) || changed(I_RIGHT) || changed(I_LEFT) );
}


// returns true WHILE a specific sensor IS PRESSED
// this function will be called after touchChanged() asserts a change.
boolean Touch::pressed(byte index) {
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
boolean Touch::anyColorPressed() {
  return ( pressed(I_RED) || pressed(I_GRN) || pressed(I_BLU) || pressed(I_YEL) );
}

// returns true if any of the buttons have switched states.
boolean Touch::anyButtonPressed() {
  return ( anyColorPressed() || pressed(I_START) || pressed(I_LEFT) || pressed(I_RIGHT) );
}

// returns the first pressed button found
color Touch::whatPressed() {
  if( pressed(I_RED) ) return (I_RED);
  if( pressed(I_GRN) ) return (I_GRN);
  if( pressed(I_BLU) ) return (I_BLU);
  if( pressed(I_YEL) ) return (I_YEL);
  if( pressed(I_START) ) return (I_START);
  if( pressed(I_RIGHT) ) return (I_RIGHT);
  if( pressed(I_LEFT) ) return (I_LEFT);
  return(N_BUTTONS);
}

nonColorButtons Touch::whatNonColorButtonPressed() {
}

// MGD new buttons
boolean Touch::startPressed() {
  // capsense
  MPR121.updateTouchData();
  return( MPR121.getTouchData(I_START) );
}
boolean Touch::rightPressed() {
  // capsense
  MPR121.updateTouchData();
  return( MPR121.getTouchData(I_RIGHT) );
}
boolean Touch::leftPressed() {
  // capsense
  MPR121.updateTouchData();
  return( MPR121.getTouchData(I_LEFT) );
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
  static int minRead[13] = { 400,400,400,400,400,400,400,400,400,400,400,400,400 }; // 300 seems to be the normal low end, but let's leave some room for drift
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
  Serial << "r/" << I_RED << ": b" << base0 << "/c" << data0 << "\t";
  Serial << "g/" << I_GRN << ": b" << base1 << "/c" << data1 << "\t";
  Serial << "b/" << I_BLU << ": b" << base2 << "/c" << data2 << "\t";
  Serial << "y/" << I_YEL << ": b" << base3 << "/c" << data3 << "\t";

  uint16_t base_start = ((uint16_t)MPR121.getRegister(0x22))<<2;
  uint16_t base_right = ((uint16_t)MPR121.getRegister(0x23))<<2;
  uint16_t base_left = ((uint16_t)MPR121.getRegister(0x24))<<2;
  uint16_t data_start = (((uint16_t)MPR121.getRegister(0x0D))<<8) | MPR121.getRegister(0x0C);
  uint16_t data_right = (((uint16_t)MPR121.getRegister(0x0F))<<8) | MPR121.getRegister(0x0E);
  uint16_t data_left = (((uint16_t)MPR121.getRegister(0x11))<<8) | MPR121.getRegister(0x10);

  Serial << "s/" << I_START << ": b" << base_start << "/c" << data_start << "\t";
  Serial << "</" << I_LEFT << ": b" << base_left << "/c" << data_left << "\t";
  Serial << ">/" << I_RIGHT << ": b" << base_right << "/c" << data_right << endl;
  
/*
  Serial << I_RED << ", " << base0 << ", " << data0 << endl;
  Serial << I_GRN << ", " << base1 << ", " << data1 << endl;
  Serial << I_BLU << ", " << base2 << ", " << data2 << endl;
  Serial << "3, " << base3 << ", " << data3 << endl;
*/
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


