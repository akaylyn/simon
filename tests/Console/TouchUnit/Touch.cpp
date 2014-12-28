#include "Touch.h"
// Walkthrough: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
// MPR121 Datasheet: http://www.adafruit.com/datasheets/MPR121.pdf

Adafruit_MPR121 cap = Adafruit_MPR121();

// buttons
Bounce redButton = Bounce(RED_BUTTON, BUTTON_DEBOUNCE_TIME);
Bounce grnButton = Bounce(GRN_BUTTON, BUTTON_DEBOUNCE_TIME);
Bounce bluButton = Bounce(BLU_BUTTON, BUTTON_DEBOUNCE_TIME);
Bounce yelButton = Bounce(YEL_BUTTON, BUTTON_DEBOUNCE_TIME);

// system outputs
// lights
LED redLight(RED_LIGHT);
LED grnLight(GRN_LIGHT);
LED bluLight(BLU_LIGHT);
LED yelLight(YEL_LIGHT);

void touchStart(uint8_t touchCount, uint8_t releaseCount) {
  Serial << F("Startup Adafruit MPR121 Capacitive Touch sensor.") << endl;

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial << F("MPR121 not found, check wiring?") << endl;
    while (1);
  }
  Serial << F("MPR121 found!") << endl;
  Serial << F("Touch count threshold set: ") << touchCount << F(". Release count threshold set: ") << releaseCount << F(".") << endl;
  cap.setThreshholds(touchCount, releaseCount);
}
void touchStart() {
  touchStart(30, 6); // take defaults from AdaFruit example
}

void touchUnitTest(boolean details, unsigned long timeout) {

  Metro unitTestTimeout(timeout);

  // Keeps track of the last pins touched
  // so we know when buttons are 'released'
  uint16_t lasttouched = 0;
  uint16_t currtouched = 0;

  while (! unitTestTimeout.check() ) {

    // Get the currently touched pads
    currtouched = cap.touched();

    for (uint8_t i = 0; i < 12; i++) {
      // it if *is* touched and *wasnt* touched before, alert!
      if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(F(" touched"));
      }
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(F(" released"));
      }
    }

    // reset our state
    lasttouched = currtouched;

    // comment out this line for detailed data from the sensor!
    if ( !details) break;

    // debugging info, what
    Serial.print(F("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x")); Serial.println(cap.touched(), HEX);
    Serial.print(F("Filt: "));
    for (uint8_t i = 0; i < 12; i++) {
      Serial.print(cap.filteredData(i)); Serial.print(F("\t"));
    }
    Serial.println();
    Serial.print(F("Base: "));
    for (uint8_t i = 0; i < 12; i++) {
      Serial.print(cap.baselineData(i)); Serial.print(F("\t"));
    }
    Serial.println();

    // put a delay so it isn't overwhelming
    delay(100);
  }
}

// configures buttons at startup
void configureManualButtons() {
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(GRN_BUTTON, INPUT_PULLUP);
  pinMode(BLU_BUTTON, INPUT_PULLUP);
  pinMode(YEL_BUTTON, INPUT_PULLUP);
}

// configures lights for buttons at startup
void configureManualLights() {
  // no setup.  LED does that for you.
  const unsigned long lightDuration = 250;
  manualLightSet(I_ALL, LED_ON);
  delay(lightDuration);
  manualLightSet(I_ALL, LED_OFF);
}

// set manual light (or I_ALL lights) to a value
void manualLightSet(byte lightIndex, byte lightLevel) {
  switch ( lightIndex ) {
    case I_RED:
      redLight.setValue(lightLevel);
      break;
    case I_GRN:
      grnLight.setValue(lightLevel);
      break;
    case I_BLU:
      bluLight.setValue(lightLevel);
      break;
    case I_YEL:
      yelLight.setValue(lightLevel);
      break;
    case I_ALL:
      redLight.setValue(lightLevel);
      grnLight.setValue(lightLevel);
      bluLight.setValue(lightLevel);
      yelLight.setValue(lightLevel);
      break;
  }
}

// check for a specific (or I_ALL) button change
boolean buttonChanged(byte lightIndex) {

  switch ( lightIndex ) {
    case I_RED:
      return (redButton.update());
      break;
    case I_GRN:
      return (grnButton.update());
      break;
    case I_BLU:
      return (yelButton.update());
      break;
    case I_YEL:
      return (yelButton.update());
      break;
    case I_ALL:
      // update the buttons and return a state change as true.
      boolean redC = redButton.update();
      boolean grnC = grnButton.update();
      boolean bluC = bluButton.update();
      boolean yelC = yelButton.update();
      // actively avoiding short-circuit evaluation so that we get an .update() to all.
      return ( redC || grnC || bluC || yelC );
      break;
  }
}

// check for a specific (or I_ALL) button being pressed
boolean buttonPressed(byte lightIndex) {

  // get an update while we're here.  not strictly neccessary, but guarantees an update while reading.
  boolean toss = buttonChanged(lightIndex);

  switch ( lightIndex ) {
    case I_RED:
      return (redButton.read() == PRESSED_BUTTON);
      break;
    case I_GRN:
      return (grnButton.read() == PRESSED_BUTTON);
      break;
    case I_BLU:
      return (bluButton.read() == PRESSED_BUTTON);
      break;
    case I_YEL:
      return (yelButton.read() == PRESSED_BUTTON);
      break;
    case I_ALL:
      return ( redButton.read() == PRESSED_BUTTON ||
               grnButton.read() == PRESSED_BUTTON ||
               bluButton.read() == PRESSED_BUTTON ||
               yelButton.read() == PRESSED_BUTTON
               );
      break;
  }
}

