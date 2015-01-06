// This file should match src/Console/Touch.cpp

#include "Touch.h"

// Walkthrough: https://github.com/labatrockwell/arduino_capactive_sensor_MPR121
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

// variables: capacitive sensing
bool touchStates[SENSORS];    // holds the current touch/prox state of all sensors
bool activeSensors[SENSORS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // holds which sensors are active (0=inactive, 1=active)
bool newData = false;         // flag that is set to true when new data is available from capacitive sensor
int irqpin = TOUCH_IRQ;               // pin that connects to notifies when data is available from capacitive sensor

boolean touchStart(uint8_t touchCount, uint8_t releaseCount) {
  // some reasonable defaults are set.
  // expect this to be called after startup
  // and again between games.
  Serial << F("Touch: startup.") << endl;

  // following Examples->capactivite_prox_sensor_mpr121

  // attach interrupt to pin - interrupt 4 is on pin 19 of the Mega which is #define TOUCH_IRQ (confusing I know)
  // we may need to switch to a polling mechanism, as the radio board uses an interrupt, so
  // these may not play well together.  let's proceed anyway.
  attachInterrupt(4, dataAvailable, FALLING);

  // set the registers on the capacitive sensing IC
  setupCapacitiveRegisters();

  Serial << F("Touch: MPR121 found?") << endl;

  return ( true );
}

/**
 * dataAvailable Callback method that runs whenever new data becomes available on from the capacitive sensor.
 *   This method was attached to the interrupt on pin 2, and is called whenever that pins goes low.
 */
void dataAvailable() {
  newData = true;
}

/**
 * readCapacitiveSensor Reads the capacitive sensor values from the MP121 IC. It makes a request to
 *   the sensor chip via the I2C/Wire connection, and then parses the sensor values which are stored on
 *   the first 13 bits of the 16-bit response msg.
 */
void readCapacitiveSensor() {
  if (true) {
    //read the touch state from the MPR121
    Wire.requestFrom(0x5A, 2);
    byte tLSB = Wire.read();
    byte tMSB = Wire.read();
    uint16_t touched = ((tMSB << 8) | tLSB); //16bits that make up the touch states
    Serial.println(touched);

    for (int i = 0; i < SENSORS; i++) { // Check what electrodes were pressed
      if (activeSensors[i] == 0) continue;
      char sensor_id [] = {'\0', '\0', '\0'};
      switch (i) {
        case 12:
          sensor_id[0] = 'P';
          break;
        default:
          if (i < 10) {
            sensor_id[0] = char( i + 48 );
          }
          else if (i < 12) {
            sensor_id[0] = char('1');
            sensor_id[1] = char( ( i % 10 ) + 48 );
          }
      }
      if (sensor_id != '\0') {
        // read the humidity level

        // if current sensor was touched (check appropriate bit on touched var)
        if (touched & (1 << i)) {
          // if current pin was not previously touched send a serial message
          if (touchStates[i] == 0) {
            Serial.print(sensor_id);
            Serial.print(":");
            Serial.println("1");
          }
          touchStates[i] = 1;
        } else {
          // if current pin was just touched send serial message
          if (touchStates[i] == 1) {
            Serial.print(sensor_id);
            Serial.print(":");
            Serial.println("0");
          }
          touchStates[i] = 0;
        }
      }
    }
    newData = false;
  }
}

/**
 * setupCapacitiveRegisters Updates all of configurations on the MP121 capacitive sensing IC. This includes
 *   setting levels for all filters, touch and proximity sensing activation and release thresholds, debounce,
 *   and auto-configurations options. At the end it activates all of the electrodes.
 */
void setupCapacitiveRegisters() {

  set_register(0x5A, ELE_CFG, 0x00);

  // Section A - filtering when data is > baseline.
  // touch sensing
  set_register(0x5A, MHD_R, 0x01);
  set_register(0x5A, NHD_R, 0x01);
  set_register(0x5A, NCL_R, 0x00);
  set_register(0x5A, FDL_R, 0x00);

  // prox sensing
  set_register(0x5A, PROX_MHDR, 0xFF);
  set_register(0x5A, PROX_NHDAR, 0xFF);
  set_register(0x5A, PROX_NCLR, 0x00);
  set_register(0x5A, PROX_FDLR, 0x00);

  // Section B - filtering when data is < baseline.
  // touch sensing
  set_register(0x5A, MHD_F, 0x01);
  set_register(0x5A, NHD_F, 0x01);
  set_register(0x5A, NCL_F, 0xFF);
  set_register(0x5A, FDL_F, 0x02);

  // prox sensing
  set_register(0x5A, PROX_MHDF, 0x01);
  set_register(0x5A, PROX_NHDAF, 0x01);
  set_register(0x5A, PROX_NCLF, 0xFF);
  set_register(0x5A, PROX_NDLF, 0xFF);

  // Section C - Sets touch and release thresholds for each electrode
  set_register(0x5A, ELE0_T, TOU_THRESH);
  set_register(0x5A, ELE0_R, REL_THRESH);

  set_register(0x5A, ELE1_T, TOU_THRESH);
  set_register(0x5A, ELE1_R, REL_THRESH);

  set_register(0x5A, ELE2_T, TOU_THRESH);
  set_register(0x5A, ELE2_R, REL_THRESH);

  set_register(0x5A, ELE3_T, TOU_THRESH);
  set_register(0x5A, ELE3_R, REL_THRESH);

  set_register(0x5A, ELE4_T, TOU_THRESH);
  set_register(0x5A, ELE4_R, REL_THRESH);

  set_register(0x5A, ELE5_T, TOU_THRESH);
  set_register(0x5A, ELE5_R, REL_THRESH);

  set_register(0x5A, ELE6_T, TOU_THRESH);
  set_register(0x5A, ELE6_R, REL_THRESH);

  set_register(0x5A, ELE7_T, TOU_THRESH);
  set_register(0x5A, ELE7_R, REL_THRESH);

  set_register(0x5A, ELE8_T, TOU_THRESH);
  set_register(0x5A, ELE8_R, REL_THRESH);

  set_register(0x5A, ELE9_T, TOU_THRESH);
  set_register(0x5A, ELE9_R, REL_THRESH);

  set_register(0x5A, ELE10_T, TOU_THRESH);
  set_register(0x5A, ELE10_R, REL_THRESH);

  set_register(0x5A, ELE11_T, TOU_THRESH);
  set_register(0x5A, ELE11_R, REL_THRESH);

  // Section D - Set the touch filter Configuration
  set_register(0x5A, FIL_CFG, 0x04);

  // Section E - Set proximity sensing threshold and release
  set_register(0x5A, PRO_T, PROX_THRESH);   // sets the proximity sensor threshold
  set_register(0x5A, PRO_R, PREL_THRESH);   // sets the proximity sensor release

  // Section F - Set proximity sensor debounce
  set_register(0x59, PROX_DEB, 0x50);  // PROX debounce

  // Section G - Set Auto Config and Auto Reconfig for prox sensing
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V
  set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V
  set_register(0x5A, ATO_CFG0, 0x0B);

  // ANDREA: sort out debounce, autoconfig and autoreconfig on the electrodes, probably
  // copying the logic for the proximity setup just above this comment?

  // Section H - Start listening to all electrodes and the proximity sensor
  set_register(0x5A, ELE_CFG, 0x3C);
}

/**
 * set_register Sets a register on a device connected via I2C. It accepts the device's address,
 *   register location, and the register value.
 * @param address The address of the I2C device
 * @param r       The register's address on the I2C device
 * @param v       The new value for the register
 */
void set_register(int address, unsigned char r, unsigned char v) {
  Wire.beginTransmission(address);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}

// calibrates the Touch interface
void touchCalibrate() {
  // when this is called, assume that nothing is touching the
  // sensors, and you've got time to restart the capsense hardware,
  // take some readings, and assure that the sensors are set up
  // correctly.
}

// returns true if any of the buttons have switched states.
boolean touchAnyChanged() {
  // avoid short-circuit eval so that each button gets an update
  boolean redC = touchChanged(I_RED);
  boolean grnC = touchChanged(I_GRN);
  boolean bluC = touchChanged(I_BLU);
  boolean yelC = touchChanged(I_YEL);
  return ( redC || grnC || bluC || yelC );
}

// returns true if any of the buttons are pressed.
boolean touchAnyPressed() {
  return (
           touchPressed(I_RED) ||
           touchPressed(I_GRN) ||
           touchPressed(I_BLU) ||
           touchPressed(I_YEL)
         );
}

// returns true if a specific button has changed
boolean touchChanged(byte touchIndex) {
  /* this function will be called very frequently, so this is where the capsense
     calls should be made and the debouncing routines should reside
  */

  switch ( touchIndex ) {
    case I_RED:
      break;
    case I_GRN:
      break;
    case I_BLU:
      break;
    case I_YEL:
      break;
    case I_ALL:
      return ( touchAnyChanged() );
      break;
    default:
      Serial << F("Touch: touchChanged Error, case=") << touchIndex << endl;
  }
}

// returns true if a specific sensor is pressed
boolean touchPressed(byte touchIndex) {
  // this function will be called after touchChanged() asserts a change.

  switch ( touchIndex ) {
    case I_RED:
      break;
    case I_GRN:
      break;
    case I_BLU:
      break;
    case I_YEL:
      break;
    case I_ALL:
      return ( touchAnyPressed() );
      break;
    default:
      Serial << F("Touch: touchPressed Error, case=") << touchIndex << endl;
  }
}


void touchUnitTest(boolean details, unsigned long timeout) {

  Metro unitTestTimeout(timeout);

  while (! unitTestTimeout.check() ) {
    readCapacitiveSensor();
  }
}
